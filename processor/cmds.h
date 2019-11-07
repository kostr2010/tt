// pushes value either from code or from register to stack
CMD_DEF("push", 'a', 
        {   
            *(eBuf->cmds + eBuf->curCmd) = 'a';
            eBuf->curCmd += CMD_SZ;
            
            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }
            
            char type = GetArgType(arg, off - (arg - line->buf));
            
            if (type == 'n') {
                *((char*)(eBuf->cmds + eBuf->curCmd)) = 'n';
                eBuf->curCmd += ARGTYPE_SZ;
                *((int*)(eBuf->cmds + eBuf->curCmd)) = (int)(atof(arg) * PRECISION);
                printf("  pushed number: %d\n", *((int*)(eBuf->cmds + eBuf->curCmd)));
                eBuf->curCmd += NUM_SZ;
            } else {
                if (strncmp("ax", arg, 2) == 0) {
                    *(char*)(eBuf->cmds + eBuf->curCmd) = 'r';
                    eBuf->curCmd += ARGTYPE_SZ;
                    *(char*)(eBuf->cmds + eBuf->curCmd) = 'a';
                    eBuf->curCmd += REG_SZ;
                } else if (strncmp("bx", arg, 2) == 0) {
                    *(char*)(eBuf->cmds + eBuf->curCmd) = 'r';
                    eBuf->curCmd += ARGTYPE_SZ;
                    *(char*)(eBuf->cmds + eBuf->curCmd) = 'b';
                    eBuf->curCmd += REG_SZ;
                } else if (strncmp("cx", arg, 2) == 0) {
                    *(char*)(eBuf->cmds + eBuf->curCmd) = 'r';
                    eBuf->curCmd += ARGTYPE_SZ;
                    *(char*)(eBuf->cmds + eBuf->curCmd) = 'c';
                    eBuf->curCmd += REG_SZ;
                } else if (strncmp("dx", arg, 2) == 0) {
                    *(char*)(eBuf->cmds + eBuf->curCmd) = 'r';
                    eBuf->curCmd += ARGTYPE_SZ;
                    *(char*)(eBuf->cmds + eBuf->curCmd) = 'd';
                    eBuf->curCmd += REG_SZ;
                } else if (strncmp("ex", arg, 2) == 0) {
                    *(char*)(eBuf->cmds + eBuf->curCmd) = 'r';
                    eBuf->curCmd += ARGTYPE_SZ;
                    *(char*)(eBuf->cmds + eBuf->curCmd) = 'e';
                    eBuf->curCmd += REG_SZ;
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
        {
            char type = mem->cmds[mem->curCmd];

            if (type == 'n') {
                mem->curCmd += ARGTYPE_SZ;
                StackPush(mem->stk, *(int*)(mem->cmds + mem->curCmd));
                printf("pushed: %d\n", *(int*)(mem->cmds + mem->curCmd));
                mem->curCmd += NUM_SZ;
            } else if (type == 'r') {
                mem->curCmd += ARGTYPE_SZ;
                StackPush(mem->stk, mem->regs[mem->cmds[mem->curCmd] - 97]); // a -> 0, b -> 1, c -> 2, d -> 3, e(0) -> 4
                printf("pushed: %d from %cx register\n", mem->regs[mem->cmds[mem->curCmd] - 97], mem->cmds[mem->curCmd]);
                mem->curCmd += REG_SZ;
            } else {
                mem->err = E_CORRUPTED_BIN;
                return mem->err;
            }
        })

// if called with no arguments, pops last value stored in stack, if register is given as an argument, value is stored in it
CMD_DEF("pop",  'b', 
        {
            *(eBuf->cmds + eBuf->curCmd) = 'b';
            eBuf->curCmd += CMD_SZ;

            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                *((char*)(eBuf->cmds + eBuf->curCmd)) = 'e';
                eBuf->curCmd += REG_SZ;
                
                return 0;
            }
            
            char type = GetArgType(arg, off - (arg - line->buf));
            
            if (type == 'n') {
                eBuf->err = E_INV_ARG_TYPE;
                
                return 1;
            } else {
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

            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                return 1;
            }

            return 0;
        }, 
        {   
            if (mem->stk->cur == 0) {
                mem->err = E_FEW_ARGS_IN_STACK;
                return mem->err;
            }

            mem->regs[mem->cmds[mem->curCmd] - 97] = StackPeek(mem->stk);
            printf("popped: %d to %cx register\n", StackPeek(mem->stk), mem->cmds[mem->curCmd]);
            StackPop(mem->stk);
            mem->curCmd += REG_SZ;
        })

// indicates function call
CMD_DEF("call", 'd', 
        {
            *(eBuf->cmds + eBuf->curCmd) = 'd';
            eBuf->curCmd += CMD_SZ;

            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }

            int lblNum = 0;
            for (lblNum; lblNum < eBuf->maxLbl; lblNum++) {
                if (eBuf->lbls[lblNum].name && strncmp(eBuf->lbls[lblNum].name, arg, strlen(eBuf->lbls[lblNum].name)) == 0) {
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
            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                
                return 1;
            }
            printf("  name (1st char): %c, addr: %d\n", *arg, *(int*)(eBuf->cmds + eBuf->curCmd - NUM_SZ));

            return 0;
        }, 
        {   
            mem->curCmd += NUM_SZ;
            printf("shall return to: %d\n", mem->curCmd);
            StackPush(mem->ret, mem->curCmd);
            printf("%d\n", mem->ret->cur);
            mem->curCmd = *(int*)(mem->cmds + mem->curCmd - NUM_SZ);
        })

// indicates end of the function declaration
CMD_DEF("ret",  'e', 
        {
            *(eBuf->cmds + eBuf->curCmd) = 'e';
            eBuf->curCmd += CMD_SZ;

            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                return 1;
            }

            return 0;
        }, 
        {   
            if (mem->ret->cur == 0) {
                mem->err = E_RET_WITHOUT_CALL;
                return mem->err;
            }

            printf("returned to %d\n", StackPeek(mem->ret));
            mem->curCmd = StackPeek(mem->ret);
            StackPop(mem->ret);
        })

// decreases value, stored in register
CMD_DEF("dec",  'f', 
        {
            *(eBuf->cmds + eBuf->curCmd) = 'f';
            eBuf->curCmd += CMD_SZ;

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

            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                return 1;
            }

            return 0;
        }, 
        {
            if (mem->cmds[mem->curCmd] != 'e') {
                double tmp = (double)(mem->regs[mem->cmds[mem->curCmd] - 97]);
                tmp = ((tmp / PRECISION) - 1) * PRECISION;
                mem->regs[mem->cmds[mem->curCmd] - 97] = (int)(tmp);
            }

            mem->curCmd += REG_SZ;
        })

// increases value, stored in register
CMD_DEF("inc",  'g', 
        {
            *(eBuf->cmds + eBuf->curCmd) = 'g';
            eBuf->curCmd += CMD_SZ;

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

            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                return 1;
            }

            return 0;
        }, 
        {
            if (mem->cmds[mem->curCmd] != 'e') {
                double tmp = (double)(mem->regs[mem->cmds[mem->curCmd] - 97]);
                tmp = ((tmp / PRECISION) + 1) * PRECISION;
                mem->regs[mem->cmds[mem->curCmd] - 97] = (int)(tmp);
            }
            
            mem->curCmd += REG_SZ;
        })

// reads number from keyboard
CMD_DEF("in",   'h', 
        {
            *(eBuf->cmds + eBuf->curCmd) = 'h';
            eBuf->curCmd += CMD_SZ;

            return 0;
        }, 
        {
            double input = 0;

            if (scanf("%lf", &input) != 1) {
                mem->err = E_INV_INPUT;
                return mem->err;
            }

            StackPush(mem->stk, (int)(input * PRECISION));
        })

// prints top element of stack
CMD_DEF("out",  'i', 
        {
            *(eBuf->cmds + eBuf->curCmd) = 'i';
            eBuf->curCmd += CMD_SZ;

            return 0;
        },
        {
            if (mem->stk->cur == 0)
                printf("stack is empty right now\n");
            else
                printf("%fl\n", (double)(StackPeek(mem->stk)) / PRECISION);
        })

// jumps to given label
CMD_DEF("jmp", 'j', 
        {
            *(eBuf->cmds + eBuf->curCmd) = 'j';
            eBuf->curCmd += CMD_SZ;
            
            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }

            int lblNum = 0;
            for (lblNum; lblNum < eBuf->maxLbl; lblNum++) {
                if (eBuf->lbls[lblNum].name && strncmp(eBuf->lbls[lblNum].name, arg, strlen(eBuf->lbls[lblNum].name)) == 0) {
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
            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                
                return 1;
            }
            printf("  name (1st char): %c, addr: %d\n", *arg, *(int*)(eBuf->cmds + eBuf->curCmd - NUM_SZ));

            return 0;
        }, 
        {
            mem->curCmd = *(int*)(mem->cmds + mem->curCmd);
        })

// jumps to given label if last stack element is zero
CMD_DEF("jz",   'k', 
        {
            *(eBuf->cmds + eBuf->curCmd) = 'k';
            eBuf->curCmd += CMD_SZ;
            
            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }

            int lblNum = 0;
            for (lblNum; lblNum < eBuf->maxLbl; lblNum++) {
                if (eBuf->lbls[lblNum].name && strncmp(eBuf->lbls[lblNum].name, arg, strlen(eBuf->lbls[lblNum].name)) == 0) {
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
            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                
                return 1;
            }
            printf("  name (1st char): %c, addr: %d\n", *arg, *(int*)(eBuf->cmds + eBuf->curCmd - NUM_SZ));

            return 0;
        }, 
        {   
            if (mem->stk->cur == 0) {
                mem->err = E_FEW_ARGS_IN_STACK;
                return mem->err;
            }

            if (StackPeek(mem->stk) == 0)
                mem->curCmd = *(int*)(mem->cmds + mem->curCmd);
        })

// jumps to given label if last two elements of stack are equal
CMD_DEF("je",   'l', 
        {
            *(eBuf->cmds + eBuf->curCmd) = 'l';
            eBuf->curCmd += CMD_SZ;

            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }

            int lblNum = 0;
            for (lblNum; lblNum < eBuf->maxLbl; lblNum++) {
                if (eBuf->lbls[lblNum].name && strncmp(eBuf->lbls[lblNum].name, arg, strlen(eBuf->lbls[lblNum].name)) == 0) {
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
            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                
                return 1;
            }
            printf("  name (1st char): %c, addr: %d\n", *arg, *(int*)(eBuf->cmds + eBuf->curCmd - NUM_SZ));

            return 0;
        }, 
        {
            if (mem->stk->cur < 2) {
                mem->err = E_FEW_ARGS_IN_STACK;
                return mem->err;
            }
            
            int tmp1 = StackPeek(mem->stk);
            StackPop(mem->stk);

            if (StackPeek(mem->stk) == tmp1)
                mem->curCmd = *(int*)(mem->cmds + mem->curCmd);
            
            StackPush(mem->stk, tmp1);
        })

// jumps to given label if last stack element is non-zero
CMD_DEF("jnz",  'm', 
        {
            *(eBuf->cmds + eBuf->curCmd) = 'm';
            eBuf->curCmd += CMD_SZ;

            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }

            int lblNum = 0;
            for (lblNum; lblNum < eBuf->maxLbl; lblNum++) {
                if (eBuf->lbls[lblNum].name && strncmp(eBuf->lbls[lblNum].name, arg, strlen(eBuf->lbls[lblNum].name)) == 0) {
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
            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                
                return 1;
            }
            printf("  name (1st char): %c, addr: %d\n", *arg, *(int*)(eBuf->cmds + eBuf->curCmd - NUM_SZ));

            return 0;
        }, 
        {
            if (mem->stk->cur != 0) {
                mem->err = E_FEW_ARGS_IN_STACK;
                return mem->err;
            }

            if (StackPeek(mem->stk) == 0)
                mem->curCmd = *(int*)(mem->cmds + mem->curCmd);
        })

// jumps to given label if last two eleents of stack are not equal
CMD_DEF("jne",  'n', 
        {
            *(eBuf->cmds + eBuf->curCmd) = 'n';
            eBuf->curCmd += CMD_SZ;

            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }

            int lblNum = 0;
            for (lblNum; lblNum < eBuf->maxLbl; lblNum++) {
                if (eBuf->lbls[lblNum].name && strncmp(eBuf->lbls[lblNum].name, arg, strlen(eBuf->lbls[lblNum].name)) == 0) {
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
            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                
                return 1;
            }
            printf("  name (1st char): %c, addr: %d\n", *arg, *(int*)(eBuf->cmds + eBuf->curCmd - NUM_SZ));

            return 0;
        },
        {
            if (mem->stk->cur < 2) {
                mem->err = E_FEW_ARGS_IN_STACK;
                return mem->err;
            }
            
            int tmp1 = StackPeek(mem->stk);
            StackPop(mem->stk);

            if (StackPeek(mem->stk) != tmp1)
                mem->curCmd = *(int*)(mem->cmds + mem->curCmd);
            
            StackPush(mem->stk, tmp1);
        })

// jumps to given label if last element of stack is qreater than prior
CMD_DEF("jg",   'o', 
        {
            *(eBuf->cmds + eBuf->curCmd) = 'o';
            eBuf->curCmd += CMD_SZ;

            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }

            int lblNum = 0;
            for (lblNum; lblNum < eBuf->maxLbl; lblNum++) {
                if (eBuf->lbls[lblNum].name && strncmp(eBuf->lbls[lblNum].name, arg, strlen(eBuf->lbls[lblNum].name)) == 0) {
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
            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                
                return 1;
            }
            printf("  name (1st char): %c, addr: %d\n", *arg, *(int*)(eBuf->cmds + eBuf->curCmd - NUM_SZ));

            return 0;
        }, 
        {
            if (mem->stk->cur < 2) {
                mem->err = E_FEW_ARGS_IN_STACK;
                return mem->err;
            }
            
            int tmp1 = StackPeek(mem->stk);
            StackPop(mem->stk);

            if (StackPeek(mem->stk) < tmp1)
                mem->curCmd = *(int*)(mem->cmds + mem->curCmd);
            
            StackPush(mem->stk, tmp1);
        })

// jumps to given label if last element of stack is lesser than prior
CMD_DEF("jl",   'p', 
        {
            *(eBuf->cmds + eBuf->curCmd) = 'p';
            eBuf->curCmd += CMD_SZ;

            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }

            int lblNum = 0;
            for (lblNum; lblNum < eBuf->maxLbl; lblNum++) {
                if (eBuf->lbls[lblNum].name && strncmp(eBuf->lbls[lblNum].name, arg, strlen(eBuf->lbls[lblNum].name)) == 0) {
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
            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                
                return 1;
            }
            printf("  name (1st char): %c, addr: %d\n", *arg, *(int*)(eBuf->cmds + eBuf->curCmd - NUM_SZ));

            return 0;
        }, 
        {
            if (mem->stk->cur < 2) {
                mem->err = E_FEW_ARGS_IN_STACK;
                return mem->err;
            }
            
            int tmp1 = StackPeek(mem->stk);
            StackPop(mem->stk);

            if (StackPeek(mem->stk) > tmp1)
                mem->curCmd = *(int*)(mem->cmds + mem->curCmd);
            
            StackPush(mem->stk, tmp1);
        })

// jumps to given label if last element of stack is qreater or equal than prior
CMD_DEF("jge",  'q', 
        {
            *(eBuf->cmds + eBuf->curCmd) = 'q';
            eBuf->curCmd += CMD_SZ;

            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }

            int lblNum = 0;
            for (lblNum; lblNum < eBuf->maxLbl; lblNum++) {
                if (eBuf->lbls[lblNum].name && strncmp(eBuf->lbls[lblNum].name, arg, strlen(eBuf->lbls[lblNum].name)) == 0) {
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
            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                
                return 1;
            }
            printf("  name (1st char): %c, addr: %d\n", *arg, *(int*)(eBuf->cmds + eBuf->curCmd - NUM_SZ));

            return 0;
        }, 
        {
            if (mem->stk->cur < 2) {
                mem->err = E_FEW_ARGS_IN_STACK;
                return mem->err;
            }
            
            int tmp1 = StackPeek(mem->stk);
            StackPop(mem->stk);

            if (StackPeek(mem->stk) <= tmp1)
                mem->curCmd = *(int*)(mem->cmds + mem->curCmd);
            
            StackPush(mem->stk, tmp1);
        })

// jumps to given label if last element of stack is lesser or equal than prior
CMD_DEF("jle",  'r', 
        {
            *(eBuf->cmds + eBuf->curCmd) = 'r';
            eBuf->curCmd += CMD_SZ;

            char* arg = FindWord(line, off, &off);
            if (arg == NULL) {
                eBuf->err = E_NO_ARG;
                
                return 1;
            }

            int lblNum = 0;
            for (lblNum; lblNum < eBuf->maxLbl; lblNum++) {
                if (eBuf->lbls[lblNum].name && strncmp(eBuf->lbls[lblNum].name, arg, strlen(eBuf->lbls[lblNum].name)) == 0) {
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
            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                
                return 1;
            }
            printf("  name (1st char): %c, addr: %d\n", *arg, *(int*)(eBuf->cmds + eBuf->curCmd - NUM_SZ));

            return 0;
        }, 
        {if (mem->stk->cur < 2) {
                mem->err = E_FEW_ARGS_IN_STACK;
                return mem->err;
            }
            
            int tmp1 = StackPeek(mem->stk);
            StackPop(mem->stk);

            if (StackPeek(mem->stk) >= tmp1)
                mem->curCmd = *(int*)(mem->cmds + mem->curCmd);
            
            StackPush(mem->stk, tmp1);})

// adds last stack element to prior
CMD_DEF("add",  's', 
        {
            *(eBuf->cmds + eBuf->curCmd) = 's';
            eBuf->curCmd += CMD_SZ;

            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                return 1;
            }

            return 0;
        }, 
        {
            if (mem->stk->cur < 2) {
                mem->err = E_FEW_ARGS_IN_STACK;
                return mem->err;
            }
            
            int tmp1 = StackPeek(mem->stk);
            StackPop(mem->stk);
            int tmp2 = StackPeek(mem->stk);
            StackPop(mem->stk);
            
            StackPush(mem->stk, tmp1 + tmp2);
        })

// substracts last stack element from prior
CMD_DEF("sub",  't', 
        {
            *(eBuf->cmds + eBuf->curCmd) = 't';
            eBuf->curCmd += CMD_SZ;

            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                return 1;
            }

            return 0;
        }, 
        {
            if (mem->stk->cur < 2) {
                mem->err = E_FEW_ARGS_IN_STACK;
                return mem->err;
            }
            
            int tmp1 = StackPeek(mem->stk);
            StackPop(mem->stk);
            int tmp2 = StackPeek(mem->stk);
            StackPop(mem->stk);
            
            StackPush(mem->stk, tmp1 - tmp2);
        })

// multiplicates last stack element to prior
CMD_DEF("mul",  'u', 
        {
            *(eBuf->cmds + eBuf->curCmd) = 'u';
            eBuf->curCmd += CMD_SZ;

            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                return 1;
            }

            return 0;
        }, 
        {
            if (mem->stk->cur < 2) {
                mem->err = E_FEW_ARGS_IN_STACK;
                return mem->err;
            }
            
            double tmp1 = (double)StackPeek(mem->stk) / PRECISION;
            StackPop(mem->stk);
            double tmp2 = (double)StackPeek(mem->stk) / PRECISION;
            StackPop(mem->stk);

            StackPush(mem->stk, (int)(tmp1 * tmp2 * PRECISION));
        })

// divides last stack element by prior
CMD_DEF("div",  'v', 
        {
            *(eBuf->cmds + eBuf->curCmd) = 'v';
            eBuf->curCmd += CMD_SZ;

            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                return 1;
            }

            return 0;
        }, 
        {
            if (mem->stk->cur < 2) {
                mem->err = E_FEW_ARGS_IN_STACK;
                return mem->err;
            }
            
            double tmp1 = (double)StackPeek(mem->stk) / PRECISION;
            StackPop(mem->stk);
            double tmp2 = (double)StackPeek(mem->stk) / PRECISION;
            StackPop(mem->stk);

            StackPush(mem->stk, (int)((tmp1 / tmp2) * PRECISION));
        })

// gets square root from last stack element
CMD_DEF("sqrt",  'w', 
        {
            *(eBuf->cmds + eBuf->curCmd) = 'w';
            eBuf->curCmd += CMD_SZ;

            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                return 1;
            }

            return 0;
        }, 
        {
            if (mem->stk->cur == 1) {
                mem->err = E_FEW_ARGS_IN_STACK;
                return 1;
            }
            
            double tmp1 = (double)(StackPeek(mem->stk)) / PRECISION;
            if (tmp1 < 0) {
                mem->err = E_INV_ARG_IN_STACK;
                return mem->err;
            }

            StackPop(mem->stk);
            
            StackPush(mem->stk, (int)(sqrt(tmp1) * PRECISION));
        })

// indicates end of the program
CMD_DEF("end",  'x', 
        {
            *(eBuf->cmds + eBuf->curCmd) = 'x';
            eBuf->curCmd += CMD_SZ;

            if (IsWhitespace(line, off) == 0) {
                eBuf->err = E_MANY_ARGS;
                return 1;
            }

            return 0;
        }, 
        {
            return 0;
        })

/*
// 
CMD_DEF("jle",  'r', 
        {
            *(eBuf->cmds + eBuf->curCmd) = 'r';
            eBuf->curCmd += CMD_SZ;

            return 0;
        }, 
        {})

*/

