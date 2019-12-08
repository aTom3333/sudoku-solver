#include "list.h"
#include <stdlib.h>
#include <stdio.h>


void hideLine(Node* n)
{
    Node* initial = n;
    do {
        if(getData(getHeader(n))->numInCol <= 0) {
            fprintf(stderr, "nul\n");
        }
        setDown(getUp(n), getDown(n));
        setUp(getDown(n), getUp(n));
        getData(getHeader(n))->numInCol--;
        n = getRight(n);
    } while(n != initial);
}

HeaderNode* createListFromMatrix(unsigned char** mat, unsigned int n)
{
    HeaderNode* first = createHeaderNode();
    int col, row, value;
    
    // Create the headers
    HeaderNode* header = first;
    for(row = 0; row < n; row++) {
        for(col = 0; col < n; col++) {
            // Cell constraint
            setRight((Node*) header, (Node*) createHeaderNode());
            setLeft(getRight((Node*) header), (Node*) header);
            header = (HeaderNode*) getRight((Node*) header);
            HeaderData* data = getData(header);
            data->constraintType = CELL;
            data->major = row;
            data->minor = col;
            setHeader((Node*) header, header);
        }
    }
    for(row = 0; row < n; row++) {
        for(value = 0; value < n; value++) {
            // Row constraint
            setRight((Node*) header, (Node*) createHeaderNode());
            setLeft(getRight((Node*) header), (Node*) header);
            header = (HeaderNode*) getRight((Node*) header);
            HeaderData* data = getData(header);
            data->constraintType = ROW;
            data->major = row;
            data->minor = value+1;
            setHeader((Node*) header, header);
        }
    }
    for(col = 0; col < n; col++) {
        for(value = 0; value < n; value++) {
            // Column constraint
            setRight((Node*) header, (Node*) createHeaderNode());
            setLeft(getRight((Node*) header), (Node*) header);
            header = (HeaderNode*) getRight((Node*) header);
            HeaderData* data = getData(header);
            data->constraintType = COLUMN;
            data->major = col;
            data->minor = value+1;
            setHeader((Node*) header, header);
        }
    }
    for(row = 0; row < n; row++) {
        for(col = 0; col < n; col++) {
            // Box constraint
            setRight((Node*) header, (Node*) createHeaderNode());
            setLeft(getRight((Node*) header), (Node*) header);
            header = (HeaderNode*) getRight((Node*) header);
            HeaderData* data = getData(header);
            data->constraintType = BOX;
            data->major = row;
            data->minor = col;
            setHeader((Node*) header, header);
        }
    }
    // Circular
    setRight((Node*) header, (Node*) first);
    setLeft((Node*) first, (Node*) header);
    
    
    int rowNum = n*n*n;
    int colNum = 4*n*n;
    
    for(row = 0; row < rowNum; row++) {
        header = (HeaderNode*) getRight((Node*) first); // Get first real header
        Node* lastInRow = NULL;
        Node* firstInRow = NULL;
        for(col = 0; col < colNum; col++) {
            if(mat[row][col]) {
                // Create Node
                Node* node = createNode();
                setHeader(node, header);
                getData(header)->numInCol++;
                
                if(firstInRow == NULL)
                    firstInRow = node;
                
                Node* lastInColumn = (Node*) header;
                while(getDown(lastInColumn)) // TODO Replace by hasDown
                    lastInColumn = getDown(lastInColumn);
                
                setDown(lastInColumn, node);
                setUp(node, lastInColumn);
                if(lastInRow)
                {
                    setRight(lastInRow, node);
                    setLeft(node, lastInRow);
                }
                
                lastInRow = node;
            }
            header = (HeaderNode*) getRight((Node*) header);
        }
        if(lastInRow) {
            // if there is at least one node in the row, make the row circular
            setLeft(firstInRow, lastInRow);
            setRight(lastInRow, firstInRow);
        }
    }
    
    // Make every column circular
    header = (HeaderNode*) getRight((Node*) first); // Get first real header
    for(col = 0; col < colNum; col++) {
        Node* lastInColumn = (Node*) header;
        while(getDown(lastInColumn))
            lastInColumn = getDown(lastInColumn);
        
        setDown(lastInColumn, (Node*) header);
        setUp((Node*) header, lastInColumn);
        
        header = (HeaderNode*) getRight((Node*) header);
    }
    
    return first;
}

void hideColumnAndLines(HeaderNode* header)
{
    // Hide header
    setLeft(getRight((Node*) header), getLeft((Node*) header));
    setRight(getLeft((Node*) header), getRight((Node*) header));
    
    Node* node = getDown((Node*) header);
    while(node != (Node*) header) {
        // Remove node from row and then hide remaining of row
        // Node is removed before because we don't want it to be removed from the current column in order to be restored later
        Node* nodeFromLine = getRight(node);
        if(nodeFromLine != node) {
            setLeft(nodeFromLine, getLeft(node));
            setRight(getLeft(node), nodeFromLine);
            hideLine(nodeFromLine);
        }
        node = getDown(node);
    }
}

void showLine(Node* n)
{
    Node* initial = n;
    do {
        setDown(getUp(n), n);
        setUp(getDown(n), n);
        getData(getHeader(n))->numInCol++;
        n = getLeft(n);
    } while(n != initial);
}

void showColumnAndLines(HeaderNode* header)
{
    Node* node = getUp((Node*) header);
    while(node != (Node*) header) {
        Node* nodeFromLine = getRight(node);
        if(nodeFromLine != node) {
            showLine(nodeFromLine);
            setLeft(nodeFromLine, node);
            setRight(getLeft(node), node);
        }
        node = getUp(node);
    }
    // Show header
    setLeft(getRight((Node*) header), (Node*) header);
    setRight(getLeft((Node*) header), (Node*) header);
}

void chooseLine(Node* n)
{
    // Call hide column for every column of the line
    n = getRight(n);
    Node* initial = n;
    do {
        // Remove n from its column
        setDown(getUp(n), getDown(n));
        setUp(getDown(n), getUp(n));
        
        // Hide remaining of column and associated lines
        hideColumnAndLines(getHeader(n));
        
        n = getRight(n);
    } while(n != initial);
}

void unchooseLine(Node* n)
{
    Node* initial = n;
    do {
        // Show remaining of column and associated lines
        showColumnAndLines(getHeader(n));
        
        // Restore n in its column
        setDown(getUp(n), n);
        setUp(getDown(n), n);

        n = getLeft(n);
    } while(n != initial);
}
