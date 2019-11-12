#define CHECK_EXTRA_ARGS \
        char* cmd = FindWord(line, off, &off);\
        if (cmd != NULL) {\
            eBuf->err = E_MANY_ARGS;\
            return 1;\
        }

#define CHECK_NO_ARGS \
        if (arg == NULL) {\
                eBuf->err = E_NO_ARG;\
                return 1;\
        }

// pushes value either from code or from register to stack
CMD_DEF("push", 'a', 
        {   
            char* arg = FindWord(line, off, &off);

            CHECK_NO_ARGS;
            
            char type = GetArgType(arg, off - (arg - line->buf));
            
            if (type == 'n') {
                *((char*)(eBuf->cmds + eBuf->curCmd)) = 'n';
                eBuf->curCmd += ARGTYPE_SZ;
                *((int*)(eBuf->cmds + eBuf->curCmd)) = (int)(atof(arg) * PRECISION);
                eBuf->curCmd += NUM_SZ;
            } else {
                *((char*)(eBuf->cmds + eBuf->curCmd)) = 'r';
                eBuf->curCmd += ARGTYPE_SZ;

                if (EbufAddReg(eBuf, arg) == 1)
                    return 1;
            }

            CHECK_EXTRA_ARGS;

            return 0;
        }, 
        {
            char type = mem->cmds[mem->curCmd];

            if (type == 'n') {
                mem->curCmd += ARGTYPE_SZ;
                StackPush(mem->stk, *(int*)(mem->cmds + mem->curCmd));
                mem->curCmd += NUM_SZ;
            } else if (type == 'r') {
                mem->curCmd += ARGTYPE_SZ;
                StackPush(mem->stk, mem->regs[mem->cmds[mem->curCmd] - 97]);
                mem->curCmd += REG_SZ;
            } else {
                mem->err = E_CORRUPTED_BIN;
                return mem->err;
            }
        },
        {
            if (buf[cur] == 'n') {
                cur += ARGTYPE_SZ;
                dprintf(fdOut, "%d", *(int*)(buf + cur));
                cur += NUM_SZ;
            } else if (buf[cur] == 'r') {
                cur += ARGTYPE_SZ;
                dprintf(fdOut, "%cx", buf[cur]);
                cur += REG_SZ;
            }
        })

// if called with no arguments, pops last value stored in stack, if register is given as an argument, value is stored in it
CMD_DEF("pop",  'b', 
        {
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
            } else
                if (EbufAddReg(eBuf, arg) == 1)
                    return 1;

            CHECK_EXTRA_ARGS;

            return 0;
        }, 
        {   
            if (mem->stk->cur == 0) {
                mem->err = E_FEW_ARGS_IN_STACK;
                return mem->err;
            }
            
            if (mem->cmds[mem->curCmd] != 'e')
                mem->regs[mem->cmds[mem->curCmd] - 97] = StackPeek(mem->stk);
        
            
            StackPop(mem->stk);
            mem->curCmd += REG_SZ;
        }, 
        {
            dprintf(fdOut, "%cx", buf[cur]);
            cur += REG_SZ;
        })

// indicates function call
CMD_DEF("call", 'd', 
        {
            char* arg = FindWord(line, off, &off);
            
            CHECK_NO_ARGS;

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

            CHECK_EXTRA_ARGS;

            return 0;
        }, 
        {   
            mem->curCmd += NUM_SZ;
            StackPush(mem->ret, mem->curCmd);
            mem->curCmd = *(int*)(mem->cmds + mem->curCmd - NUM_SZ);
        },
        {
            dprintf(fdOut, "%d", *(int*)(buf + cur));
            cur += NUM_SZ;
        })

// indicates end of the function declaration
CMD_DEF("ret",  'e', 
        {
            CHECK_EXTRA_ARGS;

            return 0;
        }, 
        {   
            if (mem->ret->cur == 0) {
                mem->err = E_RET_WITHOUT_CALL;
                return mem->err;
            }

            mem->curCmd = StackPeek(mem->ret);
            StackPop(mem->ret);
        },
        {
            //
        })

// decreases value, stored in register
CMD_DEF("dec",  'f', 
        {
            char* arg = FindWord(line, off, &off);
            
            CHECK_NO_ARGS;
            
            char type = GetArgType(arg, off - (arg - line->buf));
            
            if (type == 'n') {
                eBuf->err = E_INV_ARG_TYPE;
                
                return 1;
            } else if (type == 'r')
                if (EbufAddReg(eBuf, arg) == 1)
                    return 1;

            CHECK_EXTRA_ARGS;

            return 0;
        }, 
        {
            if (mem->cmds[mem->curCmd] != 'e') {
                double tmp = (double)(mem->regs[mem->cmds[mem->curCmd] - 97]);
                tmp = ((tmp / PRECISION) - 1) * PRECISION;
                mem->regs[mem->cmds[mem->curCmd] - 97] = (int)(tmp);
            }

            mem->curCmd += REG_SZ;
        },
        {
            dprintf(fdOut, "%cx", buf[cur]);
            cur += REG_SZ;
        })

// increases value, stored in register
CMD_DEF("inc",  'g', 
        {
            char* arg = FindWord(line, off, &off);
            
            CHECK_NO_ARGS;
            
            char type = GetArgType(arg, off - (arg - line->buf));
            
            if (type == 'n') {
                eBuf->err = E_INV_ARG_TYPE;
                
                return 1;
            } else
                if (EbufAddReg(eBuf, arg) == 1)
                    return 1;

            CHECK_EXTRA_ARGS;

            return 0;
        }, 
        {
            if (mem->cmds[mem->curCmd] != 'e') {
                double tmp = (double)(mem->regs[mem->cmds[mem->curCmd] - 97]);
                tmp = ((tmp / PRECISION) + 1) * PRECISION;
                mem->regs[mem->cmds[mem->curCmd] - 97] = (int)(tmp);
            }
            
            mem->curCmd += REG_SZ;
        },
        {
            dprintf(fdOut, "%cx", buf[cur]);
            cur += REG_SZ;
        })

// reads number from keyboard
CMD_DEF("in",   'h', 
        {
            CHECK_EXTRA_ARGS;

            return 0;
        }, 
        {
            double input = 0;

            if (scanf("%lf", &input) != 1) {
                mem->err = E_INV_INPUT;
                return mem->err;
            }

            StackPush(mem->stk, (int)(input * PRECISION));
        },
        {
            //
        })

// prints top element of stack
CMD_DEF("out",  'i', 
        {
            CHECK_EXTRA_ARGS;

            return 0;
        },
        {
            if (mem->stk->cur == 0)
                printf("stack is empty right now\n");
            else
                printf("%lf\n", (double)(StackPeek(mem->stk)) / PRECISION);
        },
        {
            //
        })

// mov dest src
CMD_DEF("mov", 'z',
        {
            char* arg = FindWord(line, off, &off);

            CHECK_NO_ARGS;

            char type = GetArgType(arg, off - (arg - line->buf));
            
            if (type == 'n') {
                eBuf->err = E_INV_ARG_TYPE;
                return 1;
            } else {
                if (EbufAddReg(eBuf, arg) == 1)
                    return 1;
            }

            arg = FindWord(line, off, &off);

            CHECK_NO_ARGS;

            type = GetArgType(arg, off - (arg - line->buf));
            
            if (type == 'n') {
                *((char*)(eBuf->cmds + eBuf->curCmd)) = 'n';
                eBuf->curCmd += ARGTYPE_SZ;
                *((int*)(eBuf->cmds + eBuf->curCmd)) = (int)(atof(arg) * PRECISION);
                eBuf->curCmd += NUM_SZ;
            } else {
                *((char*)(eBuf->cmds + eBuf->curCmd)) = 'r';
                eBuf->curCmd += ARGTYPE_SZ;

                if (EbufAddReg(eBuf, arg) == 1)
                    return 1;
            }

            CHECK_EXTRA_ARGS;

            return 0;
        },
        {   
            int reg = mem->cmds[mem->curCmd] - 97;

            mem->curCmd += REG_SZ;

            char type = mem->cmds[mem->curCmd];
            mem->curCmd += ARGTYPE_SZ;

            if (type == 'n' && reg != 4) {
                mem->regs[reg] = *(int*)(mem->cmds + mem->curCmd);
                mem->curCmd += NUM_SZ;
            } else if (type == 'n' && reg == 4) {
                mem->curCmd += NUM_SZ;
            } else if (type == 'r' && reg != 4) {
                mem->regs[reg] = mem->regs[mem->cmds[mem->curCmd] - 97];
                mem->curCmd += REG_SZ;
            } else if (type == 'r' && reg == 4) {
                mem->curCmd += REG_SZ;
            } else {
                mem->err = E_CORRUPTED_BIN;
                return mem->err;
            }
        },
        {
            dprintf(fdOut, "%cx", buf[cur]);
            cur += REG_SZ;

            if (buf[cur] == 'n') {
                cur += ARGTYPE_SZ;
                dprintf(fdOut, "%d", *(int*)(buf + cur));
                cur += NUM_SZ;
            } else if (buf[cur] == 'r') {
                cur += ARGTYPE_SZ;
                dprintf(fdOut, "%cx", buf[cur]);
                cur += REG_SZ;
            }  
        })

// jumps to given label
CMD_DEF("jmp", 'j', 
        {
            char* arg = FindWord(line, off, &off);

            CHECK_NO_ARGS;

            if (_FindLabel(eBuf, line, arg, off) != 0)
                return 1;

            CHECK_EXTRA_ARGS;

            return 0;
        }, 
        {
            mem->curCmd = *(int*)(mem->cmds + mem->curCmd);
        },
        {
            dprintf(fdOut, "%d", *(int*)(buf + cur));
            cur += NUM_SZ;
        })

// jumps to given label if last stack element is zero
CMD_DEF("jz",   'k', 
        {
            char* arg = FindWord(line, off, &off);
            
            CHECK_NO_ARGS;

            if (_FindLabel(eBuf, line, arg, off) != 0)
                return 1;

            CHECK_EXTRA_ARGS;

            return 0;
        }, 
        {   
            if (mem->stk->cur == 0) {
                mem->err = E_FEW_ARGS_IN_STACK;
                return mem->err;
            }

            if (StackPeek(mem->stk) == 0)
                mem->curCmd = *(int*)(mem->cmds + mem->curCmd);
            else 
                mem->curCmd += NUM_SZ;
        },
        {
            dprintf(fdOut, "%d", *(int*)(buf + cur));
            cur += NUM_SZ;
        })

// jumps to given label if last two elements of stack are equal
CMD_DEF("je",   'l', 
        {
            char* arg = FindWord(line, off, &off);
            
            CHECK_NO_ARGS;

            if (_FindLabel(eBuf, line, arg, off) != 0)
                return 1;
            
            CHECK_EXTRA_ARGS;

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
            else
                mem->curCmd += NUM_SZ;
            
            StackPush(mem->stk, tmp1);
        },
        {
            dprintf(fdOut, "%d", *(int*)(buf + cur));
            cur += NUM_SZ;
        })

// jumps to given label if last stack element is non-zero
CMD_DEF("jnz",  'm', 
        {
            char* arg = FindWord(line, off, &off);
            
            CHECK_NO_ARGS;

            if (_FindLabel(eBuf, line, arg, off) != 0)
                return 1;
            
            CHECK_EXTRA_ARGS;

            return 0;
        }, 
        {
            if (mem->stk->cur != 0) {
                mem->err = E_FEW_ARGS_IN_STACK;
                return mem->err;
            }

            if (StackPeek(mem->stk) == 0)
                mem->curCmd = *(int*)(mem->cmds + mem->curCmd);
            else
                mem->curCmd += NUM_SZ;
        },
        {
            dprintf(fdOut, "%d", *(int*)(buf + cur));
            cur += NUM_SZ;
        })

// jumps to given label if last two eleents of stack are not equal
CMD_DEF("jne",  'n', 
        {
            char* arg = FindWord(line, off, &off);
            
            CHECK_NO_ARGS;

            if (_FindLabel(eBuf, line, arg, off) != 0)
                return 1;
            
            CHECK_EXTRA_ARGS;

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
            else 
                mem->curCmd += NUM_SZ;
            
            StackPush(mem->stk, tmp1);
        },
        {
            dprintf(fdOut, "%d", *(int*)(buf + cur));
            cur += NUM_SZ;
        })

// jumps to given label if last element of stack is greater than prior
CMD_DEF("jg",   'o', 
        {
            char* arg = FindWord(line, off, &off);
            
            CHECK_NO_ARGS;

            if (_FindLabel(eBuf, line, arg, off) != 0)
                return 1;

            CHECK_EXTRA_ARGS;

            return 0;
        }, 
        {
            if (mem->stk->cur < 2) {
                mem->err = E_FEW_ARGS_IN_STACK;
                return mem->err;
            }
            
            int tmp1 = StackPeek(mem->stk);
            StackPop(mem->stk);

            //printf("if success, will jump to %d\n", *(int*)(mem->cmds + mem->curCmd));

            if (StackPeek(mem->stk) < tmp1)
                mem->curCmd = *(int*)(mem->cmds + mem->curCmd);
            else
                mem->curCmd += NUM_SZ;
            
            StackPush(mem->stk, tmp1);
        },
        {
            dprintf(fdOut, "%d", *(int*)(buf + cur));
            cur += NUM_SZ;
        })

// jumps to given label if last element of stack is lesser than prior
CMD_DEF("jl",   'p', 
        {
            char* arg = FindWord(line, off, &off);
            
            CHECK_NO_ARGS;

            if (_FindLabel(eBuf, line, arg, off) != 0)
                return 1;
            
            CHECK_EXTRA_ARGS;

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
            else 
                mem->curCmd += NUM_SZ;
            
            StackPush(mem->stk, tmp1);
        },
        {
            dprintf(fdOut, "%d", *(int*)(buf + cur));
            cur += NUM_SZ;
        })

// jumps to given label if last element of stack is qreater or equal than prior
CMD_DEF("jge",  'q', 
        {
            char* arg = FindWord(line, off, &off);
            
            CHECK_NO_ARGS;

            if (_FindLabel(eBuf, line, arg, off) != 0)
                return 1;
            
            CHECK_EXTRA_ARGS;

            return 0;
        }, 
        {
            if (mem->stk->cur < 2) {
                mem->err = E_FEW_ARGS_IN_STACK;
                return mem->err;
            }
            
            int tmp1 = StackPeek(mem->stk);
            StackPop(mem->stk);

            if (tmp1 >= StackPeek(mem->stk))
                mem->curCmd = *(int*)(mem->cmds + mem->curCmd);
            else 
                mem->curCmd += NUM_SZ;
            
            StackPush(mem->stk, tmp1);
        },
        {
            dprintf(fdOut, "%d", *(int*)(buf + cur));
            cur += NUM_SZ;
        })

// jumps to given label if last element of stack is lesser or equal than prior
CMD_DEF("jle",  'r', 
        {
            char* arg = FindWord(line, off, &off);
            
            CHECK_NO_ARGS;

            if (_FindLabel(eBuf, line, arg, off) != 0)
                return 1;
            
            CHECK_EXTRA_ARGS;

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
            else 
                mem->curCmd += NUM_SZ;
            
            StackPush(mem->stk, tmp1);
        },
        {
            dprintf(fdOut, "%d", *(int*)(buf + cur));
            cur += NUM_SZ;
        })

// adds last stack element to prior
CMD_DEF("add",  's', 
        {
            CHECK_EXTRA_ARGS;

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
        },
        {
            //
        })

// substracts last stack element from prior
CMD_DEF("sub",  't', 
        {
            CHECK_EXTRA_ARGS;

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
        },
        {
            //
        })

// multiplicates last stack element to prior
CMD_DEF("mul",  'u', 
        {
            CHECK_EXTRA_ARGS;

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
        },
        {
            //
        })

// divides last stack element by prior
CMD_DEF("div",  'v', 
        {
            CHECK_EXTRA_ARGS;

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
        },
        {
            //
        })

// gets square root from last stack element
CMD_DEF("sqrt",  'w', 
        {
            CHECK_EXTRA_ARGS;

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
        },
        {
            //
        })

// indicates end of the program
CMD_DEF("end",  'x', 
        {
            CHECK_EXTRA_ARGS;

            return 0;
        }, 
        {
            return 0;
        },
        {
            //
        })

CMD_DEF("alert", 'y',
        {
            char* arg = FindWord(line, off, &off);

            CHECK_NO_ARGS;

            int msgLen = line->len - (arg - line->buf);        
            if (msgLen >= DELTA_CMDS - CMD_SZ - NUM_SZ) {
                eBuf->err = E_LONG_ALERT_MSG;
                return 1;
            }

            *(int*)(eBuf->cmds + eBuf->curCmd) = msgLen;
            eBuf->curCmd += NUM_SZ;

            memcpy(eBuf->cmds + eBuf->curCmd, arg, msgLen);
            eBuf->curCmd += msgLen;

            return 0;            
        },
        {   
            int len = *(int*)(mem->cmds + mem->curCmd);
            mem->curCmd += NUM_SZ;

            fwrite(mem->cmds + mem->curCmd, sizeof(char), len, stdout);
            printf("\n");
            mem->curCmd += len;
        },
        {
            int len = *(int*)(buf + cur);
            cur += NUM_SZ;

            write(fdOut, buf + cur, len);
            cur += len;
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

