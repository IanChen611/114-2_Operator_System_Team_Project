#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define SIZE 9
#define TOTAL_THREADS 27

typedef struct {
	int type;
	int index;
	const int (*board)[SIZE];
	int *results;
} ThreadArgs;

static void print_board(const int (*board)[SIZE]) {
	for (int row = 0; row < SIZE; ++row) {
		for (int col = 0; col < SIZE; ++col) {
			printf("%d%c", board[row][col], col == SIZE - 1 ? '\n' : ' ');
		}
	}
}

/*
 * Member B responsibility:
 * implement row validation logic here.
 */
static int validate_row_placeholder(const int (*board)[SIZE], int row) {
	int seen[SIZE + 1] = {0};

	for (int column = 0; column < SIZE; ++column) {
		int value = board[row][column];

		if (value < 1 || value > SIZE) {
			return 0;
		}
		if (seen[value]) {
			return 0;
		}

		seen[value] = 1;
	}

	return 1;
}

/*
 * Member B responsibility:
 * implement column validation logic here.
 */
static int validate_column_placeholder(const int (*board)[SIZE], int column) {
	int seen[SIZE + 1] = {0};

	for (int row = 0; row < SIZE; ++row) {
		int value = board[row][column];

		if (value < 1 || value > SIZE) {
			return 0;
		}
		if (seen[value]) {
			return 0;
		}

		seen[value] = 1;
	}

	return 1;
}

/*
 * Validate a 3x3 subgrid.
 */
static int validate_subgrid_placeholder(const int (*board)[SIZE], int subgrid) {
	int seen[SIZE + 1] = {0};
	int start_row = (subgrid / 3) * 3;
	int start_column = (subgrid % 3) * 3;

	for (int row = start_row; row < start_row + 3; ++row) {
		for (int column = start_column; column < start_column + 3; ++column) {
			int value = board[row][column];

			if (value < 1 || value > SIZE) {
				return 0;
			}
			if (seen[value]) {
				return 0;
			}

			seen[value] = 1;
		}
	}

	return 1;
}

static void *worker(void *arg) {
	ThreadArgs *thread_args = (ThreadArgs *)arg;
	int result = -1;

	if (thread_args->type == 0) {
		result = validate_row_placeholder((const int (*)[SIZE])thread_args->board, thread_args->index);
	} else if (thread_args->type == 1) {
		result = validate_column_placeholder((const int (*)[SIZE])thread_args->board, thread_args->index);
	} else {
		result = validate_subgrid_placeholder((const int (*)[SIZE])thread_args->board, thread_args->index);
	}

	thread_args->results[thread_args->type * SIZE + thread_args->index] = result;

	if (thread_args->type == 0) {
		printf("Thread finished: row %d -> %s\n",
		       thread_args->index + 1, result == 1 ? "valid" : "invalid");
	} else if (thread_args->type == 1) {
		printf("Thread finished: column %d -> %s\n",
		       thread_args->index + 1, result == 1 ? "valid" : "invalid");
	} else {
		printf("Thread finished: subgrid %d -> %s\n",
		       thread_args->index + 1, result == 1 ? "valid" : "invalid");
	}

	return NULL;
}

int main(void) {
	const int board[SIZE][SIZE] = {
		{5, 3, 4, 6, 7, 8, 9, 1, 2},
		{6, 7, 2, 1, 9, 5, 3, 4, 8},
		{1, 9, 8, 3, 4, 2, 5, 6, 7},
		{8, 5, 9, 7, 6, 1, 4, 2, 3},
		{4, 2, 6, 8, 5, 3, 7, 9, 1},
		{7, 1, 3, 9, 2, 4, 8, 5, 6},
		{9, 6, 1, 5, 3, 7, 2, 8, 4},
		{2, 8, 7, 4, 1, 9, 6, 3, 5},
		{3, 4, 5, 2, 8, 6, 1, 7, 9}
	};

	pthread_t threads[TOTAL_THREADS];
	ThreadArgs args[TOTAL_THREADS];
	int results[TOTAL_THREADS] = {0};

	printf("Sudoku board:\n");
	print_board(board);
	printf("\n");

	for (int i = 0; i < TOTAL_THREADS; ++i) {
		args[i].type = i / SIZE;
		args[i].index = i % SIZE;
		args[i].board = board;
		args[i].results = results;

		if (pthread_create(&threads[i], NULL, worker, &args[i]) != 0) {
			fprintf(stderr, "Failed to create thread %d\n", i);
			return EXIT_FAILURE;
		}
	}

	for (int i = 0; i < TOTAL_THREADS; ++i) {
		if (pthread_join(threads[i], NULL) != 0) {
			fprintf(stderr, "Failed to join thread %d\n", i);
			return EXIT_FAILURE;
		}
	}

	bool all_valid = true;
	bool has_pending = false;

	for (int i = 0; i < TOTAL_THREADS; ++i) {
		if (results[i] == 0) {
			all_valid = false;
		}
		if (results[i] < 0) {
			has_pending = true;
		}
	}

	if (has_pending) {
		printf("Some validation logic is still pending.\n");
	} else if (all_valid) {
		printf("Sudoku solution is valid.\n");
	} else {
		printf("Sudoku solution is invalid.\n");
	}

	return EXIT_SUCCESS;
}
