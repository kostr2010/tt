#ifndef TREE_H_INCLUDED
#define TREE_H_INCLUDED

#define SEC_ON
#define LOG_ON

    // TreeInt //
#define TREE_INT_INIT(tree) TreeIntInit(tree, #tree)

#ifdef SEC_ON
#define TREE_INT_VERIFY(tree)   if (TreeIntVerify(tree) != OK) {\
                                TreeIntDump(tree, #tree);\
                                TreeIntFree(tree);\
                                printf("executed with errors! see dump file for details\n");\
                                exit(-1);\
                            }
#else
#define TREE_INT_VERIFY(tree)   if (TreeIntVerify(tree) != OK) {\
                                TreeIntFree(tree);\
                                printf("executed with errors! see dump file for details\n");\
                                exit(-1);\
                            }
#endif

    // TreeTxt //
#define TREE_TXT_INIT(tree) TreeTxtInit(tree, #tree)

#ifdef SEC_ON
#define TREE_TXT_VERIFY(tree)   if (TreeTxtVerify(tree) != OK) {\
                                TreeTxtDump(tree, #tree);\
                                TreeTxtFree(tree);\
                                printf("executed with errors! see dump file for details\n");\
                                exit(-1);\
                            }
#else
#define TREE_TXT_VERIFY(tree)   if (TreeTxtVerify(tree) != OK) {\
                                TreeTxtFree(tree);\
                                printf("executed with errors! see dump file for details\n");\
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
    left = 0,
    right = 1,
};

struct _NodeInt {
    int parent;
    int branch[2];
    int data;
};
typedef struct _NodeInt NodeInt;

struct _NodeTxt {
    int parent;
    int branch[2];
    char data[40];
};
typedef struct _NodeTxt NodeTxt;

struct _TreeInt {
    NodeInt* nodes;
    int* memmap;
    int max;
    int cur;
    int root;
    int free;

    int logfd;
    enum TreeErrs err;
    long hash;
};
typedef struct _TreeInt TreeInt;

struct _TreeTxt {
    NodeTxt* nodes;
    int* memmap;
    int max;
    int cur;
    int root;
    int free;

    int logfd;
    enum TreeErrs err;
    long hash;
};
typedef struct _TreeTxt TreeTxt;

//####################//

    // common //

int TreeCountSubtree(TreeInt* tree, const int node);
int _TreeCountSubtree(TreeInt* tree, const int node, int* counter);
int GetNum(char** s);
int GetStr(char** s);
int GetSpace(char** s);

    // TreeInt //
NodeInt* NodeIntAlloc();
int NodeIntInit(NodeInt* node, const int parent, const int branchL, const int branchR, const int data);
void NodeIntFree(NodeInt* node);

TreeInt* TreeIntAlloc();
int TreeIntInit(TreeInt* tree, const char* name);
void TreeIntFree(TreeInt* tree);
int TreeIntResize(TreeInt* tree, const int sizeNew);
int TreeIntSort(TreeInt* tree);
int _TreeIntSort(TreeInt* tree, const int node, const int parent, const int branch, int* counter, NodeInt* buf);

int TreeIntGetRoot(TreeInt* tree);
int TreeIntGetFree(TreeInt* tree);
int TreeIntGetCur(TreeInt* tree);
int TreeIntGetMax(TreeInt* tree);
int TreeIntFind(TreeInt* tree, const int node, int data);

int TreeIntInsertNode(TreeInt* tree, const int parent, const int branch, const int data);
int TreeIntCountSubtree(TreeInt* tree, const int node);
int _TreeIntCountSubtree(TreeInt* tree, const int node, int* counter);
int TreeIntCopySubtree(TreeInt* src, NodeInt* dst, const int node);
int _TreeIntCopySubtree(TreeInt* src, NodeInt* dst, const int node, int* pos);
int TreeIntDeleteNode(TreeInt* tree, const int node);
int _TreeIntDeleteNode(TreeInt* tree, const int node);
int TreeIntChangeNode(TreeInt* tree, const int node, int* parentNew, int* branchLNew, int* branchRNew, int* dataNew);

TreeInt* TreeIntRead(const char* pathname);
int _TreeIntRead(TreeInt* tree, char** s);
int TreeIntGetGr(char** s, TreeInt* tree, const int parent, const int branch);
int TreeIntGetNode(char** s, TreeInt* tree, const int parent, const int branch);
int TreeIntWrite(TreeInt* tree, const char* pathname);
int _TreeIntWrite(TreeInt* tree, int node, int fd);

char* GetTimestamp();

int TreeIntVerify(TreeInt* tree);
void TreeIntDump(TreeInt* tree, const char* name);
int TreeIntGetHash(TreeInt* tree);

int TreeIntUpdLog(TreeInt* tree, const char* func);

    // TreeTxt //
NodeTxt* NodeTxtAlloc();
int NodeTxtInit(NodeTxt* node, const int parent, const int branchL, const int branchR, const char* data);
void NodeTxtFree(NodeTxt* node);

TreeTxt* TreeTxtAlloc();
int TreeTxtInit(TreeTxt* tree, const char* name);
void TreeTxtFree(TreeTxt* tree);
int TreeTxtResize(TreeTxt* tree, const int sizeNew);
int TreeTxtSort(TreeTxt* tree);
int _TreeTxtSort(TreeTxt* tree, const int node, const int parent, const int branch, int* counter, NodeTxt* buf);

int TreeTxtGetRoot(TreeTxt* tree);
int TreeTxtGetFree(TreeTxt* tree);
int TreeTxtGetCur(TreeTxt* tree);
int TreeTxtGetMax(TreeTxt* tree);
int TreeTxtFind(TreeTxt* tree, const int node, const char* data);

int TreeTxtInsertNode(TreeTxt* tree, const int parent, const int branch, const char* data);
int TreeTxtCountSubtree(TreeTxt* tree, const int node);
int _TreeTxtCountSubtree(TreeTxt* tree, const int node, int* counter);
int TreeTxtCopySubtree(TreeTxt* src, NodeTxt* dst, const int node);
int _TreeTxtCopySubtree(TreeTxt* src, NodeTxt* dst, const int node, int* pos);
int TreeTxtDeleteNode(TreeTxt* tree, const int node);
int _TreeTxtDeleteNode(TreeTxt* tree, const int node);
int TreeTxtChangeNode(TreeTxt* tree, const int node, int* parentNew, int* branchLNew, int* branchRNew, char* dataNew);

TreeTxt* TreeTxtRead(const char* pathname);
int _TreeTxtRead(TreeTxt* tree, char** s);
int TreeTxtGetGr(char** s, TreeTxt* tree, const int parent, const int branch);
int TreeTxtGetNode(char** s, TreeTxt* tree, const int parent, const int branch);
int TreeTxtWrite(TreeTxt* tree, const char* pathname);
int _TreeTxtWrite(TreeTxt* tree, int node, int fd);

int TreeTxtVerify(TreeTxt* tree);
void TreeTxtDump(TreeTxt* tree, const char* name);
int TreeTxtGetHash(TreeTxt* tree);

int TreeTxtUpdLog(TreeTxt* tree, const char* func);

//####################//

#endif