#ifndef NODE_H
#define NODE_H

#include <stddef.h>

// Public interface because implementation is not required to store pointers

typedef struct Node Node;
typedef struct HeaderNode HeaderNode;
enum ConstraintType {
    CELL, COLUMN, ROW, BOX
};
typedef struct HeaderData { // TODO Pack
    int numInCol;
    int major;
    int minor;
    enum ConstraintType constraintType;
} HeaderData;

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

inline HeaderNode* getHeader(Node* n);
inline void setHeader(Node* n, HeaderNode* header);

inline HeaderData* getData(HeaderNode* header);

inline size_t sizeOfNode();
inline size_t sizeOfHeaderNode();

#endif // NODE_H