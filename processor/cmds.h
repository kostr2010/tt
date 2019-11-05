// CMD_DEF(name, num, codeAsm, codeExec);

// pushes value either from code or from register to stack
CMD_DEF("push", 'a', 
        {
            *(eBuf->buf + eBuf->curBuf) = 'a';
            eBuf->curBuf += sizeof(char);
            
            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }
            
            char type = GetArgType(arg, off - (arg - line->buf));
            
            if (type == 'n') {
                *((char*)(eBuf->buf + eBuf->curBuf)) = 'n';
                eBuf->curBuf += sizeof(char);
                *((int*)(eBuf->buf + eBuf->curBuf)) = (int)(atof(arg) * PRECISION);
                printf("  pushed number: %d\n", *((int*)(eBuf->buf + eBuf->curBuf)));
                eBuf->curBuf += sizeof(int);
            } else {
                if (strncmp("ax", arg, 2) == 0) {
                    *(char*)(eBuf->buf + eBuf->curBuf) = 'r';
                    eBuf->curBuf += sizeof(char);
                    *(char*)(eBuf->buf + eBuf->curBuf) = 'a';
                    eBuf->curBuf += sizeof(char);
                } else if (strncmp("bx", arg, 2) == 0) {
                    *(char*)(eBuf->buf + eBuf->curBuf) = 'r';
                    eBuf->curBuf += sizeof(char);
                    *(char*)(eBuf->buf + eBuf->curBuf) = 'b';
                    eBuf->curBuf += sizeof(char);
                } else if (strncmp("cx", arg, 2) == 0) {
                    *(char*)(eBuf->buf + eBuf->curBuf) = 'r';
                    eBuf->curBuf += sizeof(char);
                    *(char*)(eBuf->buf + eBuf->curBuf) = 'c';
                    eBuf->curBuf += sizeof(char);
                } else if (strncmp("dx", arg, 2) == 0) {
                    *(char*)(eBuf->buf + eBuf->curBuf) = 'r';
                    eBuf->curBuf += sizeof(char);
                    *(char*)(eBuf->buf + eBuf->curBuf) = 'd';
                    eBuf->curBuf += sizeof(char);
                } else {
                    eBuf->err = E_INV_ARG;
                    
                    return 1;
                }
            }

            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                
                return 1;
            }

            return 0;
        }, 
        {})

// if called with no arguments, pops last value stored in stack, if register is given as an argument, value is stored in it
CMD_DEF("pop",  'b', 
        {
            *(eBuf->buf + eBuf->curBuf) = 'b';
            eBuf->curBuf += sizeof(char);

            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                *((char*)(eBuf->buf + eBuf->curBuf)) = '0';
                eBuf->curBuf += sizeof(char);
                
                return 0;
            }
            
            char type = GetArgType(arg, off - (arg - line->buf));
            
            if (type == 'n') {
                eBuf->err = E_INV_ARG_TYPE;
                
                return 1;
            } else {
                if (strncmp("ax", arg, 2) == 0) {
                    *(char*)(eBuf->buf + eBuf->curBuf) = 'a';
                    eBuf->curBuf += sizeof(char);
                } else if (strncmp("bx", arg, 2) == 0) {
                    *(char*)(eBuf->buf + eBuf->curBuf) = 'b';
                    eBuf->curBuf += sizeof(char);
                } else if (strncmp("cx", arg, 2) == 0) {
                    *(char*)(eBuf->buf + eBuf->curBuf) = 'c';
                    eBuf->curBuf += sizeof(char);
                } else if (strncmp("dx", arg, 2) == 0) {
                    *(char*)(eBuf->buf + eBuf->curBuf) = 'd';
                    eBuf->curBuf += sizeof(char);
                } else {
                    eBuf->err = E_INV_ARG;
                    
                    return 1;
                }
            }

            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                return 1;
            }

            return 0;
        }, 
        {})

// indicates function call
CMD_DEF("call", 'd', 
        {
            *(eBuf->buf + eBuf->curBuf) = 'd';
            eBuf->curBuf += sizeof(char);

            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }

            int lblNum = 0;
            for (lblNum; lblNum < eBuf->maxLbl; lblNum++) {
                if (eBuf->lbls[lblNum].name && strncmp(eBuf->lbls[lblNum].name, arg, strlen(eBuf->lbls[lblNum].name)) == 0) {
                    *(int*)(eBuf->buf + eBuf->curBuf) = eBuf->lbls[lblNum].addr;
                    eBuf->curBuf += sizeof(int);

                    break;
                }
            }

            if (lblNum == eBuf->maxLbl && *(int*)(eBuf->buf + eBuf->curBuf) != -1) {
                *(int*)(eBuf->buf + eBuf->curBuf) = -1;
                eBuf->curBuf += sizeof(int);   
            } else if (lblNum == eBuf->maxLbl && *(int*)(eBuf->buf + eBuf->curBuf) == -1) {
                eBuf->err = E_UNDEC_LBL;
                return 1;
            }
            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                
                return 1;
            }
            printf("  name (1st char): %c, addr: %d\n", *arg, *(int*)(eBuf->buf + eBuf->curBuf - sizeof(int)));

            return 0;
        }, 
        {})

// indicates end of the function declaration
CMD_DEF("ret",  'e', 
        {
            *(eBuf->buf + eBuf->curBuf) = 'e';
            eBuf->curBuf += sizeof(char);

            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                return 1;
            }

            return 0;
        }, 
        {})

// decreases value, stored in register
CMD_DEF("dec",  'f', 
        {
            *(eBuf->buf + eBuf->curBuf) = 'f';
            eBuf->curBuf += sizeof(char);

            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }
            
            char type = GetArgType(arg, off - (arg - line->buf));
            
            if (type == 'n') {
                eBuf->err = E_INV_ARG_TYPE;
                
                return 1;
            } else {
                if (strncmp("ax", arg, 2) == 0) {
                    *(char*)(eBuf->buf + eBuf->curBuf) = 'a';
                    eBuf->curBuf += sizeof(char);
                } else if (strncmp("bx", arg, 2) == 0) {
                    *(char*)(eBuf->buf + eBuf->curBuf) = 'b';
                    eBuf->curBuf += sizeof(char);
                } else if (strncmp("cx", arg, 2) == 0) {
                    *(char*)(eBuf->buf + eBuf->curBuf) = 'c';
                    eBuf->curBuf += sizeof(char);
                } else if (strncmp("dx", arg, 2) == 0) {
                    *(char*)(eBuf->buf + eBuf->curBuf) = 'd';
                    eBuf->curBuf += sizeof(char);
                } else {
                    eBuf->err = E_INV_ARG;
                    
                    return 1;
                }
            }

            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                return 1;
            }

            return 0;
        }, 
        {})

// increases value, stored in register
CMD_DEF("inc",  'g', 
        {
            *(eBuf->buf + eBuf->curBuf) = 'g';
            eBuf->curBuf += sizeof(char);

            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }
            
            char type = GetArgType(arg, off - (arg - line->buf));
            
            if (type == 'n') {
                eBuf->err = E_INV_ARG_TYPE;
                
                return 1;
            } else {
                if (strncmp("ax", arg, 2) == 0) {
                    *(char*)(eBuf->buf + eBuf->curBuf) = 'a';
                    eBuf->curBuf += sizeof(char);
                } else if (strncmp("bx", arg, 2) == 0) {
                    *(char*)(eBuf->buf + eBuf->curBuf) = 'b';
                    eBuf->curBuf += sizeof(char);
                } else if (strncmp("cx", arg, 2) == 0) {
                    *(char*)(eBuf->buf + eBuf->curBuf) = 'c';
                    eBuf->curBuf += sizeof(char);
                } else if (strncmp("dx", arg, 2) == 0) {
                    *(char*)(eBuf->buf + eBuf->curBuf) = 'd';
                    eBuf->curBuf += sizeof(char);
                } else {
                    eBuf->err = E_INV_ARG;
                    
                    return 1;
                }
            }

            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                return 1;
            }

            return 0;
        }, 
        {})

// reads number from keyboard
CMD_DEF("in",   'h', 
        {
            *(eBuf->buf + eBuf->curBuf) = 'h';
            eBuf->curBuf += sizeof(char);

            return 0;
        }, 
        {})

// prints top element of stack
CMD_DEF("out",  'i', 
        {
            *(eBuf->buf + eBuf->curBuf) = 'i';
            eBuf->curBuf += sizeof(char);

            return 0;
        },
        {})

// jumps to given label
CMD_DEF("jmp", 'j', 
        {
            *(eBuf->buf + eBuf->curBuf) = 'g';
            eBuf->curBuf += sizeof(char);
            
            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }

            int lblNum = 0;
            for (lblNum; lblNum < eBuf->maxLbl; lblNum++) {
                if (eBuf->lbls[lblNum].name && strncmp(eBuf->lbls[lblNum].name, arg, strlen(eBuf->lbls[lblNum].name)) == 0) {
                    *(int*)(eBuf->buf + eBuf->curBuf) = eBuf->lbls[lblNum].addr;
                    eBuf->curBuf += sizeof(int);

                    break;
                }
            }

            if (lblNum == eBuf->maxLbl && *(int*)(eBuf->buf + eBuf->curBuf) != -1) {
                *(int*)(eBuf->buf + eBuf->curBuf) = -1;
                eBuf->curBuf += sizeof(int);   
            } else if (lblNum == eBuf->maxLbl && *(int*)(eBuf->buf + eBuf->curBuf) == -1) {
                eBuf->err = E_UNDEC_LBL;
                return 1;
            }
            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                
                return 1;
            }
            printf("  name (1st char): %c, addr: %d\n", *arg, *(int*)(eBuf->buf + eBuf->curBuf - sizeof(int)));

            return 0;
        }, 
        {})

// jumps to given label if last stack element is zero
CMD_DEF("jz",   'k', 
        {
            *(eBuf->buf + eBuf->curBuf) = 'k';
            eBuf->curBuf += sizeof(char);
            
            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }

            int lblNum = 0;
            for (lblNum; lblNum < eBuf->maxLbl; lblNum++) {
                if (eBuf->lbls[lblNum].name && strncmp(eBuf->lbls[lblNum].name, arg, strlen(eBuf->lbls[lblNum].name)) == 0) {
                    *(int*)(eBuf->buf + eBuf->curBuf) = eBuf->lbls[lblNum].addr;
                    eBuf->curBuf += sizeof(int);

                    break;
                }
            }

            if (lblNum == eBuf->maxLbl && *(int*)(eBuf->buf + eBuf->curBuf) != -1) {
                *(int*)(eBuf->buf + eBuf->curBuf) = -1;
                eBuf->curBuf += sizeof(int);   
            } else if (lblNum == eBuf->maxLbl && *(int*)(eBuf->buf + eBuf->curBuf) == -1) {
                eBuf->err = E_UNDEC_LBL;
                return 1;
            }
            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                
                return 1;
            }
            printf("  name (1st char): %c, addr: %d\n", *arg, *(int*)(eBuf->buf + eBuf->curBuf - sizeof(int)));

            return 0;
        }, 
        {})

// jumps to given label if last two elements of stack are equal
CMD_DEF("je",   'l', 
        {
            *(eBuf->buf + eBuf->curBuf) = 'l';
            eBuf->curBuf += sizeof(char);

            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }

            int lblNum = 0;
            for (lblNum; lblNum < eBuf->maxLbl; lblNum++) {
                if (eBuf->lbls[lblNum].name && strncmp(eBuf->lbls[lblNum].name, arg, strlen(eBuf->lbls[lblNum].name)) == 0) {
                    *(int*)(eBuf->buf + eBuf->curBuf) = eBuf->lbls[lblNum].addr;
                    eBuf->curBuf += sizeof(int);

                    break;
                }
            }

            if (lblNum == eBuf->maxLbl && *(int*)(eBuf->buf + eBuf->curBuf) != -1) {
                *(int*)(eBuf->buf + eBuf->curBuf) = -1;
                eBuf->curBuf += sizeof(int);   
            } else if (lblNum == eBuf->maxLbl && *(int*)(eBuf->buf + eBuf->curBuf) == -1) {
                eBuf->err = E_UNDEC_LBL;
                return 1;
            }
            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                
                return 1;
            }
            printf("  name (1st char): %c, addr: %d\n", *arg, *(int*)(eBuf->buf + eBuf->curBuf - sizeof(int)));

            return 0;
        }, 
        {})

// jumps to given label if last stack element is non-zero
CMD_DEF("jnz",  'm', 
        {
            *(eBuf->buf + eBuf->curBuf) = 'm';
            eBuf->curBuf += sizeof(char);

            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }

            int lblNum = 0;
            for (lblNum; lblNum < eBuf->maxLbl; lblNum++) {
                if (eBuf->lbls[lblNum].name && strncmp(eBuf->lbls[lblNum].name, arg, strlen(eBuf->lbls[lblNum].name)) == 0) {
                    *(int*)(eBuf->buf + eBuf->curBuf) = eBuf->lbls[lblNum].addr;
                    eBuf->curBuf += sizeof(int);

                    break;
                }
            }

            if (lblNum == eBuf->maxLbl && *(int*)(eBuf->buf + eBuf->curBuf) != -1) {
                *(int*)(eBuf->buf + eBuf->curBuf) = -1;
                eBuf->curBuf += sizeof(int);   
            } else if (lblNum == eBuf->maxLbl && *(int*)(eBuf->buf + eBuf->curBuf) == -1) {
                eBuf->err = E_UNDEC_LBL;
                return 1;
            }
            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                
                return 1;
            }
            printf("  name (1st char): %c, addr: %d\n", *arg, *(int*)(eBuf->buf + eBuf->curBuf - sizeof(int)));

            return 0;
        }, 
        {})

// jumps to given label if last two eleents of stack are not equal
CMD_DEF("jne",  'n', 
        {
            *(eBuf->buf + eBuf->curBuf) = 'n';
            eBuf->curBuf += sizeof(char);

            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }

            int lblNum = 0;
            for (lblNum; lblNum < eBuf->maxLbl; lblNum++) {
                if (eBuf->lbls[lblNum].name && strncmp(eBuf->lbls[lblNum].name, arg, strlen(eBuf->lbls[lblNum].name)) == 0) {
                    *(int*)(eBuf->buf + eBuf->curBuf) = eBuf->lbls[lblNum].addr;
                    eBuf->curBuf += sizeof(int);

                    break;
                }
            }

            if (lblNum == eBuf->maxLbl && *(int*)(eBuf->buf + eBuf->curBuf) != -1) {
                *(int*)(eBuf->buf + eBuf->curBuf) = -1;
                eBuf->curBuf += sizeof(int);   
            } else if (lblNum == eBuf->maxLbl && *(int*)(eBuf->buf + eBuf->curBuf) == -1) {
                eBuf->err = E_UNDEC_LBL;
                return 1;
            }
            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                
                return 1;
            }
            printf("  name (1st char): %c, addr: %d\n", *arg, *(int*)(eBuf->buf + eBuf->curBuf - sizeof(int)));

            return 0;
        },
        {})

// jumps to given label if last element of stack is qreater than prior
CMD_DEF("jg",   'o', 
        {
            *(eBuf->buf + eBuf->curBuf) = 'o';
            eBuf->curBuf += sizeof(char);

            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }

            int lblNum = 0;
            for (lblNum; lblNum < eBuf->maxLbl; lblNum++) {
                if (eBuf->lbls[lblNum].name && strncmp(eBuf->lbls[lblNum].name, arg, strlen(eBuf->lbls[lblNum].name)) == 0) {
                    *(int*)(eBuf->buf + eBuf->curBuf) = eBuf->lbls[lblNum].addr;
                    eBuf->curBuf += sizeof(int);

                    break;
                }
            }

            if (lblNum == eBuf->maxLbl && *(int*)(eBuf->buf + eBuf->curBuf) != -1) {
                *(int*)(eBuf->buf + eBuf->curBuf) = -1;
                eBuf->curBuf += sizeof(int);   
            } else if (lblNum == eBuf->maxLbl && *(int*)(eBuf->buf + eBuf->curBuf) == -1) {
                eBuf->err = E_UNDEC_LBL;
                return 1;
            }
            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                
                return 1;
            }
            printf("  name (1st char): %c, addr: %d\n", *arg, *(int*)(eBuf->buf + eBuf->curBuf - sizeof(int)));

            return 0;
        }, 
        {})

// jumps to given label if last element of stack is lesser than prior
CMD_DEF("jl",   'p', 
        {
            *(eBuf->buf + eBuf->curBuf) = 'p';
            eBuf->curBuf += sizeof(char);

            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }

            int lblNum = 0;
            for (lblNum; lblNum < eBuf->maxLbl; lblNum++) {
                if (eBuf->lbls[lblNum].name && strncmp(eBuf->lbls[lblNum].name, arg, strlen(eBuf->lbls[lblNum].name)) == 0) {
                    *(int*)(eBuf->buf + eBuf->curBuf) = eBuf->lbls[lblNum].addr;
                    eBuf->curBuf += sizeof(int);

                    break;
                }
            }

            if (lblNum == eBuf->maxLbl && *(int*)(eBuf->buf + eBuf->curBuf) != -1) {
                *(int*)(eBuf->buf + eBuf->curBuf) = -1;
                eBuf->curBuf += sizeof(int);   
            } else if (lblNum == eBuf->maxLbl && *(int*)(eBuf->buf + eBuf->curBuf) == -1) {
                eBuf->err = E_UNDEC_LBL;
                return 1;
            }
            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                
                return 1;
            }
            printf("  name (1st char): %c, addr: %d\n", *arg, *(int*)(eBuf->buf + eBuf->curBuf - sizeof(int)));

            return 0;
        }, 
        {})

// jumps to given label if last element of stack is qreater or equal than prior
CMD_DEF("jge",  'q', 
        {
            *(eBuf->buf + eBuf->curBuf) = 'q';
            eBuf->curBuf += sizeof(char);

            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }

            int lblNum = 0;
            for (lblNum; lblNum < eBuf->maxLbl; lblNum++) {
                if (eBuf->lbls[lblNum].name && strncmp(eBuf->lbls[lblNum].name, arg, strlen(eBuf->lbls[lblNum].name)) == 0) {
                    *(int*)(eBuf->buf + eBuf->curBuf) = eBuf->lbls[lblNum].addr;
                    eBuf->curBuf += sizeof(int);

                    break;
                }
            }

            if (lblNum == eBuf->maxLbl && *(int*)(eBuf->buf + eBuf->curBuf) != -1) {
                *(int*)(eBuf->buf + eBuf->curBuf) = -1;
                eBuf->curBuf += sizeof(int);   
            } else if (lblNum == eBuf->maxLbl && *(int*)(eBuf->buf + eBuf->curBuf) == -1) {
                eBuf->err = E_UNDEC_LBL;
                return 1;
            }
            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                
                return 1;
            }
            printf("  name (1st char): %c, addr: %d\n", *arg, *(int*)(eBuf->buf + eBuf->curBuf - sizeof(int)));

            return 0;
        }, 
        {})

// jumps to given label if last element of stack is lesser or equal than prior
CMD_DEF("jle",  'r', 
        {
            *(eBuf->buf + eBuf->curBuf) = 'r';
            eBuf->curBuf += sizeof(char);

            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }

            int lblNum = 0;
            for (lblNum; lblNum < eBuf->maxLbl; lblNum++) {
                if (eBuf->lbls[lblNum].name && strncmp(eBuf->lbls[lblNum].name, arg, strlen(eBuf->lbls[lblNum].name)) == 0) {
                    *(int*)(eBuf->buf + eBuf->curBuf) = eBuf->lbls[lblNum].addr;
                    eBuf->curBuf += sizeof(int);

                    break;
                }
            }

            if (lblNum == eBuf->maxLbl && *(int*)(eBuf->buf + eBuf->curBuf) != -1) {
                *(int*)(eBuf->buf + eBuf->curBuf) = -1;
                eBuf->curBuf += sizeof(int);   
            } else if (lblNum == eBuf->maxLbl && *(int*)(eBuf->buf + eBuf->curBuf) == -1) {
                eBuf->err = E_UNDEC_LBL;
                return 1;
            }
            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                
                return 1;
            }
            printf("  name (1st char): %c, addr: %d\n", *arg, *(int*)(eBuf->buf + eBuf->curBuf - sizeof(int)));

            return 0;
        }, 
        {})

// adds last stack element to prior
CMD_DEF("add",  's', 
        {
            *(eBuf->buf + eBuf->curBuf) = 's';
            eBuf->curBuf += sizeof(char);

            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                return 1;
            }

            return 0;
        }, 
        {})

// substracts last stack element from prior
CMD_DEF("sub",  't', 
        {
            *(eBuf->buf + eBuf->curBuf) = 't';
            eBuf->curBuf += sizeof(char);

            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                return 1;
            }

            return 0;
        }, 
        {})

// multiplicates last stack element to prior
CMD_DEF("mul",  'u', 
        {
            *(eBuf->buf + eBuf->curBuf) = 'u';
            eBuf->curBuf += sizeof(char);

            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                return 1;
            }

            return 0;
        }, 
        {})

// divides last stack element by prior
CMD_DEF("div",  'v', 
        {
            *(eBuf->buf + eBuf->curBuf) = 'v';
            eBuf->curBuf += sizeof(char);

            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                return 1;
            }

            return 0;
        }, 
        {})

// gets square root from last stack element
CMD_DEF("sqrt",  'w', 
        {
            *(eBuf->buf + eBuf->curBuf) = 'w';
            eBuf->curBuf += sizeof(char);

            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                return 1;
            }

            return 0;
        }, 
        {})

// indicates end of the program
CMD_DEF("end",  'x', 
        {
            *(eBuf->buf + eBuf->curBuf) = 'x';
            eBuf->curBuf += sizeof(char);

            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                return 1;
            }

            return 0;
        }, 
        {})

/*
// 
CMD_DEF("jle",  'r', 
        {
            *(eBuf->buf + eBuf->curBuf) = 'r';
            eBuf->curBuf += sizeof(char);

            return 0;
        }, 
        {})

*/

