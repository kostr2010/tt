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

Node* NodeAlloc() {
    Node* node = calloc(1, sizeof(Node));

    return node;
}

int NodeInit(Node* node, const int parent, const int branchL, const int branchR, const data data, int type) {
    if (node == NULL) {
        printf("[NodeInit] nullptr given!\n");
        return -1;
    } else {
        node->parent = parent;
        node->branch[Left] = branchL;
        node->branch[Right] = branchR;

        switch (type) {
            case Txt:  return NodeInitTxtHandler((struct _NodeTxt*)node, (const char**)(&data)); break;
            case Int:  return NodeInitIntHandler((struct _NodeInt*)node, (const int*)(&data)); break;
            case Math: return NodeInitMathHandler((struct _NodeMath*)node, (const MathData*)(&data)); break;
            default: printf("[NodeInit] invalid type!\n"); return -1; break;
        }
    }

    return 0;   
}

int NodeInitTxtHandler(struct _NodeTxt* node, const char** data) {
    int len = strlen(*data);

    if (len >= TEXT_MAX_SZ) {
        printf("[NodeTxtInit] too long");
        return -1;
    }else {
        memcpy(node->data, *data, len);
    }

    return 0;
}

int NodeInitIntHandler(struct _NodeInt* node, const int* data) {
    node->data = *data;

    return 0;
}

int NodeInitMathHandler(struct _NodeMath* node, const MathData* data) {
    node->data.type = data->type;
    node->data.value = data->value;

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

int TreeInit(Tree* tree, const char* name, int type) {
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
    } else if (name == NULL) {
        tree->err = E_INIT_ERR;
        printf("[TreeInit] name is nullptr!\n");
        return -1;
    } else {
        memset(tree->nodes, '\0', TREE_INIT_SZ * sizeof(Node));

        for (int i = 1; i < TREE_INIT_SZ - 1; i++)
            tree->memmap[i] = i + 1;
        tree->memmap[TREE_INIT_SZ - 1] = 0;

        tree->max = TREE_INIT_SZ;
        tree->cur = 0;
        tree->root = 0;
        tree->free = 1;

        switch (type) {
            case Txt:  tree->header.type = Txt; break;
            case Int:  tree->header.type = Int; break;
            case Math: tree->header.type = Math; break;
            default: printf("[TreeInit] invalid type!"); return -1;
        }

        #ifdef LOG_ON
        char* logName = GetTimestamp();
        logName = realloc(logName, strlen(logName) + strlen(name) + strlen("log/-.log"));
        char* logName2 = calloc(30, 1);
        memcpy(logName2, logName, strlen(logName));
        sprintf(logName, "log/%s-%s.log", name, logName2);    
        tree->logfd = open(logName, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR| S_IWUSR);
        free(logName);
        #endif

        tree->err = OK;

        #ifdef SEC_ON 
        tree->hash = TreeGetHash(tree);
        #endif
    }

    #ifdef LOG_ON
    if (tree->logfd == -1) {
        tree->err = E_LOG_DEAD;
        printf("[TreeInit] unable to open logfile at initialization point!\n");
        return -1;
    }

    if (TreeUpdLog(tree, "TreeInit") != 0)
        tree->err = E_LOG_DEAD;
    #endif

    TREE_VERIFY(tree);
    
    return 0;
}

void TreeFree(Tree* tree) {
    if (tree == NULL)
        return;
    
    if (tree->memmap != NULL)
        free(tree->memmap);

    if (tree->nodes != NULL)
        free(tree->nodes);

    #ifdef LOG_ON
    if (tree->logfd != -1)
        close(tree->logfd);
    #endif

    free(tree);

    return;
}

int TreeResize(Tree* tree, const int sizeNew) {
    TREE_VERIFY(tree);
    /*
    if (TreeSort(tree) == -1) {
        printf("[TreeResize] error while sorting tree!\n");
        return -1;
    }
    */

    if (sizeNew < DELTA || sizeNew < tree->cur) {
        printf("[TreeResize] invalid new size %d -> %d\n", tree->max, sizeNew);
        return -1;

    } else if (sizeNew == tree->max) {
        printf("[TreeResize] same size %d -> %d\n", tree->max, sizeNew);
        
        #ifdef LOG_ON
        if (TreeUpdLog(tree, "TreeResize (same size)") != 0)
            tree->err = E_LOG_DEAD;
        #endif

    } else if (sizeNew < tree->max) {
        printf("[TreeResize] shrink %d -> %d\n", tree->max, sizeNew);

        tree->max = sizeNew;
        tree->nodes = realloc(tree->nodes, sizeNew * sizeof(Node));
        tree->memmap = realloc(tree->memmap, sizeNew * sizeof(Node));        

        #ifdef LOG_ON
        if (TreeUpdLog(tree, "TreeResize (shrink)") != 0)
            tree->err = E_LOG_DEAD;
        #endif

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

        #ifdef LOG_ON
        if (TreeUpdLog(tree, "TreeResize (extend)") != 0)
            tree->err = E_LOG_DEAD;
        #endif
    }

    #ifdef SEC_ON
    tree->hash = TreeGetHash(tree);
    #endif

    TREE_VERIFY(tree);

    return 0;
}

int TreeSort(Tree* tree) {
    TREE_VERIFY(tree);
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

    #ifdef LOG_ON
    if (TreeUpdLog(tree, "TreeSort") != 0)
        tree->err = E_LOG_DEAD;
    #endif

    #ifdef SEC_ON
    tree->hash = TreeGetHash(tree);
    #endif

    TREE_VERIFY(tree);

    return 0;
}

int _TreeSort(Tree* tree, const int node, const int parent, const int branch, int* counter, Node* buf) {
    if (node == 0)
        return 0;
    
    (*counter) += 1;
    if (*counter > tree->max) {
        printf("  [_TreeTxtSort] index out of range!\n");
        return -1;
    }

    int this = *counter;
    int branchL = (tree->nodes[node]).branch[Left];
    int branchR = (tree->nodes[node]).branch[Right];

    buf[this] = tree->nodes[node];
    buf[this].parent = parent;
    if (parent != 0)
        buf[parent].branch[branch] = this;

    if (branchL != 0)
        if (_TreeSort(tree, branchL, this, Left, counter, buf) == -1)
            return -1;

    if (branchR != 0)
        if (_TreeSort(tree, branchR, this, Right, counter, buf) == -1)
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

int TreeGetCur(Tree* tree) {
    TREE_VERIFY(tree);

    return tree->cur;
}

int TreeGetMax(Tree* tree) {
    TREE_VERIFY(tree);

    return tree->max;
}

int TreeFind(Tree* tree, const int node, const data data) {
    TREE_VERIFY(tree);

    if (tree->cur == 0) {
        printf("[TreeFind] tree is empty!\n");
        return -1;
    }

    if (node == 0)
        return -1;
    
    int res = -1;
    switch (tree->header.type) {
        case Txt:  res = TreeFindTxtHandler((struct _TreeTxt*)tree, node, (const char**)(&data)); break;
        case Int:  res = TreeFindIntHandler((struct _TreeInt*)tree, node, (const int*)(&data)); break;
        case Math: res = TreeFindMathHandler((struct _TreeMath*)tree, node, (const MathData*)(&data)); break;
        default: printf("[NodeInit] invalid type!\n"); return -1;
    }

    if (res != -1)
        return res;

    res = TreeFind(tree, (tree->nodes[node]).branch[Left], data);
    if (res != -1)
        return res;

    res = TreeFind(tree, (tree->nodes[node]).branch[Right], data);
    if (res != -1)
        return res;

    TREE_VERIFY(tree);
    
    return -1;
}

int TreeFindTxtHandler(struct _TreeTxt* tree, const int node, const char** data) {
    if (strcmp(tree->nodes[node].data, *data) == 0)
        return node;
    else 
        return -1;
}

int TreeFindIntHandler(struct _TreeInt* tree, const int node, const int* data) {
    if (tree->nodes[node].data == *data)
        return node;
    else 
        return -1;
}

int TreeFindMathHandler(struct _TreeMath* tree, const int node, const MathData* data) {
    if (tree->nodes[node].data.value == data->value && tree->nodes[node].data.type == data->type) {
        return node;
    } else {
        return -1;
    }
}

int TreeInsertNode(Tree* tree, const int parent, const int branch, data* data) {
    TREE_VERIFY(tree);
    
    int addrIns = tree->free;
    /*
    if (tree->header.type == Txt) {
        if (strlen(*data) >= TEXT_MAX_SZ - 1) {
            printf("[TreeInsertNode] too long!\n");
            return -1;
        }
    }
    */
    int res = 0;

    if (tree->cur == 0) {
        tree->cur++;

        switch (tree->header.type) {
            case Txt: res = TreeInsertNodeTxtHandler((struct _TreeTxt*)tree, addrIns, (const char**)(data)); break;
            case Int: res = TreeInsertNodeIntHandler((struct _TreeInt*)tree, addrIns, (const int*)(data)); break;
            case Math: res = TreeInsertNodeMathHandler((struct _TreeMath*)tree, addrIns, (const MathData*)(data)); break;
            default: printf("[TreeInsertNode] invalid type!\n"); return -1;
        }

        if (res == -1)
            return -1;

        (tree->nodes[addrIns]).parent = 0;
        (tree->nodes[addrIns]).branch[Left] = 0;
        (tree->nodes[addrIns]).branch[Right] = 0;
        tree->root = addrIns;
        tree->free = tree->memmap[addrIns];

        #ifdef LOG_ON
        if (TreeUpdLog(tree, "TreeInsertNode (root added)") != 0)
        tree->err = E_LOG_DEAD;
        #endif
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
        }

        (tree->nodes[parent]).branch[branch] = tree->free;
        (tree->nodes[addrIns]).parent = parent;

        switch (tree->header.type) {
            case Txt: res = TreeInsertNodeTxtHandler((struct _TreeTxt*)tree, addrIns, (const char**)(data)); break;
            case Int: res = TreeInsertNodeIntHandler((struct _TreeInt*)tree, addrIns, (const int*)(data)); break;
            case Math: res = TreeInsertNodeMathHandler((struct _TreeMath*)tree, addrIns, (const MathData*)(data)); break;
            default: printf("[TreeInsertNode] invalid type!\n"); return -1;
        }

        if (res == -1)
            return -1;

        (tree->nodes[addrIns]).branch[Left] = 0;
        (tree->nodes[addrIns]).branch[Right] = 0;

        tree->free = tree->memmap[addrIns];
        tree->cur++;

        #ifdef LOG_ON
        if (TreeUpdLog(tree, "TreeInsertNode (inserted)") != 0)
            tree->err = E_LOG_DEAD;
        #endif 
    }

    #ifdef SEC_ON
    tree->hash = TreeGetHash(tree);
    #endif

    TREE_VERIFY(tree);

    return addrIns;
}

int TreeInsertNodeTxtHandler(struct _TreeTxt* tree, const int node, const char** data) {
    if (strlen(*data) >= TEXT_MAX_SZ - 1) {
        printf("[TreeInsertNode] too long!\n");
        return -1;
    }
    
    memset((tree->nodes[node]).data, '\0', TEXT_MAX_SZ);
    memcpy((tree->nodes[node]).data, *data, strlen(*data));

    tree->header.type = Txt;

    return 0;
}

int TreeInsertNodeIntHandler(struct _TreeInt* tree, const int node, const int* data) {
    (tree->nodes[node]).data = *data;
    tree->header.type = Int;

    return 0;
}

int TreeInsertNodeMathHandler(struct _TreeMath* tree, const int node, const MathData* data) {
    (tree->nodes[node]).data.type = data->type;
    (tree->nodes[node]).data.value = data->value;

    tree->header.type = Math;

    return 0;
}

int TreeCountSubtree(Tree* tree, const int node) {
    TREE_VERIFY(tree);

    int counter = 0;

    //printf("node: %d\n", node);

    if (_TreeCountSubtree(tree, node, &counter) == -1)
        return -1;

    TREE_VERIFY(tree);

    return counter;
}

int _TreeCountSubtree(Tree* tree, const int node, int* counter) {    
    if (*counter > tree->cur) {
        printf("[TreeCountSubtree] counter out of range!\n");
        tree->err = E_SEQ_CORRUPTED;
        return -1;
    }

    (*counter)++;

    int branchL = (tree->nodes[node]).branch[Left];
    int branchR = (tree->nodes[node]).branch[Right];

    //printf("branchL = %d, branchR = %d\n", branchL, branchR);

    if (branchL != 0)
        if (_TreeCountSubtree(tree, branchL, counter) == -1)
            return -1;

    if (branchR != 0)
        if (_TreeCountSubtree(tree, branchR, counter) == -1)
            return -1;

    //printf("counter: %d\n", *counter);

    return 0;
}

Node* TreeCopySubtree(Tree* src, const int node) {
    TREE_VERIFY(src);

    int subtreeSz = TreeCountSubtree(src, node);
    if (subtreeSz == -1) {
        printf("[TreeCopySubtree] can't count elements to copy subtree!\n");
        return NULL;
    }

    //printf("size: %d\n", subtreeSz);

    Node* dst = calloc(subtreeSz, sizeof(Node));
    int pos = 0;

    if (_TreeCopySubtree(src, dst, node, &pos) == -1)
        return NULL;

    #ifdef LOG_ON
    if (TreeUpdLog(src, "TreeCopySubTree") != 0)
        src->err = E_LOG_DEAD;
    #endif

    TREE_VERIFY(src);

    return dst;
}

int _TreeCopySubtree(Tree* src, Node* dst, const int node, int* pos) {
    int branchL = (src->nodes[node]).branch[Left];
    int branchR = (src->nodes[node]).branch[Right];

    dst[*pos] = src->nodes[node];
    (*pos)++;

    //printf("  [_TreeCopySubTree] inserted node %d of original tree at pos %d\n", node, *pos - 1);

    if (branchL != 0)
        if (_TreeCopySubtree(src, dst, branchL, pos) == -1)
            return -1;

    if (branchR != 0)
        if (_TreeCopySubtree(src, dst, branchR, pos) == -1)
            return -1; 

    return 0;
}

int TreeDeleteNode(Tree* tree, const int node) {
    TREE_VERIFY(tree);

    if (node == 0) {
        printf("[TreeDeleteNode] WARNING: deleting empty node!\n");

        return 0;
    }
    
    if (_TreeDeleteNode(tree, node) == -1) {
        printf("[TreeDeleteNode] can't delete node and it's subtree!\n");
        return -1;
    } 

    /*
    while (tree->cur < tree->max - DELTA && tree->max / 2 >= DELTA) {
        if (TreeResize(tree, tree->max / 2) == -1) {
            printf("[TreeDeleteNode] error while shrinking during delete!\n");
            return -1;
        }
    }
    */

    #ifdef LOG_ON
    if (TreeUpdLog(tree, "TreeDeleteNode") != 0)
        tree->err = E_LOG_DEAD;
    #endif

    #ifdef SEC_ON
    tree->hash = TreeGetHash(tree);
    #endif

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
    } else {
        int parent = (tree->nodes[node]).parent;
        int branchL = (tree->nodes[node]).branch[Left];
        int branchR = (tree->nodes[node]).branch[Right];

        //printf("%d <%d> (%d %d)\n", node, parent, branchL, branchR);

        if (branchL != 0)
            if (_TreeDeleteNode(tree, branchL) == -1)
                return -1;

        if (branchR != 0)
            if (_TreeDeleteNode(tree, branchR) == -1)
                return -1;
        
        if ((tree->nodes[node]).parent != 0 || node == tree->root)
            tree->cur--;

        if ((tree->nodes[parent]).branch[Left] == node)
            (tree->nodes[parent]).branch[Left] = 0;
        else if ((tree->nodes[parent]).branch[Right] == node)
            (tree->nodes[parent]).branch[Right] = 0;
        
        (tree->nodes[node]).parent = 0;

        tree->memmap[node] = tree->free;
        tree->free = node;
    }
    
    return 0;
}

int TreeChangeNode(Tree* tree, const int node, int* parentNew, int* branchLNew, int* branchRNew, data* dataNew) {
    TREE_VERIFY(tree);

    int branchL = (tree->nodes[node]).branch[Left];
    int branchR = (tree->nodes[node]).branch[Right];

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

    int res = -1;
    switch (tree->header.type) {
        case Txt:  res = TreeChangeNodeTxtHandler((struct _TreeTxt*)tree, node, (char**)dataNew); break;
        case Int:  res = TreeChangeNodeIntHandler((struct _TreeInt*)tree, node, (int*)dataNew); break;
        case Math: res = TreeChangeNodeMathHandler((struct _TreeMath*)tree, node, (MathData*)dataNew); break;
        default: printf("[NodeInsertNode] invalid type!\n"); return -1;
    }

    #ifdef LOG_ON
    if (TreeUpdLog(tree, "TreeChangeNode") != 0)
        tree->err = E_LOG_DEAD;
    #endif

    #ifdef SEC_ON
    tree->hash = TreeGetHash(tree);
    #endif

    TREE_VERIFY(tree);

    return res;
}

int TreeChangeNodeTxtHandler(struct _TreeTxt* tree, const int node, char** dataNew) {
    if (dataNew != NULL) {
        if (strlen(*dataNew) >= TEXT_MAX_SZ) {
            printf("[TreeChangeNode] too long!\n");
            return -1;
        } else {
            memset((tree->nodes[node]).data, '\0', TEXT_MAX_SZ);
            memcpy((tree->nodes[node]).data, *dataNew, strlen(*dataNew));
        }
    }

    return 0;
}

int TreeChangeNodeIntHandler(struct _TreeInt* tree, const int node, int* dataNew) {
    if (dataNew != NULL)
        (tree->nodes[node]).data = *dataNew;

    return 0;
}

int TreeChangeNodeMathHandler(struct _TreeMath* tree, const int node, MathData* dataNew) {
    if (dataNew != NULL) {
        (tree->nodes[node]).data.value = dataNew->value;
        (tree->nodes[node]).data.type = dataNew->type;
    }

    return 0;
}

int TreeGlueSubtree(Tree* tree, Node* subtree, int node, int branch, int nodesCount) {
    TREE_VERIFY(tree);

    //printf("nodesCount: %d\n", nodesCount);

    if (tree->nodes[node].branch[branch] != 0) {
        printf("[TreeGlueSubtree] branch is not empty!\n");
        return -1;
    } else if (tree->nodes[node].parent == 0 && node != tree->root) {
        printf("[TreeGlueSubtree] node: %d, root: %d", node , tree->root);
        printf("[TreeGlueSubtree] invalid node to glue to!\n");
        return -1;
    } 
    /*
    while (nodesCount >= tree->max - tree->cur) {
        TreeResize(tree, tree->max * 2);
    }
    */
    int index = 0;

    if (_TreeGlueSubtree(tree, subtree, &index, nodesCount, node, branch)) {
        printf("[TreeGlueSubtree] executed with errors!\n");
        TreeDeleteNode(tree, tree->nodes[node].branch[branch]);
        return -1;
    }

    #ifdef LOG_ON
    TreeUpdLog(tree, "TreeGlueSubtree");
    #endif

    #ifdef SEC_ON
    tree->hash = TreeGetHash(tree);
    #endif

    TREE_VERIFY(tree);

    return 0;
}

int _TreeGlueSubtree(Tree* tree, Node* subtree, int* index, int nodesCount, int node, int branch) {    
    if (TreeInsertNode(tree, node, branch, &(subtree[*index].data)) == -1) {
        printf("  [_TreeGlueSubtree] can't insert %d node's %d branch!\n", node, branch);
        return -1;
    }

    int this = tree->nodes[node].branch[branch];
    
    //printf("[_TreeGlueSubtree] glued index %d to addr %d \n"
    //       "                   as %d branch of %d node of original tree\n", *index, this, branch,  node);
    
    int localAddr = *index;

    if (*index < nodesCount && subtree[localAddr].branch[Left] != 0) {
        (*index)++;

        if (_TreeGlueSubtree(tree, subtree, index, nodesCount, this, Left) == -1)
            return -1;
    } else {
        //printf("index: %d count: %d subtree[localAddr].branch[Left] == %d\n", *index, nodesCount, subtree[localAddr].branch[Left]);
    }

    if (*index < nodesCount && subtree[localAddr].branch[Right] != 0) {
        (*index)++;

        if (_TreeGlueSubtree(tree, subtree, index, nodesCount, this, Right) == -1)
            return -1;
    } else {
        //printf("index: %d count: %d subtree[localAddr].branch[Left] == %d\n", *index, nodesCount, subtree[localAddr].branch[Left]);
    }

    return 0;
}


Tree* TreeRead(const char* pathname, int type) {
    Tree* tree = TreeAlloc();

    if (TREE_INIT(tree, type) == -1) {
        printf("[TreeRead] initialzation error! returning nullptr...\n");
        return NULL;
    }

    int fd = open(pathname, O_RDONLY);
    if (fd == -1) {
        printf("[TreeRead] can't open mentioned file!\n");
        return NULL;
    }

    int len = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    char* s = calloc(len, sizeof(char));

    if (read(fd, s, len) != len) {
        printf("[TreeRead] can't read file to buffer!\n");
        return NULL;
    }

    close(fd);

    if (_TreeRead(tree, &s) == -1) {
        printf("[TreeRead] error while rading!\n");
        return NULL;
    }

    #ifdef SEC_ON
    tree->hash = TreeGetHash(tree);
    #endif

    TREE_VERIFY(tree);

    return tree;
}

int _TreeRead(Tree* tree, char** s) {
    int res = TreeGetGr(s, tree, 0, 0);

    if (res == -1) {
        printf("  [_TreeRead] can't read!\n");
        return -1;
    }

    return 0;
}

int TreeGetGr(char** s, Tree* tree, const int parent, const int branch) {
    //printf("[TreeTxtGetGr] <%c>\n", **s);

    int val = 0;

    GetSpace(s);

    switch (tree->header.type) {
        case Txt:  val = TreeTxtGetNode(s, (struct _TreeTxt*)tree, parent, branch); break;
        case Int:  val = TreeIntGetNode(s, (struct _TreeInt*)tree, parent, branch); break;
        case Math: val = TreeMathGetNode(s, (struct _TreeMath*)tree, parent, branch); break;
        default: printf("[NodeInsertNode] invalid type!\n"); return -1;
    }

    if (val == -1) {
        printf("[TreeGetGr] couldn't read nodes\n");
    }

    GetSpace(s);

    if (**s != '\0' && **s != '\n' && **s != EOF) {
        printf("inv char: <%c>\n", **s);
        printf("[TreeGetGr] syntax error! no end of str detected!\n");
        return -1;
    } else {
        return val;
    }
}

int TreeTxtGetNode(char** s, struct _TreeTxt* tree, const int parent, const int branch) {
    //printf("[TreeTxtGetNode] <%c>\n", **s);
    
    int node = 0;

    int len = GetString(s);

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

        node = TreeInsertNode(tree, parent, branch, &str);
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

        if (TreeTxtGetNode(s, tree, node, Left) == -1)
            return -1;

        GetSpace(s);

        if (**s != ',')
            return -1;
        else 
            (*s)++;
        
        GetSpace(s);

        if (TreeTxtGetNode(s, tree, node, Right) == -1)
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


int TreeIntGetNode(char** s, struct _TreeInt* tree, const int parent, const int branch) {
    //printf("[GetNode] <%c>\n", **s);
    
    int val = 0;
    int node = 0;

    val = GetNumber(s);

    if (val == -1) {
        if (**s != '#') {
            return -1;
        } else {
            (*s)++;
            return 0;
        }
    } else {
        node = TreeInsertNode((Tree*)tree, parent, branch, &val);
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

        if (TreeIntGetNode(s, tree, node, Left) == -1)
            return -1;

        GetSpace(s);

        if (**s != ',')
            return -1;
        else 
            (*s)++;
        
        GetSpace(s);

        if (TreeIntGetNode(s, tree, node, Right) == -1)
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

int TreeMathGetNode(char** s, struct _TreeMath* tree, const int parent, const int branch) {
    MathData val = {};
    int node = 0;

    if (**s != '[')
        return -1;
    else 
        (*s)++;

    val.type = GetNumber(s);

    GetSpace(s);

    if (**s != ',')
        return -1;
    else 
        (*s)++;

    GetSpace(s);

    val.value = GetNumber(s);

    if (**s != ']')
        return -1;
    else 
        (*s)++;

    if (val.type == -1 || val.type == -1) {
        if (**s != '#') {
            return -1;
        } else {
            (*s)++;
            return 0;
        }
    } else {
        node = TreeInsertNode((Tree*)tree, parent, branch, &val);
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

        if (TreeMathGetNode(s, tree, node, Left) == -1)
            return -1;

        GetSpace(s);

        if (**s != ',')
            return -1;
        else 
            (*s)++;
        
        GetSpace(s);

        if (TreeMathGetNode(s, tree, node, Right) == -1)
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

int GetString(char** s) {
    char* str = *s;

    if (**s == '\"') {
        (*s)++;

        while (**s != '\"' && (isalnum(**s) != 0 || isblank(**s) != 0 || ispunct(**s) != 0)) {
            (*s)++;
        }

        if (**s != '\"') {
            printf("[TreeTxtGetString] no closing quote detected!\n");
            return -1;
        } else {
            (*s)++;
        }
    } else {
        return -1;
    }
    return *s - str - 1;
}

int GetSpace(char** s) {
    while (isblank(**s) != 0) {
        (*s)++;
    }

    return 0;
}

int GetNumber(char** s) {
    //printf("[GetNumber] <%c>\n", **s);

    int val = 0;

    if (isdigit(**s) != 0) {
        val = val * 10 + (**s - '0');
		(*s)++;
    } else {
        printf("[GetNumber] no number was read!\n");
        return -1;
    }

	while (isdigit(**s)) {
		val = val * 10 + (**s - '0');
		(*s)++;
    }

	return val;
}

int TreeWrite(Tree* tree, const char* pathname) {
    TREE_VERIFY(tree);

    int fdout = open(pathname, O_WRONLY | O_CREAT | O_TRUNC);
    if (fdout == -1) {
        printf("[TreeWrite] can't open file for writing!\n");
        perror("msg: \n");
        return -1;
    }

    if (tree->root == 0) {
        printf("[TreeWrite] empty tree given!\n");
        dprintf(fdout, "#\n");
        return 0;
    } else if (_TreeWrite(tree, tree->root, fdout) == -1) {
        printf("[TreeWrite] failed\n");
        return -1;
    }

    dprintf(fdout, "\n");

    close(fdout);

    return 0;
}

int _TreeWrite(Tree* tree, int node, int fd) {
    switch (tree->header.type) {
        case Txt:  _TreeWriteTxtHandler((struct _TreeTxt*)tree, node, fd); break;
        case Int:  _TreeWriteIntHandler((struct _TreeInt*)tree, node, fd); break;
        case Math: _TreeWriteMathHandler((struct _TreeMath*)tree, node, fd); break;
        default: printf("[NodeInsertNode] invalid type!\n"); return -1;
    }

    if (tree->nodes[node].branch[Left] != 0 || tree->nodes[node].branch[Right] != 0) {
        dprintf(fd, "(");

        if (tree->nodes[node].branch[Left] != 0) {
            _TreeWrite(tree, tree->nodes[node].branch[Left], fd);
        } else {
            dprintf(fd, "#");
        }

        dprintf(fd, ", ");

        if (tree->nodes[node].branch[Right] != 0) {
            _TreeWrite(tree, tree->nodes[node].branch[Right], fd);
        } else {
            dprintf(fd, "#");
        }

        dprintf(fd, ")");
    }

    return 0;
}

void _TreeWriteTxtHandler(struct _TreeTxt* tree, const int node, const int fd) {
    dprintf(fd, "\"%s\"", tree->nodes[node].data);

    return;
}

void _TreeWriteIntHandler(struct _TreeInt* tree, const int node, const int fd) {
    dprintf(fd, "%d", tree->nodes[node].data);

    return;
}

void _TreeWriteMathHandler(struct _TreeMath* tree, const int node, const int fd) {
    dprintf(fd, "[%d, %d]", tree->nodes[node].data.type, tree->nodes[node].data.value);

    return;
}


int TreeVerify(Tree* tree) {
    if (tree == NULL) {
        printf("[TreeVerify] tree is nullptr!\n");
        return -1;
    }

    #ifdef SEC_ON
    if (tree->hash != TreeGetHash(tree)) {
        tree->err = E_HASH_CORRUPTED;
        return tree->err;
    }
    #endif

    if (tree->cur > tree->max || tree->cur < 0 || tree->max < 0) {
        tree->err = E_SIZE_INVALID;
        return tree->err;
    }

    return tree->err;
}

#ifdef SEC_ON
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

        if (tree->nodes != NULL) {
            switch (tree->header.type) {
                case Txt:  TreeUpdLogTxtHandler((struct _TreeTxt*)tree, dumpFd); break;
                case Int:  TreeUpdLogIntHandler((struct _TreeInt*)tree, dumpFd); break;
                case Math: TreeUpdLogMathHandler((struct _TreeMath*)tree, dumpFd); break;
                default: printf("[NodeDump] invalid type!\n"); return;
            }
        }
    }

    close(dumpFd);
}

int TreeDumpTxtHandler(struct _TreeTxt* tree, const int dumpFd) {
    for (int i = 0; i < tree->max; i++)
        dprintf(dumpFd, "  [%3d] (%s [%3d, %3d] <%3d>)\n", i, tree->nodes[i].data, tree->nodes[i].branch[Left], tree->nodes[i].branch[Right], tree->nodes[i].parent);

    return 0;
}

int TreeDumpIntHandler(struct _TreeInt* tree, const int dumpFd) {
    for (int i = 0; i < tree->max; i++)
        dprintf(dumpFd, "  [%3d] (%3d [%3d, %3d] <%3d>)\n", i, tree->nodes[i].data, tree->nodes[i].branch[Left], tree->nodes[i].branch[Right], tree->nodes[i].parent);
    
    return 0;
}

int TreeDumpMathHandler(struct _TreeMath* tree, const int dumpFd) {
    for (int i = 0; i < tree->max; i++)
        dprintf(dumpFd, "  [%3d] (type: %d value: %3d [%3d, %3d] <%3d>)\n", i, tree->nodes[i].data.type, tree->nodes[i].data.value, tree->nodes[i].branch[Left], tree->nodes[i].branch[Right], tree->nodes[i].parent);
    
    return 0;
}

long TreeGetHash(Tree* tree) {
    if (tree == NULL) {
        printf("[TreeGetHash] nullptr given! returning -1\n");
        return -1;
    } else {
        return 100;
    }

    return -1;
}
#endif


#ifdef LOG_ON
int TreeUpdLog(Tree* tree, const char* func) {
    char* timeStamp = GetTimestamp();
    int res = 0;

    #ifdef SEC_ON
    res = dprintf(tree->logfd, "%s\n"
                               "passed through function [[%s]]\n"
                               "  hash: %ld\n"
                               "  tree capacity: %d\n"
                               "  tree size: %d\n"
                               "  first free element's physical address: %d\n"
                               "  root address: %d\n"
                               "  error: %d %s\n",
                               timeStamp, func, tree->hash, tree->max, tree->cur, tree->free, tree->root, tree->err, TreeErrsDesc[tree->err]);
    #else 
    res = dprintf(tree->logfd, "%s\n"
                               "passed through function [[%s]]\n"
                               "  tree capacity: %d\n"
                               "  tree size: %d\n"
                               "  first free element's physical address: %d\n"
                               "  root address: %d\n"
                               "  error: %d %s\n",
                               timeStamp, func, tree->max, tree->cur, tree->free, tree->root, tree->err, TreeErrsDesc[tree->err]);
    #endif
    free(timeStamp);

    if (res == 0)
        return -1;

    switch (tree->header.type) {
        case Txt:  TreeUpdLogTxtHandler((struct _TreeTxt*)tree, tree->logfd); break;
        case Int:  TreeUpdLogIntHandler((struct _TreeInt*)tree, tree->logfd); break;
        case Math: TreeUpdLogMathHandler((struct _TreeMath*)tree, tree->logfd); break; 
        default: printf("[NodeInsertNode] invalid type!\n"); return -1;
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

int TreeUpdLogTxtHandler(struct _TreeTxt* tree, const int fd) {
    int res = 0;

    res = dprintf(tree->logfd, "  |  &|                              val| &L| &R|  &par|\n");
    for (int i = 0; i < tree->max; i++) {
        if (((tree->nodes[i]).parent == 0 && i != tree->root) || tree->cur == 0)
            res = dprintf(tree->logfd, "  [%2d ] (%30s [ %2d, %2d] <%3d>)\n", i, tree->nodes[i].data, tree->nodes[i].branch[Left], tree->nodes[i].branch[Right], tree->nodes[i].parent);
        else 
            res = dprintf(tree->logfd, "  [%2d*] (%30s [ %2d, %2d] <%3d>)\n", i, tree->nodes[i].data, tree->nodes[i].branch[Left], tree->nodes[i].branch[Right], tree->nodes[i].parent);
    }

    return res;
}

int TreeUpdLogIntHandler(struct _TreeInt* tree, const int fd) {
    int res = 0;

    res = dprintf(tree->logfd, "  | &|   val| valL| &L| valR| &R|  &par|\n");
    for (int i = 0; i < tree->max; i++) {
        if (((tree->nodes[i]).parent == 0 && i != tree->root) || tree->cur == 0)
            res = dprintf(tree->logfd, "  [%2d ] (%3d [%3d <%2d>, %3d <%2d>] <%3d>)\n", i, tree->nodes[i].data, tree->nodes[tree->nodes[i].branch[Left]].data, tree->nodes[i].branch[Left], tree->nodes[tree->nodes[i].branch[Right]].data, tree->nodes[i].branch[Right], tree->nodes[i].parent);
        else 
            res = dprintf(tree->logfd, "  [%2d*] (%3d [%3d <%2d>, %3d <%2d>] <%3d>)\n", i, tree->nodes[i].data, tree->nodes[tree->nodes[i].branch[Left]].data, tree->nodes[i].branch[Left], tree->nodes[tree->nodes[i].branch[Right]].data, tree->nodes[i].branch[Right], tree->nodes[i].parent);
    }

    return res;
}

int TreeUpdLogMathHandler(struct _TreeMath* tree, const int fd) {
    int res = 0;

    res = dprintf(tree->logfd, "  | &|   val|               &L|  &R|  &par|\n");
    for (int i = 0; i < tree->max; i++) {
        if (((tree->nodes[i]).parent == 0 && i != tree->root) || tree->cur == 0)
            res = dprintf(tree->logfd, "  [%2d ] (type: %d value: %6d [%3d, %3d] <%3d>)\n", i, tree->nodes[i].data.type, tree->nodes[i].data.value, tree->nodes[i].branch[Left], tree->nodes[i].branch[Right], tree->nodes[i].parent);
        else 
            res = dprintf(tree->logfd, "  [%2d*] (type: %d value: %6d [%3d, %3d] <%3d>)\n", i, tree->nodes[i].data.type, tree->nodes[i].data.value, tree->nodes[i].branch[Left], tree->nodes[i].branch[Right], tree->nodes[i].parent);
    }

    return res;
}
#endif

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
