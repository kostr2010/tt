// if you're reading this code after nov-23-2019, 
// only God knows how this code works,
// because by now, only him and I know how it does.
//
// if you are ever to try to optimize this code and it fails, 
// (almost for sure) please, increase this counter as the warning
// for the next one who'll dare to fix it in future.
//
// totalHumanHoursWasted = 73
//

#include <stdio.h>
#include <unistd.h>

//#undef SEC_ON // un-undef for security measures, including dump
//#undef LOG_ON // un-undef for logging

#include "list.h"

int main() {
    DLList* lst = DLListAlloc();
    //sleep(1);
    DLLIST_INIT(lst, 10);
    DLListInsertL(lst, DLListGetHead(lst), 11);
    DLListInsertL(lst, DLListGetHead(lst), 40);
    DLListInsertL(lst, DLListGetHead(lst), 100);
    DLListInsertL(lst, DLListGetHead(lst), 45);
    DLListInsertL(lst, DLListGetHead(lst), 20);
    DLListInsertR(lst, DLListGetTail(lst), 10);
    DLListInsertR(lst, DLListGetTail(lst), 13);
    DLListInsertR(lst, DLListGetTail(lst), 12);
    DLListInsertL(lst, DLListGetHead(lst), 11);
    DLListInsertL(lst, DLListGetHead(lst), 40);
    DLListInsertL(lst, DLListGetHead(lst), 100);
    DLListInsertL(lst, DLListGetHead(lst), 45);
    DLListInsertL(lst, DLListGetHead(lst), 20);
    DLListInsertR(lst, DLListGetTail(lst), 10);
    DLListInsertR(lst, DLListGetTail(lst), 13);
    DLListInsertR(lst, DLListGetTail(lst), 12);
    DLListInsertL(lst, DLListGetHead(lst), 11);

    DLListVis(lst, "dot1.dot");

    DLListInsertL(lst, DLListGetHead(lst), 40);
    DLListInsertL(lst, DLListGetHead(lst), 100);
    DLListInsertL(lst, DLListGetHead(lst), 45);
    DLListInsertL(lst, DLListGetHead(lst), 20);
    DLListInsertR(lst, DLListGetTail(lst), 10);
    DLListInsertR(lst, DLListGetTail(lst), 13);
    DLListInsertR(lst, DLListGetTail(lst), 12);
    DLListInsertL(lst, DLListGetHead(lst), 11);
    DLListInsertL(lst, DLListGetHead(lst), 40);
    DLListInsertL(lst, DLListGetHead(lst), 100);
    DLListInsertL(lst, DLListGetHead(lst), 45);
    DLListInsertL(lst, DLListGetHead(lst), 20);
    DLListInsertR(lst, DLListGetTail(lst), 10);
    DLListInsertR(lst, DLListGetTail(lst), 13);
    DLListInsertR(lst, DLListGetTail(lst), 12);
    DLListInsertL(lst, DLListGetHead(lst), 11);
    DLListInsertL(lst, DLListGetHead(lst), 40);
    DLListInsertL(lst, DLListGetHead(lst), 100);
    DLListInsertL(lst, DLListGetHead(lst), 45);
    DLListInsertL(lst, DLListGetHead(lst), 20);
    DLListInsertR(lst, DLListGetTail(lst), 10);
    DLListInsertR(lst, DLListGetTail(lst), 13);
    DLListInsertR(lst, DLListGetTail(lst), 12);
    DLListInsertL(lst, DLListGetHead(lst), 11);
    DLListInsertL(lst, DLListGetHead(lst), 40);
    DLListInsertL(lst, DLListGetHead(lst), 100);
    DLListInsertL(lst, DLListGetHead(lst), 45);
    DLListInsertL(lst, DLListGetHead(lst), 20);
    DLListInsertR(lst, DLListGetTail(lst), 10);
    DLListInsertR(lst, DLListGetTail(lst), 13);
    DLListInsertR(lst, DLListGetTail(lst), 12);
    DLListInsertL(lst, DLListGetHead(lst), 11);
    DLListInsertL(lst, DLListGetHead(lst), 40);
    DLListInsertL(lst, DLListGetHead(lst), 100);
    DLListInsertL(lst, DLListGetHead(lst), 45);
    DLListInsertL(lst, DLListGetHead(lst), 20);
    DLListInsertR(lst, DLListGetTail(lst), 10);
    DLListInsertR(lst, DLListGetTail(lst), 13);
    DLListInsertR(lst, DLListGetTail(lst), 12);
    DLListInsertL(lst, DLListGetHead(lst), 11);
    DLListInsertL(lst, DLListGetHead(lst), 40);
    DLListInsertL(lst, DLListGetHead(lst), 100);
    DLListInsertL(lst, DLListGetHead(lst), 45);
    DLListInsertL(lst, DLListGetHead(lst), 20);
    DLListInsertR(lst, DLListGetTail(lst), 10);
    DLListInsertR(lst, DLListGetTail(lst), 13);
    DLListInsertR(lst, DLListGetTail(lst), 12);
    DLListInsertL(lst, DLListGetHead(lst), 11);
    DLListInsertL(lst, DLListGetHead(lst), 40);
    DLListInsertL(lst, DLListGetHead(lst), 100);
    DLListInsertL(lst, DLListGetHead(lst), 45);
    DLListInsertL(lst, DLListGetHead(lst), 20);
    DLListInsertR(lst, DLListGetTail(lst), 10);
    DLListInsertR(lst, DLListGetTail(lst), 13);
    DLListInsertR(lst, DLListGetTail(lst), 12);
    DLListInsertL(lst, DLListGetHead(lst), 11);
    DLListInsertL(lst, DLListGetHead(lst), 40);
    DLListInsertL(lst, DLListGetHead(lst), 100);
    DLListInsertL(lst, DLListGetHead(lst), 45);
    DLListInsertL(lst, DLListGetHead(lst), 20);
    DLListInsertR(lst, DLListGetTail(lst), 10);
    DLListInsertR(lst, DLListGetTail(lst), 13);
    DLListInsertR(lst, DLListGetTail(lst), 12);
    DLListInsertL(lst, DLListGetHead(lst), 11);
    DLListInsertL(lst, DLListGetHead(lst), 40);
    DLListInsertL(lst, DLListGetHead(lst), 100);
    DLListInsertL(lst, DLListGetHead(lst), 45);
    DLListInsertL(lst, DLListGetHead(lst), 20);
    DLListInsertR(lst, DLListGetTail(lst), 10);
    DLListInsertR(lst, DLListGetTail(lst), 13);
    DLListInsertR(lst, DLListGetTail(lst), 12);
    DLListInsertL(lst, DLListGetHead(lst), 11);
    DLListInsertL(lst, DLListGetHead(lst), 40);
    DLListInsertL(lst, DLListGetHead(lst), 100);
    DLListInsertL(lst, DLListGetHead(lst), 45);
    DLListInsertL(lst, DLListGetHead(lst), 20);
    DLListInsertR(lst, DLListGetTail(lst), 10);
    DLListInsertR(lst, DLListGetTail(lst), 13);
    DLListInsertR(lst, DLListGetTail(lst), 12);
    DLListInsertL(lst, DLListGetHead(lst), 11);
    DLListInsertL(lst, DLListGetHead(lst), 40);
    DLListInsertL(lst, DLListGetHead(lst), 100);
    DLListInsertL(lst, DLListGetHead(lst), 45);
    DLListInsertL(lst, DLListGetHead(lst), 20);
    DLListInsertR(lst, DLListGetTail(lst), 10);
    DLListInsertR(lst, DLListGetTail(lst), 13);
    DLListInsertR(lst, DLListGetTail(lst), 12);
    DLListInsertL(lst, DLListGetHead(lst), 11);
    DLListInsertL(lst, DLListGetHead(lst), 40);
    DLListInsertL(lst, DLListGetHead(lst), 100);
    DLListInsertL(lst, DLListGetHead(lst), 45);
    DLListInsertL(lst, DLListGetHead(lst), 20);
    DLListInsertR(lst, DLListGetTail(lst), 10);
    DLListInsertR(lst, DLListGetTail(lst), 13);
    DLListInsertR(lst, DLListGetTail(lst), 12);
    DLListInsertL(lst, DLListGetHead(lst), 11);
    DLListInsertL(lst, DLListGetHead(lst), 40);
    DLListInsertL(lst, DLListGetHead(lst), 100);
    DLListInsertL(lst, DLListGetHead(lst), 45);
    DLListInsertL(lst, DLListGetHead(lst), 20);
    DLListInsertR(lst, DLListGetTail(lst), 10);
    DLListInsertR(lst, DLListGetTail(lst), 13);
    DLListInsertR(lst, DLListGetTail(lst), 12);
    DLListInsertL(lst, DLListGetHead(lst), 11);
    DLListInsertL(lst, DLListGetHead(lst), 40);
    DLListInsertL(lst, DLListGetHead(lst), 100);
    DLListInsertL(lst, DLListGetHead(lst), 45);
    DLListInsertL(lst, DLListGetHead(lst), 20);
    DLListInsertR(lst, DLListGetTail(lst), 10);
    DLListInsertR(lst, DLListGetTail(lst), 13);
    DLListInsertR(lst, DLListGetTail(lst), 12);
    DLListInsertL(lst, DLListGetHead(lst), 11);
    DLListInsertL(lst, DLListGetHead(lst), 40);
    DLListInsertL(lst, DLListGetHead(lst), 100);
    DLListInsertL(lst, DLListGetHead(lst), 45);
    DLListInsertL(lst, DLListGetHead(lst), 20);
    DLListInsertR(lst, DLListGetTail(lst), 10);
    DLListInsertR(lst, DLListGetTail(lst), 13);
    DLListInsertR(lst, DLListGetTail(lst), 12);
    DLListInsertL(lst, DLListGetHead(lst), 11);
    DLListInsertL(lst, DLListGetHead(lst), 40);
    DLListInsertL(lst, DLListGetHead(lst), 100);
    DLListInsertL(lst, DLListGetHead(lst), 45);
    DLListInsertL(lst, DLListGetHead(lst), 20);
    DLListInsertR(lst, DLListGetTail(lst), 10);
    DLListInsertR(lst, DLListGetTail(lst), 13);
    DLListInsertR(lst, DLListGetTail(lst), 12);
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListSort(lst);
    DLListDelete(lst, DLListGetHead(lst));
    DLListSort(lst);
    DLListDelete(lst, DLListGetTail(lst));
    DLListSort(lst);
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));
    DLListDelete(lst, DLListGetTail(lst));
    DLListDelete(lst, DLListGetHead(lst));

    DLListSort(lst);

    DLListInsertL(lst, DLListGetHead(lst), 11);

    DLListInsertL(lst, DLListGetHead(lst), 40);

    DLListInsertL(lst, DLListGetHead(lst), 100);

    DLListInsertL(lst, DLListGetHead(lst), 45);

    DLListFree(lst);

    return 0;
}