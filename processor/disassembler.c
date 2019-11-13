// Translates .bin file to .asm file

//####################//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "consts.h"
#include "text.h"

//####################//



//####################//



//####################//

int Disasm(const char* nameIn, const char* nameOut);

//####################//

int main(int argc , char *argv[]) {
    if (argv[1] == NULL) {
        printf("no input file given!\n");
        exit(-1);
    }

    if (argv[2] == NULL) {
        printf("no output file given!\n");
        exit(-1);
    }

    int res = (Disasm(argv[1], argv[2]));

    if (res != 0) {
        printf("error occured during disassembly process! corrupted binary file!\n");
        exit(-1);
    } else
        printf("disassembly comlete. see %s file for results\n", argv[2]);

    return 0;
}

//####################//

int Disasm(const char* nameIn, const char* nameOut) {
    int fdIn = open(nameIn, O_RDONLY);
    int len = lseek(fdIn, 0, SEEK_END);
    lseek(fdIn, 0, SEEK_SET);

    char* buf = calloc(len, sizeof(char));

    if (read(fdIn, buf, len) != len) {
        printf("couldn't read binary file!\n");
        exit(-1);
    }

    umask(0);
    int fdOut = open(nameOut, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (fdOut == -1) {
        printf("couldn't create .asm file!\n");
        exit(-1);
    }

    int cur = 0, cmdNum = 0;
    while (buf[cur] != '\0') {
        switch (buf[cur]) {
        #define CMD_DEF(name, num, codeAsm, codeBin, codeDisasm)\
        case num:\
            cmdNum = cur;\
            cur += CMD_SZ;\
            dprintf(fdOut, "%s ", name);\
            codeDisasm;\
            dprintf(fdOut, "; [%d]\n", cmdNum);\
            break;
        #include "cmds.h"
        
        default:
            printf("<%d %d>\n", cur, len);
            return 1;
        }
    }

    return 0;
}