#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

//####################//

const int LABEL_NAME_MAX_SZ = 10;
const int EXECBUF_INIT_BUF_SZ = 100;
const int EXECBUF_INIT_LBLS_SZ = 10;
const int PRECISION = 1000;

//####################//

// compilation errors
enum _AsmErrs {
    OK,
    E_INV_LBL_NAME,
    E_INV_ARG_TYPE,
    E_INV_CMD,
    E_NO_ARG,
    E_INV_ARG,
    E_FEW_ARGS,
    E_MANY_ARGS,
    E_UNDEC_LBL,
    //E_NO_END_SEQ,
};
typedef enum _AsmErrs AsmErrs;

// descriptions for copiilation errors
const char* AsmErrsDesc[] = {
    "compiled without errors\n",
    "invalid label name!\n",
    "invalid argument type!\n",
    "invalid instruction sequence!\n",
    "expected an argument, but none given!\n",
    "invalig argument given!\n",
    "too few argumentd given!\n",
    "too many arguments given!\n",
    "undeclared label!\n",
    //"no end sequence given!\n",     
};

struct _Str {
    char*   buf;
    int     len;
};
typedef struct _Str Str;

struct _Label {
    char*   name;
    int     addr;
};
typedef struct _Label Label;

struct _ExecBuf {
    char*   buf;
    int     maxBuf;
    int     curBuf;
    Label*  lbls;
    int     maxLbl;
    int     curLbl;
    AsmErrs err;
};
typedef struct _ExecBuf ExecBuf;

//####################//

Str* StrAlloc();
int StrInit(Str* str, int len);
int StrFree(Str* str);

Label* LabelAlloc();
int LabelInit(Label* lbl, int len);
int LabelFree(Label* lbl);

ExecBuf* ExecBufAlloc();
int ExecBufInit(ExecBuf* eBuf);
int ExecBufFree(ExecBuf* eBuf);

int Compile(char* in, ExecBuf* eBuf);
Str* CodeRead(char* name);
int LineInterpret(Str* line, ExecBuf* eBuf);

int CountLines(char* buf, int len);
Str* DivideByLines(char* buf, int len, int nLines);
char* FindAndReplace(char* buf, int len, char old, char new);
char* FindWord(Str* str, int off, int* end);
int IsWhitespace(Str* str, int off);
char GetArgType(char* arg, int len);

//####################//

int main(int argc , char *argv[]) {
    ExecBuf* eBuf = ExecBufAlloc();

    if (ExecBufInit(eBuf) != 0) {
        printf("unable to initialize translation buffer!\n");
        exit(-1);
    }

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

    printf("/ after compilation:\n/ %d~(bytes) to store commands, %d~(bytes) to store labels\n", eBuf->maxBuf, eBuf->maxLbl * sizeof(Label));

    ExecBufFree(eBuf);

    return 0;
}

//####################//

Str* StrAlloc() {
    Str* str = calloc(1, sizeof(Str));

    return str;
}

int StrInit(Str* str, int len) {
    assert(str);

    str->buf = calloc(len, sizeof(char));
    str->len = len;

    return 0;
}

int StrFree(Str* str) {
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

    eBuf->buf = calloc(EXECBUF_INIT_BUF_SZ, sizeof(char));
    eBuf->maxBuf = EXECBUF_INIT_BUF_SZ;
    eBuf->curBuf = 0;

    eBuf->lbls = calloc(EXECBUF_INIT_LBLS_SZ, sizeof(Label));
    eBuf->maxLbl = EXECBUF_INIT_LBLS_SZ;
    eBuf->curLbl = 0;
    eBuf->err = OK;

    if (eBuf->buf && eBuf->lbls)
        return 0;
    else    
        return -1;
}

int ExecBufFree(ExecBuf* eBuf) {
    assert(eBuf);
    assert(eBuf->buf);
    assert(eBuf->lbls);

    free(eBuf->buf);

    for (int i = 0; i < eBuf->curLbl; i++)
        LabelFree(&((eBuf->lbls)[i]));

    free(eBuf->lbls);
    free(eBuf);

    return 0;
}

int Compile(char* inName, ExecBuf* eBuf) {
    assert(inName);
    assert(eBuf);

    printf("///////////////////////\n"
           "/ compilation started /\n"
           "///////////////////////\n");

    Str* code = CodeRead(inName);

    int nLines = CountLines(code->buf, code->len);
    Str* matrix = DivideByLines(code->buf, code->len, nLines);

    for (int i = 0; i < nLines; i++) {
        //printf("%d>>\n", i);
        if (LineInterpret(&(matrix[i]), eBuf) != 0) {
            eBuf->maxBuf = eBuf->curBuf;
            eBuf->curBuf = 0;
            eBuf->maxLbl = eBuf->curLbl;
            eBuf->curLbl = 0;

            StrFree(code);
            free(matrix);

            return i + 1;
        }
    }

    eBuf->maxBuf = eBuf->curBuf;
    eBuf->curBuf = 0;
    eBuf->maxLbl = eBuf->curLbl;
    eBuf->curLbl = 0;

    StrFree(code);
    free(matrix);

    printf("/////////////////////\n"
           "/ compilation ended /\n"
           "/////////////////////\n\n");

    return 0;
}

Str* CodeRead(char* name) {
    assert(name);

    Str* code = StrAlloc();

    int fd = open(name, O_RDONLY);
    if (fd == -1) {
        printf("error openning %s !", name);
        exit(-1);
    }

    StrInit(code, lseek(fd, 0, SEEK_END));
    lseek(fd, 0, SEEK_SET);

    read(fd, code->buf, code->len);
    close(fd);

    return(code);
}

int LineInterpret(Str* line, ExecBuf* eBuf) {
    assert(line);
    assert(eBuf);
    assert(line->buf);
    assert(eBuf->buf);
    assert(eBuf->lbls);

    // delete comments
    if (FindAndReplace(line->buf, line->len, ';', '\0') != NULL)
        line->len = (char*)memchr(line->buf, '\0', line->len) - line->buf;

    // check if empty
    if (line->len == 0)
        return 0;
    
    // check if whitespace
    if (IsWhitespace(line, 0) == 1)
        return 0;

    int off = 0;
    char* cmd = FindWord(line, off, &off);
    
    // looking for labels
    char* lblPos = memchr(line->buf, ':', line->len);
    if (lblPos == cmd) {
        eBuf->err = E_INV_LBL_NAME;
        return 1;
    }

    if (lblPos != NULL) {
        // if label found
        if (eBuf->curLbl >= eBuf->maxLbl - 1) {
            eBuf->lbls = realloc(eBuf->lbls, eBuf->maxLbl * 2);
            eBuf->maxLbl *= 2;
        }

        LabelInit(&(eBuf->lbls[eBuf->curLbl]), lblPos - cmd + 1);
        eBuf->lbls[eBuf->curLbl].addr = eBuf->curBuf;
        memcpy(eBuf->lbls[eBuf->curLbl].name, cmd, lblPos - cmd);
        printf("label found! addr: %d, name: %s\n", eBuf->lbls[eBuf->curLbl].addr, eBuf->lbls[eBuf->curLbl].name);
        eBuf->curLbl++;

        return 0;
    } else {
        // if label not found
        if (eBuf->curBuf >= eBuf->maxBuf - 64) {
            eBuf->buf = realloc(eBuf->buf, eBuf->maxBuf * 2);
            eBuf->maxBuf *= 2;
        }

        #define CMD_DEF(name, num, codeAsm, codeCpu) if (strncmp(name, cmd, strlen(name)) == 0) {printf("read: %s\n", name); codeAsm;/* return 0;*/}
    
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

    for (char* s = memchr(buf, '\n', len); s != NULL; s = memchr(s + 1, '\n', len - (s - buf))) {
        /*
        if (s == buf || *(s - 1) == '\n')
            continue;
        else
            nLines++;
        */
        nLines++;
    }

    if (buf[len - 1] != '\n' && buf[len - 1] != '\0')
        nLines++;

    return nLines;
}

Str* DivideByLines(char* buf, int len, int nLines) {
    assert(buf);

    Str* matrix = (Str*)calloc(nLines , sizeof(Str));
    int i = 0;

    /*
    if (*(buf) != '\n') {
        matrix[0].buf = buf;
        i++;
    }

    for (char* s = memchr(buf, '\n', len); s != NULL; s = memchr(s + 1, '\n', len - (s - buf))) {

        if (s != buf && *(s - 1) != '\n' && *(s - 1) != '\0')
            matrix[i - 1].len = s - matrix[i - 1].buf + 1;

        if (*(s + 1) != '\n' && *(s + 1) != '\0') {
            matrix[i].buf = s + 1;
            i++;
        }
    }
    */

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
        //printf("%d\n", i);
    }

    if (i == nLines)
        matrix[i - 1].len = buf + len - matrix[i - 1].buf + 1;

    for (int i = 0; i < nLines; i++) {
        //printf(">%d\n", matrix[i].len);
    }

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

char* FindWord(Str* str, int off, int* end) {
    assert(str);

    int start = 0;
    for (start = 0; isalnum(*(str->buf + off + start)) == 0 && off + start != str->len; start++);
    if (off + start == str->len)
        return NULL;
    
    char* word = str->buf + off + start;

    *end = (char*)memchr(str->buf + off + start, ' ', str->len - off - start) - str->buf;
    if (memchr(str->buf + off + start, ' ', str->len - off - start) == NULL)
        *end = str->len;
    
    return word;
}

int IsWhitespace(Str* str, int off) {
    int flag = 1;

    for (int i = 0; i < str->len - off; i++) {
        if (isspace((str->buf)[off + i]) == 0) {
            flag = 0;
            break;
        }
    }

    return flag;
}

char GetArgType(char* arg, int len) {
    int off = 0;
    
    while (isdigit(*(arg + off)) != 0 || *(arg + off) == '.')
        off++;

    if (off == len)
        // argument is numeric
        return 'n';   
    else
        // argument is alphabetic
        return 'r';
}
