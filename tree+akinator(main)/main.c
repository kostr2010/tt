#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//####################//

#include "tree.h"

//####################//

int Akinator(struct _TreeTxt* tree);
int _Akinator(struct _TreeTxt* tree, int node);
int AkinatorUpdTree(struct _TreeTxt* tree, int branchAddr);
int Define(Tree* tree, const char* object);
int _Define(Tree* tree, const int node);
int Compare(Tree* tree, const char* src, const char* dst);
int _Compare(Tree* tree, const int node, const int len1, char* path1, const int len2, char* path2, const int depth);
int GetPath(Tree* tree, int node, char* buf, int counter);

//####################//

int main() {
    struct _TreeTxt* tree = TreeRead("./input/database.txt", Txt);

    if (tree == NULL) {
        printf("error while reading!\n");
        exit(-1);
    }

    Akinator(tree);
    
    TreeWrite(tree, "input/database.txt");

    //Akinator(tree);

    Define(tree, "pepelatz");
    Define(tree, "plane");
    Define(tree, "butterfly");

    Compare(tree, "pepelatz", "plane");
    
    TreeSort(tree);
    
    TreeFree(tree);
    
    return 0;
}

//####################//

int Akinator(struct _TreeTxt* tree) {
    TREE_VERIFY(tree);

    printf("/////////////////////////////////////////////////////////////////////\n"
           "// Welcome to Akinator(c) v.alpha.03-LeGuinn by AI_gang            //\n"
           "// Just let this wander of machinery and human wit guess your word //\n"
           "/////////////////////////////////////////////////////////////////////\n\n");

    int res = _Akinator(tree, TreeGetRoot(tree));

    #ifdef SEC_ON
    tree->hash = TreeGetHash(tree);
    #endif

    TREE_VERIFY(tree);

    return res;
}

int _Akinator(struct _TreeTxt* tree, int node) {
    TREE_VERIFY(tree);

    int branchL = tree->nodes[node].branch[left];
    int branchR = tree->nodes[node].branch[right];

    printf("[AKINATOR] %s?\ny/n\n", tree->nodes[node].data);

    char ans;
    scanf("%c", &ans);
    while(getchar() != '\n');

    if (ans == 'y' 
        && (tree->nodes[branchL].branch[left] != 0 
        && tree->nodes[branchL].branch[right] != 0)) {
        _Akinator(tree, branchL);
    } else if (ans == 'n' 
               && (tree->nodes[branchR].branch[left] != 0 
               && tree->nodes[branchR].branch[right] != 0)) {
        _Akinator(tree, branchR);
    } else if (ans == 'y' 
               && (tree->nodes[branchL].branch[left] == 0 
               && tree->nodes[branchL].branch[right] == 0)) {
        printf("[AKINATOR] Is it <%s>?\ny/n\n", tree->nodes[branchL].data);
            
        scanf("%c", &ans);
        while(getchar() != '\n');

        if (ans == 'y') {
            printf("[AKINATOR] I won, as it was expected!\n");
            return 0;
        } else {
            return AkinatorUpdTree(tree, branchL);
        }
    } else if (ans == 'n' 
               && (tree->nodes[branchR].branch[left] == 0 
               && tree->nodes[branchR].branch[right] == 0)) {
        printf("[AKINATOR] Is it <%s>?\ny/n\n", tree->nodes[branchR].data);

        scanf("%c", &ans);
        while(getchar() != '\n');

        if (ans == 'y') {
            printf("[AKINATOR] I won, as it was expected!\n");
            return 0;
        } else {
            return AkinatorUpdTree(tree, branchR);
        }
    } 

    return 0;
}

int AkinatorUpdTree(struct _TreeTxt* tree, int branchAddr) {
    TREE_VERIFY(tree);

    printf("[AKINATOR] So tell me, what is this?\n");

    char* answer = calloc(40, sizeof(char));
    read(fileno(stdin), answer, 40);
    *(strchr(answer, '\n')) = '\0';

    printf("[AKINATOR] What is true for your answer and false for mine?\n");

    char* question = calloc(40, sizeof(char));
    read(fileno(stdin), question, 40);
    *(strchr(question, '\n')) = '\0';
    
    char* answerPrev = calloc(40, sizeof(char));

    memcpy(answerPrev, tree->nodes[branchAddr].data, 40);

    TreeChangeNode(tree, branchAddr, NULL, NULL, NULL, &question);
    TreeInsertNode(tree, branchAddr, left, answer);
    TreeInsertNode(tree, branchAddr, right, answerPrev);

    free(answer);
    free(answerPrev);
    free(question);

    TREE_VERIFY(tree);

    printf("[AKINATOR] This time you won, but now I became stronger!\n");

    return -1;
}

int Define(Tree* tree, const char* object) {
    TREE_VERIFY(tree);

    int pos = -1;
    if ((pos = TreeFind(tree, TreeGetRoot(tree), object)) == -1) {
        printf("[Define] no such word as <%s> in akinator's memory!\n", object);
        return -1;
    } else {
        printf("[Define] %s is something that is...\n");
        _Define(tree, pos);
    }

    return 0;
}

int _Define(Tree* tree, const int node) {
    sleep(1);

    int parent = tree->nodes[node].parent;

    if (parent == 0) {
        return 0;
    } else if (node == tree->nodes[parent].branch[right]) {
        printf("  NOT %s\n", tree->nodes[parent].data);
        _Define(tree, parent);
    } else if (node == tree->nodes[parent].branch[left]) {
        printf("  %s\n", tree->nodes[parent].data);
        _Define(tree, parent);
    }

    return 0;
}

int Compare(Tree* tree, const char* src, const char* dst) {
    TREE_VERIFY(tree);

    if (strcmp(src, dst) == 0) {
        printf("[Compare] these are the same things!\n");
        return 0;
    }

    char* pathSrc = calloc(TreeCountSubtree(tree, TreeGetRoot(tree)), sizeof(char));
    char* pathDst = calloc(TreeCountSubtree(tree, TreeGetRoot(tree)), sizeof(char));

    int posSrc = -1;
    if ((posSrc = TreeFind(tree, TreeGetRoot(tree), src)) == -1) {
        printf("[Compare] no such word as %s in akinator's memory!\n", src);
        return -1;
    }
    
    int posDst = -1;
    if ((posDst = TreeFind(tree, TreeGetRoot(tree), dst)) == -1) {
        printf("[compare] no such word as %s in akinator's memory!\n", dst);
        return -1;
    }

    GetPath(tree, posSrc, pathSrc, 0);
    GetPath(tree, posDst, pathDst, 0);

    int lenSrc = strlen(pathSrc);
    int lenDst = strlen(pathDst);

    //printf("%d, %s | %d, %s\n", lenSrc, pathSrc, lenDst, pathDst);

    printf("[Compare] comparing <%s> and <%s>, they...\n", src, dst);
    
    _Compare(tree, TreeGetRoot(tree), lenSrc, pathSrc, lenDst, pathDst, 1);
    
    return 0;
}

int _Compare(Tree* tree, const int node, const int len1, char* path1, const int len2, char* path2, const int depth) {
    //printf("depth %d, %c %c\n", depth, path1[len1 - depth], path2[len2 - depth]);
    sleep(1);

    if (depth > len1 || depth > len2) {
        return 0;
    } else if (path1[len1 - depth] == path2[len2 - depth]) {
        int branch = (path1[len1 - depth] == 'l') ? (left) : (right);
        //printf("%d\n", branch);
        branch = tree->nodes[node].branch[branch];

        //printf("%d\n", branch);

        printf("[Compare] are both ");
        
        if (path1[len1 - depth] == 'r' && path2[len2 - depth] == 'r') {
            printf("not");
        }

        printf("<%s>\n", tree->nodes[node].data);

        _Compare(tree, branch, len1, path1, len2, path2, depth + 1);
    } else if (path1[len1 - depth] == 'r' && path2[len2 - depth] == 'l') {
        printf("[Compare] but first is <%s> while second is not\n", tree->nodes[node].data);
    } else if (path1[len1 - depth] == 'l' && path2[len2 - depth] == 'r') {
        printf("[Compare] but first is not <%s> while second is\n", tree->nodes[node].data);
    }

    return 0;
}

int GetPath(Tree* tree, int node, char* buf, int counter) {
    int parent = tree->nodes[node].parent;

    if (parent != 0) {
        if (node == tree->nodes[parent].branch[left]) {
            buf[counter] = 'l';
            //buf[counter + 1] = parent; 
        } else {
            buf[counter] = 'r';
            //buf[counter + 1] = parent;
        }
        //counter++;
        
        GetPath(tree, parent, buf, counter + 1);        
    }

    return 0;
}

//####################//