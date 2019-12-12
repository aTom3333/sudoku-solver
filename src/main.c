#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "solver.h"
#include "bulk_allocator.h"



HeaderNode* selectSmallestColumn(HeaderNode* list);
int getInfoOfLine(Node* node, int* row, int* col, int* value);

int solve(int n, HeaderNode* list, int** res)
{
    stack* st = createStack(n*n);
    followWhileNoChoice(list, st);
    if(iteration(list, st)) {
        getGridFromStack(n, res, list, st);
        freeStack(st);
        return 1;
    }

    freeStack(st);
    return 0;
}

int solve2(int n, HeaderNode* list, int** res)
{
    // Most likely less efficient than the first one but I can test the other functions
    stack* noChoiceStack = createStack(n*n);
    followWhileNoChoice(list, noChoiceStack);
    getGridFromStack(n, res, list, noChoiceStack);
    freeStack(noChoiceStack);

    // Duplicate data structure for openMP

    int proc = omp_get_num_procs();
    omp_set_num_threads(proc);
    HeaderNode** lists = malloc(proc * sizeof(HeaderNode*));
    int i;
    for(i = 0; i < proc; i++) {
        lists[i] = malloc(sizeOfHeaderNode() + n*n*4*sizeOfHeaderNode() + n*n*n*4*sizeOfNode());
        memcpy(lists[i], list, sizeOfHeaderNode() + n*n*4*sizeOfHeaderNode() + n*n*n*4*sizeOfNode());
    }

    int solved = 0;
    volatile int flag = 0;
    dynarray* arr = widthExploration(n, 3, list);
    size_t size = getSizeDynArray(arr);
    #pragma omp parallel for shared(solved, flag)
    for(i = 0; i < size; i++) {
        if(flag) continue; // Avoid using break for openMP
        int localSolved = 0;
        stack* st = getAtDynArray(arr, i);
        int threadId = omp_get_thread_num();
        HeaderNode* myList = lists[threadId];
        localSolved = solveSubBranch(n, myList, st);
        if(localSolved == 1) {
            getGridFromStack(n, res, myList, st);
            solved = 1;
        }
        if(localSolved)
            flag = 1;
    }
    freeDynArray(arr);

    return solved;
}

int countColumn(HeaderNode* list) {
    HeaderNode* header = (HeaderNode*) getRight((Node*) list);
    int c = 0;
    while(header != list) {
        c++;
        header = (HeaderNode*) getRight((Node*) header);
    }
    return c;
}

int countRowGoingDown(HeaderNode* header) {
    Node* node = getDown((Node*) header);
    int c = 0;
    while(node != (Node*) header) {
        c++;
        node = getDown(node);
    }
    return c;
}

int countRowGoingUp(HeaderNode* header) {
    Node* node = getUp((Node*) header);
    int c = 0;
    while(node != (Node*) header) {
        c++;
        node = getUp(node);
    }
    return c;
}

void checkList(HeaderNode* list) {
    HeaderNode* header = (HeaderNode*) getRight((Node*) list);
    while(header != list) {
        int c1 = getData(header)->numInCol;
        int c2 = countRowGoingDown(header);
        int c3 = countRowGoingUp(header);
        if(c1 != c2 || c2 != c3) {
            fprintf(stderr, "problème\n");
        }

        header = (HeaderNode*) getRight((Node*) header);
    }
}

int iteration(HeaderNode* list, stack* st)
{
    //checkList(list);
    if(getRight((Node*) list) == (Node*) list) {
        // No more column
        return 1;
    }
    int found = 0;
    HeaderNode* column = selectSmallestColumn(list);
    //hideColumnAndLines(column);
    Node* node = getDown((Node*) column);
    while(node != (Node*) column && !found) {
        push(st, node, list);
        chooseLine(node);
        found = iteration(list, st);
        unchooseLine(node);

        if(!found)
            pop(st, list);

        node = getDown(node);
    }
    //showColumnAndLines(column);
    return found;
}




int iterationMPI(HeaderNode* list, stack* st, int* counter, int p, int id, MPI_Request* someoneFound)
{
    if((*counter)++ > 10000) {
        int someoneElseFound = 0;
        #pragma omp critical
        MPI_Test(someoneFound, &someoneElseFound, MPI_STATUS_IGNORE);
        if(someoneElseFound)
            return 2;
        *counter = 0;
    }
    //checkList(list);
    if(getRight((Node*) list) == (Node*) list) {
        // No more column
        return 1;
    }
    int found = 0;
    HeaderNode* column = selectSmallestColumn(list);
    //hideColumnAndLines(column);
    Node* node = getDown((Node*) column);
    while(node != (Node*) column && !found) {
        push(st, node, list);
        chooseLine(node);
        found = iterationMPI(list, st, counter, p, id, someoneFound);
        unchooseLine(node);

        if(!found)
            pop(st, list);

        node = getDown(node);
    }
    //showColumnAndLines(column);
    return found;
}


HeaderNode* selectSmallestColumn(HeaderNode* list)
{
    HeaderNode* current = (HeaderNode*) getRight((Node*) list);
    HeaderNode* best = current;
    while(current != list) {
        if(getData(current)->numInCol < getData(best)->numInCol)
            best = current;
        current = (HeaderNode*) getRight((Node*) current);
    }

    return best;
}

HeaderNode* selectBiggestColumn(HeaderNode* list)
{
    HeaderNode* current = (HeaderNode*) getRight((Node*) list);
    HeaderNode* best = current;
    while(current != list) {
        if(getData(current)->numInCol > getData(best)->numInCol)
            best = current;
        current = (HeaderNode*) getRight((Node*) current);
    }

    return best;
}

int getInfoOfLine(Node* node, int* row, int* col, int* value)
{
    Node* initial = node;
    int hasRow = 0, hasCol = 0, hasValue = 0;
    do {
        HeaderData* data = getData(getHeader(node));
        switch(data->constraintType) {
            case CELL:
                hasRow = hasCol = 1;
                *row = data->major;
                *col = data->minor;
                break;
            case ROW:
                hasRow = hasValue = 1;
                *row = data->major;
                *value = data->minor;
                break;
            case COLUMN:
                hasCol = hasValue = 1;
                *col = data->major;
                *value = data->minor;
                break;
            default:;
        }
        node = getRight(node);
    } while((initial != node) && (!hasRow || !hasCol || !hasValue));

    return hasRow && hasCol && hasValue;
}



void followStack(HeaderNode* list, stack* st)
{
    size_t size = getSize(st);
    int i;
    for(i = 0; i < size; i++) {
        Node* line = getAt(st, i, list);
        chooseLine(line);
    }
}

void revertStack(HeaderNode* list, stack* st)
{
    size_t size = getSize(st);
    int i;
    for(i = size-1; i >= 0; i--) {
        Node* line = getAt(st, i, list);
        unchooseLine(line);
    }
}

void followWhileNoChoice(HeaderNode* list, stack* st)
{
    HeaderNode* column = selectSmallestColumn(list);
    while(getData(column)->numInCol == 1) {
        Node* node = getDown((Node*) column);
        push(st, node, list);
        chooseLine(node);
        column = selectSmallestColumn(list);
    }
}


void widthExplorationIteration(int n, int depth, HeaderNode* list, dynarray* arr, stack* st) {
    if(getRight((Node*) list) == (Node*) list) {
        // No more column
        append(arr, copyStack(st, n*n));
        return;
    }
    HeaderNode* column = selectBiggestColumn(list);
    Node* node = getDown((Node*) column);
    while(node != (Node*) column) {
        push(st, node, list);
        if(depth > 1)
        {
            chooseLine(node);
            widthExplorationIteration(n, depth-1, list, arr, st);
            unchooseLine(node);
        }
        else
        {
            append(arr, copyStack(st, n*n));
        }
        pop(st, list);

        node = getDown(node);
    }
}

void freeStackFn(void* st) {
    freeStack((stack*)st);
}

dynarray* widthExploration(int n, int depth, HeaderNode* list)
{
    dynarray* arr = createDynArray(10, freeStackFn);
    stack* st = createStack(n*n);
    widthExplorationIteration(n, depth, list, arr, st);
    freeStack(st);
    return arr;
}

int solveSubBranch(int n, HeaderNode* list, stack* st)
{
    followStack(list, st);
    int found = iteration(list, st);
    revertStack(list, st);
    return found;
}

int solveSubBranchMPI(int n, HeaderNode* list, stack* st, int p, int id, MPI_Request* someoneFound)
{
    followStack(list, st);
    int counter = 0;
    int found = iterationMPI(list, st, &counter, p, id, someoneFound);
    revertStack(list, st);
    return found;
}

void getGridFromStack(int n, int** grid, HeaderNode* list, stack* st)
{
    int i;
    for(i = 0; i < getSize(st); i++) {
        Node* line = getAt(st, i, list);
        int row, col, value;
        if(getInfoOfLine(line, &row, &col, &value)) {
            grid[row][col] = value;
        } else {
            fprintf(stderr, "Impossible de récupérer les infos de la ligne\n");
        }
    }
}

int solveMPI(int n, HeaderNode* list, int** res, int p, int id, MPI_Request* someoneFound)
{
    stack* noChoiceStack = createStack(n*n);
    followWhileNoChoice(list, noChoiceStack);
    getGridFromStack(n, res, list, noChoiceStack);
    freeStack(noChoiceStack);

    int solved = 0;
    dynarray* arr = widthExploration(n, 3, list);
    size_t size = getSizeDynArray(arr);
    int i;
    int start = (id / p) * size;
    int end = ((id+1) / p) * size;
    for(i = start; i < end; i++) {
        stack* st = getAtDynArray(arr, i);
        solved = solveSubBranchMPI(n, list, st, p, id, someoneFound);

        if(solved == 1) {
            getGridFromStack(n, res, list, st);
            break;
        }
    }
    freeDynArray(arr);

    return solved;
}



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
