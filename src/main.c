#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int main(int argc, char** argv) {
    //int n = 25;
    //num n;    	
    MPI_Init(&argc, &argv);
    int id, p;
    MPI_Comm_rank (MPI_COMM_WORLD, &id);
    MPI_Comm_size (MPI_COMM_WORLD, &p);

    num* sudoku_data = NULL, **sudoku = NULL;
    HeaderNode* list = NULL;
    int n, n_sqrt;
    memory_chunk mc;
    
    if(id == 0)
    {
        n_sqrt = takeSize();
        if(n_sqrt > 100)
        {
            fprintf(stderr, "sûrement pas volontaire\n");
            exit(1);
        }
    }
    MPI_Bcast(&n_sqrt, 1, MPI_INT, 0, MPI_COMM_WORLD);
    n = n_sqrt * n_sqrt;
    
    void* listBuffer;
    size_t listBufferSize = sizeOfHeaderNode() + n*n*4*sizeOfHeaderNode() + n*n*n*4*sizeOfNode();
    sudoku_data = malloc(n * n * sizeof(num));
    sudoku = malloc(n * sizeof(num*));
    int i;
    for(i = 0; i < n; i++)
        sudoku[i] = sudoku_data + i * n;
    
    if(id == 0) {
        getSudoku(n, sudoku);

        printSudoku(n, sudoku);

        list = createListFromSudoku(sudoku, n_sqrt, &mc);
        listBuffer = list;
    } else {
        listBuffer = malloc(listBufferSize);
        list = listBuffer;
    }
    
    MPI_Bcast(listBuffer, listBufferSize, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    // Our linked list data structure is in one block and linked using offset so we can bit copy it
    
    MPI_Request someoneFound;
    
    num* receivingSudoku = malloc(n*n* sizeof(num));
    MPI_Irecv(receivingSudoku, n*n, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &someoneFound);
    
    MPI_Request* deadSlaves;
    int* slaveFound;
    if(id == 0)
    {
        deadSlaves = malloc((p - 1) * sizeof(MPI_Request));
        slaveFound = malloc((p - 1) * sizeof(int));
        for(i = 1; i < p; i++) {
            MPI_Irecv(NULL, 0, MPI_INT, i, 1, MPI_COMM_WORLD, &deadSlaves[i-1]);
            slaveFound[i-1] = 0;
        }
    }

    int found = 0;
    int ret = solveMPI(n, list, sudoku, p, id, &someoneFound);
    if(ret == 2) {
        // Someone else found the solution
        found = 1;
        printf("I'm %d and I'm here\n", id);
        if(id == 0) {
            printf("Slave found the solution\n");
            // Tell everyone to stop
            for(i = 1; i < p; i++) {
                MPI_Request req;
                MPI_Isend(NULL, 0, MPI_INT, i, 0, MPI_COMM_WORLD, &req);
            }
            // Keep the data
            memcpy(sudoku_data, receivingSudoku, n*n*sizeof(num));
        }
    }
    else if(ret == 1) {
        // WE found it
        found = 1;
        if(id != 0) {
            // Let's tell master
            //MPI_Request req;
            printf("I'm slave %d and I found the solution\n", id);
            MPI_Send(sudoku_data, n*n, MPI_INT, 0, 0, MPI_COMM_WORLD);
        } else {
            printf("Master found the solution\n");
            // Tell everyone to stop
            MPI_Request req;
            for(i = 1; i < p; i++) {
                MPI_Isend(NULL, 0, MPI_INT, i, 0, MPI_COMM_WORLD, &req);
            }
        }
    }
    else if(ret == 0) {
        // Nobody found it before we finished
        if(id == 0) {
            // Wait for every slave
            for(i = 1; i < p; i++) {
                MPI_Wait(&deadSlaves[i-1], NULL);
            }
            // Test if someone found
            MPI_Test(&someoneFound, &found, NULL);
            if(found)
                memcpy(sudoku_data, receivingSudoku, n*n* sizeof(num));
        }
    }
    
    
    if(id == 0)
    {
        if(found)
        {
            printf("Solution trouvée (%d)\n", found);
            printSudoku(n, sudoku);
        }
        else {
            printf("Pas de solution trouvée\n");
        }
        free_memory_chunk(&mc); // Free every node used during the algorithm
        //for(i = 1; i < p; i++) {
        //    MPI_Request_free(&deadSlaves[i-1]);
        //}
    } else {
        free(listBuffer);
        MPI_Request req;
        MPI_Isend(NULL, 0, MPI_INT, 0, 1, MPI_COMM_WORLD, &req); // Tell master we are dead
    }

    //MPI_Request_free(&someoneFound);
    free(receivingSudoku);
    free(sudoku);
    free(sudoku_data);
    MPI_Finalize();
    
    return 0;
}
