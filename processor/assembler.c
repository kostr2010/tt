#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "consts.h"

//####################//

const int LABEL_NAME_MAX_SZ = 10;
const int EXECBUF_INIT_BUF_SZ = 100;
const int EXECBUF_INIT_LBLS_SZ = 10;

//####################//

// compilation errors
enum _AssemblerErrs {
    OK,
    E_INV_LBL_NAME,
    E_INV_ARG_TYPE,
    E_INV_CMD,
    E_NO_ARG,
    E_INV_ARG,
    E_FEW_ARGS,
    E_MANY_ARGS,
    E_UNDEC_LBL,
    E_LONG_ALERT_MSG,
    //E_NO_END_SEQ,
};
typedef enum _AssemblerErrs AsmErrs;

// descriptions for copiilation errors
const char* AsmErrsDesc[] = {
    "compiled without errors\n",
    "invalid label name!\n",
    "invalid argument type!\n",
    "invalid instruction sequence!\n",
    "expected an argument, but none given!\n",
    "invalid argument given!\n",
    "too few argumentd given!\n",
    "too many arguments given!\n",
    "undeclared label!\n",
    "alert message is too long!\n",
    //"no end sequence given!\n",     
};

struct _String {
    char*   buf;
    int     len;
};
typedef struct _String String;

struct _Label {
    char*   name;
    int     addr;
};
typedef struct _Label Label;

struct _ExecBuf {
    char*   cmds;
    int     maxCmd;
    int     curCmd;
    Label*  lbls;
    int     maxLbl;
    int     curLbl;
    AsmErrs err;
};
typedef struct _ExecBuf ExecBuf;

//####################//

String* StringAlloc();
int StringInit(String* str, int len);
int StringFree(String* str);

Label* LabelAlloc();
int LabelInit(Label* lbl, int len);
int LabelFree(Label* lbl);

ExecBuf* ExecBufAlloc();
int ExecBufInit(ExecBuf* eBuf);
int ExecBufFree(ExecBuf* eBuf);

int Compile(char* in, ExecBuf* eBuf);
String* CodeRead(char* name);
int LineInterpret(String* line, ExecBuf* eBuf);

int CountLines(char* buf, int len);
String* DivideByLines(char* buf, int len, int nLines);
char* FindAndReplace(char* buf, int len, char old, char new);
char* FindWord(String* str, int off, int* end);
int IsWhitespace(String* str, int off);
char GetArgType(char* arg, int len);
int _FindLabel(ExecBuf* eBuf, String* line, char* lblPtr, int off);
int EbufAddReg(ExecBuf* eBuf, char* arg);

//####################//

int main(int argc , char *argv[]) {
    ExecBuf* eBuf = ExecBufAlloc();
    if (ExecBufInit(eBuf) != 0) {
        printf("unable to initialize translation buffer!\n");
        exit(-1);
    }

    printf("\ncompilation started..\n\n");

    int res = Compile(argv[1], eBuf);
    if (res != 0) {
        printf("compilation error in line %d! %s\n", res, AsmErrsDesc[eBuf->err]);
        exit(res);
    }

    res = Compile(argv[1], eBuf);
    if (res != 0) {
        printf("compilation error in line %d! %s\n", res, AsmErrsDesc[eBuf->err]);
        exit(res);
    }

    printf("occupied %d~(bytes) to store commands, %d~(bytes) to store labels\n\n", eBuf->maxCmd, eBuf->maxLbl * sizeof(Label));

    int fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        printf("can't create binary!\n");
        exit(-1);
    }

    if (write(fd, eBuf->cmds, eBuf->maxCmd) != eBuf->maxCmd) {
        printf("unable to write cmd fuffer!\n");
        exit(-1);
    }

    printf("compilation terminated successfully\n\n");

    close(fd);
    ExecBufFree(eBuf);

    return 0;
}

//####################//

String* StringAlloc() {
    String* str = calloc(1, sizeof(String));

    return str;
}

int StringInit(String* str, int len) {
    assert(str);

    str->buf = calloc(len, sizeof(char));
    str->len = len;

    return 0;
}

int StringFree(String* str) {
    assert(str);
    assert(str->buf);

    free(str->buf);
    free(str);

    return 0;
}

Label* LabelAlloc() {
    Label* lbl = calloc(1, sizeof(Label));

    return lbl;
}

int LabelInit(Label* lbl, int len) {
    assert(lbl);
    
    lbl->name = calloc(len, sizeof(char));
    lbl->addr = 0;

    return 0;
}

int LabelFree(Label* lbl) {
    assert(lbl);
    assert(lbl->name);

    free(lbl->name);
    free(lbl);

    return 0;
}

ExecBuf* ExecBufAlloc() {
    ExecBuf* eBuf = calloc(1, sizeof(ExecBuf));

    return eBuf;
}

int ExecBufInit(ExecBuf* eBuf) {
    assert(eBuf);

    eBuf->cmds = calloc(EXECBUF_INIT_BUF_SZ, sizeof(char));
    eBuf->maxCmd = EXECBUF_INIT_BUF_SZ;
    eBuf->curCmd = 0;

    eBuf->lbls = calloc(EXECBUF_INIT_LBLS_SZ, sizeof(Label));
    eBuf->maxLbl = EXECBUF_INIT_LBLS_SZ;
    eBuf->curLbl = 0;
    eBuf->err = OK;

    if (eBuf->cmds && eBuf->lbls)
        return 0;
    else    
        return -1;
}

int ExecBufFree(ExecBuf* eBuf) {
    assert(eBuf);
    assert(eBuf->cmds);
    assert(eBuf->lbls);

    free(eBuf->cmds);

    for (int i = 0; i < eBuf->curLbl; i++)
        LabelFree(&((eBuf->lbls)[i]));

    free(eBuf->lbls);
    free(eBuf);

    return 0;
}

int Compile(char* inName, ExecBuf* eBuf) {
    assert(inName);
    assert(eBuf);

    String* code = CodeRead(inName);

    int nLines = CountLines(code->buf, code->len);
    String* matrix = DivideByLines(code->buf, code->len, nLines);

    for (int i = 0; i < nLines; i++) {
        if (LineInterpret(&(matrix[i]), eBuf) != 0) {
            eBuf->curCmd = 0;
            eBuf->curLbl = 0;

            StringFree(code);
            free(matrix);

            return i + 1;
        }
    }

    eBuf->curCmd = 0;
    eBuf->curLbl = 0;

    StringFree(code);
    free(matrix);

    return 0;
}

String* CodeRead(char* name) {
    assert(name);

    String* code = StringAlloc();

    int fd = open(name, O_RDONLY);
    if (fd == -1) {
        printf("error openning %s !", name);
        exit(-1);
    }

    StringInit(code, lseek(fd, 0, SEEK_END));
    lseek(fd, 0, SEEK_SET);

    read(fd, code->buf, code->len);
    close(fd);

    return(code);
}

int LineInterpret(String* line, ExecBuf* eBuf) {
    assert(line);
    assert(eBuf);
    assert(line->buf);
    assert(eBuf->cmds);
    assert(eBuf->lbls);

    // delete comments
    if (FindAndReplace(line->buf, line->len, ';', '\0') != NULL)
        line->len = (char*)memchr(line->buf, '\0', line->len) - line->buf;

    // check if empty
    if (line->len == 0)
        return 0;

    int off = 0;
    char* cmd = FindWord(line, off, &off);

    // check if whitespace    
    if (cmd == NULL)
        return 0;

    char* lblPos = memchr(line->buf, ':', line->len);

    if (lblPos == cmd) {
        // if no label name was given
        eBuf->err = E_INV_LBL_NAME;
        
        return 1;
    } else if (lblPos != NULL) {
        // if label found
        if (eBuf->curLbl >= eBuf->maxLbl - DELTA_LBLS) {
            eBuf->lbls = realloc(eBuf->lbls, eBuf->maxLbl * 2);
            bzero(eBuf->lbls + eBuf->maxLbl, eBuf->maxLbl);
            eBuf->maxLbl *= 2;
        }

        LabelInit(&(eBuf->lbls[eBuf->curLbl]), lblPos - cmd + 1);
        eBuf->lbls[eBuf->curLbl].addr = eBuf->curCmd;
        memcpy(eBuf->lbls[eBuf->curLbl].name, cmd, lblPos - cmd);
        eBuf->curLbl++;

        return 0;
    } else if (lblPos == NULL) {
        // if label not found
        if (eBuf->curCmd >= eBuf->maxCmd - DELTA_CMDS) {
            eBuf->cmds = realloc(eBuf->cmds, eBuf->maxCmd * 2);
            eBuf->maxCmd *= 2;
        }

        int cmdLen = off - (cmd - line->buf);
        #define CMD_DEF(name, num, codeAsm, codeCpu) \
                if (cmdLen == strlen(name) && strncmp(name, cmd, (cmdLen < strlen(name)) ? (cmdLen) : (strlen(name))) == 0) {\
                    /*printf("%d %s:\n", eBuf->curCmd, name);*/\
                    *(eBuf->cmds + eBuf->curCmd) = num;\
                    eBuf->curCmd += CMD_SZ;\
                    codeAsm;\
                }
    
        #include "cmds.h"
    }
    
    // if there was no match in cmds.h, nor it was label
    eBuf->err = E_INV_CMD;
    
    return 1;
}

int CountLines(char* buf, int len) {
    assert(len >= 0);
    assert(buf);

    int nLines = 0;

    for (char* s = memchr(buf, '\n', len); s != NULL; s = memchr(s + 1, '\n', len - (s - buf))) 
        nLines++;

    if (buf[len - 1] != '\n' && buf[len - 1] != '\0')
        nLines++;

    return nLines;
}

String* DivideByLines(char* buf, int len, int nLines) {
    assert(buf);

    String* matrix = (String*)calloc(nLines , sizeof(String));
    int i = 0;

    matrix[0].buf = buf;
    i++;

    for (char* s = memchr(buf, '\n', len); s != NULL; s = memchr(s + 1, '\n', len - (s - buf))) {
        if (*(s - 1) == '\n') {
            matrix[i - 1].len = 0;
        } else {
            matrix[i - 1].len = s - matrix[i - 1].buf;
        }

        matrix[i].buf = s + 1;
        i++;
    }

    if (i == nLines)
        matrix[i - 1].len = buf + len - matrix[i - 1].buf;

    return matrix;
}

char* FindAndReplace(char* buf, int len, char old, char new) {
    assert(buf);

    char* s = memchr(buf, old, len);
    if (s != NULL)
        *s = new;

    return s;
}

void FindAndReplaceAll(char* buf, int len, char old, char new) {
    assert(buf);

    char* s = FindAndReplace(buf, len, old, new);

    while(s != NULL)
        s = FindAndReplace(s + 1, buf + len - s - 1, old, new);
}

char* FindWord(String* str, int off, int* end) {
    assert(str);

    int start = 0;
    for (start = 0; isgraph(*(str->buf + off + start)) == 0 && off + start != str->len; start++);
    if (off + start == str->len)
        return NULL;
    
    char* word = str->buf + off + start;

    if (memchr(str->buf + off + start, ' ', str->len - off - start) != NULL) {
        *end = (char*)memchr(str->buf + off + start, ' ', str->len - off - start) - str->buf;
    } else if ((char*)memchr(str->buf + off + start, '\n', str->len - off - start) != NULL) {
        *end = (char*)memchr(str->buf + off + start, '\n', str->len - off - start) - str->buf;
    } else {
        *end = str->len;
    }

    return word;
}

char GetArgType(char* arg, int len) {
    int off = 0;
    
    while (isdigit(*(arg + off)) != 0 || *(arg + off) == '.' ||*(arg + off) == '-' || *(arg + off) == '+')
        off++;

    if (off == len)
        // argument is numeric
        return 'n';   
    else
        // argument is alphabetic
        return 'r';
}

int _FindLabel(ExecBuf* eBuf, String* line, char* lblPtr, int off) {
    int lblNum = 0;

    for (lblNum; lblNum < eBuf->maxLbl; lblNum++) {
        int minLen = 0;
        int eq = 0;

        if (eBuf->lbls[lblNum].name != NULL) {
            int lblLen = strlen(eBuf->lbls[lblNum].name);
            int argLen = off - (lblPtr - line->buf);
            minLen = (lblLen < argLen) ? (lblLen) : (argLen);
            eq = (lblLen == argLen) ? (1) : (0);
        }
        if (eBuf->lbls[lblNum].name && eq == 1 && strncmp(eBuf->lbls[lblNum].name, lblPtr, minLen) == 0) {
            *(int*)(eBuf->cmds + eBuf->curCmd) = eBuf->lbls[lblNum].addr;
            eBuf->curCmd += NUM_SZ;
            break;
        }
    }

    if (lblNum == eBuf->maxLbl && *(int*)(eBuf->cmds + eBuf->curCmd) != -1) {
        *(int*)(eBuf->cmds + eBuf->curCmd) = -1;
        eBuf->curCmd += NUM_SZ;   
    } else if (lblNum == eBuf->maxLbl && *(int*)(eBuf->cmds + eBuf->curCmd) == -1) {
        eBuf->err = E_UNDEC_LBL;
        return 1;
    }

    return 0;
}

int EbufAddReg(ExecBuf* eBuf, char* arg) {
    if (strncmp("ax", arg, 2) == 0) {
        *(char*)(eBuf->cmds + eBuf->curCmd) = 'a';
        eBuf->curCmd += REG_SZ;
    } else if (strncmp("bx", arg, 2) == 0) {
        *(char*)(eBuf->cmds + eBuf->curCmd) = 'b';
        eBuf->curCmd += REG_SZ;
    } else if (strncmp("cx", arg, 2) == 0) {
        *(char*)(eBuf->cmds + eBuf->curCmd) = 'c';
        eBuf->curCmd += REG_SZ;
    } else if (strncmp("dx", arg, 2) == 0) {
        *(char*)(eBuf->cmds + eBuf->curCmd) = 'd';
        eBuf->curCmd += REG_SZ;
    } else if (strncmp("ex", arg, 2) == 0) {
        *(char*)(eBuf->cmds + eBuf->curCmd) = 'e';
        eBuf->curCmd += REG_SZ;
    } else {
        eBuf->err = E_INV_ARG;
        return 1;
    }
}