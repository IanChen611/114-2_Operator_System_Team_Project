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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 10
#define MAX_RESOURCES 10

static int n, m;
static int available[MAX_RESOURCES];
static int allocation[MAX_PROCESSES][MAX_RESOURCES];
static int max_demand[MAX_PROCESSES][MAX_RESOURCES];
static int need[MAX_PROCESSES][MAX_RESOURCES];

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
/*
 * Returns 1 if the current state is safe and fills safe_seq[] with the
 * safe execution order; returns 0 if the state is unsafe.
 */
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

            /* Check Need[i] <= Work */
            int feasible = 1;
            for (int j = 0; j < m; j++) {
                if (need[i][j] > work[j]) { feasible = 0; break; }
            }
            if (!feasible) continue;

            /* Simulate process i completing and releasing resources */
            for (int j = 0; j < m; j++)
                work[j] += allocation[i][j];
            safe_seq[count++] = i;
            finish[i] = 1;
            found = 1;
        }
        if (!found) break;  /* no progress → unsafe */
    }

    return count == n;
}

/* ── Resource Request Algorithm ──────────────────────────────────────── */
/*
 * Attempt to grant a resource request from process `pid`.
 * Returns 1 if the request is granted, 0 otherwise.
 */
static int resource_request(int pid, int request[])
{
    printf("P%d requests: [", pid);
    for (int j = 0; j < m; j++) {
        printf("%d", request[j]);
        if (j < m - 1) printf(", ");
    }
    printf("]\n\n");

    /* Step 1 – request must not exceed declared maximum need */
    for (int j = 0; j < m; j++) {
        if (request[j] > need[pid][j]) {
            printf("DENIED: P%d exceeded its maximum claim on R%d "
                   "(request=%d, need=%d).\n",
                   pid, j, request[j], need[pid][j]);
            return 0;
        }
    }

    /* Step 2 – resources must be available */
    for (int j = 0; j < m; j++) {
        if (request[j] > available[j]) {
            printf("WAIT: P%d must wait — insufficient resources on R%d "
                   "(request=%d, available=%d).\n",
                   pid, j, request[j], available[j]);
            return 0;
        }
    }

    /* Step 3 – pretend to allocate and check safety */
    for (int j = 0; j < m; j++) {
        available[j]       -= request[j];
        allocation[pid][j] += request[j];
        need[pid][j]       -= request[j];
    }

    int safe_seq[MAX_PROCESSES];
    if (safety_algorithm(safe_seq)) {
        printf("GRANTED: system remains in a safe state.\n");
        printf("Safe sequence: ");
        for (int i = 0; i < n; i++) {
            printf("P%d", safe_seq[i]);
            if (i < n - 1) printf(" -> ");
        }
        printf("\n");

        printf("\nUpdated state after granting request:\n");
        compute_need();
        print_state();
        return 1;
    }

    /* Unsafe — rollback */
    for (int j = 0; j < m; j++) {
        available[j]       += request[j];
        allocation[pid][j] -= request[j];
        need[pid][j]       += request[j];
    }
    printf("DENIED: granting this request would lead to an unsafe state. "
           "Rolling back.\n");
    return 0;
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
        "\n"
        "  -n        number of processes\n"
        "  -m        number of resource types\n"
        "  -avail    available resource vector (m values)\n"
        "  -alloc    allocation matrix (n*m values, row-major)\n"
        "  -max      maximum demand matrix (n*m values, row-major)\n"
        "  -request  optional: resource request from process <pid> "
                    "(m values)\n"
        "\n"
        "Example:\n"
        "  %s -n 5 -m 3 \\\n"
        "       -avail 3 3 2 \\\n"
        "       -alloc 0 1 0  2 0 0  3 0 2  2 1 1  0 0 2 \\\n"
        "       -max   7 5 3  3 2 2  9 0 2  2 2 2  4 3 3 \\\n"
        "       -request 1 1 0 2\n",
        prog, prog);
}

/* Textbook default: 5 processes, 3 resource types (Silberschatz 10e §8.6) */
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

    if (argc < 2) {
        load_defaults();
        compute_need();
    } else {
    for (int i = 1; i < argc; i++) {

        if (strcmp(argv[i], "-n") == 0) {
            if (++i >= argc) { fprintf(stderr, "Missing value for -n\n"); return 1; }
            n = atoi(argv[i]);
            if (n <= 0 || n > MAX_PROCESSES) {
                fprintf(stderr, "Error: -n must be 1..%d\n", MAX_PROCESSES);
                return 1;
            }

        } else if (strcmp(argv[i], "-m") == 0) {
            if (++i >= argc) { fprintf(stderr, "Missing value for -m\n"); return 1; }
            m = atoi(argv[i]);
            if (m <= 0 || m > MAX_RESOURCES) {
                fprintf(stderr, "Error: -m must be 1..%d\n", MAX_RESOURCES);
                return 1;
            }

        } else if (strcmp(argv[i], "-avail") == 0) {
            if (n == 0 || m == 0) {
                fprintf(stderr, "Error: specify -n and -m before -avail\n");
                return 1;
            }
            for (int j = 0; j < m; j++) {
                if (++i >= argc) {
                    fprintf(stderr, "Error: -avail needs %d values\n", m);
                    return 1;
                }
                available[j] = atoi(argv[i]);
            }
            avail_set = 1;

        } else if (strcmp(argv[i], "-alloc") == 0) {
            if (n == 0 || m == 0) {
                fprintf(stderr, "Error: specify -n and -m before -alloc\n");
                return 1;
            }
            for (int p = 0; p < n; p++) {
                for (int j = 0; j < m; j++) {
                    if (++i >= argc) {
                        fprintf(stderr,
                            "Error: -alloc needs %d values (%d*%d)\n",
                            n * m, n, m);
                        return 1;
                    }
                    allocation[p][j] = atoi(argv[i]);
                }
            }
            alloc_set = 1;

        } else if (strcmp(argv[i], "-max") == 0) {
            if (n == 0 || m == 0) {
                fprintf(stderr, "Error: specify -n and -m before -max\n");
                return 1;
            }
            for (int p = 0; p < n; p++) {
                for (int j = 0; j < m; j++) {
                    if (++i >= argc) {
                        fprintf(stderr,
                            "Error: -max needs %d values (%d*%d)\n",
                            n * m, n, m);
                        return 1;
                    }
                    max_demand[p][j] = atoi(argv[i]);
                }
            }
            max_set = 1;

        } else if (strcmp(argv[i], "-request") == 0) {
            if (n == 0 || m == 0) {
                fprintf(stderr, "Error: specify -n and -m before -request\n");
                return 1;
            }
            if (++i >= argc) {
                fprintf(stderr, "Error: -request needs a process id\n");
                return 1;
            }
            request_pid = atoi(argv[i]);
            if (request_pid < 0 || request_pid >= n) {
                fprintf(stderr, "Error: pid %d out of range [0, %d)\n",
                        request_pid, n);
                return 1;
            }
            for (int j = 0; j < m; j++) {
                if (++i >= argc) {
                    fprintf(stderr,
                        "Error: -request needs pid + %d resource values\n", m);
                    return 1;
                }
                request_vec[j] = atoi(argv[i]);
            }
            has_request = 1;

        } else if (strcmp(argv[i], "-h") == 0 ||
                   strcmp(argv[i], "--help") == 0) {
            usage(argv[0]);
            return 0;

        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            usage(argv[0]);
            return 1;
        }
    }

    /* Validate required options */
    if (n == 0 || m == 0 || !avail_set || !alloc_set || !max_set) {
        fprintf(stderr,
            "Error: -n, -m, -avail, -alloc, and -max are all required.\n\n");
        usage(argv[0]);
        return 1;
    }

    /* Validate Max >= Allocation */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (max_demand[i][j] < allocation[i][j]) {
                fprintf(stderr,
                    "Error: Max[%d][%d]=%d < Allocation[%d][%d]=%d\n",
                    i, j, max_demand[i][j], i, j, allocation[i][j]);
                return 1;
            }
        }
    }

    compute_need();
    } /* end else (argc >= 2) */

    /* ── Initial state ───────────────────────────────────────────────── */
    printf("=== Banker's Algorithm ===\n\n");
    print_state();

    /* ── Safety check on current state ──────────────────────────────── */
    printf("\n--- Safety Check ---\n");
    int safe_seq[MAX_PROCESSES];
    if (safety_algorithm(safe_seq)) {
        printf("System is in a SAFE state.\n");
        printf("Safe sequence: ");
        for (int i = 0; i < n; i++) {
            printf("P%d", safe_seq[i]);
            if (i < n - 1) printf(" -> ");
        }
        printf("\n");
    } else {
        printf("System is in an UNSAFE state!\n");
    }

    /* ── Optional resource request ───────────────────────────────────── */
    if (has_request) {
        printf("\n--- Resource Request ---\n");
        resource_request(request_pid, request_vec);
    }

    return 0;
}
