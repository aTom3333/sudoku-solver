#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "solver.h"


typedef int num;

int takeSize(num n){
	scanf("%d", &n);
	n = n * n; //n => n²
	return n;
}

void getSudoku(num n,  num grid[n][n]){
	int i, j;
	for(i = 0; i < n; i++){  //Read the numbers
		for(j = 0; j < n; j++){
			scanf("%d", &grid[i][j]);
		}
	}
}


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
        printf("\n");
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
        printf("\n");
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


// TODO Temp
int** emptySudoku(int n) {
    int* data = calloc(sizeof(int), n*n);
    int** mat = malloc(n * sizeof(int*));
    int i;
    for(i = 0; i < n; i++) {
        mat[i] = data + i*n;
    }
    return mat;
}

void printSudoku(int n, int** s) {
    int i,j;
    for(i = 0; i < n; i++) {
        for(j = 0; j < n; j++) {
            printf("%d ", s[i][j]);
        }
        puts("");
    }
}

HeaderNode* createDebugList() {
    HeaderNode* headers[6];
    int i =0;
    for(i = 0; i < 3; i++) {
        headers[i] = createHeaderNode();
        setHeader(headers[i], headers[i]);
        getData(headers[i])->constraintType = CELL;
        getData(headers[i])->major = 0;
        getData(headers[i])->minor = i;
        if(i > 0) {
            setLeft(headers[i], headers[i-1]);
            setRight(headers[i-1], headers[i]);
        }
    }
    for(i = 3; i < 6; i++) {
        headers[i] = createHeaderNode();
        setHeader(headers[i], headers[i]);
        getData(headers[i])->constraintType = ROW;
        getData(headers[i])->major = 0;
        getData(headers[i])->minor = i;
        setLeft(headers[i], headers[i-1]);
        setRight(headers[i-1], headers[i]);
    }
    
    Node* lastOfLine[9] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
    Node* lastOfCol[6] = {headers[0], headers[1], headers[2], headers[3], headers[4], headers[5]};
    
    int toCreate[][2] = {
        {0, 0},
        {0, 3},
        {1, 0},
        {1, 4},
        {2, 0},
        {2, 5},
        {3, 1},
        {3, 3},
        {4, 1},
        {4, 4},
        {5, 1},
        {5, 5},
        {6, 2},
        {6, 3},
        {7, 2},
        {7, 4},
        {8, 2},
        {8, 5},
    };
    for(i = 0; i < 18; i++) {
        int row = toCreate[i][0];
        int col = toCreate[i][1];
        Node* n = createNode();
        setHeader(n, headers[col]);
        getData(headers[col])->numInCol++;
        if(lastOfCol[col]) {
            setUp(n, lastOfCol[col]);
            setDown(lastOfCol[col], n);
        }
        if(lastOfLine[row]) {
            setLeft(n, lastOfLine[row]);
            setRight(n, lastOfLine[row]);
            setRight(lastOfLine[row], n);
            setLeft(lastOfLine[row], n);
        }
        lastOfCol[col] = n;
        lastOfLine[row] = n;
    }
    for(i = 0; i < 6; i++) {
        setUp(headers[i], lastOfCol[i]);
        setDown(lastOfCol[i], headers[i]);
    }
    
    HeaderNode* list = createHeaderNode();
    setRight(list, headers[0]);
    setLeft(headers[0], list);
    setLeft(list, headers[5]);
    setRight(headers[5], list);
    
    return list;
}

int main() {
    int n = 4;
    //num n;    	
    unsigned char **p;
    num grid[n][n];

    //n = takeSize(n);
    getSudoku(n, grid);


    p = create_cover_matrix(n);
    //print_cover_matrix(p, n);
    
    //puts("");
    int** sudoku = emptySudoku(n);
    //printSudoku(n, sudoku);

    //puts("");
    HeaderNode* list = createListFromMatrix(p, n);
    //HeaderNode* list = createDebugList();
    
    solve(n, list, sudoku);
    printSudoku(n, sudoku);
    
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
    
    // TODO Free

    return 0;
}
