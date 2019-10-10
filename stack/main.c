#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

//####################//

#define SECURITY_FLAG 0                                                                                     // if set to 0, no security measures will be applied.
#define STACK_INIT(stack, max) _StackConstruct(stack, #stack, max)                                          // macros for Stack_t constructor.
#define STACK_VERIFY(stack) if (SECURITY_FLAG && _StackOK(stack) != 0) {_StackDump(stack); assert(!"ok");}  // macros for stack verification.
#define STACK_LOG(stack, name) if (SECURITY_FLAG) {_StackLog(stack, name);}                                 // macros for stack logging.
#define STACK_GET_HASH(stack) (SECURITY_FLAG) ? (_StackGetHash(stack)) : (0)

typedef int data_t;                             // stack's data type.

const long CANARY       = 0x1FEED5ADB16B00B5;   // canary value.
data_t POISON           = (data_t)(173);        // poisonous value.

// array of errors descriptions.
char* ErrDesc[] = {             
    "stack is OK",
    "stack is pointed by NULL",
    "buf is pointed by NULL",
    "cur is invalid (cur < 0 or cur > max)",
    "max is invalid (max <= 0)",
    "first stack canary is modified",
    "second stack canary is modified",
    "first buffer canary is modified",
    "second beffer canary is modified",
    "hash-sum has changed unexpectedly",
};

// stack errors.
enum Err {
    OK,                 // stack is valid;
    E_NULL_PTR_STACK,   // stack is pointed by NULL;
    E_NULL_PTR_BUF,     // buf is pointed by NULL;
    E_INVALID_CUR,      // cur is invalid;
    E_INVALID_MAX,      // max is invalid;
    E_CANARY1_DEAD,     // first stack canary is modified;
    E_CANARY2_DEAD,     // second stack canary is modified;
    E_CANARY1_BUF_DEAD, // first buffer canary is modified;
    E_CANARY2_BUF_DEAD, // second beffer canary is modified;
    E_INVALID_HASH,     // hash-sum has changed unexpectedly.
};

// stack structure.
struct _Stack_t {
    long canary1;       // first of canaries, surrounding structure;
    char* name;         // name of the stack. filled automatically during STACK_INIT call;
    int max;            // max capacity;
    int cur;            // current filling;
    char* buf;          // pointer to the buffer of stack;
    enum Err err;       // error, occured during programm. "OK" by default;
    long int hash;      // hash sum of structure;
    FILE* log;          // log file stream. to be opened in STACK_INIT and closed in StackFree function;
    long canary2;       // second of canaries, surrounding structure.
};
typedef struct _Stack_t Stack_t;

//####################//

/**
 * @brief StackAlloc function allocates memory for Stack_t structure. Should be used to initialize Stack_t* varible.
 * 
 * @return Stack_t* st - pointer to allocated chunk of memory, NULL if failed.
 * 
 * @note DO NOT FORGET TO CALL StackFree FUNCTION IN ORDER TO FREE THE STRUCTURE.
 */
Stack_t* StackAlloc();   

/**
 * @brief _StackConstruct function initializes fields of stack. not intended for user usage, only through STACK_INIT call.
 * 
 * @param Stack_t* st      - pointer to stack structure;
 * @param const char* name - name of th stack variable;
 * @param int max          - max capacity of stack.
 * 
 * @return int err         - 0 if executed correctly, 1 otherwise.
 * 
 * @note DO NOT FORGET TO CALL StackFree FUNCTION IN ORDER TO FREE THE STRUCTURE.
 */
int _StackConstruct(Stack_t* st, const char* name, int max);

/**
 * @brief StackFree function frees Stack_t structure.
 * 
 * @param Stack_t* st - pointer to stack structure.
 * 
 * @return int err    -  0 if executed correctly, 1 otherwise.
 */
int StackFree(Stack_t* st);

/**
 * @brief StackResize function changes size of buffer to newSize. unused memory will be filled with poison.
 * 
 * @param Stack_t* st - pointer to stack structure;
 * @param int newSize - new max capacity of buffer. should be greater than st->cur.
 * 
 * @return int err    -  0 if executed correctly, 1 otherwise.
 */
int StackResize(Stack_t* st, int newSize);

/**
 * @brief StackPush function pushes data value to the top of the stack. resizes buffer if it is full.
 * 
 * @param Stack_t* st - pointer to stack structure;
 * @param data_t data - value to push to the stack.
 * 
 * @return int err    -  0 if executed correctly, 1 otherwise.
 */
int StackPush(Stack_t* st, data_t data);

/**
 * @brief StackPop function marks top element of the stack as unused memory. resizes buffer if current filling is less than max/2 - 10.
 * 
 * @param Stack_t* st - pointer to stack structure.
 * 
 * @return int err    -  0 if executed correctly, 1 otherwise.
 */
int StackPop(Stack_t* st);

/**
 * @brief StackPeek function returns top element of stack.
 * 
 * @param Stack_t* st - pointer to stack structure.
 * 
 * @return int top    - top element of the stack or poison if stack is empty.
 */
data_t StackPeek(Stack_t* st);

/**
 * @brief _StackOK function checks stack integrity. not intended for user usage, only through STACK_VERIFY call.
 * 
 * @param Stack_t* st - pointer to stack structure.
 * 
 * @return int err    - error from enum Err. 
 */
int _StackOK(Stack_t* st);

/**
 * @brief _StackLog function updates stack's log file, including function, in which stack was changed, stack integrity, buffer data. not intended for user usage.
 * 
 * @param Stack_t* st - pointer to stack structure.
 * 
 * @return            - nothing.
 */
void _StackLog(Stack_t* st, char* funcName);

/**
 * @brief _StackDump function dumps stack's full information in case of integrity failure. not intended for user usage, only through STACK_VERIFY call.
 * 
 * @param Stack_t* st - pointer to stack structure.
 * 
 * @return int err    - nothing
 */
void _StackDump(Stack_t* st);

/**
 * @brief _StackGetHash function calculates an unique value for stack in order to check it's integrity. not intended for user usage.
 * 
 * @param Stack_t* st - pointer to stack structure.
 * 
 * @return int hash   - hash sum. 
 */
int _StackGetHash(Stack_t* st);

//####################//

int main() {
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

    //st->canary2 = 0;

    StackFree(st);

    return 0;
}

//####################//

Stack_t* StackAlloc() {
    Stack_t* st = calloc(1, sizeof(Stack_t));

    return st;
}

int _StackConstruct(Stack_t* st, const char* name, int max) {
    assert(st);
    assert(name);

    st->canary1 = CANARY;

    st->name    = calloc(1, sizeof(char) * strlen(name));

    if (st->name == NULL) {
        printf("couldn't allocate memory for stack name. try smaller one\n");
        return 1;
    }

    memcpy(st->name, name, strlen(name));

    st->max = max;
    st->cur = 0;

    st->buf = calloc(1, 2 * sizeof(CANARY) + max * sizeof(data_t));

    if (st->buf == NULL) {
        printf("couldn't allocate memory for stack buffer. try smaller max\n");
        return 1;
    }

    *((long*)(st->buf)) = CANARY;
    *((long*)(st->buf + sizeof(CANARY) + max * sizeof(data_t))) = CANARY;

    for (int i = 0; i < max; i++)
        *((data_t*)(st->buf + sizeof(CANARY) + i * sizeof(data_t))) = POISON;

    st->err       = OK;
    st->canary2   = CANARY;
    st->hash      = STACK_GET_HASH(st);

    printf("%ld\n", st->hash);

    char* logName = calloc(1, sizeof(char) * strlen(name) + 5);
    memcpy(logName, name, strlen(name)); 
    st->log = fopen(strcat(logName, ".log"), "w");
    fclose(st->log);
    st->log = fopen(logName, "a");
    free(logName);

    STACK_LOG(st, "_StackConstruct");

    printf("*\n");

    STACK_VERIFY(st);

    return 0;
}

int StackFree(Stack_t* st) {
    STACK_VERIFY(st);

    if (fclose(st->log)) {
        printf("couldn't close log file. try crying on the floor\n");
        return 1;
    }

    free(st->name);
    free(st->buf);
    free(st);

    return 0;
}

int StackResize(Stack_t* st, int newSize) {
    STACK_VERIFY(st);

    if (newSize < st->cur) {
        printf("invalid <newSize>!\n");
        return 1;
    }

    st->buf = realloc(st->buf, 2 * sizeof(CANARY) + newSize * sizeof(data_t));

    if (st->buf == NULL) {
        printf("couldn't allocate memory for new stack buffer. try commiting suicide\n");
        return 1;
    }

    st->max = newSize;

    for (int i = 0; st->cur + i < st->max; i++)
        *((data_t*)(st->buf + sizeof(CANARY) + (st->cur + i) * sizeof(data_t))) = POISON;

    *((long*)(st->buf + sizeof(CANARY) + st->max * sizeof(data_t))) = CANARY;
    st->hash = STACK_GET_HASH(st);

    STACK_LOG(st, "StackResize");

    STACK_VERIFY(st);

    return 0;
}

int StackPush(Stack_t* st, data_t data) {
    STACK_VERIFY(st);

    if (st->cur == st->max - 1) {
        if (StackResize(st, st->max * 2) != 0)
            return 1;
    }

    *((data_t*)(st->buf + sizeof(CANARY) + st->cur * sizeof(data_t))) = data;
    (st->cur)++;
    st->hash = STACK_GET_HASH(st);

    STACK_LOG(st, "StackPush");

    STACK_VERIFY(st);

    return 0;
}

int StackPop(Stack_t* st) {
    STACK_VERIFY(st);

    if (st->cur == 0) {
        printf("stack is already empty, unable to pop!\n");
        return -1;
    }

    if (st->cur < (st->max / 2) - 10) {
        if (StackResize(st, st->max / 2) != 0)
            return 1;
    }

    (st->cur)--;
    st->hash = STACK_GET_HASH(st);

    STACK_LOG(st, "StackPop");

    STACK_VERIFY(st);

    return 0;
}

data_t StackPeek(Stack_t* st) {
    STACK_VERIFY(st);

    STACK_LOG(st, "StackPeek");

    if (st->cur != 0)
        return *((data_t*)(st->buf + sizeof(CANARY) + (st->cur - 1) * sizeof(data_t)));
    else
        return POISON;
}

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

    fprintf(st->log, "went trhough [%s] function.\nnow stack <%s> looks like this:\nerrno = <%d> (%s)\n", funcName, st->name, st->err, ErrDesc[st->err]);

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
                       ErrDesc[st->err], 
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
