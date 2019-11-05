#include "stack.h"

//####################//

int* ax, bx, cx, dx;

//####################//

Stack_t* StackAlloc() {
    Stack_t* st = calloc(1, sizeof(Stack_t));

    return st;
}

int _StackConstruct(Stack_t* st, const char* name, int max) {
    assert(st);
    assert(name);

    #ifdef SEC_ON
    st->canary1 = CANARY;
    #endif

    st->name    = calloc(1, sizeof(char) * strlen(name));

    if (st->name == NULL) {
        printf("couldn't allocate memory for stack name. try smaller one\n");
        return 1;
    }

    memcpy(st->name, name, strlen(name));

    st->max = max;
    st->cur = 0;

    #ifdef SEC_ON
    st->buf = calloc(1, 2 * sizeof(CANARY) + max * sizeof(data_t));
    #else
    st->buf = calloc(1, max * sizeof(data_t));
    #endif

    if (st->buf == NULL) {
        printf("couldn't allocate memory for stack buffer. try smaller max\n");
        return 1;
    }

    #ifdef SEC_ON
    *((long*)(st->buf)) = CANARY;
    *((long*)(st->buf + sizeof(CANARY) + max * sizeof(data_t))) = CANARY;

    for (int i = 0; i < max; i++)
        *((data_t*)(st->buf + sizeof(CANARY) + i * sizeof(data_t))) = POISON;

    st->err       = OK;
    st->canary2   = CANARY;
    st->hash      = _StackGetHash(st);

    char* logName = calloc(1, sizeof(char) * strlen(name) + 5);
    memcpy(logName, name, strlen(name)); 
    st->log = fopen(strcat(logName, ".log"), "w");
    fclose(st->log);
    st->log = fopen(logName, "a");
    free(logName);

    _StackLog(st, "_StackConstruct");

    STACK_VERIFY(st);
    #endif

    return 0;
}

int StackFree(Stack_t* st) {
    #ifdef SEC_ON
    STACK_VERIFY(st);

    if (fclose(st->log)) {
        printf("couldn't close log file. try crying on the floor\n");
        return 1;
    }
    #endif

    free(st->name);
    free(st->buf);
    free(st);

    return 0;
}

int StackResize(Stack_t* st, int newSize) {
    #ifdef SEC_ON
    STACK_VERIFY(st);
    #endif

    if (newSize < st->cur) {
        printf("invalid <newSize>!\n");
        return 1;
    }

    #ifdef SEC_ON
    st->buf = realloc(st->buf, 2 * sizeof(CANARY) + newSize * sizeof(data_t));
    #else
    st->buf = realloc(st->buf, newSize * sizeof(data_t));
    #endif

    if (st->buf == NULL) {
        printf("couldn't allocate memory for new stack buffer. try commiting suicide\n");
        return 1;
    }

    st->max = newSize;

    #ifdef SEC_ON
    for (int i = 0; st->cur + i < st->max; i++)
        *((data_t*)(st->buf + sizeof(CANARY) + (st->cur + i) * sizeof(data_t))) = POISON;

    *((long*)(st->buf + sizeof(CANARY) + st->max * sizeof(data_t))) = CANARY;
    st->hash = _StackGetHash(st);

    _StackLog(st, "StackResize");

    STACK_VERIFY(st);
    #endif

    return 0;
}

int StackPush(Stack_t* st, data_t data) {
    #ifdef SEC_ON
    STACK_VERIFY(st);
    #endif

    if (st->cur == st->max - 1) {
        if (StackResize(st, st->max * 2) != 0)
            return 1;
    }

    #ifdef SEC_ON
    *((data_t*)(st->buf + sizeof(CANARY) + st->cur * sizeof(data_t))) = data;
    #else
    *((data_t*)(st->buf + st->cur * sizeof(data_t))) = data;
    #endif

    (st->cur)++;
    
    #ifdef SEC_ON
    st->hash = _StackGetHash(st);

    _StackLog(st, "StackPush");

    STACK_VERIFY(st);
    #endif

    return 0;
}

int StackPop(Stack_t* st) {
    #ifdef SEC_ON
    STACK_VERIFY(st);
    #endif

    if (st->cur == 0) {
        printf("stack is already empty, unable to pop!\n");
        return -1;
    }

    if (st->cur < (st->max / 2) - OFF) {
        if (StackResize(st, st->max / 2) != 0)
            return 1;
    }

    (st->cur)--;

    #ifdef SEC_ON
    st->hash = _StackGetHash(st);

    _StackLog(st, "StackPop");

    STACK_VERIFY(st);
    #endif

    return 0;
}

data_t StackPeek(Stack_t* st) { //TODO: ifdef for SEC_ON
    #ifdef SEC_ON
    STACK_VERIFY(st);

    _StackLog(st, "StackPeek");

    if (st->cur != 0)
        return *((data_t*)(st->buf + sizeof(CANARY) + (st->cur - 1) * sizeof(data_t)));
    else
        return POISON;
    #else
    if (st->cur != 0)
        return *((data_t*)(st->buf + (st->cur - 1) * sizeof(data_t)));
    else
        return -1;
    #endif
}

#ifdef SEC_ON
int _StackOK(Stack_t* st) {
    if (st == NULL)
        return E_NULL_PTR_STACK;        
    
    if (st->buf == NULL) {
        st->err = E_NULL_PTR_BUF;
        return E_NULL_PTR_BUF;
    }

    if (st->max <= 0) {
        st->err = E_INVALID_MAX;
        return E_INVALID_MAX;
    }

    if (st->cur < 0 || st->cur > st->max) {
        st->err = E_INVALID_CUR;
        return E_INVALID_CUR;
    }

    if (_StackGetHash(st) != st->hash) {
        st->err = E_INVALID_HASH;
        return E_INVALID_HASH;
    }

    if (st->canary1 != CANARY) {
        st->err = E_CANARY1_DEAD;
        return E_CANARY1_DEAD;
    }

    if (st->canary2 != CANARY) {
        st->err = E_CANARY2_DEAD;
        return E_CANARY2_DEAD;
    }

    if (*((long*)(st->buf)) != CANARY) {
        st->err = E_CANARY1_BUF_DEAD;
        return E_CANARY1_BUF_DEAD;
    }

    if (*((long*)(st->buf + sizeof(CANARY) + st->max * sizeof(data_t))) != CANARY) {
        st->err = E_CANARY2_BUF_DEAD;
        return E_CANARY2_BUF_DEAD;
    }

    return OK;
}

void _StackLog(Stack_t* st, char* funcName) {
    STACK_VERIFY(st);

    fprintf(st->log, "went trhough [%s] function.\nnow stack <%s> looks like this:\nerrno = <%d> (%s)\n", funcName, st->name, st->err, StackErrsDesc[st->err]);

    for(int i = 0; i < st->max; i++) {
        if (i < st->cur)
            fprintf(st->log, "   *[%d] = %d\n", i, *((data_t*)(st->buf + 1 * sizeof(CANARY) + i * sizeof(data_t))));
        else
            fprintf(st->log, "    [%d] = %d\n", i, *((data_t*)(st->buf + 1 * sizeof(CANARY) + i * sizeof(data_t))));
    }

    fprintf(st->log, "\n####################\n\n");
}

void _StackDump(Stack_t* st) {
    
    FILE* fdump = fopen("dump.log", "w");
    fclose(fdump);
    fdump = fopen("dump.log", "a");

    if (st == NULL)
        fprintf(fdump, "!!!ERROR <1> (stack is pointed by nullptr)!!!\n"
                       "stack pointer == nullptr\n");
    else  if (st->err != OK) {
        fprintf(fdump, "!!!ERROR <%d> (%s)!!!\n"
                       "Stack_t <%s> [%p] {\n"
                       "  canary1          = %lx\n"
                       "  current filling  = %d\n"
                       "  maximum capacity = %d\n"
                       "  errno            = %d\n"
                       "  hash             = %ld\n"
                       "  canary2          = %lx\n"
                       "  buffer buf [%p]:\n",
                       st->err, 
                       StackErrsDesc[st->err], 
                       st->name, 
                       st, 
                       st->canary1, 
                       st->cur, 
                       st->max, 
                       st->err, 
                       st->hash, 
                       st->canary2, 
                       st->buf);

        for(int i = 0; i < st->max; i++) {
            if (i < st->cur)
                fprintf(fdump, "   *[%d] = %d\n", i, *((data_t*)(st->buf + 1 * sizeof(CANARY) + i * sizeof(data_t))));
            else
                fprintf(fdump, "    [%d] = %d\n", i, *((data_t*)(st->buf + 1 * sizeof(CANARY) + i * sizeof(data_t))));
        }

        fprintf(fdump, "}\n");

        fclose(st->log);
        free(st->name);
        free(st->buf);
        free(st);
    }

    fclose(fdump);
}

int _StackGetHash(Stack_t* st) {
    assert(st);
    assert(st->buf);

    int hash     = 0;
    int hash_buf = *((long*)(st->buf)) + st->max * *((long*)(st->buf + sizeof(CANARY) + (st->max + 1) * sizeof(data_t)));

    for (int i = 1; i <= st->max; i++)
        hash_buf += ((int)(*((data_t*)(st->buf + sizeof(CANARY) + (i - 1) * sizeof(data_t))))) * i;

    hash = st->canary1 + 2 * strlen(st->name) + 3 * st->max + 4 * st->cur + 5 * hash_buf + 6 * st->err + 7 * st->canary2;

    return hash;
}
#endif
