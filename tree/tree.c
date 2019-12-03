#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

//####################//

#include "tree.h"

//####################//

const char* TreeErrsDesc[] = {
    "tree is ok\n",
    "hash is corrupted!\n",
    "initialization error!\n",
    "reallocation error!\n",
    "logical sequence corrupted!\n",
    "invalide tree size!\n",
    "logfile not created or unexpectedly closed!\n",
};

const int TREE_INIT_SZ = 20;
const int DELTA = 20;

//####################//

Node* NodeAlloc() {
    Node* node = calloc(1, sizeof(Node));

    return node;
}

int NodeInit(Node* node, const int parent, const int branchL, const int branchR, const data data) {
    if (node == NULL) {
        printf("[NodeInit] nullptr given!\n");
        return -1;
    } else {
        node->parent = parent;
        node->branch[left] = branchL;
        node->branch[right] = branchR;
        node->data = data;
    }

    return 0;
}

void NodeFree(Node* node) {
    if (node == NULL)
        return;
    else
        free(node);
    
    return;
}


Tree* TreeAlloc() {
    Tree* tree = calloc(1, sizeof(Tree));
    tree->nodes = calloc(TREE_INIT_SZ, sizeof(Node));
    tree->memmap = calloc(TREE_INIT_SZ, sizeof(int));

    return tree;
} 

int TreeInit(Tree* tree, const char* name) {
    if (tree == NULL) {
        printf("[TreeInit] nullptr was given!\n");
        return -1;
    } else if (tree->nodes == NULL) {
        printf("[TreeInit] tree->nodes is nullptr!\n");
        tree->err = E_INIT_ERR;
        return -1;
    } else if (tree->memmap == NULL) {
        tree->err = E_INIT_ERR;
        printf("[TreeInit] tree->memmap is nullptr!\n");
        return -1;
    } else {
        memset(tree->nodes, '\0', TREE_INIT_SZ);

        for (int i = 1; i < TREE_INIT_SZ - 1; i++)
            tree->memmap[i] = i + 1;
        tree->memmap[TREE_INIT_SZ - 1] = 0;

        tree->max = TREE_INIT_SZ;
        tree->cur = 0;
        tree->root = 0;
        tree->free = 1;

        char* logName = GetTimestamp();
        logName = realloc(logName, strlen(logName) + strlen(name) + strlen("log/-.log"));
        char* logName2 = calloc(30, 1);
        memcpy(logName2, logName, strlen(logName));
        sprintf(logName, "log/%s-%s.log", name, logName2);    
        tree->logfd = open(logName, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR| S_IWUSR);
        free(logName);

        tree->err = OK;
        tree->hash = TreeGetHash(tree);
    }

    if (tree->logfd == -1) {
        tree->err = E_LOG_DEAD;
        printf("[TreeInit] unable to open logfile at initialization point!\n");
        return -1;
    }

    if (TreeUpdLog(tree, "TreeInit") != 0)
        tree->err = E_LOG_DEAD;

    REE_VERIFY(tree);
    
    return 0;
}

void TreeFree(Tree* tree) {
    if (tree == NULL)
        return;
    
    if (tree->memmap != NULL)
        free(tree->memmap);

    if (tree->nodes != NULL)
        free(tree->nodes);

    if (tree->logfd != -1)
        close(tree->logfd);

    free(tree);

    return;
}

int TreeResize(Tree* tree, const int sizeNew) {
    if (TreeSort(tree) == -1) {
        printf("[TreeResize] error while sorting tree!\n");
        return -1;
    }

    if (sizeNew < DELTA || sizeNew < tree->cur) {
        printf("[TreeResize] invalid new size %d -> %d\n", tree->max, sizeNew);
        
        if (TreeUpdLog(tree, "TreeResize (inv size)") != 0)
            tree->err = E_LOG_DEAD;

        return -1;

    } else if (sizeNew == tree->max) {
        printf("[TreeResize] same size %d -> %d\n", tree->max, sizeNew);

        if (TreeUpdLog(tree, "TreeResize (same size)") != 0)
            tree->err = E_LOG_DEAD;

    } else if (sizeNew < tree->max) {
        printf("[TreeResize] shrink %d -> %d\n", tree->max, sizeNew);

        tree->max = sizeNew;
        tree->nodes = realloc(tree->nodes, sizeNew * sizeof(Node));
        tree->memmap = realloc(tree->memmap, sizeNew * sizeof(Node));        

        if (TreeUpdLog(tree, "TreeResize (shrink)") != 0)
            tree->err = E_LOG_DEAD;

    } else if (sizeNew > tree->max) {
        printf("[TreeResize] extend %d -> %d\n", tree->max, sizeNew);

        tree->nodes = realloc(tree->nodes, sizeNew * sizeof(Node));
        memset(tree->nodes + tree->max * sizeof(Node), '\0', (sizeNew - tree->max) * sizeof(Node));

        tree->memmap = realloc(tree->memmap, sizeNew * sizeof(int));
        for (int i = tree->max - 1; i < sizeNew - 1; i++)
            tree->memmap[i] = i + 1;
        tree->memmap[sizeNew - 1] = 0;

        if (tree->free == 0)
            tree->free = tree->max;
        
        tree->max = sizeNew;

        if (TreeUpdLog(tree, "TreeResize (extend)") != 0)
            tree->err = E_LOG_DEAD;
    }

    REE_VERIFY(tree);

    return 0;
}

int TreeSort(Tree* tree) {
    int counter = 0;

    Node* nodesNew = calloc(tree->max, sizeof(Node));

    if (_TreeSort(tree, tree->root, 0, 0, &counter, nodesNew) == -1)
        return -1;

    for (int i = 1; i < tree->max - 1; i++)
        tree->memmap[i] = i + 1;
    tree->memmap[tree->max - 1] = 0;

    free(tree->nodes);
    tree->nodes = nodesNew;
    tree->free = tree->cur + 1;

    if (TreeUpdLog(tree, "TreeSort") != 0)
        tree->err = E_LOG_DEAD;

    REE_VERIFY(tree);

    return 0;
}

int _TreeSort(Tree* tree, const int node, const int parent, const int branch, int* counter, Node* buf) {
    if (node == 0)
        return 0;
    
    (*counter) += 1;
    if (*counter > tree->max) {
        printf("[_TreeSort] index out of range!\n");
        return -1;
    }

    int this = *counter;
    int branchL = (tree->nodes[node]).branch[left];
    int branchR = (tree->nodes[node]).branch[right];

    buf[this] = tree->nodes[node];
    buf[this].parent = parent;
    if (parent != 0)
        buf[parent].branch[branch] = this;

    if (branchL != 0)
        if (_TreeSort(tree, branchL, this, left, counter, buf) == -1)
            return -1;

    if (branchR != 0)
        if (_TreeSort(tree, branchR, this, right, counter, buf) == -1)
            return -1;
    
    return *counter;
}


int TreeGetRoot(Tree* tree) {
    assert(tree);

    return tree->root;
}

int TreeGetFree(Tree* tree) {
    assert(tree);

    return tree->free;
}

int TreeGetCur(Tree* tree) {
    assert(tree);

    return tree->cur;
}

int TreeGetMax(Tree* tree) {
    assert(tree);

    return tree->max;
}

int TreeFind(Tree* tree, const int node, data data) {
    if (tree->cur == 0) {
        printf("[TreeFind] tree is empty!\n");
        return -1;
    }

    if (node == 0)
        return -1;
    
    if (tree->nodes[node].data == data)
        return node;
    
    int res = -1;

    res = TreeFind(tree, (tree->nodes[node]).branch[left], data);
    if (res != -1)
        return res;

    res = TreeFind(tree, (tree->nodes[node]).branch[right], data);
    if (res != -1)
        return res;
    
    return -1;
}

int TreeCountSubtree(Tree* tree, const int node) {
    int counter = 0;

    if (_TreeCountSubtree(tree, node, &counter) == -1)
        return -1;

    return counter;
}

int _TreeCountSubtree(Tree* tree, const int node, int* counter) {
    if (*counter > tree->cur) {
        printf("[TreeCountSubtree] counter out of range!\n");
        tree->err = E_SEQ_CORRUPTED;
        return -1;
    }

    int branchL = (tree->nodes[node]).branch[left];
    int branchR = (tree->nodes[node]).branch[right];

    if (branchL != 0)
        if (_TreeCountSubtree(tree, branchL, counter) == -1)
            return -1;

    if (branchR != 0)
        if (_TreeCountSubtree(tree, branchR, counter) == -1)
            return -1;

    *counter++;

    return 0;
}


int TreeInsertNode(Tree* tree, const int parent, const int branch, const data data) {
    int addrIns = tree->free;

    if (tree->cur == 0) {
        tree->cur++;
        (tree->nodes[addrIns]).data = data;
        (tree->nodes[addrIns]).parent = 0;
        (tree->nodes[addrIns]).branch[left] = 0;
        (tree->nodes[addrIns]).branch[right] = 0;
        tree->root = addrIns;
        tree->free = tree->memmap[addrIns];

        if (TreeUpdLog(tree, "TreeInsertNode (root added)") != 0)
        tree->err = E_LOG_DEAD;
    } else if ((tree->nodes[parent]).branch[branch] != 0) {
        printf("[TreeInsertNode] branch %d of node %d is already occupied!\n", branch, parent);
        return -1;
    } else if ((tree->nodes[parent]).parent == 0 && parent != tree->root) {
        printf("[TreeInsertNode] trying to insert in unlinked chunk!\n");
        return -1;
    } else {
        if (tree->cur >= tree->max - 2) {
            if (TreeResize(tree, tree->max * 2) == -1)
                return -1;
        } else {
            (tree->nodes[parent]).branch[branch] = tree->free;
            (tree->nodes[addrIns]).parent = parent;
            (tree->nodes[addrIns]).data = data;
            (tree->nodes[addrIns]).branch[left] = 0;
            (tree->nodes[addrIns]).branch[right] = 0;

            tree->free = tree->memmap[addrIns];
            tree->cur++;

            if (TreeUpdLog(tree, "TreeInsertNode (inserted)") != 0)
            tree->err = E_LOG_DEAD;
        }
    }

    TREE_VERIFY(tree);

    return addrIns;
}    

// returns subtree size, else - -1;
int TreeCopySubtree(Tree* src, Node* dst, const int node) {
    int subtreeSz = TreeCountSubtree(src, node);
    if (subtreeSz == -1) {
        printf("[TreeCopySubtree] can't count elements to copy subtree!\n");
        return -1;
    }

    dst = calloc(subtreeSz, sizeof(Node));
    int pos = 0;

    if (_TreeCopySubtree(src, dst, node, &pos) == -1)
        return -1;

    if (TreeUpdLog(src, "TreeCopySubTree") != 0)
        src->err = E_LOG_DEAD;

    return subtreeSz;
}

int _TreeCopySubtree(Tree* src, Node* dst, const int node, int* pos) {
    int branchL = (src->nodes[node]).branch[left];
    int branchR = (src->nodes[node]).branch[right];

    dst[*pos] = src->nodes[node];
    *pos++;

    if (branchL != 0)
        if (_TreeCopySubtree(src, dst, branchL, pos) == -1)
            return -1;

    if (branchR != 0)
        if (_TreeCopySubtree(src, dst, branchR, pos) == -1)
            return -1; 

    return 0;
}

int TreeDeleteNode(Tree* tree, const int node) {
    if (_TreeDeleteNode(tree, node) == -1) {
        printf("[TreeDeleteNode] can't delete node and it's subtree!\n");
        return -1;
    }

    if (TreeUpdLog(tree, "TreeDeleteNode") != 0)
        tree->err = E_LOG_DEAD;

    TREE_VERIFY(tree);
    
    return 0;
}

int _TreeDeleteNode(Tree* tree, const int node) {
    if (tree->cur == 0) {
        printf("[TreeDeleteNode] tree is already empty!\n");
        return -1;
    } else if (tree->cur < 0) {
        printf("[TreeDeleteNode] tree->cur went negative while deleting!\n");
        return -1;
    } else if (tree->cur < tree->max - DELTA && tree->max / 2 >= DELTA) {
        if (TreeResize(tree, tree->max / 2) == -1) {
            printf("[TreeDeleteNode] error while shrinking during delete!\n");
            return -1;
        }
    } else {
        int parent = (tree->nodes[node]).parent;
        int branchL = (tree->nodes[node]).branch[left];
        int branchR = (tree->nodes[node]).branch[right];

        if (branchL != 0)
            if (_TreeDeleteNode(tree, branchL) == -1)
                return -1;

        if (branchR != 0)
            if (_TreeDeleteNode(tree, branchR) == -1)
                return -1;
        
        if ((tree->nodes[node]).parent != 0 || node == tree->root)
            tree->cur--;

        if ((tree->nodes[parent]).branch[left] == node)
            (tree->nodes[parent]).branch[left] = 0;
        else if ((tree->nodes[parent]).branch[right] == node)
            (tree->nodes[parent]).branch[right] = 0;
        
        (tree->nodes[node]).parent = 0;

        tree->memmap[node] = tree->free;
        tree->free = node;
    }
    
    return 0;
}

int TreeChangeNode(Tree* tree, const int node, int* parentNew, int* branchLNew, int* branchRNew, data* dataNew) {
    int branchL = (tree->nodes[node]).branch[left];
    int branchR = (tree->nodes[node]).branch[right];

    if (parentNew != NULL) 
        (tree->nodes[node]).parent = *parentNew;

    if (branchLNew != NULL) {
        (tree->nodes[branchL]).parent = 0;
        branchL = *branchLNew;
    }

    if (branchRNew != NULL) {
        (tree->nodes[branchR]).parent = 0;
        branchR = *branchRNew;
    }

    if (dataNew != NULL)
        (tree->nodes[node]).data = *dataNew;

    if (TreeUpdLog(tree, "TreeChangeNode") != 0)
        tree->err = E_LOG_DEAD;

    TREE_VERIFY(tree);

    return 0;
}


Tree* TreeRead(const char* pathname) {
    Tree* tree = TreeAlloc();

    if (TREE_INIT(tree) == -1) {
        printf("[TreeRead] initialzation error! returning nullptr...\n");
        return NULL;
    }


}

int TreeWrite(Tree* tree, const char* pathname) {
    return 0;
}


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


int TreeVerify(Tree* tree) {
    return 0;
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

        dprintf(dumpFd, "%s\n"
                        "  hash: %ld\n"
                        "  tree capacity: %d\n"
                        "  tree size: %d\n"
                        "  first free element's physical address: %d\n"
                        "  root address: %d\n"
                        "  error: %d %s\n",
                        timeStamp, tree->hash, tree->max, tree->cur, tree->free, tree->root, tree->err, TreeErrsDesc[tree->err]);
        free(timeStamp);

        if (tree->nodes != NULL)
            for (int i = 0; i < tree->max; i++)
                dprintf(dumpFd, "  [%3d] (%3d [%3d, %3d] <%3d>)\n", i, tree->nodes[i].data, tree->nodes[i].branch[0], tree->nodes[i].branch[1], tree->nodes[i].parent);
    }

    close(dumpFd);
}

int TreeGetHash(Tree* tree) {
    return 0;
}


int TreeUpdLog(Tree* tree, const char* func) {
    char* timeStamp = GetTimestamp();

    int res = 0;

    res = dprintf(tree->logfd, "%s\n"
                               "passed through function [[%s]]\n"
                               "  hash: %ld\n"
                               "  tree capacity: %d\n"
                               "  tree size: %d\n"
                               "  first free element's physical address: %d\n"
                               "  root address: %d\n"
                               "  error: %d %s\n",
                               timeStamp, func, tree->hash, tree->max, tree->cur, tree->free, tree->root, tree->err, TreeErrsDesc[tree->err]);
    free(timeStamp);

    if (res == 0)
        return -1;

    for (int i = 0; i < tree->max; i++) {
        if (((tree->nodes[i]).parent == 0 && i != tree->root) || tree->cur == 0)
            res = dprintf(tree->logfd, "  [%3d ] (%3d [%3d, %3d] <%3d>)\n", i, tree->nodes[i].data, tree->nodes[i].branch[0], tree->nodes[i].branch[1], tree->nodes[i].parent);
        else 
            res = dprintf(tree->logfd, "  [%3d*] (%3d [%3d, %3d] <%3d>)\n", i, tree->nodes[i].data, tree->nodes[i].branch[0], tree->nodes[i].branch[1], tree->nodes[i].parent);
        if (res == 0)
            return -1;
    }

    res = dprintf(tree->logfd, "\n  memmmap:\n  ");
    if (res == 0)
        return -1;

    for (int i = 0; i < tree->max; i++) {
        res = dprintf(tree->logfd, "[%3d]", i);
        if (res == 0)
            return -1;
    }

    res = dprintf(tree->logfd, "\n  ");
    if (res == 0)
        return -1;
    
    for (int i = 0; i < tree->max; i++) {
        res = dprintf(tree->logfd, "[%3d]", tree->memmap[i]);
        if (res == 0)
            return -1;
    }

    res = dprintf(tree->logfd, "\n\n");
    if (res == 0)
        return -1;

    return 0;
}