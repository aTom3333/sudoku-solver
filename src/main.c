#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "solver.h"
#include "bulk_allocator.h"


typedef int num;

int takeSize(){
    int n;
	scanf("%d", &n);
	return n;
}

void getSudoku(int n,  num** grid){
	int i, j;
	for(i = 0; i < n; i++){  //Read the numbers
		for(j = 0; j < n; j++){
		    int var;
			scanf("%d", &var);
			grid[i][j] = var;
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

void cnt_ligne_colonne_adj(unsigned char **p, int start, int n, int mat[n][n], int i, int j, int compteur){
    int a;
    int k;
    for(k = start; k < start + n; k++){
        if(p[k][compteur] == 1){
            for(a = 0; a < n; a++){
                if(mat[i][a]==(k+1)%n && mat[i][a]!=0){
                    p[k][compteur] = 0;
                    break;
                }                           
            }
            for(a = 0; a < n; a++){
                if(mat[a][j]==(k+1)%n && mat[a][j]!=0){
                    p[k][compteur] = 0;
                    break;
                }
            }
        }
    }
}

void cnt_ligne_colonne_adj2(unsigned char **p, int colonne, int n, int mat[n][n], int i, int j, int compteur){
    int a, k;
    int start = compteur*n;
    int c = colonne;
    for(k = start; k < start + n; k++){
        if(p[k][c] == 1){
            for(a = 0; a < n; a++){
                if(mat[i][a]==(k+1)%n && mat[i][a]!=0){
                    p[k][c] = 0;
                    break;
                }   
                if(mat[i][a]==n){
                    p[k][colonne+n-1] = 0;
                }                       
            }   
            for(a = 0; a < n; a++){
                if(mat[a][j]==(k+1)%n && mat[a][j]!=0){
                    p[k][c] = 0;
                    break;
                }
                if(mat[a][j]==n){
                    p[k][colonne+n-1] = 0;
                }
            }
        }
        c++;
    }
}

void ligne_colonnes(unsigned char ** p, num n, num mat[n][n]){
    int compteur=0;
    int i, j, k;
     for(i = 0; i < n; i++){
        for(j = 0; j < n; j++){
            if(mat[i][j]!=0){ //Cases remplies
                int start = compteur*n;
                for(k = start; k < start + n; k++){
                    if(p[k][compteur] == 1 && (k+1)%n != mat[i][j]){
                        p[k][compteur] = 0;
                    }
                }
                if(mat[i][j]==n){
                    p[start+n-1][compteur] = 1;
                }
            }
            else{  //Contraintes dûes aux lignes et aux colonnes
                int start = compteur*n;
                cnt_ligne_colonne_adj(p, start, n, mat, i, j, compteur);
            }
            compteur++;
        }
    }
}

void ligne_nombre(unsigned char ** p, num n, num mat[n][n]){
    int colonne = n*n;
    int compteur = 0;
    int i, j, k;
    for(i = 0; i < n; i++){
        for(j= 0; j<n; j++){
            if(mat[i][j]!=0){
                int start = compteur*n;
                int c = colonne;
                for(k = start; k < start + n;k++){
                    if(p[k][c]==1 && (k+1)%n!=mat[i][j]){
                        p[k][c]=0;
                    }
                    c++;
                }
                if(mat[i][j]==n){
                    p[start+n-1][c-1] = 1;
                }
            }
            else{               
                cnt_ligne_colonne_adj2(p, colonne, n, mat, i, j, compteur);
            }
            compteur++;
        }
        colonne+=n;
    }
}

void colonne_nombre(unsigned char ** p, num n, num mat[n][n]){
    int i, j, k;
    int compteur = 0;
    int colonne = n*n*2;
    for(i = 0; i < n; i++){
        for(j = 0; j < n; j++){
            if(mat[i][j]!=0){
                int start = compteur * n;
                int c = colonne;
                for(k = start; k < start+n; k++){
                    if(p[k][c]==1 && (k+1)%n!=mat[i][j]){
                        p[k][c]=0;
                    }
                    c++;
                }
                if(mat[i][j]==n){
                    p[start+n-1][c-1] = 1;
                }
            }
            else{
                cnt_ligne_colonne_adj2(p, colonne, n, mat, i, j, compteur);
            }
            compteur++;
            colonne+=n;
        }
        colonne=n*n*2;
    }
}

void fillInCover(unsigned char ** p, num n, num mat[n][n]){
     

    // Ligne-Colonne
    ligne_colonnes(p, n, mat);

    //ligne-Nombre
    ligne_nombre(p,n,mat);

    //Colonne-Nombre
    colonne_nombre(p,n,mat);

    

    //Boîte-Nombre

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

void printSudoku(int n, num** s) {
    int i,j;
    for(i = 0; i < n; i++) {
        for(j = 0; j < n; j++) {
            printf("%d ", (int)s[i][j]);
        }
        puts("");
    }
}

int main() {
    //int n = 25;
    //num n;    	

    int n_sqrt = takeSize();
    if(n_sqrt > 100) {
        fprintf(stderr, "sûrement pas volontaire\n");
        exit(1);
    }
    int n = n_sqrt * n_sqrt;
    num* sudoku_data = malloc(n*n* sizeof(num));
    num** sudoku = malloc(n*sizeof(num*));
    int i;
    for(i = 0; i < n; i++)
        sudoku[i] = sudoku_data + i*n;
    getSudoku(n, sudoku);


    //unsigned char** p = create_cover_matrix(n);
    //print_cover_matrix(p, n);
    
    //puts("");
    printSudoku(n, sudoku);

    //puts("");
    memory_chunk mc; // Only one memory_chunk for now
    //HeaderNode* list = createListFromMatrix(p, n, &mc);
    HeaderNode* list = createListFromSudoku(sudoku, n_sqrt, &mc);
    //free(p[0]);
    //free(p);
    //HeaderNode* list = createDebugList();
    
    solve2(n, list, sudoku);
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
    free_memory_chunk(&mc); // Free every node used during the algorithm
    free(sudoku);
    free(sudoku_data);

    return 0;
}
