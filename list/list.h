#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

#define SEC_ON
#define LOG_ON

#define DLLIST_INIT(list, size) DLListInit(list, #list, size)

#ifdef SEC_ON
#define DLLIST_VERIFY(list) if (DLListVerify(list) != OK) {\
                                    /*DLListVis(list, #list);*/\
                                    DLListDump(list, #list);\
                                    DLListFree(list);\
                                    printf("executed with errors! see dump file for details\n");\
                                    exit(-1);\
                                }
#else
#define DLLIST_VERIFY(list) if (DLListVerify(list) != OK) {\
                                    DLListFree(list);\
                                    printf("executed with errors! see dump file for details\n");\
                                    exit(-1);\
                                }
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
    int dataMax;        // data array capacity
    int dataCur;        // data array current load

    int* next;          // sequence of physical addresses, indicates next element
    int* prev;          // sequence of physical addresses, indicates previous element

    int free;           // physical address of first free element
    int head;           // physical address of first element
    int tail;           // physical address of last element

    enum DLListErrs err;// error code

    #ifdef LOG_ON
    int logFd;          // file descryptor for log file
    #endif

    char isSorted;      // y/n
    #ifdef SEC_ON
    long hash;          // hash sum (for protecc)
    long canary2;       // canary value (for protecc)
    #endif
};
typedef struct _DLList DLList;

//####################//

/**
 * @brief this function allocates memory for pointer to the DLList structure.
 * 
 * @return DLList* list - pointer to the allocated memory. 
 */
DLList* DLListAlloc();

/**
 * @brief this function initializes DLList structure pointed by list.
 * @note better use DLLIST_INIT(list) macro, it automatically assigns list's name.
 * 
 * @param DLList* list     - pointer to the structure;
 * @param const char* name - name for the future structure;
 * @param int size         - initial size of the list.
 * 
 * @return int res         - 0 if initialised successfully, non-zero else.
 */
int DLListInit(DLList* list, const char* name, int size);

/**
 * @brief this function deallocates all of the memory, assigned to the list.
 * 
 * @param DLList* list - pointer to the structure.
 */
void DLListFree(DLList* list);

/**
 * @brief this function sets list's size to sizeNew.
 * @note not intended for user usage.
 * 
 * @param DLList* list      - pointer to the structure;
 * @param const int sizeNew - new size of the list.
 * 
 * @return int res          - 0 if success, non-zero otherwise.
 */
int DLListResize(DLList* list, const int sizeNew);

#ifdef SEC_ON
/**
 * @brief this function verifies list for possible errors. if any found,
 *        sets list's field err to the corresponding value, returns this value.
 * @note not intended for direct user usage. better use DLLIST_VERIFY(list) macro,
 *       because upon finding error, it will automatically generate dump file and free the list.
 * 
 * @param DLList* list - pointer to the structure.
 * 
 * @return int errCode - error code, 0 if list is OK.
 */
int DLListVerify(DLList* list);

/**
 * @brief this function calculates list's current hash sum.
 * @note not intended for direct user usage.
 * 
 * @param DLList* list - pointer to the structure.
 * 
 * @return int hash    - calculated hash sum.
 */
int DLListGetHash(DLList* list);

/**
 * @brief this function dumps all info on curent state of the list in case of error or corruption.
 * @note not intended for direct user usage. is included in DLLIST_VERIFY(list) macro.
 * 
 * @param DLList* list     - pointer to the structure;
 * @param const char* name - name for the dump file.
 */
void DLListDump(DLList* list, const char* name);
#endif
#ifdef LOG_ON
/**
 * @brief this function updates list's log file.
 * @note not intended for direct user usage. 
 * included in every function, that somehow modifies list's interns.
 * 
 * @param DLList* list     - pointer to the structure;
 * @param const char* func - name of hte function, where log was called.
 */
void DLListUpdLog(DLList* list, const char* func);
#endif

/**
 * @brief this fuction gets list's current load.
 * 
 * @param DLList* list - pointer to the structure.
 * 
 * @return int len     - list's length. 
 */
int DLListGetLen(DLList* list);

/**
 * @brief this fuction gets list's current capacity.
 * 
 * @param DLList* list - pointer to the structure.
 * 
 * @return int cap     - list's capacity. 
 */
int DLListGetCap(DLList* list);

/**
 * @brief this fuction gets list's current physical address of the head.
 * 
 * @param DLList* list - pointer to the structure.
 * 
 * @return int head    - list's head. 
 */
int DLListGetHead(DLList* list);

/**
 * @brief this fuction gets list's current physical address of the tail.
 * 
 * @param DLList* list - pointer to the structure.
 * 
 * @return int tail    - list's tail. 
 */
int DLListGetTail(DLList* list);

/**
 * @brief this fuction gets element's physical address.
 * 
 * @param DLList* list          - pointer to the structure;
 * @param const int addrLogical - element's logical number.
 * 
 * @return int addr             - element's physical address. 
 */
int DLListGetPhysAddr(DLList* list, const int addrLogical);


/**
 * @brief this function inserts element with given value left-side of the element with given physical address.
 * 
 * @param DLList* list           - pointer to the structure;
 * @param const int addrPhysical - physical address of the element;
 * @param const data dat         - value of the new element.
 * 
 * @return int addr              - physical address of the new element.
 */
int DLListInsertL(DLList* list, const int addrPhysical, const data dat);

/**
 * @brief this function inserts element with given value right-side of the element with given physical address.
 * 
 * @param DLList* list           - pointer to the structure;
 * @param const int addrPhysical - physical address of the element;
 * @param const data dat         - value of the new element.
 * 
 * @return int addr              - physical address of the new element.
 */
int DLListInsertR(DLList* list, const int addrPhysical, const data dat);

/**
 * @brief this function deletes element with given physical address.
 * 
 * @param DLList* list           - pointer to the structure;
 * @param const int addrPhysical - physical address of the element.
 * 
 * @return int res               - 0 if deleted successfully, non-zero otherwise. 
 */
int DLListDelete(DLList* list, const int addrPhysical);


/**
 * @brief this function searches for first instance of element with given value.
 * 
 * @param DLList* list   - pointer to the structure;
 * @param const data dat - value to search for.
 * 
 * @return addr          - physical address of the firsh instance. -1 if not found.
 */
int DLListFind(DLList* list, const data dat);

/**
 * @brief this function sorts given list, so that physical number of each element would correspond to the logical one of it.
 * 
 * @param DLList* list - pointer to the structure.
 */
void DLListSort(DLList* list);


/**
 * @brief this function returns pointer to the string, containing current tiestamp in format [dd-mm-yyyy|hh:mm:ss].
 * @note don't forget to free this pointer after use!
 * 
 * @return char* res - pointer to the string.
 */
char* GetTimestamp();

/**
 * @brief this function visualizec current state of the list in file list.png.
 * 
 * @param DLList* list     - pointer to the structure;
 * @param const char* name - name of the .dot file. 
 */
void DLListVis(DLList* list, const char* name);

#endif