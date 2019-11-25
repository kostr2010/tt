#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "tree.h"

//####################//

const int TREE_INIT_SIZE = 10;
const int DELTA = 10;

const char* TreeErrsDesc[] = {
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

int NodeInit(Node* node, const int parent, const data value, const  int rank) {
    assert(node);

    if (parent < 0)
        return -1;

    node->parent = parent;

    node->value = value;

    for (int i = 0; i < 3; i++)
        node->children[i] = 0;
    
    return 0;
}

void NodeFree(Node* node) {
    if (node == NULL)
        return;

    free(node);
}


Tree* TreeAlloc() {
    Tree* tree = calloc(1, sizeof(Tree));

    return tree;
}

int TreeInit(Tree* tree, const char* name, const int rank) {
    assert(tree);

    tree->rank = rank;  
    tree->nodes = calloc(TREE_INIT_SIZE, sizeof(Node));
    
    tree->memMap = calloc(TREE_INIT_SIZE, sizeof(int));
    for (int i = 1; i < TREE_INIT_SIZE - 1; i++)
        tree->memMap[i] = i + 1;
    tree->memMap[TREE_INIT_SIZE - 1] = 0;

    tree->nodesMax = TREE_INIT_SIZE;
    tree->nodesCur = 0;
    tree->root = 0;
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

    #ifdef LOG_ON
    if (TreeUpdLog(tree, "TreeInit") != 0)
        tree->err = E_LOGFILE_DEAD;
    #endif

    TREE_VERIFY(tree);

    return 0;
}

void TreeFree(Tree* tree) {
    if (tree == NULL)
        return;

    if (tree->nodes != NULL)
        free(tree->nodes);

    if (tree->memMap != NULL)
        free(tree->memMap);

    close(tree->logFd);

    free(tree);
}

int TreeResize(Tree* tree, const int sizeNew) {
    TREE_VERIFY(tree);
    
    if (sizeNew < DELTA || sizeNew < tree->nodesCur) {
        printf("invalid new size %d -> %d\n", tree->nodesMax, sizeNew);
        
        #ifdef LOG_ON
        if (TreeUpdLog(tree, "TreeResize (inv size)") != 0)
            tree->err = E_LOGFILE_DEAD;
        #endif

        return -1;
    }
    if (sizeNew == tree->nodesMax) {
        printf("same size %d -> %d\n", tree->nodesMax, sizeNew);
        
        #ifdef LOG_ON
        if (TreeUpdLog(tree, "TreeResize (same size)") != 0)
            tree->err = E_LOGFILE_DEAD;
        #endif

        return 0;
    } else if (sizeNew < tree->nodesMax) {
        printf("shrink %d -> %d\n", tree->nodesMax, sizeNew);

        TreeSort(tree);

        tree->nodesMax = sizeNew;
        tree->nodes = realloc(tree->nodes, sizeNew * sizeof(Node));
        tree->memMap = realloc(tree->memMap, sizeNew * sizeof(Node));        

        #ifdef LOG_ON
        if (TreeUpdLog(tree, "TreeResize (shrink)") != 0)
            tree->err = E_LOGFILE_DEAD;
        #endif
    } else if (sizeNew > tree->nodesMax) {
        printf("extend %d -> %d\n", tree->nodesMax, sizeNew);

        tree->memMap = realloc(tree->memMap, sizeNew * sizeof(int));
        for (int i = tree->nodesMax - 1; i < sizeNew - 1; i++)
            tree->memMap[i] = i + 1;
        tree->memMap[sizeNew - 1] = 0;

        tree->nodes = realloc(tree->nodes, sizeNew * sizeof(Node));

        tree->nodesMax = sizeNew;

        #ifdef LOG_ON
        if (TreeUpdLog(tree, "TreeResize (extend)") != 0)
            tree->err = E_LOGFILE_DEAD;
        #endif
    }

    TREE_VERIFY(tree);

    return 0;
}

int TreeSort(Tree* tree) {
    TREE_VERIFY(tree);

    int counter = 0;

    Node* nodesNew = calloc(tree->nodesMax, sizeof(Node));

    int res = _TreeSort(tree, tree->root, 0, 0, &counter, nodesNew);

    for (int i = 1; i < tree->nodesMax; i++)
        tree->memMap[i] = i + 1;
    free(tree->nodes);
    tree->memMap[tree->nodesMax - 1] = 0;

    tree->nodes = nodesNew;
    tree->free = tree->nodesCur + 1;

    #ifdef LOG_ON
    if (TreeUpdLog(tree, "TreeSort") != 0)
        tree->err = E_LOGFILE_DEAD;
    #endif
    
    TREE_VERIFY(tree);

    return res;
}

int _TreeSort(Tree* tree, const int node, const int parent, const int branch, int* counter, Node* buf) {
    if (node == 0)
        return 0;
    
    (*counter) += 1;
    if (*counter > tree->nodesMax)
        return -1;

    int this = *counter;

    buf[this] = tree->nodes[node];
    buf[this].parent = parent;
    if (parent != 0)
        buf[parent].children[branch] = this;

    for (int i = 0; i < tree->rank; i++)
        if (_TreeSort(tree, (tree->nodes[node]).children[i], this, i, counter, buf) != 0)
            return -1;
    
    return *counter;
}


int TreeGetRoot(Tree* tree) {
    TREE_VERIFY(tree);

    return tree->root;
}

int TreeGetFree(Tree* tree) {
    TREE_VERIFY(tree);

    return tree->free;
}

int TreeFind(Tree* tree, const data value) {}


int TreeAddNode(Tree* tree, const int addr, const int branch, const data value) {}

int TreeDelete(Tree* tree, const int addr) {}


char* GetTimestamp() {
    time_t ltime;
	struct tm result;
	char date[11];
    char hrs[9];

	ltime = time(NULL);
	localtime_r(&ltime, &result);
    strftime(date, 11, "%F", &result);
    strftime(hrs, 9, "%X", &result);
    char* logName = calloc(22, sizeof(char));
    sprintf(logName, "[%s|%s]", date, hrs);

    return logName;
}


#ifdef SEC_ON
int TreeVerify(Tree* tree) {
    if (tree == NULL)
        return -1;
    
    if (tree->err != OK)
        return tree->err;
}

void TreeDump(Tree* tree, const char* name) {
    char* dumpName = calloc(1 + strlen(name) + strlen("dump/.log"), sizeof(char));
    sprintf(dumpName, "dump/%s.log", name);
    int dumpFd = open(dumpName, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
    free(dumpName);

    if (tree == NULL) {
        dprintf(dumpFd, "ERROR: NULL pointer to the structure!\n");
    } else {
        char* timeStamp = GetTimestamp();

        int res = 0;

        res = dprintf(dumpFd, "%s\n"
                                   "  hash: %ld\n"
                                   "  tree capacity: %d\n"
                                   "  tree size: %d\n"
                                   "  first free element's physical address: %d\n"
                                   "  root address: %d\n"
                                   "  rank: %d\n"
                                   "  error: %d %s\n",
                                   timeStamp, tree->hash, tree->nodesMax, tree->nodesCur, tree->free, tree->root, tree->rank, tree->err, TreeErrsDesc[tree->err]);
        free(timeStamp);

        if (res == 0)
            return -1;
        if (tree->nodes != NULL) {
            for (int i = 0; i < tree->nodesMax; i++) {
                res = dprintf(dumpFd, "  [%3d] (%3d [%3d, %3d, %3d] <%3d>)\n", i, tree->nodes[i].value, tree->nodes[i].children[0], tree->nodes[i].children[1], tree->nodes[i].children[2], tree->nodes[i].parent);
                if (res == 0)
                    return -1;
            }
        }
    }

    close(dumpFd);
}

int TreeGetHash(Tree* tree) {
    return 0;
}
#endif


#ifdef LOG_ON
int TreeUpdLog(Tree* tree, const char* func) {
    TREE_VERIFY(tree);
    char* timeStamp = GetTimestamp();

    int res = 0;

    res = dprintf(tree->logFd, "%s\n"
                               "passed through function [[%s]]\n"
                               "  hash: %ld\n"
                               "  tree capacity: %d\n"
                               "  tree size: %d\n"
                               "  first free element's physical address: %d\n"
                               "  root address: %d\n"
                               "  rank: %d\n"
                               "  error: %d %s\n",
                               timeStamp, func, tree->hash, tree->nodesMax, tree->nodesCur, tree->free, tree->root, tree->rank, tree->err, TreeErrsDesc[tree->err]);
    free(timeStamp);

    if (res == 0)
        return -1;

    for (int i = 0; i < tree->nodesMax; i++) {
        res = dprintf(tree->logFd, "  [%3d] (%3d [%3d, %3d, %3d] <%3d>)\n", i, tree->nodes[i].value, tree->nodes[i].children[0], tree->nodes[i].children[1], tree->nodes[i].children[2], tree->nodes[i].parent);
        if (res == 0)
            return -1;
    }

    TREE_VERIFY(tree);
    return 0;
}
#endif 

//####################//



//####################//

//####################//

//####################//