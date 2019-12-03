// doubly linked list implementation

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "list.h"

//####################//

#ifdef SEC_ON
const long CANARY = 0x1FEED5ADB16B00B5; // canary value.
const int POISON = (data)(371);         // poison value
#endif
const int DELTA = 10;                   // delta from size/2 to shink the list

const char* DLListErrDesc[] = {
    "list is ok\n",
    #ifdef SEC_ON
    "hash corrupted!\n",
    "canary1 dead!\n",
    "canary2 dead!\n",
    #endif
    #ifdef LOG_ON
    "unable to open/create log file or it was closed unexpectedly!\n",
    #endif
    "initialization error, unable to allocate memory!\n",
    "reallocation error, unable to allocate memory!\n",
    "list overwlow!\n",
    "list undeflow!\n",
    "element sequence cprrupted!\n",
    "invalid list size!\n",

};

//####################//

DLList* DLListAlloc() {
    DLList* list = calloc(1, sizeof(DLList));
    
    return list;
}

int DLListInit(DLList* list, const char* name, int size) {
    assert(list);

    list->data = calloc(size, sizeof(data));

    list->dataMax = size;
    list->dataCur = 0;

    list->next = calloc(size, sizeof(int));
    for (int i = 1; i < size - 1; i++)
        list->next[i] = i + 1;
    list->next[size - 1] = 0;

    list->prev = calloc(size, sizeof(int));

    if (list->data == NULL || list->prev == NULL || list->next == NULL)
        return E_INIT_ERR;
        
    list->free = 1;
    list->head = 0;
    list->tail = 0;

    #ifdef LOG_ON
    char* logName = GetTimestamp();
    logName = realloc(logName, strlen(logName) + strlen(name) + strlen("log/-.log"));
    char* logName2 = calloc(30, 1);
    memcpy(logName2, logName, strlen(logName));
    sprintf(logName, "log/%s-%s.log", name, logName2);    
    list->logFd = open(logName, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR| S_IWUSR);
    fsync(list->logFd);
    free(logName);
    
    if (list->logFd == -1)
        return E_LOG_DEAD;
    #endif

    list->isSorted = 'y';

    #ifdef SEC_ON
    list->canary1 = CANARY;
    list->canary2 = CANARY;
    list->err = OK;
    list->hash = DLListGetHash(list);
    #endif
    #ifdef LOG_ON
    DLListUpdLog(list, "DLListInit");
    #endif

    return OK;
}

void DLListFree(DLList* list) {
    if (list == NULL)
        return;
    
    if (list->data != NULL)
        free(list->data);
    
    if (list->next != NULL)
        free(list->next);
    
    if (list->prev != NULL)
        free(list->prev);
    
    #ifdef LOG_ON
    if (list->logFd != -1)
        close(list->logFd);
    #endif

    free(list);

    return;    
}   

int DLListResize(DLList* list, const int sizeNew) {
    DLLIST_VERIFY(list);
    
    if (sizeNew < DELTA) {
        printf("invalid new size! %d -> %d\n", list->dataMax, sizeNew);
        return -1;
    } else if (sizeNew == list->dataMax) {
        printf("same size %d -> %d\n", list->dataMax, sizeNew);
        return 0;
    } else if (sizeNew < list->dataMax) {
        printf("shrink %d -> %d\n", list->dataMax, sizeNew);
        DLListSort(list);
        list->dataCur = (list->dataCur > sizeNew) ? (sizeNew - DELTA) : (list->dataCur);
        list->dataMax = sizeNew;
        list->data = realloc(list->data, sizeNew * sizeof(data));
        list->next = realloc(list->next, sizeNew * sizeof(int));
        list->prev = realloc(list->prev, sizeNew * sizeof(int));
        list->tail = list->dataCur;
        list->next[list->tail] = 0;
    } else if (sizeNew > list->dataMax) {
        printf("extend %d -> %d\n", list->dataMax, sizeNew);
        list->data = realloc(list->data, sizeNew * sizeof(data));
        list->next = realloc(list->next, sizeNew * sizeof(int));

        for (int i = list->dataMax; i < sizeNew - 1; i++)
            list->next[i] = i + 1;
        list->free = list->dataMax;
        list->next[sizeNew - 1] = 0;


        list->prev = realloc(list->prev, sizeNew * sizeof(int));
        memset(list->prev + list->dataMax * sizeof(int), '\0', (sizeNew - list->dataMax) * sizeof(int));
        list->dataMax = sizeNew;
    }

    #ifdef SEC_ON
    list->hash = DLListGetHash(list);
    #endif

    DLLIST_VERIFY(list);

    #ifdef LOG_ON
    DLListUpdLog(list, "DLListResize");
    #endif

    return 0;
}


int DLListVerify(DLList* list) {
    if (list == NULL)
        return -1;

    if (list->dataCur > list->dataMax || list->dataCur < 0) {
        list->err = E_INV_SIZE;
        return E_INV_SIZE;
    }

    #ifdef LOG_ON
    if (list->logFd == -1) {
        list->err = E_LOG_DEAD;
        return E_LOG_DEAD;
    }
    #endif
    #ifdef SEC_ON
    if (list->canary1 != CANARY) {
        list->err = E_CANARY1_DEAD;
        return E_CANARY1_DEAD;
    }
    if (list->canary2 != CANARY) {
        list->err = E_CANARY2_DEAD;
        return E_CANARY2_DEAD;
    }

    if (list->hash != DLListGetHash(list)) {
        list->err = E_HASH_CORRUPTED;
        return E_HASH_CORRUPTED;
    }
    #endif
    if (list->dataCur < 0) {
        list->err = E_LIST_UNDERFLOW;
        return E_LIST_UNDERFLOW;
    }

    if (list->dataCur >= list->dataMax) {
        list->err = E_LIST_OVERWLOW;
        return E_LIST_OVERWLOW;
    }

    int elemCount = 0;
    for (int i = list->head; i != 0; i = list->next[i]) {
        if (elemCount >= list->dataMax 
            || ((list->prev[i] != 0 && i != list->next[list->prev[i]])
            && (list->next[i] != 0 && i != list->prev[list->next[i]]))
            || elemCount > list->dataMax) {
            list->err = E_CORRUPTED_SEQ;
            return E_CORRUPTED_SEQ;
        }
        elemCount++;
    }
    return OK;
}

#ifdef SEC_ON
int DLListGetHash(DLList* list) {
    assert(list);
    assert(list->data);
    assert(list->next);
    assert(list->prev);

    long dataHash = 0, nextHash = 0, prevHash = 0;

    for (int i = 0; i < list->dataMax; i++) {
        dataHash += ((i + 1) * list->data[i]) % POISON;
        nextHash += ((i + 1) * list->next[i]) % POISON;
        prevHash += ((i + 1) * list->prev[i]) % POISON;
    }

    long hash = (list->canary1 % POISON) * 1
                 + (list->canary2 % POISON) * 2
                 + (dataHash) * 3
                 + (nextHash) * 4
                 + (prevHash) * 5
                 + list->dataCur * 6
                 + list->dataMax * 7
                 + list->free * 8
                 + list->tail * 9
                 + list->head * 1
                 + list->err * 3
                 + list->isSorted * 4;

    return hash;
}

void DLListDump(DLList* list, const char* name) {
    char* dumpName = calloc(1 + strlen(name) + strlen("dump/.log"), sizeof(char));
    sprintf(dumpName, "dump/%s.log", name);
    int dumpFd = open(dumpName, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
    free(dumpName);

    if (list == NULL) {
        dprintf(dumpFd, "ERROR: NULL pointer to the structure!\n");
    } else {
        char* timeStamp = GetTimestamp();

        dprintf(dumpFd, "%s\n"
                             "  canary1: %ld\n"
                             "  CANARY: [%ld]\n"
                             "  canary2: %ld\n"
                             "  hash: %ld\n"
                             "  list capacity: %d\n"
                             "  list size: %d\n"
                             "  head's physical address: %d\n"
                             "  tail's physical address %d\n"
                             "  first free element's physical address: %d\n"
                             "  sorted: %c\n"
                             "  addr: ",
                             timeStamp, list->canary1, CANARY, list->canary2, list->hash, list->dataMax, list->dataCur, list->head, list->tail, list->free, list->isSorted);

        for (int i = 0; i < list->dataMax; i++) {
        if (list->prev[i] == 0 && (i != list->head && i != list->tail || list->dataCur == 0))
            dprintf(list->logFd, "[%3d ]", i);
        else 
            dprintf(list->logFd, "[%3d *]", i);
    }
        dprintf(dumpFd, "\n"
                             "  data: ");
        if (list->data != NULL)
            for (int i = 0; i < list->dataMax; i++)
                dprintf(dumpFd, "[%5d]", list->data[i]);
        dprintf(dumpFd, "\n"
                             "  next: ");
        if (list->next != NULL)
            for (int i = 0; i < list->dataMax; i++)
                dprintf(dumpFd, "[%5d]", list->next[i]);
        dprintf(dumpFd, "\n"
                             "  prev: ");
        if (list->prev != NULL)
            for (int i = 0; i < list->dataMax; i++)
                dprintf(dumpFd, "[%5d]", list->prev[i]);
        dprintf(dumpFd, "\n");

        dprintf(dumpFd, "  current state: [error №%d] %s\n\n", list->err, DLListErrDesc[list->err]);
    }

    close(dumpFd);
}
#endif
#ifdef LOG_ON
void DLListUpdLog(DLList* list, const char* func) {
    char* timeStamp = GetTimestamp();

    dprintf(list->logFd, "%s\n"
                         "passed through function [[%s]]\n"
                         "  canary1: %ld\n"
                         "  CANARY: [%ld]\n"
                         "  canary2: %ld\n"
                         "  hash: %ld\n"
                         "  list capacity: %d\n"
                         "  list size: %d\n"
                         "  head's physical address: %d\n"
                         "  tail's physical address %d\n"
                         "  first free element's physical address: %d\n"
                         "  sorted: %c\n"
                         "  addr: ",
                         timeStamp, func, list->canary1, CANARY, list->canary2, list->hash, list->dataMax, list->dataCur, list->head, list->tail, list->free, list->isSorted);
    free(timeStamp);

    for (int i = 0; i < list->dataMax; i++) {
        if (list->prev[i] == 0 && (i != list->head && i != list->tail || list->dataCur == 0))
            dprintf(list->logFd, "[%3d  ]", i);
        else 
            dprintf(list->logFd, "[%3d *]", i);
    }
    dprintf(list->logFd, "\n"
                         "  data: ");

    for (int i = 0; i < list->dataMax; i++) {
        dprintf(list->logFd, "[%5d]", list->data[i]);
    }
    dprintf(list->logFd, "\n"
                         "  next: ");

    for (int i = 0; i < list->dataMax; i++) {
        dprintf(list->logFd, "[%5d]", list->next[i]);
    }
    dprintf(list->logFd, "\n"
                         "  prev: ");

    for (int i = 0; i < list->dataMax; i++) {
        dprintf(list->logFd, "[%5d]", list->prev[i]);
    }
    dprintf(list->logFd, "\n");

    dprintf(list->logFd, "  current state: [error №%d] %s\n\n", list->err, DLListErrDesc[list->err]);
}
#endif

int DLListGetLen(DLList* list) {
    DLLIST_VERIFY(list);

    return list->dataCur;

    DLLIST_VERIFY(list);
}

int DLListGetCap(DLList* list) {
    DLLIST_VERIFY(list);

    return list->dataMax;

    DLLIST_VERIFY(list);
}

int DLListGetHead(DLList* list) {
    DLLIST_VERIFY(list);

    return list->head;

    DLLIST_VERIFY(list);
}

int DLListGetTail(DLList* list) {
    DLLIST_VERIFY(list);

    return list->tail;

    DLLIST_VERIFY(list);
}

int DLListGetPhysAddr(DLList* list, const int addrLogical) {
    DLLIST_VERIFY(list);

    if (list->isSorted == 'y')
        return addrLogical;

    int addrPhysical = list->head;

    if (addrLogical >= list->dataMax) {
        printf("no such address. too big\n");
        return -1;
    } else {
        for (int i = addrLogical; i > 0; i--)
            addrPhysical = list->next[addrPhysical];
    }

    DLLIST_VERIFY(list);

    return addrPhysical;
}


int DLListInsertL(DLList* list, const int addrPhysical, const data dat) {
    DLLIST_VERIFY(list);

    if (list->dataCur >= list->dataMax - 1)
        DLListResize(list, list->dataMax * 2);

    if (list->prev[addrPhysical] == 0 
        && (addrPhysical != list->head 
        && addrPhysical != list->tail)) {
        printf("unappropriate address to insert %d!\n", addrPhysical);
        DLListUpdLog(list, "DLListInsertL, unappropriate address to insert!");
        return -1;
    }

    int addrIns = list->free;
    int addrNewFree = list->next[list->free];
    list->data[list->free] = dat;

    if (list->dataCur == 0) {
        list->prev[addrIns] = 0;
        list->next[addrIns] = 0;
        list->tail = addrIns;
        list->head = addrIns;
    } else if (addrPhysical == list->head) {
        list->prev[addrIns] = 0;
        list->next[addrIns] = list->head;
        list->prev[list->head] = addrIns;
        list->head = addrIns;
    } else if (addrPhysical != list->head) {
        list->prev[addrIns] = list->prev[addrPhysical];
        list->next[addrIns] = addrPhysical;
        list->next[list->prev[addrPhysical]] = addrIns;
        list->prev[addrPhysical] = addrIns;
    }

    list->dataCur++;
    list->isSorted = 'n';
    list->free = addrNewFree;

    #ifdef SEC_ON
    list->hash = DLListGetHash(list);
    #endif
    DLLIST_VERIFY(list);
    #ifdef LOG_ON
    DLListUpdLog(list, "DLListInsertL");
    #endif

    //if (list->isSorted == 'n')
    //    DLListSort(list);

    return addrIns;
}

int DLListInsertR(DLList* list, const int addrPhysical, const data dat) {
    DLLIST_VERIFY(list);

    if (list->dataCur >= list->dataMax - 1)
        DLListResize(list, list->dataMax * 2);

    if (list->prev[addrPhysical] == 0 
        && (addrPhysical != list->head 
        || addrPhysical != list->tail)) {
        printf("unappropriate address to insert %d!\n", addrPhysical);
        DLListUpdLog(list, "DLListInsertR, unappropriate address to insert!");
        return -1;
    }

    int addrIns = list->free;
    int addrNewFree = list->next[addrIns];
    list->data[addrIns] = dat;

    if (list->dataCur == 0) {
        list->prev[addrIns] = 0;
        list->next[addrIns] = 0;
        list->tail = addrIns;
        list->head = addrIns;
    } else if (addrPhysical == list->tail) {
        list->next[addrIns] = 0;
        list->prev[addrIns] = list->tail;
        list->next[list->tail] = addrIns;
        list->tail = addrIns;
    } else if (addrPhysical != list->head) {
        list->next[addrIns] = list->next[addrPhysical];
        list->prev[addrIns] = addrPhysical;
        list->prev[list->next[addrPhysical]] = addrIns;
        list->next[addrPhysical] = addrIns;
        list->isSorted = 'n';
    }

    list->dataCur++;
    list->free = addrNewFree;

    #ifdef SEC_ON
    list->hash = DLListGetHash(list);
    #endif

    DLLIST_VERIFY(list);
    #ifdef LOG_ON
    DLListUpdLog(list, "DLListInsertR");
    #endif

    //if (list->isSorted == 'n')
    //    DLListSort(list);

    return addrIns;
}

int DLListDelete(DLList* list, const int addrPhysical) {
    DLLIST_VERIFY(list);

    if (list->dataCur == 0) {
        printf("list is already empty!\n");
        return -1;
    }

    if (list->prev[addrPhysical] == 0 
        && (addrPhysical != list->head 
        && addrPhysical != list->tail)) {
        printf("unappropriate address to delete %d %d!\n", addrPhysical, list->head);
        DLListUpdLog(list, "DLListDelete, unappropriate address to delete!");
        return -1;
    } 

    if (addrPhysical == list->head) {
        list->prev[list->next[addrPhysical]] = 0;
        list->head = list->next[addrPhysical];
        list->isSorted = 'n';
    } else if (addrPhysical == list->tail) {
        list->next[list->prev[addrPhysical]] = 0;
        list->tail = list->prev[addrPhysical];
    } else {
        list->prev[list->next[addrPhysical]] = list->prev[addrPhysical];
        list->next[list->prev[addrPhysical]] = list->next[addrPhysical];
        list->isSorted = 'n';
    }

    list->prev[addrPhysical] = 0;
    list->next[addrPhysical] = list->free;
    list->free = addrPhysical;
    list->dataCur--;

    #ifdef SEC_ON
    list->hash = DLListGetHash(list);
    #endif

    DLLIST_VERIFY(list);
    #ifdef LOG_ON
    DLListUpdLog(list, "DLListDelete");
    #endif

    if (list->dataCur <= list->dataMax / 2 - DELTA)
        DLListResize(list, list->dataMax / 2); 

    //if (list->isSorted == 'n')
    //    DLListSort(list);

    return 0;
}


int DLListFind(DLList* list, const data dat) {
    DLLIST_VERIFY(list);

    int addr = -1;

    for (int i = list->head; i != 0; i = list->next[i])
        if (list->data[i] == dat) {
            addr = i;
            break;
        }

    DLLIST_VERIFY(list);

    return addr;
}

void DLListSort(DLList* list) {
    DLLIST_VERIFY(list);

    if (list->isSorted == 'y' || list->dataCur < 1) {
        #ifdef LOG_ON
        DLListUpdLog(list, "DLListSort (size < 1, list unchanged)");
        #endif
        return;
    }

    data* dataSorted = calloc(list->dataMax, sizeof(data));

    int count = 1;
    for (int i = list->head; i != 0; i = list->next[i]) {
        dataSorted[count] = list->data[i];
        count++;
    }

    free(list->data);
    list->data = dataSorted;

    for (int i = 1; i < list->dataMax - 1; i++)
        list->next[i] = i + 1;

    memset((char*)(list->prev), '\0', list->dataMax * sizeof(int));
    for (int i = 1; i <= list->dataCur; i++) {
        list->prev[i] = i - 1;
    }

    list->isSorted = 'y';
    list->head = 1;
    list->tail = list->dataCur;
    list->free = list->next[list->tail];
    list->next[list->tail] = 0;
    list->prev[list->head] = 0;

    #ifdef SEC_ON
    list->hash = DLListGetHash(list);
    #endif
    DLLIST_VERIFY(list);
    #ifdef SEC_ON
    DLListUpdLog(list, "DLListSort");
    #endif
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

void DLListVis(DLList* list, const char* name) {
    //DLLIST_VERIFY(list);
    assert(name);

    //DLListSort(list);

	FILE* dot = fopen(name, "w");

	fprintf(dot, "digraph G {\n");

	for(int i = 1; i <= list->dataCur; i++){
		fprintf(dot, "%d[label = %d];\n", i, list->data[i]);
	}
	
	int i = list->head;

	while(list->next[i] > 0){
		fprintf(dot, "%d->", i);
		i = list->next[i];
	}

	fprintf(dot,"%d;\n}\n", i);
	fclose(dot);

    char* pngName = calloc(strlen("dot  -Tpng -o list.png") + strlen(name), sizeof(char));
    sprintf(pngName, "dot %s -Tpng -o list.png", name);

    system(pngName);
    free(pngName);

    //DLLIST_VERIFY(list);
}
