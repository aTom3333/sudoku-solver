#include "list.h"
#include <stdlib.h>
#include <stdio.h>
#include "bulk_allocator.h"


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

HeaderNode* createListFromMatrix(unsigned char** mat, unsigned int n, memory_chunk* mc)
{
    // We will allocate one HeaderNode to represent the root, n*n*4 HeaderNode for every column
    // and as many Node as they are 1s in the matrix. There is n*n*n row in the matrix and 
    // each row can have a maximum of 4 1s, so the number of Node is smaller than n*n*n*4
    size_t allocSize = sizeOfHeaderNode() + n*n*4*sizeOfHeaderNode() + n*n*n*4*sizeOfNode();
    init_memory_chunk(mc, allocSize);
    
    HeaderNode* first = createHeaderNode(mc);
    int col, row, value;
    
    // Create the headers
    HeaderNode* header = first;
    for(row = 0; row < n; row++) {
        for(col = 0; col < n; col++) {
            // Cell constraint
            setRight((Node*) header, (Node*) createHeaderNode(mc));
            setLeft(getRight((Node*) header), (Node*) header);
            header = (HeaderNode*) getRight((Node*) header);
            markNoDown((Node*) header);
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
            setRight((Node*) header, (Node*) createHeaderNode(mc));
            setLeft(getRight((Node*) header), (Node*) header);
            header = (HeaderNode*) getRight((Node*) header);
            markNoDown((Node*) header);
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
            setRight((Node*) header, (Node*) createHeaderNode(mc));
            setLeft(getRight((Node*) header), (Node*) header);
            header = (HeaderNode*) getRight((Node*) header);
            markNoDown((Node*) header);
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
            setRight((Node*) header, (Node*) createHeaderNode(mc));
            setLeft(getRight((Node*) header), (Node*) header);
            markNoDown((Node*) header);
            header = (HeaderNode*) getRight((Node*) header);
            markNoDown((Node*) header);
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
                Node* node = createNode(mc);
                setHeader(node, header);
                getData(header)->numInCol++;
                
                if(firstInRow == NULL)
                    firstInRow = node;
                
                Node* lastInColumn = (Node*) header;
                while(hasDown(lastInColumn))
                    lastInColumn = getDown(lastInColumn);
                
                setDown(lastInColumn, node);
                setUp(node, lastInColumn);
                markNoDown(node);
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
        while(hasDown(lastInColumn))
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


static int countRowGoingDown(HeaderNode* header) {
    Node* node = getDown((Node*) header);
    int c = 0;
    while(node != (Node*) header) {
        c++;
        node = getDown(node);
    }
    return c;
}

static int countRowGoingUp(HeaderNode* header) {
    Node* node = getUp((Node*) header);
    int c = 0;
    while(node != (Node*) header) {
        c++;
        node = getUp(node);
    }
    return c;
}

static void checkList(HeaderNode* list) {
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

HeaderNode* createListFromSudoku(int** sudoku, unsigned int n_sqrt, struct memory_chunk* mc)
{
    unsigned int n = n_sqrt*n_sqrt;
    
    // We will allocate one HeaderNode to represent the root, n*n*4 HeaderNode for every column
    // and as many Node as they are 1s in the matrix. There is n*n*n row in the matrix and 
    // each row can have a maximum of 4 1s, so the number of Node is smaller than n*n*n*4
    size_t allocSize = sizeOfHeaderNode() + n*n*4*sizeOfHeaderNode() + n*n*n*4*sizeOfNode();
    init_memory_chunk(mc, allocSize);

    HeaderNode* first = createHeaderNode(mc);
    int col, row, value;
    
    HeaderNode** headers = malloc(n*n*4* sizeof(HeaderNode*));
    int headerCount = 0;

    // Create the headers
    HeaderNode* header = first;
    for(row = 0; row < n; row++) {
        for(col = 0; col < n; col++) {
            // Cell constraint
            HeaderNode* headerNode = createHeaderNode(mc);
            headers[headerCount++] = headerNode;
            setRight((Node*) header, (Node*) headerNode);
            setLeft(getRight((Node*) header), (Node*) header);
            header = (HeaderNode*) getRight((Node*) header);
            HeaderData* data = getData(header);
            data->constraintType = CELL;
            data->major = row;
            data->minor = col;
            data->isKnown = 0;
            setHeader((Node*) header, header);
        }
    }
    for(row = 0; row < n; row++) {
        for(value = 0; value < n; value++) {
            // Row constraint
            HeaderNode* headerNode = createHeaderNode(mc);
            headers[headerCount++] = headerNode;
            setRight((Node*) header, (Node*) headerNode);
            setLeft(getRight((Node*) header), (Node*) header);
            header = (HeaderNode*) getRight((Node*) header);
            HeaderData* data = getData(header);
            data->constraintType = ROW;
            data->major = row;
            data->minor = value+1;
            data->isKnown = 0;
            setHeader((Node*) header, header);
        }
    }
    for(col = 0; col < n; col++) {
        for(value = 0; value < n; value++) {
            // Column constraint
            HeaderNode* headerNode = createHeaderNode(mc);
            headers[headerCount++] = headerNode;
            setRight((Node*) header, (Node*) headerNode);
            setLeft(getRight((Node*) header), (Node*) header);
            header = (HeaderNode*) getRight((Node*) header);
            HeaderData* data = getData(header);
            data->constraintType = COLUMN;
            data->major = col;
            data->minor = value+1;
            data->isKnown = 0;
            setHeader((Node*) header, header);
        }
    }
    for(row = 0; row < n; row++) {
        for(col = 0; col < n; col++) {
            // Box constraint
            HeaderNode* headerNode = createHeaderNode(mc);
            headers[headerCount++] = headerNode;
            setRight((Node*) header, (Node*) headerNode);
            setLeft(getRight((Node*) header), (Node*) header);
            header = (HeaderNode*) getRight((Node*) header);
            HeaderData* data = getData(header);
            data->constraintType = BOX;
            data->major = row;
            data->minor = col;
            data->isKnown = 0;
            setHeader((Node*) header, header);
        }
    }
    // Circular
    setRight((Node*) header, (Node*) first);
    setLeft((Node*) first, (Node*) header);


    int rowNum = n*n*n;
    int colNum = 4*n*n;
    int n_sq = n*n;
    
    int box_row, row_in_box, box_col, col_in_box;
    for(box_row = 0; box_row < n_sqrt; box_row++) {
        for(row_in_box = 0; row_in_box < n_sqrt; row_in_box++) {
            row = box_row*n_sqrt + row_in_box;
            for(box_col = 0; box_col < n_sqrt; box_col++) {
                int box_no = box_row*n_sqrt + box_col;
                for(col_in_box = 0; col_in_box < n_sqrt; col_in_box++) {
                    col = box_col*n_sqrt + col_in_box;
                    if(sudoku[row][col]) {
                        // Value given, add the row and then remove it with the right columns
                        value = sudoku[row][col];
                        int usedHeaders[4] = {
                            row*n + col, // Row-Column constraint
                            row*n + value-1 + n_sq, // Row-Value constraint
                            col*n + value-1 + 2*n_sq, // Column-Value constraint
                            box_no*n + value-1 + 3*n_sq // Box-Value constraint
                        };
                        int i;
                        Node* firstNode = NULL;
                        Node* lastNode = NULL;
                        for(i = 0; i < 4; i++) {
                            HeaderNode* header = headers[usedHeaders[i]];

                            Node* nodeToRemove = getDown((Node*) header);
                            while(nodeToRemove != (Node*) header) {
                                // Remove node from row and then hide remaining of row
                                // Node is removed before because we don't want it to be removed from the current column in order to be restored later
                                Node* nodeFromLine = getRight(nodeToRemove);
                                if(nodeFromLine != nodeToRemove) {
                                    setLeft(nodeFromLine, getLeft(nodeToRemove));
                                    setRight(getLeft(nodeToRemove), nodeFromLine);
                                    hideLine(nodeFromLine);
                                }
                                nodeToRemove = getDown(nodeToRemove);
                            }
                            setUp((Node*) header, (Node*) header);
                            setDown((Node*) header, (Node*) header);
                            getData(header)->numInCol = 0;
                            
                            Node* node = createNode(mc);
                            setHeader(node, header);
                            getData(header)->isKnown = 1;
                            getData(header)->numInCol++;
                            // Insert node in column
                            if(hasUp((Node*) header))
                            {
                                setUp(node, getUp((Node*) header));
                                setDown(node, (Node*) header);
                            } else {
                                setUp(node, (Node*) header);
                                setDown(node, (Node*) header);
                            }
                            setUp((Node*) header, node);
                            setDown(getUp(node), node);

                            if(firstNode == NULL)
                                firstNode = node;
                            if(lastNode) {
                                // Insert in row
                                setRight(lastNode, node);
                                setLeft(node, lastNode);
                            }

                            lastNode = node;
                        }
                        // Make row circular
                        setLeft(firstNode, lastNode);
                        setRight(lastNode, firstNode);
                    } else {
                        for(value = 1; value <= n; value++) {
                            int usedHeaders[4] = {
                                row*n + col, // Row-Column constraint
                                row*n + value-1 + n_sq, // Row-Value constraint
                                col*n + value-1 + 2*n_sq, // Column-Value constraint
                                box_no*n + value-1 + 3*n_sq // Box-Value constraint
                            };
                            int i;
                            int needLine = 1;
                            for(i = 0; i < 4; i++) {
                                if(getData(headers[usedHeaders[i]])->isKnown) {
                                    needLine = 0;
                                    break;
                                }
                            }
                            if(needLine) {
                                Node* firstNode = NULL;
                                Node* lastNode = NULL;
                                for(i = 0; i < 4; i++) {
                                    Node* node = createNode(mc);
                                    HeaderNode* header = headers[usedHeaders[i]];
                                    setHeader(node, header);
                                    getData(header)->numInCol++;
                                    // Insert node in column
                                    if(hasUp((Node*) header))
                                    {
                                        setUp(node, getUp((Node*) header));
                                        setDown(node, (Node*) header);
                                    } else {
                                        setUp(node, (Node*) header);
                                        setDown(node, (Node*) header);
                                    }
                                    setUp((Node*) header, node);
                                    setDown(getUp(node), node);
                                    
                                    if(firstNode == NULL)
                                        firstNode = node;
                                    if(lastNode) {
                                        // Insert in row
                                        setRight(lastNode, node);
                                        setLeft(node, lastNode);
                                    }
                                    
                                    lastNode = node;
                                }
                                // Make row circular
                                setLeft(firstNode, lastNode);
                                setRight(lastNode, firstNode);
                            }
                        }
                    }
                }
            }
        }
    }
    
    free(headers);

    return first;
}
