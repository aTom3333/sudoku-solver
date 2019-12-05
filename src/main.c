#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void print_cover_matrix(unsigned char **p, int n) {
    int row_number = n * n * n;
    int col_number = n * n * 4;
    int i, j;

    printf("Matrice de %d lignes et %d colonnes :", row_number, col_number);
    if (n <= 4) {
        for (i = 0; i < row_number; i++) {
            printf("\n%.2d\t", i);
            for (j = 0; j < n * n * 4; j++)
                if (p[i][j] != 1)
                    printf(".");
                else {
                    printf("\033[0;32m");
                    printf("%u", p[i][j]);
                    printf("\033[0m");
                }
        }
    } else {
        printf("\nCELL CONSTRAINT  :\n");
        for (i = 0; i < row_number; i++) {
            printf("\n%.2d\t", i);
            for (j = 0; j < n * n; j++)
                if (p[i][j] != 1)
                    printf(".");
                else {
                    printf("\033[0;32m");
                    printf("%u", p[i][j]);
                    printf("\033[0m");
                }
        }

        printf("\nROW CONSTRAINT  :\n");
        for (i = 0; i < row_number; i++) {
            printf("\n%.2d\t", i);
            for (j = n * n; j < n * n * 2; j++)
                if (p[i][j] != 1)
                    printf(".");
                else {
                    printf("\033[0;32m");
                    printf("%u", p[i][j]);
                    printf("\033[0m");
                }
        }

        printf("\nCOL CONSTRAINT  :\n");
        for (i = 0; i < row_number; i++) {
            printf("\n%.2d\t", i);
            for (j = n * n * 2; j < n * n * 3; j++)
                if (p[i][j] != 1)
                    printf(".");
                else {
                    printf("\033[0;32m");
                    printf("%u", p[i][j]);
                    printf("\033[0m");
                }
        }

        printf("\nBOX CONSTRAINT  :\n");
        for (i = 0; i < row_number; i++) {
            printf("\n%.2d\t", i);
            for (j = n * n * 3; j < n * n * 4; j++)
                if (p[i][j] != 1)
                    printf(".");
                else {
                    printf("\033[0;32m");
                    printf("%u", p[i][j]);
                    printf("\033[0m");
                }
        }
    }
}

unsigned char **create_cover_matrix(int n) {
    int row_number = n * n * n;
    int col_number = n * n * 4;
    int sqrt_n = (int) sqrt(n);
    int i, j, counter, fixed_counter;
    int row, r, c;

    unsigned char *cover_matrix = malloc(sizeof(unsigned char) * row_number * col_number);
    unsigned char **p = malloc(sizeof(unsigned char *) * row_number);

    // Fill matrix with 0
    for (i = 0; i < row_number; i++) {
        p[i] = &cover_matrix[i * col_number];
        for (j = 0; j < col_number; j++)
            p[i][j] = 0;
    }

    // cell constraint
    counter = -1;
    for (i = 0; i < row_number; i++) {
        if (i % n == 0)
            counter++;
        p[i][counter] = 1;
    }

    // row constraint
    counter = n * n;
    fixed_counter = n * n;
    for (i = 0; i < row_number; i++) {
        if (i % (n * n) == 0 && i != 0)
            fixed_counter += n;
        if (counter == fixed_counter + n)
            counter = fixed_counter;
        p[i][counter] = 1;
        counter++;
    }

    // col constraint
    counter = n * n * 2;
    for (i = 0; i < row_number; i++) {
        p[i][counter] = 1;
        counter++;
        if (counter == n * n * 3)
            counter = n * n * 2;
    }

    // block constraint
    for (row = 0, r = 0; r < n; r++)
        for (c = 0; c < n; c++)
            for (i = 0; i < n; i++, row++)
                p[row][n * n * 3 + i + n * (sqrt_n * (r / sqrt_n) + (c / sqrt_n))] = 1;

    return p;
}

int main() {
    int n = 4;

    unsigned char **p;
    p = create_cover_matrix(n);
    print_cover_matrix(p, n);

    /*for(i = 1; i <= 9; i++) {
        for(j = 1; j <= 9; j++) {
            int k = sudoku[i - 1][j - 1];
            if (k != 0){ // zero out in the constraint board
                for(int num = 1; num <= 9; num++) {
                    if (num != k) {
                        //printf("p[%d] = 0\n", (i - 1) * 3 * 3 + (j - 1) * 9 + (k - 1));

                        for (int col = 0; col < (n * n * 4); col++)
                            p[(i - 1) * 3 * 3 + (j - 1) * 9 + (k - 1)][col] = 0;
                        //Arrays.fill(R[getIdx(i, j, num)], 0);
                    }
                }
            }
        }
    }
    print_cover_matrix(p, n);*/

    return 0;
}