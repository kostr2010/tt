#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "tree.h"

//####################//

const int TREE_INIT_SIZE = 10;

const char TreeErrsDesc[] = {
    "tree is ok\n",
    "error occured during initialization. couldn't allocate memory for structure!\n",
    "overflow occured!\n", 
    "undeflow occured!\n",
    "logical sequence of nodes is corrupted!\n",
    "invalid stack size!\n",
    "hash was unexpectedly modified!\n",
    "log file was not opened or was closed unexpectedly!\n",
};

//####################//

Node* NodeAlloc() {
    Node* node = calloc(1, sizeof(Node));

    return node;
}

int NodeInit(Node* node, int parent, data value, int rank) {
    if (parent < 0 || node == NULL)
        return -1;

    node->parent = parent;

    node->value = value;

    node->children = calloc(rank, sizeof(int));
    if (node->children = NULL)
        return -1;

    for (int i = 0; i < rank; i++)
        node->children[i] = 0;
    
    return 0;
}

void NodeFree(Node* node) {
    if (node == NULL)
        return;

    if (node->children != NULL)
        free(node->children);

    free(node);
}


Tree* TreeAlloc() {
    Tree* tree = calloc(1, sizeof(Tree));

    return tree;
}

int TreeInit(Tree* tree, const char* name, int rank) {
    assert(tree);

    tree->rank = rank;  
    tree->nodes = calloc(TREE_INIT_SIZE, sizeof(Node));
    
    tree->memMap = calloc(TREE_INIT_SIZE, sizeof(int));
    for (int i = 1; i < TREE_INIT_SIZE - 1; i++)
        tree->memMap[i] = i + 1;

    tree->nodesMax = TREE_INIT_SIZE;
    tree->nodesCur = 0;
    tree->root = -1;
    tree->free = 1;
    tree->err = OK;

    if (tree->nodes == NULL || tree->memMap == NULL)
        return E_INIT_ERR;

    #ifdef SEC_ON
    tree->hash = TreeGetHash(tree);
    #endif

    #ifdef LOG_ON
    char* logName = GetTimestamp();
    logName = realloc(logName, strlen(logName) + strlen(name) + strlen("log/-.log"));
    char* logName2 = calloc(30, 1);
    memcpy(logName2, logName, strlen(logName));
    sprintf(logName, "log/%s-%s.log", name, logName2);    
    tree->logFd = open(logName, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR| S_IWUSR);
    free(logName);
    
    if (tree->logFd == -1)
        return E_LOGFILE_DEAD;
    #endif

    return 0;
}

void TreeFree(Tree* tree) {
    if (tree == NULL)
        return;

    if (tree->nodes != NULL) {
        for (int i = 0; i < tree->nodesMax; i++)
            NodeFree(&(tree->nodes[i]));
        free(tree->nodes);
    }

    if (tree->memMap != NULL)
        free(tree->memMap);

    close(tree->logFd);

    free(tree);
}

int TreeResize(Tree* tree, int sizeNew) {
    
}

void TreeSort(Tree* tree) {
    
}


int TreeGetRoot(Tree* tree) {}

int TreeFind(Tree* tree, data value) {}


int TreeAddNode(Tree* tree, int addr, int child, data value) {}

int TreeDelete(Tree* tree, int addr) {}


#ifdef SEC_ON
int TreeVerify(Tree* tree) {}

void TreeDump(Tree* tree) {}

int TreeGetHash(Tree* tree) {}
#endif


#ifdef LOG_ON
void TreeUpdLog(Tree* tree) {}
#endif 

//####################//



//####################//

//####################//

//####################//