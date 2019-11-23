#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

#define DLLIST_INIT(list, size) DLListInit(list, #list, size)

#ifdef SEC_ON
#define DLLIST_VERIFY(list) if (DLListVerify(list) != OK) {\
                                    DLListDump(list, #list);\
                                    DLListFree(list);\
                                    printf("executed with errors! see dump file for details\n");\
                                    exit(-1);\
                                }
#else
#define DLLIST_VERIFY(list) {}
#endif

//####################//

typedef int data;       // stack's data type.

enum DLListErrs {
    OK,
    #ifdef SEC_ON
    E_HASH_CORRUPTED,
    E_CANARY1_DEAD,
    E_CANARY2_DEAD,
    #endif
    #ifdef LOG_ON
    E_LOG_DEAD,
    #endif
    E_INIT_ERR,
    E_REALLOC_ERR,
    E_LIST_OVERWLOW,
    E_LIST_UNDERFLOW,
    E_CORRUPTED_SEQ,
    E_INV_SIZE,

};

// double linked list structure
struct _DLList {
    #ifdef SEC_ON
    long canary1;       // canary value (for protecc)
    #endif

    data* data;         // data array
    int dataMax;         // data array capacity
    int dataCur;         // data array current load

    int* next;          // sequence of physical addresses, indicates next element
    int* prev;          // sequence of physical addresses, indicates previous element

    int free;           // physical address of first free element
    int head;           // physical address of first element
    int tail;           // physical address of last element

    #ifdef LOG_ON
    int logFd;          // file descryptor for log file
    #endif

    char isSorted;       // y/n
    #ifdef SEC_ON
    long hash;          // hash sum (for protecc)
    enum DLListErrs err;// error code
    long canary2;       // canary value (for protecc)
    #endif
};
typedef struct _DLList DLList;

//####################//

DLList* DLListAlloc();
int DLListInit(DLList* list, const char* name, int size);
void DLListFree(DLList* list);
int DLListResize(DLList* list, const int sizeNew);

#ifdef SEC_ON
int DLListVerify(DLList* list);
int DLListGetHash(DLList* list);
void DLListDump(DLList* list, const char* name);
#endif
#ifdef LOG_ON
void DLListUpdLog(DLList* list, const char* func);
#endif

int DLListGetLen(DLList* list);
int DLListGetCap(DLList* list);
int DLListGetHead(DLList* list);
int DLListGetTail(DLList* list);
int DLListGetPhysAddr(DLList* list, const int addrLogical);

int DLListInsertL(DLList* list, const int addrPhysical, const data dat);
int DLListInsertR(DLList* list, const int addrPhysical, const data dat);
int DLListDelete(DLList* list, const int addrPhysical);

int DLListFind(DLList* list, const data dat);
void DLListSort(DLList* list);

char* GetTimestamp();
void DLListVis(DLList* list, const char* name);

#endif