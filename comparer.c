// Curtis Fenner, 2017

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "vector.h"

#define ColorNormal  "\x1B[0m"
#define ColorRed  "\x1B[30;41m"
#define ColorGreen  "\x1B[30;42m"
#define ColorGray  "\x1B[30;1m"

#define NEW_ARRAY(T, n) ((T*)malloc((n)*sizeof(T)))

////////////////////////////////////////////////////////////////////////////////

type_vector(char, char_vector);
type_vector(char_vector*, char_vector_vector);
type_vector(int, int_vector);
type_vector(int_vector*, int_vector_vector);

type_pair(int_vector*, int_vector*, int_vector_pair);

// RETURNS size of the console (at time of invocation)
// has a .ws_row
// has a .ws_col
static struct winsize getConsoleSize() {
	// source: http://stackoverflow.com/questions/1022957/getting-terminal-width-in-c

	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	return w;
}

// SOURCE: http://stackoverflow.com/a/19278547
// Converts 'c' to the 'char' it represents on a stream;
// for use on the output of fgetc.
static char intToChar(int c) {
	return (char) ((c > CHAR_MAX) ? (c - (UCHAR_MAX + 1)) : c);
}

// NOTE ignores '\r' characters
static char_vector_vector* makeLines(FILE* file) {
	char_vector_vector* out = char_vector_vector_make();
	char_vector_vector_append(out, char_vector_make());
	while (true) {
		// Get the next character off the stream
		int front = fgetc(file);
		if (front == EOF) {
			break;
		}
		char next = intToChar(front);

		char_vector* row = char_vector_vector_last(out);

		// Treat certain characters specially
		if (next == '\t') {
			size_t spaces = 4 - char_vector_size(row)%4;
			for (size_t k = 0; k < spaces; k++) {
				char_vector_append(row, ' ');
			}
		} else if (next == '\r') {
			continue;
		} else if (next == '\n') {
			char_vector_vector_append(out, char_vector_make());
			continue;
		} else {
			char_vector_append(row, next);
		}
	}
	return out;
}

// RETURNS whether or not the contents of the two vectors are exactly equal
static bool contentsEqual(char_vector const* a, char_vector const* b) {
	if (char_vector_size(a) != char_vector_size(b)) {
		return false;
	}
	for (size_t k = 0; k < char_vector_size(a); k++) {
		if (char_vector_get(a, k, SRC) != char_vector_get(b, k, SRC)) {
			return false;
		}
	}
	return true;
}

// RETURNS a vector of indices into a and b
int_vector_pair* longestCommonSubsequence(char_vector_vector* a, char_vector_vector* b) {
	int_vector_vector* lcs = int_vector_vector_make();
	char_vector_vector* moves = char_vector_vector_make();
	for (size_t x = 0; x <= char_vector_vector_size(a); x++) {
		// create row for lengths
		int_vector* row = int_vector_make();
		int_vector_vector_append(lcs, row);
		// create row for moves
		char_vector* moveRow = char_vector_make();
		char_vector_vector_append(moves, moveRow);

		// scan 2D
		for (size_t y = 0; y <= char_vector_vector_size(b); y++) {
			if (x == 0 || y == 0) {
				int_vector_append(row, 0);
				char_vector_append(moveRow, '0');
				continue;
			}
			if (contentsEqual(char_vector_vector_get(a, x-1, SRC), char_vector_vector_get(b, y-1, SRC))) {
				int_vector_append(row, 1 + int_vector_get(int_vector_vector_get(lcs, x-1, SRC), y-1, SRC));
				char_vector_append(moveRow, '=');
				continue;
			}
			int optionA = int_vector_get(int_vector_vector_get(lcs, x-1, SRC), y, SRC);
			int optionB = int_vector_get(int_vector_vector_get(lcs, x, SRC), y-1, SRC);
			if (optionA > optionB) {
				int_vector_append(row, optionA);
				char_vector_append(moveRow, 'x');
			} else {
				int_vector_append(row, optionB);
				char_vector_append(moveRow, 'y');
			}
		}
	}

	// Search the lengths grid to produce indices
	int_vector* outA = int_vector_make();
	int_vector* outB = int_vector_make();

	size_t x = char_vector_vector_size(a);
	size_t y = char_vector_vector_size(b);
	while (x > 0 && y > 0) {
		char mode = char_vector_get(char_vector_vector_get(moves, x, SRC), y, SRC);
		if (mode == '=') {
			int_vector_append(outA, (int)x-1);
			int_vector_append(outB, (int)y-1);
			x--;
			y--;
		} else if (mode == 'x') {
			x--;
		} else if (mode == 'y') {
			y--;
		}
	}

	// TODO: release proper
	int_vector_vector_release(lcs);
	char_vector_vector_release(moves);

	return int_vector_pair_make(outA, outB);
}

// RETURNS a newly allocated char* buffer owned by caller
char* paddedString(const char* reference, size_t size) {
	char* out = NEW_ARRAY(char, size+1);
	size_t len = strlen(reference);
	for (size_t i = 0; i < size; i++) {
		if (i < len) {
			out[i] = reference[i];
		} else {
			out[i] = ' ';
		}
	}
	out[size] = 0;
	return out;
}

int main (int argc, char **argv) {
	// Validate the arguments
	if (argc != 3) {
		printf("usage:\n\tcomparer <filename left> <filename right>\n");
		return EXIT_FAILURE;
	}

	// Open the files for reading
	FILE* leftFile = fopen(argv[1], "r");
	FILE* rightFile = fopen(argv[2], "r");
	if (leftFile == NULL) {
		printf("could not open `%s`\n", argv[1]);
		return EXIT_FAILURE;
	} else if (rightFile == NULL) {
		printf("could not open `%s`\n", argv[2]);
		return EXIT_FAILURE;
	}

	// Get the size of the console
	struct winsize consoleSize = getConsoleSize();
	size_t width = consoleSize.ws_col;
	if (width > 191) {
		width = 191;
	}
	width--;

	// Compute the size of the file body on the left/right
	size_t codeWidth = (width - 19) / 2;
	width = codeWidth * 2 + 19;

	// Allocate buffers for the left and right file
	char_vector_vector* leftContents = makeLines(leftFile);
	char_vector_vector* rightContents = makeLines(rightFile);

	int_vector_pair* inCommon = longestCommonSubsequence(leftContents, rightContents);
	assert(int_vector_size(inCommon->left) == int_vector_size(inCommon->right));
	//for (size_t i = 0; i < int_vector_size(inCommon->left); i++) {
	//	printf("common lines %d\t%d\n", 1+int_vector_get(inCommon->left, i, SRC), 1+int_vector_get(inCommon->right, i, SRC));
	//}

	// Output the files in two columns:
	// 1) Print headers in two columns
	putchar('+');
	for (size_t k = 0; k < width-2; k++) {
		putchar('-');
	}
	printf("+\n");
	char *leftPadded = paddedString(argv[1], codeWidth);
	char* rightPadded = paddedString(argv[2], codeWidth);
	printf("| *---- %s | *---- %s |\n", leftPadded, rightPadded);
	free(leftPadded);
	free(rightPadded);
	putchar('+');
	for (size_t k = 0; k < width - 2; k++) {
		putchar('-');
	}
	printf("+\n");

	// 2) Print body in two columns
	size_t leftLineNumber = 1;
	size_t rightLineNumber = 1;
	size_t leftOffset = 0;
	size_t rightOffset = 0;
	size_t leftLineCount = char_vector_vector_size(leftContents);
	size_t rightLineCount = char_vector_vector_size(rightContents);

	char_vector* blank = char_vector_make();

	while (leftLineNumber <= leftLineCount || rightLineNumber <= rightLineCount) {
		char_vector* leftLine;
		if (leftLineNumber-1 >= leftLineCount) {
			leftLine = blank;
		} else {
			leftLine = char_vector_vector_get(leftContents, leftLineNumber-1, SRC);
		}
		char_vector* rightLine;
		if (rightLineNumber-1 >= rightLineCount) {
			rightLine = blank;
		} else {
			rightLine = char_vector_vector_get(rightContents, rightLineNumber-1, SRC);
		}

		printf("| "); // bar space

		// Compute left/right status
		char leftStatus = '-';
		char rightStatus = '+';

		bool leftAtCommon = int_vector_size(inCommon->left) > 0 && int_vector_last(inCommon->left)+1 == (int)leftLineNumber;
		bool rightAtCommon = int_vector_size(inCommon->right) > 0 && int_vector_last(inCommon->right)+1 == (int)rightLineNumber;
		if (leftAtCommon) {
			leftStatus = ' ';
			// don't show left common line unless right also
			if (!rightAtCommon) {
				//printf("\n<delaying left %d>\n", (int)leftLineNumber);
				leftLine = blank;
			}
		}
		if (rightAtCommon) {
			rightStatus = ' ';
			// don't show right common line unless right also
			if (!leftAtCommon) {
				//printf("\n<delaying right %d>\n", (int)rightLineNumber);
				rightLine = blank;
			}
		}

		//printf("## common queues: %d <> %d\n",
		//	(int)int_vector_size(inCommon->left),
		//	(int)int_vector_size(inCommon->right)
		//);
		//printf("## next common lines: %d <> %d\n",
		//	int_vector_size(inCommon->left) > 0 ? int_vector_last(inCommon->left)+1 : -100,
		//	int_vector_size(inCommon->right) > 0 ? int_vector_last(inCommon->right)+1 : -100
		//);
		//printf("## at commons %s <> %s\n", leftAtCommon ? "YES" : "no", rightAtCommon ? "YES" : "no");

		// Show left status
		if (leftStatus == '-') {
			printf(ColorRed);
		}
		if (leftOffset == 0 && leftLine != blank) {
			putchar(leftStatus); // left status
			printf("%4d", (int)leftLineNumber % 10000); // left line number
		} else {
			printf("     ");
		}
		printf(" "); // space

		// Fill left row
		for (size_t k = 0; k < codeWidth; k++) {
			size_t index = k + leftOffset;
			if (index < char_vector_size(leftLine)) {
				putchar(char_vector_get(leftLine, index, SRC));
			} else {
				putchar(' ');
			}
		}

		// Draw central divider
		printf(ColorNormal " | ");

		// Show right status
		if (rightStatus == '+') {
			printf(ColorGreen);
		}
		if (rightOffset == 0 && rightLine != blank) {
			putchar(rightStatus); // right status
			printf("%4d", (int)rightLineNumber % 10000); // right line number
		} else {
			printf("     ");
		}
		printf(" "); // space

		// Fill right row
		for (size_t k = 0; k < codeWidth; k++) {
			size_t index = k + rightOffset;
			if (index < char_vector_size(rightLine)) {
				putchar(char_vector_get(rightLine, index, SRC));
			} else {
				putchar(' ');
			}
		}

		// Draw right-side bar
		printf(ColorNormal" |\n");

		// Advance the cursor along the current line
		if (leftLine != blank) {
			leftOffset += codeWidth;
		}
		if (rightLine != blank) {
			rightOffset += codeWidth;
		}

		// Advance the cursor to the next line
		if (leftOffset >= char_vector_size(leftLine) && rightOffset >= char_vector_size(rightLine)) {
			if (leftLine != blank) {
				leftLineNumber++;
				leftOffset = 0;
				if (leftAtCommon) {
					int_vector_pop(inCommon->left);
				}
			}
			if (rightLine != blank) {
				rightLineNumber++;
				rightOffset = 0;
				if (rightAtCommon) {
					int_vector_pop(inCommon->right);
				}
			}
		}
	}

	// Release buffers allocated with NEW_ARRAY

	return EXIT_SUCCESS;
}
