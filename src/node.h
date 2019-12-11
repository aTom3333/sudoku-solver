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
    int major; // TODO Change type
    int minor;
    unsigned char isKnown;
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

#endif // NODE_H