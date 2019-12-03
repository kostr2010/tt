#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

//####################//

#define SEC_ON
#define LOG_ON

#define TREE_INIT(tree, rank)    TreeInit(tree, #tree, rank)

#ifdef SEC_ON
#define TREE_VERIFY(tree)   if (TreeVerify(tree) != OK) {\
                                TreeDump(tree, #tree);\
                                printf("executed with errors! see dump file for details\n");\
                                exit(-1);\
                            }
#else
#define DLLIST_VERIFY(list) {}
#endif

//####################//

enum Children {
    left,
    mid,
    right,
};

enum TreeErrs {
    OK,
    E_INIT_ERR,
    E_RESIZE_ERR,
    E_TREE_OVERFLOW,
    E_TEEE_UNDERFLOW,
    E_SEQ_CORRUPTED,
    E_SIZE_INVALID,

    #ifdef SEC_ON
    E_HASH_CORUPTED,
    #endif

    #ifdef LOG_ON
    E_LOGFILE_DEAD,
    #endif
};

// node's data type
typedef int data;

// tree's node structure
struct _Node {  
    data value;         // node's value
    int children[3];    // node's branches, enumerated from left to right
    int parent;         // node parent's physical address
};
typedef struct _Node Node;

// tree structure
struct _Tree {
    int rank;           // number of children per node (from 1 to 3)
    Node* nodes;        // array of tree's nodes
    int* memMap;        // memory map of the array, links free chunks of nodes array
    int nodesMax;       // capacity of the nodes array
    int nodesCur;       // current load of the nodes array
    int root;           // address of the root
    int free;           // address of the first free element
    enum TreeErrs err;  // error code of the tree

    #ifdef SEC_ON
    long hash;          // hash sum of the tree
    #endif

    #ifdef LOG_ON
    int logFd;          // file descriptor of the log file
    #endif
};
typedef struct _Tree Tree;

//####################//

Node* NodeAlloc();
int NodeInit(Node* node, const int parent, const data value);
void NodeFree(Node* node);

Tree* TreeAlloc();
int TreeInit(Tree* tree, const char* name, const int rank);
void TreeFree(Tree* tree);
int TreeResize(Tree* tree, const int sizeNew);
int TreeSort(Tree* tree);
int _TreeSort(Tree* tree, const int node, const int parent, const int branch, int* counter, Node* buf);

int TreeGetRoot(Tree* tree);
int TreeGetFree(Tree* tree);
int TreeFind(Tree* tree, const int node, data value);

int TreeAddNode(Tree* tree, const int node, const int branch, const data value);
int TreeDeleteNode(Tree* tree, const int node);
int _TreeDeleteNode(Tree* tree, const int node);
int TreeChangeNode(Tree* tree, const int node, const data valueNew);

char* GetTimestamp();

#ifdef SEC_ON
int TreeVerify(Tree* tree);
void TreeDump(Tree* tree, const char* name);
int TreeGetHash(Tree* tree);
#endif

#ifdef LOG_ON
int TreeUpdLog(Tree* tree, const char* func);
#endif 

//####################//

#endif