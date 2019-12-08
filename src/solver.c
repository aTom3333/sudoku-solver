//
// Created by thomas on 05/12/19.
//

#include <stdio.h>
#include "solver.h"
#include "stack.h"
#include "list.h"


int iteration(int k, HeaderNode* list, stack* st);
HeaderNode* selectColumn(HeaderNode* list);
int getInfoOfLine(Node* node, int* row, int* col, int* value);

int solve(int n, HeaderNode* list, int** res)
{
    stack st = createStack(n*n);
    
    if(iteration(0, list, &st)) {
        int i;
        for(i = 0; i < st.size; i++) {
            Node* line = st.data[i];
            int row, col, value;
            if(getInfoOfLine(line, &row, &col, &value)) {
                res[row][col] = value;
            } else {
                fprintf(stderr, "Impossible de récupérer les infos de la ligne\n");
            }
        }
        
        freeStack(&st);
        return 1;
    }
    
    freeStack(&st);
    return 0;
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

int iteration(int k, HeaderNode* list, stack* st)
{
    checkList(list);
    if(getRight((Node*) list) == (Node*) list) {
        // No more column
        return 1;
    }
    int found = 0;
    HeaderNode* column = selectColumn(list);
    //hideColumnAndLines(column);
    Node* node = getDown((Node*) column);
    while(node != (Node*) column && !found) {
        push(st, node);
        chooseLine(node);
        found = iteration(k+1, list, st);
        unchooseLine(node);
        
        if(!found)
            pop(st);
        
        node = getDown(node);
    }
    //showColumnAndLines(column);
    return found;
}

HeaderNode* selectColumn(HeaderNode* list)
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
