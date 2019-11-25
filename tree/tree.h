#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

//####################//

#define SEC_ON
#define LOG_ON

#define TREE_INIT(tree, rank)    TreeInit(tree, #tree, rank)

#define TREE_VERIFY(tree)   if (TreeVerify(tree) != OK) {\
                                TreeDump(tree);\
                                printf("executed with errors! see dump file for details\n");\
                                exit(-1);\
                            }

//####################//

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
    int* children;      // node's branches, enumerated from left to right
    int parent;         // node parent's physical address
};
typedef struct _Node Node;

// tree structure
struct _Tree {
    int rank;           // number of children per node
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
int NodeInit(Node* node, int parent, data value, int rank);
void NodeFree(Node* node);

Tree* TreeAlloc();
int TreeInit(Tree* tree, const char* name, int rank);
void TreeFree(Tree* tree);
int TreeResize(Tree* tree, int sizeNew);
void TreeSort(Tree* tree);

int TreeGetRoot(Tree* tree);
int TreeFind(Tree* tree, data value);

int TreeAddNode(Tree* tree, int addr, int child, data value);
int TreeDelete(Tree* tree, int addr);

#ifdef SEC_ON
int TreeVerify(Tree* tree);
void TreeDump(Tree* tree);
int TreeGetHash(Tree* tree);
#endif

#ifdef LOG_ON
void TreeUpdLog(Tree* tree);
#endif 

//####################//

#endif