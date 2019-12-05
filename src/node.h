#ifndef NODE_H
#define NODE_H

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


Node* createNode();
void freeNode(Node* n);

HeaderNode* createHeaderNode();
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

#endif // NODE_H