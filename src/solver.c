//
// Created by thomas on 05/12/19.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "solver.h"
#include "stack.h"
#include "list.h"


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
