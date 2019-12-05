#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tree.h"

//####################//


//####################//

int Akinator(TreeTxt* tree);
int _Akinator(TreeTxt* tree, int node);
int AkinatorUpdTree(TreeTxt* tree, int branchAddr);

//####################//

int main() {
    TreeTxt* tree = TreeTxtRead("./input/database.txt");

    if (tree == NULL) {
        printf("error while reading!\n");
        exit(-1);
    }

    Akinator(tree);

    TreeTxtWrite(tree, "input/database.txt");

    TreeTxtSort(tree);

    TreeTxtFree(tree);

    return 0;
}

//####################//

int Akinator(TreeTxt* tree) {
    printf("/////////////////////////////////////////////////////////////////////\n"
           "// Welcome to Akinator(c) v.alpha.03-LeGuinn by AI_gang            //\n"
           "// Just let this wander of machinery and human wit guess your word //\n"
           "/////////////////////////////////////////////////////////////////////\n\n");

    int res = _Akinator(tree, TreeTxtGetRoot(tree));

    return res;
}

int _Akinator(TreeTxt* tree, int node) {
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

int AkinatorUpdTree(TreeTxt* tree, int branchAddr) {
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

    TreeTxtChangeNode(tree, branchAddr, NULL, NULL, NULL, question);
    TreeTxtInsertNode(tree, branchAddr, left, answer);
    TreeTxtInsertNode(tree, branchAddr, right, answerPrev);

    free(answer);
    free(answerPrev);
    free(question);

    printf("[AKINATOR] This time you won, but now I became stronger!\n");

    return -1;
}

//####################//