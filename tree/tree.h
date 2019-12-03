#ifndef TREE_H_INCLUDED
#define TREE_H_INCLUDED

#define SEC_ON
#define LOG_ON

#define TREE_INIT(tree) TreeInit(tree, #tree)

#ifdef SEC_ON
#define TREE_VERIFY(tree)   if (TreeVerify(tree) != OK) {\
                                TreeDump(tree, #tree);\
                                TreeFree(tree);\
                                printf("executed with errors! see dump file for details\n");\
                                exit(-1);\
                            }
#else
#define TREE_VERIFY(tree)   if (TreeVerify(tree) != OK) {\
                                TreeFree(tree);\
                                printf("executed with errors! see dump file for details\n");\
                                exit(-1);\
                            }
#endif

//####################//

typedef int data;

enum TreeErrs {
    OK,
    E_HASH_CORRUPTED,
    E_INIT_ERR,
    E_REALLOC_ERR,
    E_SEQ_CORRUPTED,
    E_SIZE_INVALID,
    E_LOG_DEAD,
};

enum Branches {
    left = 0,
    right = 1,
};

/*
struct _NodeStr {
    int parent;
    int children[2];
    char data[20];
};
typedef struct _NodeStr NodeStr;
*/

struct _NodeInt {
    int parent;
    int branch[2];
    int data;
};
typedef struct _NodeInt Node;

struct _Tree {
    Node* nodes;
    int* memmap;
    int max;
    int cur;
    int root;
    int free;

    int logfd;
    enum TreeErrs err;
    long hash;
};
typedef struct _Tree Tree;

//####################//

Node* NodeAlloc();
int NodeInit(Node* node, const int parent, const int branchL, const int branchR, const data data);
void NodeFree(Node* node);

Tree* TreeAlloc();
int TreeInit(Tree* tree, const char* name);
void TreeFree(Tree* tree);
int TreeResize(Tree* tree, const int sizeNew);
int TreeSort(Tree* tree);
int _TreeSort(Tree* tree, const int node, const int parent, const int branch, int* counter, Node* buf);

int TreeGetRoot(Tree* tree);
int TreeGetFree(Tree* tree);
int TreeGetCur(Tree* tree);
int TreeGetMax(Tree* tree);
int TreeFind(Tree* tree, const int node, data data);
int TreeCountSubtree(Tree* tree, const int node);
int _TreeCountSubtree(Tree* tree, const int node, int* counter);

int TreeInsertNode(Tree* tree, const int parent, const int branch, const data data);
int TreeCopySubtree(Tree* src, Node* dst, const int node);
int _TreeCopySubtree(Tree* src, Node* dst, const int node, int* pos);
int TreeDeleteNode(Tree* tree, const int node);
int _TreeDeleteNode(Tree* tree, const int node);
int TreeChangeNode(Tree* tree, const int node, int* parentNew, int* branchLNew, int* branchRNew, data* dataNew);

Tree* TreeRead(const char* pathname);
int TreeWrite(Tree* tree, const char* pathname);

char* GetTimestamp();

int TreeVerify(Tree* tree);
void TreeDump(Tree* tree, const char* name);
int TreeGetHash(Tree* tree);

int TreeUpdLog(Tree* tree, const char* func);

//####################//


//####################//

#endif