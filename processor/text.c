#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "text.h"

//###################//

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

    if (str->buf != NULL)
        free(str->buf);
    free(str);

    return 0;
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