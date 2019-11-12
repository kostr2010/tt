#ifndef TEXT_H_INCLUDED
#define TEXT_H_INCLUDED

//####################//

// string structure
struct _String {
    char*   buf;    // pointer to the beginning of string
    int     len;    // string length
};
typedef struct _String String;

//####################//

/**
 * @brief allocates memory to String structure.
 * 
 * @return String* str  - pointer to allocated memory block.
 */
String* StringAlloc();
/**
 * @brief initializes String type structure. memory should be previously allocated via StringAlloc().
 * 
 * 
 * @param String* str   - pointer to the structure;
 * @param int len       - length of the string.
 * 
 * @return int res      - if 0, initialized successfully, if 1 - error occured. 
 */
int StringInit(String* str, int len);
/**
 * @brief deallocates memory, allocated for String structure.
 * 
 * @param String* str   - pointer to the structure.
 * 
 * @return int res      - if 0, freed successfully, if 1 - error occured.
 * 
 */
int StringFree(String* str);

/**
 * @brief соunts all strings in the buffer, indicated by buf in the first len bytes
 * 
 * @param char* buf     - pointer to the beginning of buffer;
 * @param int len       - length of the buffer.
 * 
 * @return int res      - number of lines in the buffer. 
 */
int CountLines(char* buf, int len);
/**
 * @brief divides given buffer buf by lines, creating an array of pointers to String structure.
 * 
 * @param char* buf     - pointer to the beginning of buffer;
 * @param int len       - length of the buffer;
 * @param int nLines    - number of lines in buffer. can be obtained via CountLines() function.
 * 
 * @return String* res  - array of String structures, where each element is the string in buf. 
 */
String* DivideByLines(char* buf, int len, int nLines);
/**
 * @brief finds the first entry of old in first len bytes of buf and replaces it with new.
 * 
 * @param char* buf     - pointer to the beginning of buffer;
 * @param int len       - length of the buffer;
 * @param char old      - character to replace;
 * @param char new      - character to replace old with.
 * 
 * @return char* res    - pointer to the found and replaced symbol. 
 */
char* FindAndReplace(char* buf, int len, char old, char new);
/**
 * @bief looks for the word (non-blank sequence of chars) in the str with offset off from beginning of the buffer.
 * 
 * @param String* str   - pointer to the string to search word in;
 * @param int off       - offset from the beginning of the buffer;
 * @param int* end      - here the offset of the end of the wors is placed.
 * 
 * @return char* res    - pointer to the beginning of the word.
 */
char* FindWord(String* str, int off, int* end);
/**
 * @brief determines if the rest of the str starting from of bytes is whitespace.
 * 
 * @param String* str   - pointer to the string to search word in;
 * @param int off       - offset.
 * 
 * @return int res      - 1 if it is whitespace, 0 if not.
 */
int IsWhitespace(String* str, int off);
 
//####################//
 
#endif
