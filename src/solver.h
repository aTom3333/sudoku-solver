#ifndef SOLVER_H
#define SOLVER_H

#include "node.h"
#include "stack.h"
#include "dynarray.h"


int solve(int n, HeaderNode* list, int** res); // Simple solving (one proc)
int iteration(HeaderNode* list, stack* st);

int solveSubBranch(int n, HeaderNode* list, stack* st);

void followWhileNoChoice(HeaderNode* list, stack* st);

void followStack(HeaderNode* list, stack* st);
void revertStack(HeaderNode* list, stack* st);

dynarray* widthExploration(int n, int depth, HeaderNode* list);

void getGridFromStack(int n, int** grid, HeaderNode* list, stack* st);

#endif // SOLVER_H