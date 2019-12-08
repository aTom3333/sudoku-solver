#include "node.h"
#include <stdlib.h>
#include <string.h>
#include "bulk_allocator.h"


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

Node* createNode(memory_chunk* mc)
{
    Node* node = allocate(mc, sizeOfNode());
    memset(node, 0, sizeOfNode());
    return node;
}

void freeNode(Node* n)
{
}

inline void setHeader(Node* n, HeaderNode* header)
{
    n->header = header;
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
    return n->left != NULL;
}

inline int hasRight(Node* n)
{
    return n->right != NULL;
}

inline int hasUp(Node* n)
{
    return n->up != NULL;
}

inline int hasDown(Node* n)
{
    return n->down != NULL;
}

inline void markNoLeft(Node* n)
{
    n->left = NULL;
}

inline void markNoRight(Node* n)
{
    n->right = NULL;
}

inline void markNoUp(Node* n)
{
    n->up = NULL;
}

inline void markNoDown(Node* n)
{
    n->down = NULL;
}


