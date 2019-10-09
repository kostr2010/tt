#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

/**
 * @brief structure implementing simple string type.
 * 
 * @param char* buf - pointer to the beginning of the string;
 * @param int len - length of the string. 
 */
struct _Str_t {
    char* buf;
    int len;
};
typedef struct _Str_t Str_t;

/**
 * @brief reads .txt file with given name/path and returns char* array, containing file data.
 *
 * @param char* filename - name of .txt file or it's full path.
 *
 * @return char* textBuf - buffer, containing .txt's data, NULL if it was impossible to read file.
 *
 * @note !!!free after use!!!!
 */
Str_t ReadText(char* filename);

/**
 * @brief gets a string (array of chars) and returns matrix of: pointer to the first non-space symbol and pointers to the beginning of every new non-empty line in buf.
 *
 * @param char* buf - pointer to the beginning of the buffer;
 * @param int len - length of the buffer;
 * @param int nLines - quantity of non-empty lines in the buffer.
 *
 * @return Str_t* matrix - array of struct _Str_t (a.k.a. Str_t), where matrix[i].buf - pointer to beginning of the non-empty line, matrix[i].len - it's length, including '\n' or '\0' at the end.
 */
Str_t* DivideByLines(char* buf, int len, int nLines);

/**
 * @brief looks for the first entry of old and replaces it with new.
 *
 * @param char* buf - pointer to the beginning of the buffer;
 * @param int len - length of the buffer;
 * @param int old - char to replace;
 * @param int new - char to replace with.
 *
 * @return char* s - pointerto modified symbol.
 */
char* FindAndReplace(char* buf, int len, char old, char new);

/**
 * @brief macro for FindAndReplace, but instead of replacing only the first entry, replaces all of them.
 *
 * @param char* buf - pointer to the beginning of the buffer;
 * @param int len - length of the buffer;
 * @param int old - char to replace;
 * @param int new - char to replace with.
 *
 * @return nothing.
 */
void FindAndReplaceAll(char* buf, int len, char old, char new);

/**
 * @brief counts all non-empty lines in the given buffer.
 *
 * @param char* buf - pointer to the beginning of the buffer;
 * @param int len - length of the buffer.
 *
 * @return int nLines - quantity of non-empty lines in the buffer.
 */
int CountLines(char* buf, int len);

/**
 * @brief quicksort implementation.
 *
 * @param void* array - array to sort;
 * @param int nElem - quantity of elements in array;
 * @param int sizeElem - size of one element in bytes;
 * @param int (*Compare)(void* elem1, void* elem2) - pointer to comparing function.
 *
 * @return nothing.
 */
void QuickSort(void* array, int nElem, int sizeElem, int (*Compare)(void* elem1, void* elem2));

/**
 * @brief comparing function for Str_t type elements. compares strings from beginning to the end, ignoring all non-alphabetical sumbols and register.
 *
 * @param Str_t* str1 - first object of comparation;
 * @param Str_t* str2 - second object of comparation.
 *
 * @return int res - if res < 0, then str1 < str2, if res == 0, then str1 == str2, if res > 0, then str1 > str2.
 */
int CompareFw(Str_t* str1, Str_t* str2);

/**
 * @brief comparing function for Str_t type elements. compares strings from the end to beginning, ignoring all non-alphabetical sumbols and register.
 *
 * @param Str_t* str1 - first object of comparation;
 * @param Str_t* str2 - second object of comparation.
 *
 * @return int res - if res < 0, then str1 < str2, if res == 0, then str1 == str2, if res > 0, then str1 > str2.
 */
int CompareBw(Str_t* str1, Str_t* str2);

/**
 * @brief this function finds first alphabetical character in buf, starting from buf + offset position.
 *
 * char* buf - array, in which we search;
 * int len - length of given array;
 * int offset - offset from beginning in bytes;
 *
 * @return offset - offset of the first alphabetical symbol, starting from the beginning of array;
 */
int FindAlphaFw(char* buf, int len, int offset);

/**
 * @brief this function finds last alphabetical character in buf, starting from buf + len - offset position.
 *
 * char* buf - array, in which we search;
 * int len - length of given array;
 * int offset - offset from the end in bytes;
 *
 * @return offset - offset of the first alphabetical symbol, starting from the end of array;
 */
int FindAlphaBw(char* buf, int len, int offset);

/**
 * @brief function to write out the results of the Onegin task.
 */
void OWriteOutSorted(Str_t* matrixOrig, Str_t* matrixSoertedFw, Str_t* matrixSortedBw, int nLines, char* filename);

/**
 * @brief function to write out the results of the Onegin task.
 */
void OReturnAsItWas(Str_t* textBuf);

/**
 * @brief checks if there are any files with given name. if there are not, creates the new one avaliable for writing and reading. if there are any files with that name, it clears them clean.
 *
 * @param char* filename - name of the desired file.
 */
void CreateNewFile(char* filename);

/**
 * @brief - destructor function for Str_t data type.
 * 
 * @param Str_t* string - pointer to the desired string.
 * 
 * @return nothing. 
 */
void freeStr_t(Str_t* string);

int main() {
    char* filenameIn = "input.txt";
    char* filenameOut = "output.txt";

    printf("reading original text...\n");
    Str_t textBuf = ReadText(filenameIn);
    printf("done.\n\n");

    if (textBuf.buf == NULL) {
        printf("no file with such name or path.\n");
        return -2;
    }

    int nLines = CountLines(textBuf.buf, textBuf.len);

    if (nLines == 0) {
        printf("the file is empty or consists only of white tabs\n");
        return -1;
    }

    Str_t* matrixOrig = DivideByLines(textBuf.buf, textBuf.len, nLines);
    FindAndReplaceAll(textBuf.buf, textBuf.len, '\n', '\0');

    printf("lines: %d\n", nLines);

    Str_t* matrixSortedFw = (Str_t*)calloc(nLines, sizeof(Str_t));
    Str_t* matrixSortedBw = (Str_t*)calloc(nLines, sizeof(Str_t));

    memcpy(matrixSortedFw, matrixOrig, nLines * sizeof(Str_t));
    memcpy(matrixSortedBw, matrixOrig, nLines * sizeof(Str_t));

    printf("sorting from beginning...\n");
    //QuickSort(matrixSortedFw, nLines, sizeof(Str_t), &CompareFw);
    qsort(matrixSortedFw, nLines, sizeof(Str_t), &CompareFw);
    printf("done.\n\n");

    printf("sorting from the end...\n");
    qsort(matrixSortedBw, nLines, sizeof(Str_t), &CompareBw);
    //QuickSort(matrixSortedBw, nLines, sizeof(Str_t), &CompareBw);
    printf("done.\n");

    OReturnAsItWas(&textBuf);

    printf("writing out sorted text...\n");
    CreateNewFile(filenameOut);
    OWriteOutSorted(matrixOrig, matrixSortedFw, matrixSortedBw, nLines, filenameOut);
    printf("done. check output.txt\n");

    free(textBuf.buf);
    free(matrixSortedFw);
    free(matrixSortedBw);
    free(matrixOrig);

    return 0;
}

Str_t ReadText(char* filename) {
    assert(filename);

    int fildes = open(filename, O_RDWR);
    int textLen = lseek(fildes, 0, SEEK_END);

    lseek(fildes, 0, SEEK_SET);

    Str_t textBuf = (Str_t){(char*)calloc(textLen, sizeof(char)), textLen};

    read(fildes, textBuf.buf, textLen);
    close(fildes);

    return textBuf;
}

Str_t* DivideByLines(char* buf, int len, int nLines) {
    assert(buf);

    Str_t* matrix = (Str_t*)calloc(nLines , sizeof(Str_t));
    int i = 0;

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

    if (i == nLines)
        matrix[i - 1].len = buf + len - matrix[i - 1].buf + 1;

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

int CountLines(char* buf, int len) {
    assert(len >= 0);
    assert(buf);

    int nLines = 0;

    for (char* s = memchr(buf, '\n', len); s != NULL; s = memchr(s + 1, '\n', len - (s - buf))) {
        if (s == buf || *(s - 1) == '\n')
            continue;
        else
            nLines++;
    }

    if (buf[len - 1] != '\n' && buf[len - 1] != '\0')
        nLines++;

    return nLines;
}

void QuickSort(void* array, int nElem, int sizeElem, int (*Compare)(void* elem1, void* elem2)) {
    assert(array);

    if (nElem < 2)
        return;

    void* pivot = array;
    int i = 0, j = 0;

    for (i = 0, j = nElem - 1; j >= 0 && i < nElem ; i++, j--) {
        while (Compare(array + i * sizeElem, pivot) < 0)
            i++;

        while (Compare(array + j * sizeElem, pivot) > 0)
            j--;

        if (i > j)
            break;

        void* tmp = calloc(1, sizeElem);

        memcpy(tmp, array + i * sizeElem, sizeElem);
        memcpy(array + i * sizeElem, array + j * sizeElem, sizeElem);
        memcpy(array + j * sizeElem, tmp, sizeElem);

        free(tmp);
    }

    QuickSort(array, i, sizeElem, *Compare);
    QuickSort(array + i * sizeElem, nElem - i, sizeElem, *Compare);
}

int CompareFw(Str_t* str1, Str_t* str2) {
    assert(str1);
    assert(str2);
    assert(str1->buf);
    assert(str2->buf);

    int off1 = FindAlphaFw(str1->buf, str1->len, 0);
    int off2 = FindAlphaFw(str2->buf, str2->len, 0);

    while (tolower(str1->buf[off1]) == tolower(str2->buf[off2])) {
        off1 = FindAlphaFw(str1->buf, str1->len, off1 + 1);
        off2 = FindAlphaFw(str1->buf, str2->len, off2 + 1);

        if (str1->buf[off1] == '\0' || str2->buf[off2] == '\0')
            return tolower(str1->buf[off1]) - tolower(str2->buf[off2]);
    }

    return tolower(str1->buf[off1]) - tolower(str2->buf[off2]);
}

int CompareBw(Str_t* str1, Str_t* str2) {
    assert(str1);
    assert(str2);
    assert(str1->buf);
    assert(str2->buf);

    int off1 = FindAlphaBw(str1->buf, str1->len, 0);
    int off2 = FindAlphaBw(str2->buf, str2->len, 0);

    while (tolower(str1->buf[str1->len - off1 - 1]) == tolower(str2->buf[str2->len - off2 - 1])) {
        off1 = FindAlphaBw(str1->buf, str1->len, off1 + 1);
        off2 = FindAlphaBw(str2->buf, str2->len, off2 + 1);

        if (str1->buf[str1->len - off1 - 1] == '\0' || str2->buf[str2->len - off2 - 1] == '\0')
            return tolower(str1->buf[str1->len - off1 - 1]) - tolower(str2->buf[str2->len - off2 - 1]);
    }

    return tolower(str1->buf[str1->len - off1 - 1]) - tolower(str2->buf[str2->len - off2 - 1]);
}

int FindAlphaFw(char* str, int len, int offset) {
    assert(str);

    while (!isalpha(str[offset]) && str[offset] != '\0' && offset < len - 1)
        offset++;

    return offset;
}

int FindAlphaBw(char* str, int len, int offset) {
    assert(str);

    while (!isalpha(str[len - 1 - offset]) && offset < len)
        offset++;

    if (offset == len)
        return 0;

    return offset;
}

void OWriteOutSorted(Str_t* matrixOrig, Str_t* matrixSortedFw, Str_t* matrixSortedBw, int nLines, char* filename) {
    assert(matrixOrig);
    assert(matrixSortedFw);
    assert(matrixSortedBw);
    assert(filename);

    int fildes = open(filename, O_WRONLY | O_APPEND);
    assert(fildes != -1);

    write(fildes, "sorted from beginning:\n", 23);

    for (int i = 0; i < nLines; i++) {
        write(fildes, matrixSortedFw[i].buf, matrixSortedFw[i].len);
    }
    write(fildes, "\n#########", 10);
    write(fildes, "\n#########", 10);
    write(fildes, "\n#########", 10);
    write(fildes, "\n#########\n\n", 12);

    write(fildes, "sorted from the end:\n", 21);

    for (int i = 0; i < nLines; i++) {
        write(fildes, matrixSortedBw[i].buf, matrixSortedBw[i].len);
    }
    write(fildes, "\n#########", 10);
    write(fildes, "\n#########", 10);
    write(fildes, "\n#########", 10);
    write(fildes, "\n#########\n\n", 12);

    write(fildes, "original:\n", 10);

    for (int i = 0; i < nLines; i++) {
        write(fildes, matrixOrig[i].buf, matrixOrig[i].len);
    }
    write(fildes, "\n#########", 10);
    write(fildes, "\n#########", 10);
    write(fildes, "\n#########", 10);
    write(fildes, "\n#########\n\n", 12);

    close(fildes);
}

void OReturnAsItWas(Str_t* textBuf) {
    assert(textBuf);

    for (char* s = memchr(textBuf->buf, '\0', textBuf->len); s != NULL; s = memchr(s + 1, '\0', textBuf->len - (s - textBuf->buf)))
        *s = '\n';
}

void CreateNewFile(char* filename) {
    assert(filename);

    int fildes = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    close(fildes);
}

void freeStr_t(Str_t* string) {
    assert(string);
    
    free(&string->buf);
}