#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <omp.h>
#include <mpi.h>


typedef int num;

typedef struct memory_chunk {
    void* start;
    void* current;
} memory_chunk;

void init_memory_chunk(memory_chunk* mc, size_t capacity)
{
    mc->start = malloc(capacity);
    mc->current = mc->start;
}

void* allocate(memory_chunk* mc, size_t size)
{
    // Simplest allocation possible, every memory allocation will be next to the other ones,
    // increasing the risk of corruption in case their is a buffer overrun but we expect our program
    // to be correct :)
    // We also don't check alignment, we would need to do that in a real allocator
    // but for this application, every allocation will has a size multiple of 8 so we
    // will always be 8-bytes aligned (hopefully)
    // Finally we don't check if we have exhausted the memory chunk, we expect the
    // program to be correct and reserve the correct amount of memory upfront
    void* p = mc->current;
    mc->current += size;
    return p;
}

void free_memory_chunk(memory_chunk* mc)
{
    // Free the whole chunk, every allocation made with this memory chunk is now free'd
    free(mc->start);
}


struct dynarray;
typedef struct dynarray dynarray;

typedef void(*freeFn)(void*);

// Simple dynamic array, not overly optimized because not used in hot path
struct dynarray {
    size_t capacity;
    size_t size;
    void** data;
    freeFn freeFunc;
};

dynarray* createDynArray(size_t capacity, freeFn freeFunction);
void freeDynArray(dynarray*);

void append(dynarray* arr, void* elem);

inline size_t getSizeDynArray(dynarray* arr);
inline void* getAtDynArray(dynarray* arr, size_t index);

dynarray* createDynArray(size_t capacity, freeFn freeFunction)
{
    dynarray* arr = malloc(sizeof(dynarray));
    arr->data = malloc(capacity * sizeof(void*));
    arr->capacity = capacity;
    arr->size = 0;
    arr->freeFunc = freeFunction;
    return arr;
}

void freeDynArray(dynarray* arr)
{
    size_t i;
    for(i = 0; i < getSizeDynArray(arr); i++) {
        arr->freeFunc(getAtDynArray(arr, i));
    }
    free(arr->data);
    free(arr);
}

inline size_t getSizeDynArray(dynarray* arr)
{
    return arr->size;
}

inline void* getAtDynArray(dynarray* arr, size_t index)
{
    return arr->data[index];
}

void append(dynarray* arr, void* elem)
{
    if(arr->size == arr->capacity) {
        arr->capacity *= 1.5;
        arr->data = realloc(arr->data, arr->capacity * sizeof(void*));
    }
    arr->data[arr->size++] = elem;
}


typedef struct Node Node;
typedef struct HeaderNode HeaderNode;
enum ConstraintType {
    CELL, COLUMN, ROW, BOX
};
typedef struct HeaderData { // TODO Pack
    int numInCol;
    int major; // TODO Change type
    int minor;
    unsigned char isKnown;
    enum ConstraintType constraintType;
} HeaderData;


// For now links are implemented as pointer but they may be implemented as offset later
struct Node {
    int32_t header;
    int32_t left;
    int32_t right;
    int32_t up;
    int32_t down;
};

struct HeaderNode {
    Node node; // Node is the first field so that HeaderNode* can be reinterpreted as Node*
    HeaderData data;
};

struct memory_chunk; // forward declaration
Node* createNode(struct memory_chunk* mc);
void freeNode(Node* n);

HeaderNode* createHeaderNode(struct memory_chunk* mc);
void freeHeaderNode(HeaderNode* n);

inline Node* getLeft(Node* n);
inline Node* getRight(Node* n);
inline Node* getUp(Node* n);
inline Node* getDown(Node* n);

inline void setLeft(Node* n, Node* left);
inline void setRight(Node* n, Node* right);
inline void setUp(Node* n, Node* up);
inline void setDown(Node* n, Node* down);

inline int hasLeft(Node* n);
inline int hasRight(Node* n);
inline int hasUp(Node* n);
inline int hasDown(Node* n);

inline void markNoLeft(Node* n);
inline void markNoRight(Node* n);
inline void markNoUp(Node* n);
inline void markNoDown(Node* n);

inline HeaderNode* getHeader(Node* n);
inline void setHeader(Node* n, HeaderNode* header);

inline HeaderData* getData(HeaderNode* header);

inline size_t sizeOfNode();
inline size_t sizeOfHeaderNode();

inline Node* getLeft(Node* n) {
    return (Node*)((void*)n + n->left);
}

inline Node* getRight(Node* n)
{
    return (Node*)((void*)n + n->right);
}

inline Node* getUp(Node* n)
{
    return (Node*)((void*)n + n->up);
}

inline Node* getDown(Node* n)
{
    return (Node*)((void*)n + n->down);
}

inline HeaderNode* getHeader(Node* n)
{
    return (HeaderNode*)((void*)n + n->header);
}

inline void setLeft(Node* n, Node* left)
{
    n->left = (void*)left - (void*)n;
}

inline void setRight(Node* n, Node* right)
{
    n->right = (void*)right - (void*)n;
}

inline void setUp(Node* n, Node* up)
{
    n->up = (void*)up - (void*)n;
}

inline void setDown(Node* n, Node* down)
{
    n->down = (void*)down - (void*)n;
}

int counter = 0;
Node* createNode(memory_chunk* mc)
{
    counter++;
    Node* node = allocate(mc, sizeOfNode());
    memset(node, 0, sizeOfNode());
    return node;
}

void freeNode(Node* n)
{
}

inline void setHeader(Node* n, HeaderNode* header)
{
    n->header = (void*) header - (void*)n;
}

inline HeaderData* getData(HeaderNode* header)
{
    return &header->data;
}

inline HeaderNode* createHeaderNode(memory_chunk* mc)
{
    HeaderNode* node = allocate(mc, sizeOfHeaderNode());
    memset(node, 0, sizeOfHeaderNode());
    return node;
}

inline void freeHeaderNode(HeaderNode* n)
{
}

// As the structure of Node is not public, we provide a function to get its size for allocation purpose
inline size_t sizeOfNode()
{
    return sizeof(Node);
}

// Same
inline size_t sizeOfHeaderNode()
{
    return sizeof(HeaderNode);
}

inline int hasLeft(Node* n)
{
    return n->left != -1;
}

inline int hasRight(Node* n)
{
    return n->right != -1;
}

inline int hasUp(Node* n)
{
    return n->up != -1;
}

inline int hasDown(Node* n)
{
    return n->down != -1;
}

inline void markNoLeft(Node* n)
{
    n->left = -1;
}

inline void markNoRight(Node* n)
{
    n->right = -1;
}

inline void markNoUp(Node* n)
{
    n->up = -1;
}

inline void markNoDown(Node* n)
{
    n->down = -1;
}


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


typedef struct stack stack;

struct stack {
    int32_t* data;
};

stack* createStack(size_t capacity)
{
    stack* st = malloc(sizeof(struct stack));
    st->data = malloc((capacity+1) * sizeof(int32_t));
    st->data[0] = 0;
    return st;
}

void freeStack(stack* st)
{
    free(st->data);
    free(st);
}

void push(stack* st, Node* node, HeaderNode* list)
{
    st->data[++(st->data[0])] = (void*)node - (void*)list;
}

Node* pop(stack* st, HeaderNode* list)
{
    return st->data[st->data[0]--] + (void*)list;
}

inline size_t getSize(stack* st)
{
    return st->data[0];
}

inline Node* getAt(stack* st, size_t index, HeaderNode* list)
{
    return st->data[index+1] + (void*)list;
}

void* getRawData(stack* st)
{
    return st->data;
}

stack* fromRawData(void* data)
{
    stack* st = malloc(sizeof(struct stack));
    st->data = data;
    return st;
}

stack* copyStack(stack* src, size_t capacity)
{
    stack* dst = malloc(sizeof(struct stack));
    dst->data = malloc((capacity+1) * sizeof(int32_t));
    memcpy(dst->data, src->data, (src->data[0]+1)* sizeof(int32_t));
    return dst;
}

void printStack(stack* st)
{
    printf("size: %d\n", st->data[0]);
    int i;
    for(i = 0; i < st->data[0]; i++) {
        printf("\t%d", st->data[i+1]);
    }
    puts("");
}

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

unsigned char **create_cover_matrix(int n_sqrt) {
    int n = n_sqrt * n_sqrt;
    int row_number = n * n * n;
    int col_number = n * n * 4;
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
                p[row][n * n * 3 + i + n * (n_sqrt * (r / n_sqrt) + (c / n_sqrt))] = 1;

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
