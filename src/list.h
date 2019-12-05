#ifndef LIST_H
#define LIST_H

#include "node.h"


HeaderNode* createListFromMatrix(unsigned char** mat, unsigned int n);

void hideLine(Node* n);
void hideColumnAndLines(HeaderNode* header);

#endif // LIST_H