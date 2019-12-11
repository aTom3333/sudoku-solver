#include "node.h"
#include <stdlib.h>
#include <string.h>
#include "bulk_allocator.h"
#include <stdint.h>


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


