#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

#include "stack.h"
#include "consts.h"

//####################//

#ifdef SEC_ON
#undef SEC_ON
#endif

//####################//

enum _MemoryErrs {
    OK,
    E_CORRUPTED_BIN,
    E_FEW_ARGS_IN_STACK,
    E_INV_ARG_IN_STACK,
    E_INIT_ERR,
    E_RET_WITHOUT_CALL,
    E_INV_INPUT,
};
typedef enum _MemoryErrs MemErrs;

const char* MemErrsDesc[] = {
    "executed normally",
    "binary file corrupted or incorrect!\n",
    "too few arguments in stack to perform this operation!\n",
    "invalid argument in stack for this operation!\n",
    "unable to initialize memory for execution!\n",
    "return read, but no call was made!\n",
    "invalid input given!\n"
};

struct _Memory {
    char* cmds;
    int maxCmd;
    int curCmd;
    int* regs; // ax, bx, cd, dx, ex (null reg)
    Stack_t* stk;
    Stack_t* ret;
    MemErrs err;
};
typedef struct _Memory Memory;

//####################//

Memory* MemoryAlloc();
int MemoryInit(Memory* mem, int sz);
int MemoryFree(Memory* mem);

int Interpret(char* name);

//####################//

int main(int argc, char** argv) {
    int res = Interpret(argv[1]);
    if (res != 0) {
        printf("error while executing %s ! %s\n", argv[1], MemErrsDesc[res]);
        exit(res);
    }

    /*
    Stack_t* st = StackAlloc();
    STACK_INIT(st, 10);
    
    for (int i = 0; i < 10; i++) {
        printf("pushing %d...\n", i);
        StackPush(st, i);
    }

    for (int i = 0; i < 11; i++) {
        printf("popping %d...\n", StackPeek(st));
        StackPop(st);
    }

    StackFree(st);
    */

    return 0;
}

//####################//

Memory* MemoryAlloc() {
    Memory* mem = calloc(1, sizeof(Memory));

    return mem;
}

int MemoryInit(Memory* mem, int sz) {
    assert(mem);
    
    mem->cmds = calloc(sz, sizeof(char));
    mem->maxCmd = sz;
    mem->curCmd = 0;
    mem->regs = calloc(5, sizeof(int));
    mem->stk = StackAlloc();
    _StackConstruct(mem->stk, "stk", 10);
    mem->ret = StackAlloc();
    _StackConstruct(mem->ret, "ret", 10);
    mem->err = OK;

    if (mem->cmds == NULL || mem->ret == NULL)
        return 1;

    return 0;
}

int MemoryFree(Memory* mem) {
    assert(mem);
    assert(mem->cmds);
    assert(mem->regs);
    assert(mem->ret);
    assert(mem->stk);

    free(mem->cmds);
    free(mem->regs);
    StackFree(mem->ret);
    StackFree(mem->stk);
    free(mem);

    return 0;
}

int Interpret(char* name) {
    int fd = open(name, O_RDONLY);
    int len = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    Memory* mem = MemoryAlloc();
    if (mem == NULL || MemoryInit(mem, len) != 0) {
        mem->err = E_INIT_ERR;
        return mem->err;        
    }

    read(fd, mem->cmds, len);
    close(fd);
    
    while (mem->curCmd < mem->maxCmd) {
        printf("%d, %d, %d, %d, %d\n", mem->regs[0], mem->regs[1], mem->regs[2], mem->regs[3], mem->regs[4]);

        switch (mem->cmds[mem->curCmd]) {
        #define CMD_DEF(name, num, codeAsm, codeBin)\
        case num:\
            printf("read %s at addr: %d\n", name, mem->curCmd);\
            mem->curCmd += CMD_SZ;\
            codeBin;\
            break;
        #include "cmds.h"

        default:
            mem->err = E_CORRUPTED_BIN;
            return 1;
        }

    }

    MemoryFree(mem);

    return 0;
}
