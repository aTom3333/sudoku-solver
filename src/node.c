#include "node.h"
#include <stdlib.h>


// For now links are implemented as pointer but they may be implemented as offset later
struct Node {
    struct HeaderNode* header;
    struct Node* left;
    struct Node* right;
    struct Node* up;
    struct Node* down;
};

struct HeaderNode {
    Node node; // Node is the first field so that HeaderNode* can be reinterpreted as Node*
    HeaderData data;
};

inline Node* getLeft(Node* n) {
    return n->left;
}

inline Node* getRight(Node* n)
{
    return n->right;
}

inline Node* getUp(Node* n)
{
    return n->up;
}

inline Node* getDown(Node* n)
{
    return n->down;
}

inline HeaderNode* getHeader(Node* n)
{
    return n->header;
}

inline void setLeft(Node* n, Node* left)
{
    n->left = left;
}

inline void setRight(Node* n, Node* right)
{
    n->right = right;
}

inline void setUp(Node* n, Node* up)
{
    n->up = up;
}

inline void setDown(Node* n, Node* down)
{
    n->down = down;
}

Node* createNode()
{
    return malloc(sizeof(Node));
}

void freeNode(Node* n)
{
    free(n);
}

inline void setHeader(Node* n, HeaderNode* header)
{
    n->header = header;
}

inline HeaderData* getData(HeaderNode* header)
{
    return &header->data;
}

HeaderNode* createHeaderNode()
{
    return malloc(sizeof(HeaderNode));
}

void freeHeaderNode(HeaderNode* n)
{
    free(n);
}


