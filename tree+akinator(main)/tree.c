#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
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
const int TEXT_MAX_SZ = 40;

//####################//

///////////////////////
// TreeInt & NodeInt //
///////////////////////

NodeInt* NodeIntAlloc() {
    NodeInt* node = calloc(1, sizeof(NodeInt));

    return node;
}

int NodeIntInit(NodeInt* node, const int parent, const int branchL, const int branchR, const int data) {
    if (node == NULL) {
        printf("[NodeIntInit] nullptr given!\n");
        return -1;
    } else {
        node->parent = parent;
        node->branch[left] = branchL;
        node->branch[right] = branchR;
        node->data = data;
    }

    return 0;
}

void NodeIntFree(NodeInt* node) {
    if (node == NULL)
        return;
    else
        free(node);
    
    return;
}


TreeInt* TreeIntAlloc() {
    TreeInt* tree_i = calloc(1, sizeof(TreeInt));
    tree_i->nodes = calloc(TREE_INIT_SZ, sizeof(NodeInt));
    tree_i->memmap = calloc(TREE_INIT_SZ, sizeof(int));

    return tree_i;
} 

int TreeIntInit(TreeInt* tree_i, const char* name) {
    if (tree_i == NULL) {
        printf("[TreeIntInit] nullptr was given!\n");
        return -1;
    } else if (tree_i->nodes == NULL) {
        printf("[TreeIntInit] tree_i->nodes is nullptr!\n");
        tree_i->err = E_INIT_ERR;
        return -1;
    } else if (tree_i->memmap == NULL) {
        tree_i->err = E_INIT_ERR;
        printf("[TreeIntInit] tree_i->memmap is nullptr!\n");
        return -1;
    } else {
        memset(tree_i->nodes, '\0', TREE_INIT_SZ * sizeof(NodeInt));

        for (int i = 1; i < TREE_INIT_SZ - 1; i++)
            tree_i->memmap[i] = i + 1;
        tree_i->memmap[TREE_INIT_SZ - 1] = 0;

        tree_i->max = TREE_INIT_SZ;
        tree_i->cur = 0;
        tree_i->root = 0;
        tree_i->free = 1;

        char* logName = GetTimestamp();
        logName = realloc(logName, strlen(logName) + strlen(name) + strlen("log/-.log"));
        char* logName2 = calloc(30, 1);
        memcpy(logName2, logName, strlen(logName));
        sprintf(logName, "log/%s-%s.log", name, logName2);    
        tree_i->logfd = open(logName, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR| S_IWUSR);
        free(logName);

        tree_i->err = OK;
        tree_i->hash = TreeIntGetHash(tree_i);
    }

    if (tree_i->logfd == -1) {
        tree_i->err = E_LOG_DEAD;
        printf("[TreeIntInit] unable to open logfile at initialization point!\n");
        return -1;
    }

    if (TreeIntUpdLog(tree_i, "TreeIntInit") != 0)
        tree_i->err = E_LOG_DEAD;

    TREE_INT_VERIFY(tree_i);
    
    return 0;
}

void TreeIntFree(TreeInt* tree_i) {
    if (tree_i == NULL)
        return;
    
    if (tree_i->memmap != NULL)
        free(tree_i->memmap);

    if (tree_i->nodes != NULL)
        free(tree_i->nodes);

    if (tree_i->logfd != -1)
        close(tree_i->logfd);

    free(tree_i);

    return;
}

int TreeIntResize(TreeInt* tree_i, const int sizeNew) {
    if (TreeIntSort(tree_i) == -1) {
        printf("[TreeIntResize] error while sorting tree_i!\n");
        return -1;
    }

    if (sizeNew < DELTA || sizeNew < tree_i->cur) {
        printf("[TreeIntResize] invalid new size %d -> %d\n", tree_i->max, sizeNew);
        
        if (TreeIntUpdLog(tree_i, "TreeIntResize (inv size)") != 0)
            tree_i->err = E_LOG_DEAD;

        return -1;

    } else if (sizeNew == tree_i->max) {
        printf("[TreeIntResize] same size %d -> %d\n", tree_i->max, sizeNew);

        if (TreeIntUpdLog(tree_i, "TreeIntResize (same size)") != 0)
            tree_i->err = E_LOG_DEAD;

    } else if (sizeNew < tree_i->max) {
        printf("[TreeIntResize] shrink %d -> %d\n", tree_i->max, sizeNew);

        tree_i->max = sizeNew;
        tree_i->nodes = realloc(tree_i->nodes, sizeNew * sizeof(NodeInt));
        tree_i->memmap = realloc(tree_i->memmap, sizeNew * sizeof(NodeInt));        

        if (TreeIntUpdLog(tree_i, "TreeIntResize (shrink)") != 0)
            tree_i->err = E_LOG_DEAD;

    } else if (sizeNew > tree_i->max) {
        printf("[TreeIntResize] extend %d -> %d\n", tree_i->max, sizeNew);

        tree_i->nodes = realloc(tree_i->nodes, sizeNew * sizeof(NodeInt));
        memset(tree_i->nodes + tree_i->max * sizeof(NodeInt), '\0', (sizeNew - tree_i->max) * sizeof(NodeInt));

        tree_i->memmap = realloc(tree_i->memmap, sizeNew * sizeof(int));
        for (int i = tree_i->max - 1; i < sizeNew - 1; i++)
            tree_i->memmap[i] = i + 1;
        tree_i->memmap[sizeNew - 1] = 0;

        if (tree_i->free == 0)
            tree_i->free = tree_i->max;
        
        tree_i->max = sizeNew;

        if (TreeIntUpdLog(tree_i, "TreeIntResize (extend)") != 0)
            tree_i->err = E_LOG_DEAD;
    }

    TREE_INT_VERIFY(tree_i);

    return 0;
}

int TreeIntSort(TreeInt* tree_i) {
    TREE_INT_VERIFY(tree_i);
    
    int counter = 0;

    NodeInt* nodesNew = calloc(tree_i->max, sizeof(NodeInt));

    if (_TreeIntSort(tree_i, tree_i->root, 0, 0, &counter, nodesNew) == -1)
        return -1;

    for (int i = 1; i < tree_i->max - 1; i++)
        tree_i->memmap[i] = i + 1;
    tree_i->memmap[tree_i->max - 1] = 0;

    free(tree_i->nodes);
    tree_i->nodes = nodesNew;
    tree_i->free = tree_i->cur + 1;

    if (TreeIntUpdLog(tree_i, "TreeIntSort") != 0)
        tree_i->err = E_LOG_DEAD;

    TREE_INT_VERIFY(tree_i);

    return 0;
}

int _TreeIntSort(TreeInt* tree_i, const int node, const int parent, const int branch, int* counter, NodeInt* buf) {
    if (node == 0)
        return 0;
    
    (*counter) += 1;
    if (*counter > tree_i->max) {
        printf("[_TreeIntSort] index out of range!\n");
        return -1;
    }

    int this = *counter;
    int branchL = (tree_i->nodes[node]).branch[left];
    int branchR = (tree_i->nodes[node]).branch[right];

    buf[this] = tree_i->nodes[node];
    buf[this].parent = parent;
    if (parent != 0)
        buf[parent].branch[branch] = this;

    if (branchL != 0)
        if (_TreeIntSort(tree_i, branchL, this, left, counter, buf) == -1)
            return -1;

    if (branchR != 0)
        if (_TreeIntSort(tree_i, branchR, this, right, counter, buf) == -1)
            return -1;
    
    return *counter;
}


int TreeIntGetRoot(TreeInt* tree_i) {
    TREE_INT_VERIFY(tree_i);

    return tree_i->root;
}

int TreeIntGetFree(TreeInt* tree_i) {
    TREE_INT_VERIFY(tree_i);

    return tree_i->free;
}

int TreeIntGetCur(TreeInt* tree_i) {
    TREE_INT_VERIFY(tree_i);

    return tree_i->cur;
}

int TreeIntGetMax(TreeInt* tree_i) {
    TREE_INT_VERIFY(tree_i);

    return tree_i->max;
}

int TreeIntFind(TreeInt* tree_i, const int node, int data) {
    if (tree_i->cur == 0) {
        printf("[TreeIntFind] tree_i is empty!\n");
        return -1;
    }

    if (node == 0)
        return -1;
    
    if (tree_i->nodes[node].data == data)
        return node;
    
    int res = -1;

    res = TreeIntFind(tree_i, (tree_i->nodes[node]).branch[left], data);
    if (res != -1)
        return res;

    res = TreeIntFind(tree_i, (tree_i->nodes[node]).branch[right], data);
    if (res != -1)
        return res;
    
    return -1;
}

int TreeIntCountSubtree(TreeInt* tree_i, const int node) {
    int counter = 0;

    if (_TreeIntCountSubtree(tree_i, node, &counter) == -1)
        return -1;

    return counter;
}

int _TreeIntCountSubtree(TreeInt* tree_i, const int node, int* counter) {
    if (*counter > tree_i->cur) {
        printf("[TreeIntCountSubtree] counter out of range!\n");
        tree_i->err = E_SEQ_CORRUPTED;
        return -1;
    }

    int branchL = (tree_i->nodes[node]).branch[left];
    int branchR = (tree_i->nodes[node]).branch[right];

    if (branchL != 0)
        if (_TreeIntCountSubtree(tree_i, branchL, counter) == -1)
            return -1;

    if (branchR != 0)
        if (_TreeIntCountSubtree(tree_i, branchR, counter) == -1)
            return -1;

    *counter++;

    return 0;
}


int TreeIntInsertNode(TreeInt* tree_i, const int parent, const int branch, const int data) {
    TREE_INT_VERIFY(tree_i);
    
    int addrIns = tree_i->free;

    if (tree_i->cur == 0) {
        tree_i->cur++;
        (tree_i->nodes[addrIns]).data = data;
        (tree_i->nodes[addrIns]).parent = 0;
        (tree_i->nodes[addrIns]).branch[left] = 0;
        (tree_i->nodes[addrIns]).branch[right] = 0;
        tree_i->root = addrIns;
        tree_i->free = tree_i->memmap[addrIns];

        if (TreeIntUpdLog(tree_i, "TreeIntInsertNodeInt (root added)") != 0)
        tree_i->err = E_LOG_DEAD;
    } else if ((tree_i->nodes[parent]).branch[branch] != 0) {
        printf("[TreeIntInsertNodeInt] branch %d of node %d is already occupied!\n", branch, parent);
        return -1;
    } else if ((tree_i->nodes[parent]).parent == 0 && parent != tree_i->root) {
        printf("[TreeIntInsertNodeInt] trying to insert in unlinked chunk!\n");
        return -1;
    } else {
        if (tree_i->cur >= tree_i->max - 2) {
            if (TreeIntResize(tree_i, tree_i->max * 2) == -1)
                return -1;
        } else {
            (tree_i->nodes[parent]).branch[branch] = tree_i->free;
            (tree_i->nodes[addrIns]).parent = parent;
            (tree_i->nodes[addrIns]).data = data;
            (tree_i->nodes[addrIns]).branch[left] = 0;
            (tree_i->nodes[addrIns]).branch[right] = 0;

            tree_i->free = tree_i->memmap[addrIns];
            tree_i->cur++;

            if (TreeIntUpdLog(tree_i, "TreeIntInsertNodeInt (inserted)") != 0)
            tree_i->err = E_LOG_DEAD;
        }
    }

    TREE_INT_VERIFY(tree_i);

    return addrIns;
}    

// returns subtree size, else - -1;
int TreeIntCopySubtree(TreeInt* src, NodeInt* dst, const int node) {
    int subtreeSz = TreeIntCountSubtree(src, node);
    if (subtreeSz == -1) {
        printf("[TreeIntCopySubtree] can't count elements to copy subtree!\n");
        return -1;
    }

    dst = calloc(subtreeSz, sizeof(NodeInt));
    int pos = 0;

    if (_TreeIntCopySubtree(src, dst, node, &pos) == -1)
        return -1;

    if (TreeIntUpdLog(src, "TreeIntCopySubTreeInt") != 0)
        src->err = E_LOG_DEAD;

    return subtreeSz;
}

int _TreeIntCopySubtree(TreeInt* src, NodeInt* dst, const int node, int* pos) {
    int branchL = (src->nodes[node]).branch[left];
    int branchR = (src->nodes[node]).branch[right];

    dst[*pos] = src->nodes[node];
    *pos++;

    if (branchL != 0)
        if (_TreeIntCopySubtree(src, dst, branchL, pos) == -1)
            return -1;

    if (branchR != 0)
        if (_TreeIntCopySubtree(src, dst, branchR, pos) == -1)
            return -1; 

    return 0;
}

int TreeIntDeleteNode(TreeInt* tree_i, const int node) {
    TREE_INT_VERIFY(tree_i);
    
    if (_TreeIntDeleteNode(tree_i, node) == -1) {
        printf("[TreeIntDeleteNode] can't delete node and it's subtree!\n");
        return -1;
    }

    if (TreeIntUpdLog(tree_i, "TreeIntDeleteNode") != 0)
        tree_i->err = E_LOG_DEAD;

    TREE_INT_VERIFY(tree_i);
    
    return 0;
}

int _TreeIntDeleteNode(TreeInt* tree_i, const int node) {
    if (tree_i->cur == 0) {
        printf("[TreeIntDeleteNode] tree is already empty!\n");
        return -1;
    } else if (tree_i->cur < 0) {
        printf("[TreeIntDeleteNode] tree->cur went negative while deleting!\n");
        return -1;
    } else if (tree_i->cur < tree_i->max - DELTA && tree_i->max / 2 >= DELTA) {
        if (TreeIntResize(tree_i, tree_i->max / 2) == -1) {
            printf("[TreeIntDeleteNode] error while shrinking during delete!\n");
            return -1;
        }
    } else {
        int parent = (tree_i->nodes[node]).parent;
        int branchL = (tree_i->nodes[node]).branch[left];
        int branchR = (tree_i->nodes[node]).branch[right];

        if (branchL != 0)
            if (_TreeIntDeleteNode(tree_i, branchL) == -1)
                return -1;

        if (branchR != 0)
            if (_TreeIntDeleteNode(tree_i, branchR) == -1)
                return -1;
        
        if ((tree_i->nodes[node]).parent != 0 || node == tree_i->root)
            tree_i->cur--;

        if ((tree_i->nodes[parent]).branch[left] == node)
            (tree_i->nodes[parent]).branch[left] = 0;
        else if ((tree_i->nodes[parent]).branch[right] == node)
            (tree_i->nodes[parent]).branch[right] = 0;
        
        (tree_i->nodes[node]).parent = 0;

        tree_i->memmap[node] = tree_i->free;
        tree_i->free = node;
    }
    
    return 0;
}

int TreeIntChangeNode(TreeInt* tree_i, const int node, int* parentNew, int* branchLNew, int* branchRNew, int* dataNew) {
    TREE_INT_VERIFY(tree_i);

    int branchL = (tree_i->nodes[node]).branch[left];
    int branchR = (tree_i->nodes[node]).branch[right];

    if (parentNew != NULL) 
        (tree_i->nodes[node]).parent = *parentNew;

    if (branchLNew != NULL) {
        (tree_i->nodes[branchL]).parent = 0;
        branchL = *branchLNew;
    }

    if (branchRNew != NULL) {
        (tree_i->nodes[branchR]).parent = 0;
        branchR = *branchRNew;
    }

    if (dataNew != NULL)
        (tree_i->nodes[node]).data = *dataNew;

    if (TreeIntUpdLog(tree_i, "TreeIntChangeNode") != 0)
        tree_i->err = E_LOG_DEAD;

    TREE_INT_VERIFY(tree_i);

    return 0;
}


TreeInt* TreeIntRead(const char* pathname) {
    TreeInt* tree_i = TreeIntAlloc();

    if (TREE_INT_INIT(tree_i) == -1) {
        printf("[TreeIntRead] initialzation error! returning nullptr...\n");
        return NULL;
    }

    int fd = open(pathname, O_RDONLY);
    if (fd == -1) {
        printf("[TreeIntRead] can't open mentioned file!\n");
        return NULL;
    }

    int len = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    char* s = calloc(len, sizeof(char));

    if (read(fd, s, len) != len) {
        printf("[TreeIntRead] can't read file to buffer!\n");
        return NULL;
    }

    close(fd);

    if (_TreeIntRead(tree_i, &s) == -1) {
        printf("[TreeIntRead] error while rading!\n");
        return NULL;
    }

    TREE_INT_VERIFY(tree_i);

    return tree_i;
}

int _TreeIntRead(TreeInt* tree_i, char** s) {
    int res = TreeIntGetGr(s, tree_i, 0, 0);

    if (res == -1) {
        printf("[_TreeIntRead] can't read!\n");
        return -1;
    }

    return 0;
}

int TreeIntGetGr(char** s, TreeInt* tree_i, const int parent, const int branch) {
    //printf("[GetGr] <%c>\n", **s);

    int val = 0;

    GetSpace(s);

    val = TreeIntGetNode(s, tree_i, parent, branch);

    if (val == -1) {
        printf("[GetGr] hui tam\n");
        return -1;
    }

    GetSpace(s);

    if (**s != '\0' && **s != '\n') {
        printf("[GetGr] <%d>\n", **s);
        printf("[GetGr] syntax error! no end of str detected!\n");
        return -1;
    } else {
        return val;
    }
}

int TreeIntGetNode(char** s, TreeInt* tree_i, const int parent, const int branch) {
    //printf("[GetNode] <%c>\n", **s);
    
    int val = 0;
    int node = 0;

    val = GetNum(s);

    if (val == -1) {
        if (**s != '#') {
            return -1;
        } else {
            (*s)++;
            return 0;
        }
    } else {
        node = TreeIntInsertNode(tree_i, parent, branch, val);
        if (node == -1) {
            printf("[GetNode] can't insert!\n");
            return -1;
        } else {
            //printf("[GetNode] inserted %d at pos, parent: %d, branch: %d\n", val, parent, branch);
        }
    }

    while (**s == '(') {
        (*s)++;

        GetSpace(s);

        if (TreeIntGetNode(s, tree_i, node, left) == -1)
            return -1;

        GetSpace(s);

        if (**s != ',')
            return -1;
        else 
            (*s)++;
        
        GetSpace(s);

        if (TreeIntGetNode(s, tree_i, node, right) == -1)
            return -1;

        GetSpace(s);
        
        if (**s != ')')
            return -1;
        else
            (*s)++;
    }

    GetSpace(s);

    return 0;
}

int GetNum(char** s) {
    //printf("[GetNum] <%c>\n", **s);

    int val = 0;

    if (isdigit(**s) != 0) {
        val = val * 10 + (**s - '0');
		(*s)++;
    } else {
        printf("[GetNum] no number was read!\n");
        return -1;
    }

	while (isdigit(**s)) {
		val = val * 10 + (**s - '0');
		(*s)++;
    }

	return val;
}

int GetSpace(char** s) {
    while (isblank(**s) != 0) {
        (*s)++;
    }

    return 0;
}

int TreeIntWrite(TreeInt* tree_i, const char* pathname) {
    if (TreeIntVerify(tree_i) != OK) {
        printf("[TreeIntWrite] tree corrupted!\n");
        return -1;
    }

    int fdout = open(pathname, O_WRONLY | O_CREAT | O_TRUNC);
    if (fdout == -1) {
        printf("[TreeIntWrite] can't open file for writing!\n");
        perror("msg: \n");
        return -1;
    }

    if (tree_i->root == 0) {
        printf("[TreeIntWrite] empty tree given!\n");
        dprintf(fdout, "#\n");
        return 0;
    } else if (_TreeIntWrite(tree_i, tree_i->root, fdout) == -1) {
        printf("hui\n");
        return -1;
    }

    return 0;
}

int _TreeIntWrite(TreeInt* tree_i, int node, int fd) {
    dprintf(fd, "%d", tree_i->nodes[node].data);

    if (tree_i->nodes[node].branch[left] != 0 || tree_i->nodes[node].branch[right] != 0) {
        dprintf(fd, "(");

        if (tree_i->nodes[node].branch[left] != 0) {
            //dprintf(fd, "%d", tree->nodes[tree->nodes[node].branch[left]].data);
            _TreeIntWrite(tree_i, tree_i->nodes[node].branch[left], fd);
        } else {
            dprintf(fd, "#");
        }

        dprintf(fd, ", ");

        if (tree_i->nodes[node].branch[right] != 0) {
            //dprintf(fd, "%d", tree->nodes[tree->nodes[node].branch[right]].data);
            _TreeIntWrite(tree_i, tree_i->nodes[node].branch[right], fd);
        } else {
            dprintf(fd, "#");
        }

        dprintf(fd, ")");
    }

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


int TreeIntVerify(TreeInt* tree_i) {
    return tree_i->err;
}

void TreeIntDump(TreeInt* tree_i, const char* name) {
    char* dumpName = calloc(1 + strlen(name) + strlen("dump/.log"), sizeof(char));
    sprintf(dumpName, "dump/%s.log", name);
    int dumpFd = open(dumpName, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
    free(dumpName);

    if (tree_i == NULL) {
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
                        timeStamp, tree_i->hash, tree_i->max, tree_i->cur, tree_i->free, tree_i->root, tree_i->err, TreeErrsDesc[tree_i->err]);
        free(timeStamp);

        if (tree_i->nodes != NULL)
            for (int i = 0; i < tree_i->max; i++)
                dprintf(dumpFd, "  [%3d] (%3d [%3d, %3d] <%3d>)\n", i, tree_i->nodes[i].data, tree_i->nodes[i].branch[left], tree_i->nodes[i].branch[right], tree_i->nodes[i].parent);
    }

    close(dumpFd);
}

int TreeIntGetHash(TreeInt* tree_i) {
    return 0;
}


int TreeIntUpdLog(TreeInt* tree_i, const char* func) {
    char* timeStamp = GetTimestamp();

    int res = 0;

    res = dprintf(tree_i->logfd, "%s\n"
                               "passed through function [[%s]]\n"
                               "  hash: %ld\n"
                               "  tree capacity: %d\n"
                               "  tree size: %d\n"
                               "  first free element's physical address: %d\n"
                               "  root address: %d\n"
                               "  error: %d %s\n",
                               timeStamp, func, tree_i->hash, tree_i->max, tree_i->cur, tree_i->free, tree_i->root, tree_i->err, TreeErrsDesc[tree_i->err]);
    free(timeStamp);

    if (res == 0)
        return -1;
    dprintf(tree_i->logfd, "  | &|   val| valL| &L| valR| &R|  &par|\n");
    for (int i = 0; i < tree_i->max; i++) {
        if (((tree_i->nodes[i]).parent == 0 && i != tree_i->root) || tree_i->cur == 0)
            res = dprintf(tree_i->logfd, "  [%2d ] (%3d [%3d <%2d>, %3d <%2d>] <%3d>)\n", i, tree_i->nodes[i].data, tree_i->nodes[tree_i->nodes[i].branch[left]].data, tree_i->nodes[i].branch[left], tree_i->nodes[tree_i->nodes[i].branch[right]].data, tree_i->nodes[i].branch[right], tree_i->nodes[i].parent);
        else 
            res = dprintf(tree_i->logfd, "  [%2d*] (%3d [%3d <%2d>, %3d <%2d>] <%3d>)\n", i, tree_i->nodes[i].data, tree_i->nodes[tree_i->nodes[i].branch[left]].data, tree_i->nodes[i].branch[left], tree_i->nodes[tree_i->nodes[i].branch[right]].data, tree_i->nodes[i].branch[right], tree_i->nodes[i].parent);
        if (res == 0)
            return -1;
    }

    res = dprintf(tree_i->logfd, "\n  memmmap:\n  ");
    if (res == 0)
        return -1;

    for (int i = 0; i < tree_i->max; i++) {
        res = dprintf(tree_i->logfd, "[%3d]", i);
        if (res == 0)
            return -1;
    }

    res = dprintf(tree_i->logfd, "\n  ");
    if (res == 0)
        return -1;
    
    for (int i = 0; i < tree_i->max; i++) {
        res = dprintf(tree_i->logfd, "[%3d]", tree_i->memmap[i]);
        if (res == 0)
            return -1;
    }

    res = dprintf(tree_i->logfd, "\n\n");
    if (res == 0)
        return -1;

    return 0;
}

///////////////////////
// TreeTxt & NodeTxt //
///////////////////////

NodeTxt* NodeTxtAlloc() {
    NodeTxt* node = calloc(1, sizeof(NodeTxt));

    return node;
}

int NodeTxtInit(NodeTxt* node, const int parent, const int branchL, const int branchR, const char* data) {
    if (node == NULL) {
        printf("[NodeTxtInit] nullptr given!\n");
        return -1;
    } else {
        node->parent = parent;
        node->branch[left] = branchL;
        node->branch[right] = branchR;

        int len = strlen(data);
        if (len >= TEXT_MAX_SZ) {
            printf("[NodeTxtInit] too long");
            return -1;
        }else {
            memcpy(node->data, data, len);
        }
    }

    return 0;   
}

void NodeTxtFree(NodeTxt* node) {
    if (node == NULL)
        return;
    else
        free(node);
    
    return;
}


TreeTxt* TreeTxtAlloc() {
    TreeTxt* tree = calloc(1, sizeof(TreeTxt));
    tree->nodes = calloc(TREE_INIT_SZ, sizeof(NodeTxt));
    tree->memmap = calloc(TREE_INIT_SZ, sizeof(int));

    return tree;
}

int TreeTxtInit(TreeTxt* tree, const char* name) {
    if (tree == NULL) {
        printf("[TreeTxtInit] nullptr was given!\n");
        return -1;
    } else if (tree->nodes == NULL) {
        printf("[TreeTxtInit] tree->nodes is nullptr!\n");
        tree->err = E_INIT_ERR;
        return -1;
    } else if (tree->memmap == NULL) {
        tree->err = E_INIT_ERR;
        printf("[TreeTxtInit] tree->memmap is nullptr!\n");
        return -1;
    } else {
        memset(tree->nodes, '\0', TREE_INIT_SZ * sizeof(NodeTxt));

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
        tree->hash = TreeTxtGetHash(tree);
    }

    if (tree->logfd == -1) {
        tree->err = E_LOG_DEAD;
        printf("[TreeTxtInit] unable to open logfile at initialization point!\n");
        return -1;
    }

    if (TreeTxtUpdLog(tree, "TreeTxtInit") != 0)
        tree->err = E_LOG_DEAD;

    TREE_TXT_VERIFY(tree);
    
    return 0;
}

void TreeTxtFree(TreeTxt* tree) {
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

int TreeTxtResize(TreeTxt* tree, const int sizeNew) {
    if (TreeTxtSort(tree) == -1) {
        printf("[TreeTxtResize] error while sorting tree!\n");
        return -1;
    }

    if (sizeNew < DELTA || sizeNew < tree->cur) {
        printf("[TreeTxtResize] invalid new size %d -> %d\n", tree->max, sizeNew);
        
        if (TreeTxtUpdLog(tree, "TreeTxtResize (inv size)") != 0)
            tree->err = E_LOG_DEAD;

        return -1;

    } else if (sizeNew == tree->max) {
        printf("[TreeTxtResize] same size %d -> %d\n", tree->max, sizeNew);

        if (TreeTxtUpdLog(tree, "TreeTxtResize (same size)") != 0)
            tree->err = E_LOG_DEAD;

    } else if (sizeNew < tree->max) {
        printf("[TreeTxtResize] shrink %d -> %d\n", tree->max, sizeNew);

        tree->max = sizeNew;
        tree->nodes = realloc(tree->nodes, sizeNew * sizeof(NodeTxt));
        tree->memmap = realloc(tree->memmap, sizeNew * sizeof(NodeTxt));        

        if (TreeTxtUpdLog(tree, "TreeTxtResize (shrink)") != 0)
            tree->err = E_LOG_DEAD;

    } else if (sizeNew > tree->max) {
        printf("[TreeTxtResize] extend %d -> %d\n", tree->max, sizeNew);

        tree->nodes = realloc(tree->nodes, sizeNew * sizeof(NodeTxt));
        memset(tree->nodes + tree->max * sizeof(NodeTxt), '\0', (sizeNew - tree->max) * sizeof(NodeTxt));

        tree->memmap = realloc(tree->memmap, sizeNew * sizeof(int));
        for (int i = tree->max - 1; i < sizeNew - 1; i++)
            tree->memmap[i] = i + 1;
        tree->memmap[sizeNew - 1] = 0;

        if (tree->free == 0)
            tree->free = tree->max;
        
        tree->max = sizeNew;

        if (TreeTxtUpdLog(tree, "TreeTxtResize (extend)") != 0)
            tree->err = E_LOG_DEAD;
    }

    TREE_TXT_VERIFY(tree);

    return 0;
}

int TreeTxtSort(TreeTxt* tree) {
    TREE_TXT_VERIFY(tree);
    
    int counter = 0;

    NodeTxt* nodesNew = calloc(tree->max, sizeof(NodeTxt));

    if (_TreeTxtSort(tree, tree->root, 0, 0, &counter, nodesNew) == -1)
        return -1;

    for (int i = 1; i < tree->max - 1; i++)
        tree->memmap[i] = i + 1;
    tree->memmap[tree->max - 1] = 0;

    free(tree->nodes);
    tree->nodes = nodesNew;
    tree->free = tree->cur + 1;

    if (TreeTxtUpdLog(tree, "TreeTxtSort") != 0)
        tree->err = E_LOG_DEAD;

    TREE_TXT_VERIFY(tree);

    return 0;
}

int _TreeTxtSort(TreeTxt* tree, const int node, const int parent, const int branch, int* counter, NodeTxt* buf) {
    if (node == 0)
        return 0;
    
    (*counter) += 1;
    if (*counter > tree->max) {
        printf("[_TreeTxtSort] index out of range!\n");
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
        if (_TreeTxtSort(tree, branchL, this, left, counter, buf) == -1)
            return -1;

    if (branchR != 0)
        if (_TreeTxtSort(tree, branchR, this, right, counter, buf) == -1)
            return -1;
    
    return *counter;
}


int TreeTxtGetRoot(TreeTxt* tree) {
    TREE_TXT_VERIFY(tree);

    return tree->root;
}

int TreeTxtGetFree(TreeTxt* tree) {
    TREE_TXT_VERIFY(tree);

    return tree->free;
}

int TreeTxtGetCur(TreeTxt* tree) {
    TREE_TXT_VERIFY(tree);

    return tree->cur;
}

int TreeTxtGetMax(TreeTxt* tree) {
    TREE_TXT_VERIFY(tree);

    return tree->max;
}

int TreeTxtFind(TreeTxt* tree, const int node, const char* data) {
    if (tree->cur == 0) {
        printf("[TreeTxtFind] tree is empty!\n");
        return -1;
    }

    if (node == 0)
        return -1;
    
    if (strcmp(tree->nodes[node].data, data) == 0)
        return node;
    
    int res = -1;

    res = TreeTxtFind(tree, (tree->nodes[node]).branch[left], data);
    if (res != -1)
        return res;

    res = TreeTxtFind(tree, (tree->nodes[node]).branch[right], data);
    if (res != -1)
        return res;
    
    return -1;
}

int TreeTxtInsertNode(TreeTxt* tree, const int parent, const int branch, const char* data) {
    TREE_TXT_VERIFY(tree);
    
    int addrIns = tree->free;

    if (strlen(data) >= TEXT_MAX_SZ - 1) {
        printf("[TreeTxtInsertNode] too long!\n");
        return -1;
    }

    if (tree->cur == 0) {
        tree->cur++;
        memset((tree->nodes[addrIns]).data, '\0', TEXT_MAX_SZ);
        memcpy((tree->nodes[addrIns]).data, data, strlen(data));
        (tree->nodes[addrIns]).parent = 0;
        (tree->nodes[addrIns]).branch[left] = 0;
        (tree->nodes[addrIns]).branch[right] = 0;
        tree->root = addrIns;
        tree->free = tree->memmap[addrIns];

        if (TreeTxtUpdLog(tree, "TreeTxtInsertNode (root added)") != 0)
        tree->err = E_LOG_DEAD;
    } else if ((tree->nodes[parent]).branch[branch] != 0) {
        printf("[TreeTxtInsertNode] branch %d of node %d is already occupied!\n", branch, parent);
        return -1;
    } else if ((tree->nodes[parent]).parent == 0 && parent != tree->root) {
        printf("[TreeTxtInsertNode] trying to insert in unlinked chunk!\n");
        return -1;
    } else {
        if (tree->cur >= tree->max - 2) {
            if (TreeTxtResize(tree, tree->max * 2) == -1)
                return -1;
        } else {
            (tree->nodes[parent]).branch[branch] = tree->free;
            (tree->nodes[addrIns]).parent = parent;
            memset((tree->nodes[addrIns]).data, '\0', TEXT_MAX_SZ);
            memcpy((tree->nodes[addrIns]).data, data, strlen(data));
            (tree->nodes[addrIns]).branch[left] = 0;
            (tree->nodes[addrIns]).branch[right] = 0;

            tree->free = tree->memmap[addrIns];
            tree->cur++;

            if (TreeTxtUpdLog(tree, "TreeTxtInsertNode (inserted)") != 0)
            tree->err = E_LOG_DEAD;
        }
    }

    TREE_TXT_VERIFY(tree);

    return addrIns;
}

int TreeTxtCountSubtree(TreeTxt* tree, const int node) {
    int counter = 0;

    if (_TreeTxtCountSubtree(tree, node, &counter) == -1)
        return -1;

    return counter;
}

int _TreeTxtCountSubtree(TreeTxt* tree, const int node, int* counter) {
    if (*counter > tree->cur) {
        printf("[TreeTxtCountSubtree] counter out of range!\n");
        tree->err = E_SEQ_CORRUPTED;
        return -1;
    }

    int branchL = (tree->nodes[node]).branch[left];
    int branchR = (tree->nodes[node]).branch[right];

    if (branchL != 0)
        if (_TreeTxtCountSubtree(tree, branchL, counter) == -1)
            return -1;

    if (branchR != 0)
        if (_TreeTxtCountSubtree(tree, branchR, counter) == -1)
            return -1;

    *counter++;

    return 0;
}

int TreeTxtCopySubtree(TreeTxt* src, NodeTxt* dst, const int node) {
    int subtreeSz = TreeTxtCountSubtree(src, node);
    if (subtreeSz == -1) {
        printf("[TreeTxtCopySubtree] can't count elements to copy subtree!\n");
        return -1;
    }

    dst = calloc(subtreeSz, sizeof(NodeTxt));
    int pos = 0;

    if (_TreeTxtCopySubtree(src, dst, node, &pos) == -1)
        return -1;

    if (TreeTxtUpdLog(src, "TreeTxtCopySubTreeInt") != 0)
        src->err = E_LOG_DEAD;

    return subtreeSz;
}

int _TreeTxtCopySubtree(TreeTxt* src, NodeTxt* dst, const int node, int* pos) {
    int branchL = (src->nodes[node]).branch[left];
    int branchR = (src->nodes[node]).branch[right];

    dst[*pos] = src->nodes[node];
    *pos++;

    if (branchL != 0)
        if (_TreeTxtCopySubtree(src, dst, branchL, pos) == -1)
            return -1;

    if (branchR != 0)
        if (_TreeTxtCopySubtree(src, dst, branchR, pos) == -1)
            return -1; 

    return 0;
}

int TreeTxtDeleteNode(TreeTxt* tree, const int node) {
    TREE_TXT_VERIFY(tree);
    
    if (_TreeTxtDeleteNode(tree, node) == -1) {
        printf("[TreeTxtDeleteNode] can't delete node and it's subtree!\n");
        return -1;
    }

    if (TreeTxtUpdLog(tree, "TreeTxtDeleteNode") != 0)
        tree->err = E_LOG_DEAD;

    TREE_TXT_VERIFY(tree);
    
    return 0;
}

int _TreeTxtDeleteNode(TreeTxt* tree, const int node) {
    if (tree->cur == 0) {
        printf("[TreeTxtDeleteNode] tree is already empty!\n");
        return -1;
    } else if (tree->cur < 0) {
        printf("[TreeTxtDeleteNode] tree->cur went negative while deleting!\n");
        return -1;
    } else if (tree->cur < tree->max - DELTA && tree->max / 2 >= DELTA) {
        if (TreeTxtResize(tree, tree->max / 2) == -1) {
            printf("[TreeTxtDeleteNode] error while shrinking during delete!\n");
            return -1;
        }
    } else {
        int parent = (tree->nodes[node]).parent;
        int branchL = (tree->nodes[node]).branch[left];
        int branchR = (tree->nodes[node]).branch[right];

        if (branchL != 0)
            if (_TreeTxtDeleteNode(tree, branchL) == -1)
                return -1;

        if (branchR != 0)
            if (_TreeTxtDeleteNode(tree, branchR) == -1)
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

int TreeTxtChangeNode(TreeTxt* tree, const int node, int* parentNew, int* branchLNew, int* branchRNew, char* dataNew) {
    TREE_TXT_VERIFY(tree);

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

    if (dataNew != NULL) {
        if (strlen(dataNew) >= TEXT_MAX_SZ) {
            printf("[TreeTxtChangeNode] too long!\n");
            return -1;
        } else {
            memset((tree->nodes[node]).data, '\0', TEXT_MAX_SZ);
            memcpy((tree->nodes[node]).data, dataNew, strlen(dataNew));
        }
    }

    if (TreeTxtUpdLog(tree, "TreeTxtChangeNode") != 0)
        tree->err = E_LOG_DEAD;

    TREE_TXT_VERIFY(tree);

    return 0;
}


TreeTxt* TreeTxtRead(const char* pathname) {
    TreeTxt* tree = TreeTxtAlloc();

    if (TREE_TXT_INIT(tree) == -1) {
        printf("[TreeTxtRead] initialzation error! returning nullptr...\n");
        return NULL;
    }

    int fd = open(pathname, O_RDONLY);
    if (fd == -1) {
        printf("[TreeTxtRead] can't open mentioned file!\n");
        return NULL;
    }

    int len = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    char* s = calloc(len, sizeof(char));

    if (read(fd, s, len) != len) {
        printf("[TreeTxtRead] can't read file to buffer!\n");
        return NULL;
    }

    close(fd);

    if (_TreeTxtRead(tree, &s) == -1) {
        printf("[TreeTxtRead] error while rading!\n");
        return NULL;
    }

    TREE_TXT_VERIFY(tree);

    return tree;
}

int _TreeTxtRead(TreeTxt* tree, char** s) {
    int res = TreeTxtGetGr(s, tree, 0, 0);

    if (res == -1) {
        printf("[_TreeTxtRead] can't read!\n");
        return -1;
    }

    return 0;
}

int TreeTxtGetGr(char** s, TreeTxt* tree, const int parent, const int branch) {
    //printf("[TreeTxtGetGr] <%c>\n", **s);

    int val = 0;

    GetSpace(s);

    val = TreeTxtGetNode(s, tree, parent, branch);

    if (val == -1) {
        printf("[TreeTxtGetGr] her tam\n");
    }

    GetSpace(s);

    if (**s != '\0' && **s != '\n' && **s != EOF) {
        printf("<%s>\n", *s - 7);
        printf("[TreeTxtGetGr] syntax error! no end of str detected!\n");
        return -1;
    } else {
        return val;
    }
}

int TreeTxtGetNode(char** s, TreeTxt* tree, const int parent, const int branch) {
    //printf("[TreeTxtGetNode] <%c>\n", **s);
    
    int node = 0;

    int len = GetStr(s);

    if (len == -1) {
        if (**s != '#') {
            return -1;
        } else {
            (*s)++;
            return 0;
        }
    } else {
        char* str = calloc(len, sizeof(char));
        memcpy(str, *s - len, len - 1);

        node = TreeTxtInsertNode(tree, parent, branch, str);
        if (node == -1) {
            printf("[TreeTxtGetNode] can't insert!\n");
            return -1;
        } else {
            //printf("[TreeTxtGetNode] inserted %s at pos, parent: %d, branch: %d\n", str, parent, branch);
        }

        free(str);
    }

    while (**s == '(') {
        (*s)++;

        GetSpace(s);

        if (TreeTxtGetNode(s, tree, node, left) == -1)
            return -1;

        GetSpace(s);

        if (**s != ',')
            return -1;
        else 
            (*s)++;
        
        GetSpace(s);

        if (TreeTxtGetNode(s, tree, node, right) == -1)
            return -1;

        GetSpace(s);
        
        if (**s != ')')
            return -1;
        else
            (*s)++;
    }

    GetSpace(s);

    return 0;
}

int GetStr(char** s) {
    char* str = *s;

    if (**s == '\"') {
        (*s)++;

        while (**s != '\"' && (isalnum(**s) != 0 || isblank(**s) != 0 || ispunct(**s) != 0)) {
            (*s)++;
        }

        if (**s != '\"') {
            printf("[TreeTxtGetStr] no closing quote detected!\n");
            return -1;
        } else {
            (*s)++;
        }
    } else {
        return -1;
    }
    return *s - str - 1;
}

int TreeTxtWrite(TreeTxt* tree, const char* pathname) {
    TREE_TXT_VERIFY(tree);

    int fdout = open(pathname, O_WRONLY | O_CREAT | O_TRUNC);
    if (fdout == -1) {
        printf("[TreeTxtWrite] can't open file for writing!\n");
        perror("msg: \n");
        return -1;
    }

    if (tree->root == 0) {
        printf("[TreeTxtWrite] empty tree given!\n");
        dprintf(fdout, "#\n");
        return 0;
    } else if (_TreeTxtWrite(tree, tree->root, fdout) == -1) {
        printf("failed\n");
        return -1;
    }

    dprintf(fdout, "\n");

    close(fdout);

    return 0;
}

int _TreeTxtWrite(TreeTxt* tree, int node, int fd) {
    dprintf(fd, "\"%s\"", tree->nodes[node].data);

    if (tree->nodes[node].branch[left] != 0 || tree->nodes[node].branch[right] != 0) {
        dprintf(fd, "(");

        if (tree->nodes[node].branch[left] != 0) {
            _TreeTxtWrite(tree, tree->nodes[node].branch[left], fd);
        } else {
            dprintf(fd, "#");
        }

        dprintf(fd, ", ");

        if (tree->nodes[node].branch[right] != 0) {
            _TreeTxtWrite(tree, tree->nodes[node].branch[right], fd);
        } else {
            dprintf(fd, "#");
        }

        dprintf(fd, ")");
    }

    return 0;
}


int TreeTxtVerify(TreeTxt* tree) {
    return tree->err;
}

void TreeTxtDump(TreeTxt* tree, const char* name) {
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
                dprintf(dumpFd, "  [%3d] (%s [%3d, %3d] <%3d>)\n", i, tree->nodes[i].data, tree->nodes[i].branch[left], tree->nodes[i].branch[right], tree->nodes[i].parent);
    }

    close(dumpFd);
}

int TreeTxtGetHash(TreeTxt* tree) {
    return 0;
}


int TreeTxtUpdLog(TreeTxt* tree, const char* func) {
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
    dprintf(tree->logfd,               "  |  &|          val| &L| &R|  &par|\n");
    for (int i = 0; i < tree->max; i++) {
        if (((tree->nodes[i]).parent == 0 && i != tree->root) || tree->cur == 0)
            res = dprintf(tree->logfd, "  [%2d ] (%10s [ %2d, %2d] <%3d>)\n", i, tree->nodes[i].data, tree->nodes[i].branch[left], tree->nodes[i].branch[right], tree->nodes[i].parent);
        else 
            res = dprintf(tree->logfd, "  [%2d*] (%10s [ %2d, %2d] <%3d>)\n", i, tree->nodes[i].data, tree->nodes[i].branch[left], tree->nodes[i].branch[right], tree->nodes[i].parent);
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