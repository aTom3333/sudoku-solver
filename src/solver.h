#ifndef SOLVER_H
#define SOLVER_H

#include <mpi.h>
#include "node.h"
#include "stack.h"
#include "dynarray.h"


int solve(int n, HeaderNode* list, int** res); // Simple solving (one proc)
int solve2(int n, HeaderNode* list, int** res); // Solver by cutting the problem (one proc)

int solveMPI(int n, HeaderNode* list, int** res, int p, int id, MPI_Request* someoneFound);

int iteration(HeaderNode* list, stack* st);

int solveSubBranch(int n, HeaderNode* list, stack* st);
int solveSubBranchMPI(int n, HeaderNode* list, stack* st, int p, int id, MPI_Request* someoneFound);

void followWhileNoChoice(HeaderNode* list, stack* st);

void followStack(HeaderNode* list, stack* st);
void revertStack(HeaderNode* list, stack* st);

dynarray* widthExploration(int n, int depth, HeaderNode* list);

void getGridFromStack(int n, int** grid, HeaderNode* list, stack* st);

#endif // SOLVER_H