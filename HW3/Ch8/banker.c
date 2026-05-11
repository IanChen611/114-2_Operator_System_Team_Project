/*
 * Banker's Algorithm Implementation
 *
 * Usage:
 *   banker -n <num_processes> -m <num_resources>
 *          -avail  <r0> <r1> ...
 *          -alloc  <p0r0> <p0r1> ... <p1r0> <p1r1> ...
 *          -max    <p0r0> <p0r1> ... <p1r0> <p1r1> ...
 *         [-request <pid> <r0> <r1> ...]
 *
 * Example (from textbook: 5 processes, 3 resource types):
 *   banker -n 5 -m 3
 *          -avail  3 3 2
 *          -alloc  0 1 0  2 0 0  3 0 2  2 1 1  0 0 2
 *          -max    7 5 3  3 2 2  9 0 2  2 2 2  4 3 3
 *         -request 1 1 0 2
 */

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define MAX_PROCESSES 10
#define MAX_RESOURCES 10

static int n, m;
static int available[MAX_RESOURCES];
static int allocation[MAX_PROCESSES][MAX_RESOURCES];
static int max_demand[MAX_PROCESSES][MAX_RESOURCES];
static int need[MAX_PROCESSES][MAX_RESOURCES];

static pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;
static int simulation_mode = 0;

/* ── helpers ─────────────────────────────────────────────────────────── */

static void compute_need(void)
{
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            need[i][j] = max_demand[i][j] - allocation[i][j];
}

static void print_matrix(const char *title,
                         int matrix[MAX_PROCESSES][MAX_RESOURCES])
{
    printf("%s:\n     ", title);
    for (int j = 0; j < m; j++) printf("R%-3d", j);
    printf("\n");
    for (int i = 0; i < n; i++) {
        printf("P%-3d ", i);
        for (int j = 0; j < m; j++) printf("%-4d", matrix[i][j]);
        printf("\n");
    }
}

static void print_vector(const char *title, int vec[], int len)
{
    printf("%s:\n     ", title);
    for (int j = 0; j < len; j++) printf("R%-3d", j);
    printf("\n     ");
    for (int j = 0; j < len; j++) printf("%-4d", vec[j]);
    printf("\n");
}

static void print_state(void)
{
    printf("Processes: %d   Resource Types: %d\n\n", n, m);
    print_matrix("Allocation", allocation);
    printf("\n");
    print_matrix("Max", max_demand);
    printf("\n");
    print_matrix("Need", need);
    printf("\n");
    print_vector("Available", available, m);
}

/* ── Safety Algorithm ────────────────────────────────────────────────── */
static int safety_algorithm(int safe_seq[])
{
    int work[MAX_RESOURCES];
    int finish[MAX_PROCESSES] = {0};

    for (int j = 0; j < m; j++)
        work[j] = available[j];

    int count = 0;
    while (count < n) {
        int found = 0;
        for (int i = 0; i < n; i++) {
            if (finish[i]) continue;

            int feasible = 1;
            for (int j = 0; j < m; j++) {
                if (need[i][j] > work[j]) { feasible = 0; break; }
            }
            if (!feasible) continue;

            for (int j = 0; j < m; j++)
                work[j] += allocation[i][j];
            if (safe_seq) safe_seq[count] = i;
            count++;
            finish[i] = 1;
            found = 1;
        }
        if (!found) break;
    }

    return count == n;
}

/* ── Resource Request Algorithm ──────────────────────────────────────── */
static int resource_request(int pid, int request[])
{
    if (!simulation_mode) {
        printf("P%d requests: [", pid);
        for (int j = 0; j < m; j++) {
            printf("%d", request[j]);
            if (j < m - 1) printf(", ");
        }
        printf("]\n\n");
    }

    /* Step 1 – request must not exceed declared maximum need */
    for (int j = 0; j < m; j++) {
        if (request[j] > need[pid][j]) {
            if (!simulation_mode)
                printf("DENIED: P%d exceeded its maximum claim on R%d (request=%d, need=%d).\n",
                       pid, j, request[j], need[pid][j]);
            return -1; // Error: invalid request
        }
    }

    /* Step 2 – resources must be available */
    for (int j = 0; j < m; j++) {
        if (request[j] > available[j]) {
            if (!simulation_mode)
                printf("WAIT: P%d must wait — insufficient resources on R%d (request=%d, available=%d).\n",
                       pid, j, request[j], available[j]);
            return 0; // Must wait
        }
    }

    /* Step 3 – pretend to allocate and check safety */
    for (int j = 0; j < m; j++) {
        available[j]       -= request[j];
        allocation[pid][j] += request[j];
        need[pid][j]       -= request[j];
    }

    if (safety_algorithm(NULL)) {
        if (!simulation_mode) {
            printf("GRANTED: system remains in a safe state.\n");
            compute_need();
            // print_state(); // optional
        }
        return 1; // Success
    }

    /* Unsafe — rollback */
    for (int j = 0; j < m; j++) {
        available[j]       += request[j];
        allocation[pid][j] -= request[j];
        need[pid][j]       += request[j];
    }
    if (!simulation_mode)
        printf("DENIED: granting this request would lead to an unsafe state. Rolling back.\n");
    return 0; // Unsafe, must wait/retry
}

/* ── Resource Release ───────────────────────────────────────────────── */
static void resource_release(int pid, int release[])
{
    if (!simulation_mode) {
        printf("P%d releases: [", pid);
        for (int j = 0; j < m; j++) {
            printf("%d", release[j]);
            if (j < m - 1) printf(", ");
        }
        printf("]\n\n");
    }

    for (int j = 0; j < m; j++) {
        available[j]       += release[j];
        allocation[pid][j] -= release[j];
        need[pid][j]       += release[j];
    }
}

/* ── Multithreaded Simulation ────────────────────────────────────────── */
void* customer(void* arg)
{
    int pid = *((int*)arg);
    free(arg);

    while (1) {
        // 1. Generate random request
        int request[MAX_RESOURCES];
        int has_non_zero_request = 0;

        pthread_mutex_lock(&state_mutex);
        for (int j = 0; j < m; j++) {
            if (need[pid][j] > 0)
                request[j] = rand() % (need[pid][j] + 1);
            else
                request[j] = 0;
            if (request[j] > 0) has_non_zero_request = 1;
        }

        if (has_non_zero_request) {
            printf(">> P%d requesting resources...\n", pid);
            int result = resource_request(pid, request);
            if (result == 1) {
                printf("<< P%d: Request GRANTED.\n", pid);
            } else if (result == 0) {
                printf("<< P%d: Request DENIED/WAIT (Insufficient or Unsafe).\n", pid);
            }
        }
        pthread_mutex_unlock(&state_mutex);

        // Sleep between request and release
        usleep((rand() % 1000 + 500) * 1000); // 0.5 - 1.5 seconds

        // 2. Generate random release
        int release[MAX_RESOURCES];
        int has_non_zero_release = 0;

        pthread_mutex_lock(&state_mutex);
        for (int j = 0; j < m; j++) {
            if (allocation[pid][j] > 0)
                release[j] = rand() % (allocation[pid][j] + 1);
            else
                release[j] = 0;
            if (release[j] > 0) has_non_zero_release = 1;
        }

        if (has_non_zero_release) {
            printf(">> P%d releasing resources...\n", pid);
            resource_release(pid, release);
            printf("<< P%d: Resources released.\n", pid);
        }
        pthread_mutex_unlock(&state_mutex);

        // Sleep before next loop
        usleep((rand() % 2000 + 1000) * 1000); // 1 - 3 seconds
    }

    return NULL;
}

/* ── Command-line parsing ────────────────────────────────────────────── */

static void usage(const char *prog)
{
    fprintf(stderr,
        "Usage:\n"
        "  %s -n <processes> -m <resources>\n"
        "       -avail  <r0> <r1> ...\n"
        "       -alloc  <p0r0> <p0r1> ... <p1r0> <p1r1> ...\n"
        "       -max    <p0r0> <p0r1> ... <p1r0> <p1r1> ...\n"
        "      [-request <pid> <r0> <r1> ...]\n"
        "      [-sim]\n"
        "\n"
        "  -n        number of processes\n"
        "  -m        number of resource types\n"
        "  -avail    available resource vector (m values)\n"
        "  -alloc    allocation matrix (n*m values, row-major)\n"
        "  -max      maximum demand matrix (n*m values, row-major)\n"
        "  -request  optional: resource request from process <pid> (m values)\n"
        "  -sim      optional: run multithreaded simulation\n",
        prog);
}

static void load_defaults(void)
{
    printf("[No arguments given — loading textbook default example]\n\n");
    n = 5; m = 3;
    int avail[]  = {3, 3, 2};
    int alloc[5][3] = {{0,1,0},{2,0,0},{3,0,2},{2,1,1},{0,0,2}};
    int maxd[5][3]  = {{7,5,3},{3,2,2},{9,0,2},{2,2,2},{4,3,3}};
    for (int j = 0; j < m; j++) available[j] = avail[j];
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++) {
            allocation[i][j] = alloc[i][j];
            max_demand[i][j] = maxd[i][j];
        }
}

int main(int argc, char *argv[])
{
    int request_pid = -1;
    int request_vec[MAX_RESOURCES] = {0};
    int has_request = 0;
    int avail_set = 0, alloc_set = 0, max_set = 0;

    srand(time(NULL));

    if (argc < 2) {
        load_defaults();
        compute_need();
    } else {
        int config_provided = 0;
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "-m") == 0 ||
                strcmp(argv[i], "-avail") == 0 || strcmp(argv[i], "-alloc") == 0 ||
                strcmp(argv[i], "-max") == 0) {
                config_provided = 1;
            }
        }

        if (!config_provided) {
            load_defaults();
        }

        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-n") == 0) {
                if (++i >= argc) { fprintf(stderr, "Missing value for -n\n"); return 1; }
                n = atoi(argv[i]);
            } else if (strcmp(argv[i], "-m") == 0) {
                if (++i >= argc) { fprintf(stderr, "Missing value for -m\n"); return 1; }
                m = atoi(argv[i]);
            } else if (strcmp(argv[i], "-avail") == 0) {
                if (n == 0 || m == 0) { fprintf(stderr, "Error: specify -n and -m before -avail\n"); return 1; }
                for (int j = 0; j < m; j++) {
                    if (++i >= argc) { fprintf(stderr, "Error: -avail needs %d values\n", m); return 1; }
                    available[j] = atoi(argv[i]);
                }
                avail_set = 1;
            } else if (strcmp(argv[i], "-alloc") == 0) {
                if (n == 0 || m == 0) { fprintf(stderr, "Error: specify -n and -m before -alloc\n"); return 1; }
                for (int p = 0; p < n; p++) {
                    for (int j = 0; j < m; j++) {
                        if (++i >= argc) { fprintf(stderr, "Error: -alloc needs %d values\n", n * m); return 1; }
                        allocation[p][j] = atoi(argv[i]);
                    }
                }
                alloc_set = 1;
            } else if (strcmp(argv[i], "-max") == 0) {
                if (n == 0 || m == 0) { fprintf(stderr, "Error: specify -n and -m before -max\n"); return 1; }
                for (int p = 0; p < n; p++) {
                    for (int j = 0; j < m; j++) {
                        if (++i >= argc) { fprintf(stderr, "Error: -max needs %d values\n", n * m); return 1; }
                        max_demand[p][j] = atoi(argv[i]);
                    }
                }
                max_set = 1;
            } else if (strcmp(argv[i], "-request") == 0) {
                if (n == 0 || m == 0) { fprintf(stderr, "Error: specify -n and -m before -request\n"); return 1; }
                if (++i >= argc) { fprintf(stderr, "Error: -request needs a process id\n"); return 1; }
                request_pid = atoi(argv[i]);
                for (int j = 0; j < m; j++) {
                    if (++i >= argc) { fprintf(stderr, "Error: -request needs pid + %d values\n", m); return 1; }
                    request_vec[j] = atoi(argv[i]);
                }
                has_request = 1;
            } else if (strcmp(argv[i], "-sim") == 0) {
                simulation_mode = 1;
            } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
                usage(argv[0]); return 0;
            }
        }

        if (config_provided && (n == 0 || m == 0 || !avail_set || !alloc_set || !max_set)) {
            fprintf(stderr, "Error: When providing custom config, -n, -m, -avail, -alloc, and -max are all required.\n");
            usage(argv[0]); return 1;
        }
        compute_need();
    }

    printf("=== Banker's Algorithm ===\n\n");
    print_state();

    printf("\n--- Initial Safety Check ---\n");
    int safe_seq[MAX_PROCESSES];
    if (safety_algorithm(safe_seq)) {
        printf("System is in a SAFE state.\nSafe sequence: ");
        for (int i = 0; i < n; i++) { printf("P%d%s", safe_seq[i], (i < n - 1 ? " -> " : "")); }
        printf("\n");
    } else {
        printf("System is in an UNSAFE state!\n");
    }

    if (has_request) {
        printf("\n--- Resource Request ---\n");
        resource_request(request_pid, request_vec);
    }

    if (simulation_mode) {
        printf("\n--- Starting Multithreaded Simulation ---\n");
        printf("Press Ctrl+C to stop.\n\n");
        pthread_t threads[MAX_PROCESSES];
        for (int i = 0; i < n; i++) {
            int *arg = malloc(sizeof(*arg));
            *arg = i;
            pthread_create(&threads[i], NULL, customer, arg);
        }
        for (int i = 0; i < n; i++) {
            pthread_join(threads[i], NULL);
        }
    }

    return 0;
}

