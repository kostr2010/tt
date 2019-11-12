#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

//####################//

#define STACK_INIT(stack, max) _StackConstruct(stack, #stack, max)                          // macros for Stack_t constructor.
#define STACK_VERIFY(stack) if (_StackOK(stack) != 0) {_StackDump(stack); assert(!"ok");}   // macros for stack verification.

//####################//

typedef int data_t;                       // stack's data type.

#ifdef SEC_ON
const long CANARY = 0x1FEED5ADB16B00B5;   // canary value.
data_t POISON     = (data_t)(173);        // poisonous value. 

// array of errors descriptions.
const char* StackErrsDesc[] = {             
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
enum StackErrs {
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
#endif

// stack structure.
struct _Stack_t {
    #ifdef SEC_ON
    long canary1;       // first of canaries, surrounding structure;
    #endif

    char* name;         // name of the stack. filled automatically during STACK_INIT call;
    int max;            // max capacity;
    int cur;            // current filling;
    char* buf;          // pointer to the buffer of stack;

    #ifdef SEC_ON
    enum StackErrs err;       // error, occured during programm. "OK" by default;
    long int hash;      // hash sum of structure;
    FILE* log;          // log file stream. to be opened in STACK_INIT and closed in StackFree function;
    long canary2;       // second of canaries, surrounding structure.
    #endif
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

#endif