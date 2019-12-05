#include "list.h"
#include <stdlib.h>


void hideLine(Node* n)
{
    Node* initial = n;
    while(getRight(n) != initial) {
        setDown(getUp(n), getDown(n));
        setUp(getDown(n), getUp(n));
        n = getRight(n);
    }
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
            setRight((Node*) first, createNode());
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
            setRight((Node*) first, createNode());
            setLeft(getRight((Node*) header), (Node*) header);
            header = (HeaderNode*) getRight((Node*) header);
            HeaderData* data = getData(header);
            data->constraintType = ROW;
            data->major = row;
            data->minor = value;
            setHeader((Node*) header, header);
        }
    }
    for(col = 0; col < n; col++) {
        for(value = 0; value < n; value++) {
            // Column constraint
            setRight((Node*) first, createNode());
            setLeft(getRight((Node*) header), (Node*) header);
            header = (HeaderNode*) getRight((Node*) header);
            HeaderData* data = getData(header);
            data->constraintType = COLUMN;
            data->major = col;
            data->minor = value;
            setHeader((Node*) header, header);
        }
    }
    for(row = 0; row < n; row++) {
        for(col = 0; col < n; col++) {
            // Box constraint
            setRight((Node*) first, createNode());
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
                if(firstInRow == NULL)
                    firstInRow = node;
                
                Node* lastInColumn = (Node*) header;
                while(getDown(lastInColumn))
                    lastInColumn = getDown(lastInColumn);
                
                setDown(lastInColumn, node);
                setUp(node, lastInColumn);
                setRight(lastInRow, node);
                setLeft(node, lastInRow);
                
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
    }
    
    return first;
}

void hideColumnAndLines(HeaderNode* header)
{
    Node* node = getDown((Node*) header);
    while(getDown(node) != (Node*) header) {
        // Remove node from row and then hide remaining of row
        // Node is removed before because we don't want it to be removed from the current column in order to be restored later
        Node* nodeFromLine = getRight(node);
        if(nodeFromLine != node) {
            setLeft(nodeFromLine, getRight(node));
            setRight(getLeft(node), nodeFromLine);
            hideLine(nodeFromLine);
        }
    }
    // Hide header
    setLeft(getRight((Node*) header), getLeft((Node*) header));
    setRight(getLeft((Node*) header), getRight((Node*) header));
}
