#ifndef TREE_H_INCLUDED
#define TREE_H_INCLUDED

#define SEC_ON
#define LOG_ON

//#undef SEC_ON
//#undef LOG_ON

#define TREE_INIT(tree, type) TreeInit(tree, #tree, type)

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
                                printf("executed with errors!\n");\
                                exit(-1);\
                            }
#endif

//####################//

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
    Left,
    Right,
};

enum NodeTypes {
    Int,
    Txt,
    Math,
};

struct _MathData {
    int type;
    int value;
};
typedef struct _MathData MathData;

struct _Header {
    enum NodeTypes type;
};
typedef struct _Header Header;

typedef MathData data;

struct _NodeInt {
    int parent;
    int branch[2];
    int data;
};
typedef struct _NodeInt NodeInt;

struct _NodeMath {
    int parent;
    int branch[2];
    MathData data;
};
typedef struct _NodeMath Node;

struct _NodeTxt {
    int parent;
    int branch[2];
    char data[40];
};
typedef struct _NodeTxt NodeTxt;

struct _TreeInt {
    Header header;
    struct _NodeInt* nodes;
    int* memmap;
    int max;
    int cur;
    int root;
    int free;

    #ifdef LOG_ON
    int logfd;
    #endif
    enum TreeErrs err;
    #ifdef SEC_ON
    long hash;
    #endif
};
typedef struct _TreeInt TreeInt;

struct _TreeTxt {
    Header header;
    struct _NodeTxt* nodes;
    int* memmap;
    int max;
    int cur;
    int root;
    int free;

    #ifdef LOG_ON
    int logfd;
    #endif
    enum TreeErrs err;
    #ifdef SEC_ON
    long hash;
    #endif
};
typedef struct _TreeTxt TreeTxt;

struct _TreeMath {
    Header header;
    struct _NodeMath* nodes;
    int* memmap;
    int max;
    int cur;
    int root;
    int free;

    #ifdef LOG_ON
    int logfd;
    #endif
    enum TreeErrs err;
    #ifdef SEC_ON
    long hash;
    #endif
};
typedef struct _TreeMath Tree;

//####################//

Node* NodeAlloc();
int NodeInit(Node* node, const int parent, const int branchL, const int branchR, const data data, int type);
    int NodeInitIntHandler(struct _NodeInt* node, const int* data);
    int NodeInitTxtHandler(struct _NodeTxt* node, const char** data);
    int NodeInitMathHandler(struct _NodeMath* node, const MathData* data);
void NodeFree(Node* node);

Tree* TreeAlloc();
int TreeInit(Tree* tree, const char* name, int type);
void TreeFree(Tree* tree);
int TreeResize(Tree* tree, const int sizeNew);
int TreeSort(Tree* tree);
int _TreeSort(Tree* tree, const int node, const int parent, const int branch, int* counter, Node* buf);

int TreeGetRoot(Tree* tree);
int TreeGetFree(Tree* tree);
int TreeGetCur(Tree* tree);
int TreeGetMax(Tree* tree);
int TreeFind(Tree* tree, const int node, const data data);
    int TreeFindTxtHandler(struct _TreeTxt* tree, const int node, const char** data);
    int TreeFindIntHandler(struct _TreeInt* tree, const int node, const int* data);
    int TreeFindMathHandler(struct _TreeMath* tree, const int node, const MathData* data);

int TreeInsertNode(Tree* tree, const int parent, const int branch, data* data);
    int TreeInsertNodeTxtHandler(struct _TreeTxt* tree, const int node, const char** data);
    int TreeInsertNodeIntHandler(struct _TreeInt* tree, const int node, const int* data);
    int TreeInsertNodeMathHandler(struct _TreeMath* tree, const int node, const MathData* data);
int TreeCountSubtree(Tree* tree, const int node);
int _TreeCountSubtree(Tree* tree, const int node, int* counter);
Node* TreeCopySubtree(Tree* src, const int node);
int _TreeCopySubtree(Tree* src, Node* dst, const int node, int* pos);
int TreeDeleteNode(Tree* tree, const int node);
int _TreeDeleteNode(Tree* tree, const int node);
int TreeChangeNode(Tree* tree, const int node, int* parentNew, int* branchLNew, int* branchRNew, data* dataNew);
    int TreeChangeNodeTxtHandler(struct _TreeTxt* tree, const int node, char** dataNew);
    int TreeChangeNodeIntHandler(struct _TreeInt* tree, const int node, int* dataNew);
    int TreeChangeNodeMathHandler(struct _TreeMath* tree, const int node, MathData* dataNew);
int TreeGlueSubtree(Tree* tree, Node* subtree, int node, int branch, int nodesCount);
    int _TreeGlueSubtree(Tree* tree, Node* subtree, int* index, int nodesCount, int node, int branch);
Tree* TreeRead(const char* pathname, int type);
int _TreeRead(Tree* tree, char** s);
int TreeGetGr(char** s, Tree* tree, const int parent, const int branch);
    int TreeTxtGetNode(char** s, struct _TreeTxt* tree, const int parent, const int branch);
    int TreeIntGetNode(char** s, struct _TreeInt* tree, const int parent, const int branch);
    int TreeMathGetNode(char** s, struct _TreeMath* tree, const int parent, const int branch);
int GetNumber(char** s);
int GetString(char** s);
int GetSpace(char** s);
int TreeWrite(Tree* tree, const char* pathname);
int _TreeWrite(Tree* tree, int node, int fd);
    void _TreeWriteTxtHandler(struct _TreeTxt* tree, const int node, const int fd);
    void _TreeWriteIntHandler(struct _TreeInt* tree, const int node, const int fd);
    void _TreeWriteMathHandler(struct _TreeMath* tree, const int node, const int fd);

int TreeVerify(Tree* tree);
#ifdef SEC_ON
void TreeDump(Tree* tree, const char* name);
    int TreeDumpTxtHandler(struct _TreeTxt* tree, const int fd);
    int TreeDumpIntHandler(struct _TreeInt* tree, const int fd);
    int TreeDumpMathHandler(struct _TreeMath* tree, const int dumpFd);
long TreeGetHash(Tree* tree);
#endif

#ifdef LOG_ON
int TreeUpdLog(Tree* tree, const char* func);
    int TreeUpdLogTxtHandler(struct _TreeTxt* tree, const int fd);
    int TreeUpdLogIntHandler(struct _TreeInt* tree, const int fd);
    int TreeUpdLogMathHandler(struct _TreeMath* tree, const int fd);
#endif

char* GetTimestamp();

//####################//

#endif