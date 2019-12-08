#ifndef LIST_H
#define LIST_H

#include "node.h"

struct memory_chunk;
HeaderNode* createListFromMatrix(unsigned char** mat, unsigned int n, struct memory_chunk* mc);

void hideLine(Node* n);
void hideColumnAndLines(HeaderNode* header);

void showLine(Node* n);
void showColumnAndLines(HeaderNode* header);

void chooseLine(Node* n);
void unchooseLine(Node* n);

#endif // LIST_H