#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "compiler.h"

#define MAX_CODE_LENGTH 200
#define MAX_SYMBOL_COUNT 50
#define MAX_REG_COUNT 10
#define ERROR_CODE -100

// generated code
instruction *code;
int codeIdx; // (codeIdx)

// symbol table
symbol *table;
int tIndex;
lexeme *list;
int listIndex;
int level;
int err;
int registercounter;


void emit(int opname, int reg, int level, int mvalue);
void addToSymbolTable(int k, char n[], int s, int l, int a, int m);
void mark();
int multipledeclarationcheck(char name[]);
int findsymbol(char name[], int kind);
int block();
int varDec();
int procDec();
int statement();
int expression();
int condition();
int term();
int factor();
void printparseerror(int err_code);
void printsymboltable();
void printassemblycode();

instruction *parse(lexeme *list, int printTable, int printCode)
{

	// set up program variables
	code = malloc(sizeof(instruction) * MAX_CODE_LENGTH);
	codeIdx = 0;
	table = malloc(sizeof(symbol) * MAX_SYMBOL_COUNT);
	tIndex = 0;

	// DO STUFF HERE (PROGRAM)
	int isERROR;
	registercounter = -1;
	emit(7, 0, 0, 0); // JMP
	addToSymbolTable(3, "main", 0, 0, 0, 0);
	level = -1;
	isERROR = block();
	if (isERROR == ERROR_CODE)
	{
		printparseerror(err);
		return NULL;
	}
	// check for error1
	if (list[listIndex].type != periodsym)
	{
		printparseerror(1);
		return NULL;
	}
	emit(11, 0, 0, 0); // HLT
	code[0].m = table[0].addr;
	for (int i = 0; i < codeIdx; i++)
		if (code[i].opcode == 5)
			code[i].m = table[code[i].m].addr;

	// print off table and code
	if (printTable)
		printsymboltable();
	if (printCode)
		printassemblycode();

	// mark the end of the code
	code[codeIdx].opcode = -1;

	return code;
}

int block()
{
	int isERROR;
	level++;
	int prodecureindex = tIndex - 1;

	int x = varDec();
	if (x == ERROR_CODE)
		return ERROR_CODE;

	isERROR = procDec();
	if (isERROR == ERROR_CODE)
		return ERROR_CODE;

	table[prodecureindex].addr = codeIdx;
	emit(6, 0, 0, x); // INC
	isERROR = statement();
	if (isERROR == ERROR_CODE)
		return ERROR_CODE;

	mark();
	level--;
	return 0;
}

int varDec()
{
    int memSize = 3;
	char symbolName[12];
	int arraySize = 0;

	if (list[listIndex].type == varsym)
    {
        do
        {
            listIndex++;
            if (list[listIndex].type != identsym)
            {
                err = 2;
                return ERROR_CODE;
            }
            if (multipledeclarationcheck(list[listIndex].name) != -1)
            {
                err = 3;
                return ERROR_CODE;
            }

            strcpy(symbolName, list[listIndex].name);
            listIndex++;
            if (list[listIndex].type == lbracketsym)
            {
                listIndex++;
                if (list[listIndex].type != numbersym || list[listIndex].value == 0)
                {
                    err = 4;
                    return ERROR_CODE;
                }

                arraySize = list[listIndex].value;
                listIndex++;
                token_type tempy = list[listIndex].type;
                if (tempy == multsym || tempy == divsym || tempy == modsym || tempy == addsym || tempy == subsym)
                {
                    err = 4;
                    return ERROR_CODE;
                }
                else if (tempy != rbracketsym)
                {
                    err = 5;
                    return ERROR_CODE;
                }

                listIndex++;
                addToSymbolTable(2, symbolName, arraySize, level, memSize, 0);
                memSize += arraySize;
            }

            else
            {
                addToSymbolTable(1, symbolName, 0, level, memSize, 0);
                memSize++;
            }
        }
        while (list[listIndex].type == commasym);

        if (list[listIndex].type == identsym)
        {
            err = 6;
            return ERROR_CODE;
        }
        else if (list[listIndex].type != semicolonsym)
        {
            err = 7;
            return ERROR_CODE;
        }
        listIndex++;
        return memSize;
    }

    else
        return memSize;
}

int procDec()
{
	int isERROR;
	char symbolname[12];
	while (list[listIndex].type == procsym)
	{
		listIndex++;
		if (list[listIndex].type != identsym)
		{
			err = 2;
			return ERROR_CODE;
		}
		else if (multipledeclarationcheck(list[listIndex].name) != -1)
		{
			err = 3;
			return ERROR_CODE;
		}
		strcpy(symbolname, list[listIndex].name);
		listIndex++;
		if (list[listIndex].type != semicolonsym)
		{
			err = 8;
			return ERROR_CODE;
		}
		listIndex++;
		addToSymbolTable(3, symbolname, 0, level, 0, 0);
		isERROR = block();
		if (isERROR == ERROR_CODE)
			return ERROR_CODE;

		if (list[listIndex].type != semicolonsym)
		{
			err = 7;
			return ERROR_CODE;
		}
		listIndex++;
		emit(2, 0, 0, 0);
	}
	return 0;
}

int statement()
{
    char symbolname[12];
	int symidx;
	int arrayidxreg;
	int varlocreg;
 	int jpcidx;
	int jmpidx;
	int loopidx;
	int isERROR;

	// assignment =================================================
	if (list[listIndex].type == identsym)
	{
		strcpy(symbolname, list[listIndex].name);
		listIndex++;
		if (list[listIndex].type == lbracketsym)
		{
			listIndex++;
			symidx = findsymbol(symbolname, 2);
			if (symidx == -1)
			{
				if (findsymbol(symbolname, 1) != -1)
				{
					err = 11;
					return ERROR_CODE;
				}
				else if (findsymbol(symbolname, 3) != -1)
				{
					err = 9;
					return ERROR_CODE;
				}
				else
				{
					err = 10;
					return ERROR_CODE;
				}
			}
			isERROR = expression();
			if (isERROR == ERROR_CODE)
				return ERROR_CODE;

			arrayidxreg = registercounter;
			if (list[listIndex].type != rbracketsym)
			{
				err = 5;
				return ERROR_CODE;
			}
			listIndex++;
			if (list[listIndex].type != assignsym)
			{
				err = 13;
				return ERROR_CODE;
			}
			listIndex++;
			emit(1, registercounter+1, 0, table[symidx].addr); // LIT
			emit(13, arrayidxreg, arrayidxreg, registercounter+1); // ADD
			isERROR = expression();
			if (isERROR == ERROR_CODE)
				return ERROR_CODE;
			registercounter++;
			if (registercounter >= 10)
			{
				err = 14;
				return ERROR_CODE;
			}
			//emit(1, registercounter, 0, table[symidx].addr); // LIT [MOVED TO BEFORE EXPRESSION CALL]
			//emit(13, arrayidxreg, arrayidxreg, registercounter); // ADD [MOVED TO BEFORE EXPRESSION CALL]
			registercounter--;
			emit(4, registercounter, level - table[symidx].level, arrayidxreg); // STO
			registercounter -= 2;
		}
		else
		{
			symidx = findsymbol(symbolname, 1);
			if (symidx == -1)
			{
				if (findsymbol(symbolname, 2) != -1)
				{
					err = 12;
					return ERROR_CODE;
				}
				else if (findsymbol(symbolname, 3) != -1)
				{
					err = 9;
					return ERROR_CODE;
				}
				else
				{
					err = 10;
					return ERROR_CODE;
				}
			}
			registercounter++;
			if (registercounter >= 10)
			{
				err = 14;
				return ERROR_CODE;
			}
			emit(1, registercounter, 0, table[symidx].addr); // LIT
			varlocreg = registercounter;
			if (list[listIndex].type != assignsym)
			{
				err = 13;
				return ERROR_CODE;
			}
			listIndex++;
			isERROR = expression();
			if (isERROR == ERROR_CODE)
				return ERROR_CODE;
			emit(4, registercounter, level - table[symidx].level, varlocreg); // STO
			registercounter -= 2;
		}
	}

	// call ===========================================================
  if(list[listIndex].type == callsym)
  {
    listIndex++;
    if (list[listIndex].type != identsym)
    {
      err = 15;
      return ERROR_CODE;
    }
    symidx = findsymbol(list[listIndex].name, 3);
    if (symidx == -1)
    {
      if (findsymbol(list[listIndex].name, 1) != -1 || findsymbol(list[listIndex].name, 2) != -1)
      {
        err = 15;
        return ERROR_CODE;
      }
      else
      {
        err = 10;
        return ERROR_CODE;
      }
    }
    emit(5, 0, level - table[symidx].level, symidx); // CAL
    listIndex++;
   }

	// begin-end ======================================================
	if (list[listIndex].type == beginsym)
	{
		do
		{
			listIndex++;
			isERROR = statement();
			if (isERROR == ERROR_CODE)
				return ERROR_CODE;
		}
		while (list[listIndex].type == semicolonsym);
		if (list[listIndex].type != endsym)
		{
			token_type tempy = list[listIndex].type;

			if (tempy == identsym || tempy == callsym || tempy == beginsym || tempy == ifsym ||
				tempy == dosym || tempy == readsym || tempy == writesym)
			{
		  	err = 16;
			  return ERROR_CODE;
			}
			else
			{
				err = 17;
				return ERROR_CODE;
			}
		}
		listIndex++;
	}

	//
	// if =============================================================
	if(list[listIndex].type == ifsym)
	{
		listIndex++;
		isERROR = condition();
		if (isERROR == ERROR_CODE)
			return ERROR_CODE;
		jpcidx = codeIdx;
		emit(8, registercounter, 0, 0); // JPC
		registercounter--;
		if (list[listIndex].type != questionsym)
		{
			err = 18;
			return ERROR_CODE;
		}
		listIndex++;
		isERROR = statement();
		if (isERROR == ERROR_CODE)
			return ERROR_CODE;
		if (list[listIndex].type == colonsym)
		{
			listIndex++;
			jmpidx = codeIdx;
			emit(7, 0, 0, 0); // JMP
			code[jpcidx].m = codeIdx;
			isERROR = statement();
			if (isERROR == ERROR_CODE)
				return ERROR_CODE;
			code[jmpidx].m = codeIdx;
		}
		else
		{
			code[jpcidx].m = codeIdx;
		}
	}

	// do-while =======================================================
	if(list[listIndex].type == dosym)
	{
		listIndex++;
		loopidx = codeIdx;
		isERROR = statement();
		if (isERROR == ERROR_CODE)
			return ERROR_CODE;
		if (list[listIndex].type != whilesym)
		{
			err = 19;
			return ERROR_CODE;
		}
		listIndex++;
		isERROR = condition();
		if (isERROR == ERROR_CODE)
			return ERROR_CODE;
		registercounter++;

		if (registercounter >= 10)
		{
		 err = 14;
		 return ERROR_CODE;
		}
		emit(1, registercounter, 0, 0); // LIT
		emit(18, registercounter - 1, registercounter - 1, registercounter); // EQL
		registercounter--;
		emit(8, registercounter, 0, loopidx); // JPC
		registercounter--;
	}

	// read ===========================================================
	if (list[listIndex].type == readsym)
	{
		listIndex++;
		if (list[listIndex].type != identsym)
		{
			err = 20;
			return ERROR_CODE;
		}
		strcpy(symbolname, list[listIndex].name);
		listIndex++;
		if (list[listIndex].type == lbracketsym)
		{
			listIndex++;
			symidx = findsymbol(symbolname, 2);
			if (symidx == -1)
			{
				if (findsymbol(symbolname, 1) != -1)
				{
					err = 11;
					return ERROR_CODE;
				}
				else if (findsymbol(symbolname, 3) != -1)
				{
					err = 9;
					return ERROR_CODE;
				}
				else
				{
					err = 10;
					return ERROR_CODE;
				}
			}
			isERROR = expression();
			if (isERROR == ERROR_CODE)
				return ERROR_CODE;
			arrayidxreg = registercounter;
			if (list[listIndex].type != rbracketsym)
			{
				err = 5;
				return ERROR_CODE;
			}
			listIndex++;
			registercounter++;
			if (registercounter >= 10)
			{
				err = 14;
				return ERROR_CODE;
			}
			//emit(10, registercounter, 0, 0); // RED [MOVED TO AFTER LIT & ADD]
			//registercounter++; [REMOVED]
			if (registercounter >= 10)
			{
				err = 14;
				return ERROR_CODE;
			}
			emit(1, registercounter, 0, table[symidx].addr); // LIT
			emit(13, arrayidxreg, arrayidxreg, registercounter); // ADD
			//registercounter--; [REMOVED]
            emit(10, registercounter, 0, 0); // RED
			emit(4, registercounter, level - table[symidx].level, arrayidxreg); // STO
			registercounter -= 2;
		}
		else
		{
			//listIndex++;
			symidx = findsymbol(symbolname, 1);
			if (symidx == -1)
			{
				if (findsymbol(symbolname, 2) != -1)
				{
					err = 12;
					return ERROR_CODE;
				}
				else if (findsymbol(symbolname, 3) != -1)
				{
					err = 9;
					return ERROR_CODE;
				}
				else
				{
					err = 10;
					return ERROR_CODE;
				}
			}
			registercounter++;
			if (registercounter >= 10)
			{
				err = 14;
				return ERROR_CODE;
			}
			emit(1, registercounter, 0, table[symidx].addr); // LIT
			varlocreg = registercounter;
			registercounter++;
			if (registercounter >= 10)
			{
				err = 14;
				return ERROR_CODE;
			}
			emit(10, registercounter, 0, 0); // RED
			emit(4, registercounter, level - table[symidx].level, varlocreg); // STO
			//registercounter--;
      registercounter -= 2;
		}
	}

	// write ==========================================================
	if(list[listIndex].type == writesym)
	{
		listIndex++;
		isERROR = expression();
		if (isERROR == ERROR_CODE)
			return ERROR_CODE;
		emit(9, registercounter, 0, 0); // WRT
		registercounter--;
	}
    return 0;
}

int condition()
{
	int isERROR;
	isERROR = expression();
	if (isERROR == ERROR_CODE)
		return ERROR_CODE;
	if (list[listIndex].type == eqlsym)
	{
		listIndex++;
		isERROR = expression();
		if (isERROR == ERROR_CODE)
			return ERROR_CODE;
		emit(18, registercounter - 1, registercounter - 1, registercounter); // EQL
		registercounter--;
	}
	else if (list[listIndex].type == neqsym)
	{
		listIndex++;
		isERROR = expression();
		if (isERROR == ERROR_CODE)
			return ERROR_CODE;
		emit(19, registercounter - 1, registercounter - 1, registercounter); // NEQ
		registercounter--;
	}
	else if (list[listIndex].type == lsssym)
	{
		listIndex++;
		isERROR = expression();
		if (isERROR == ERROR_CODE)
			return ERROR_CODE;
		emit(20, registercounter - 1, registercounter - 1, registercounter); // LSS
		registercounter--;
	}
	else if (list[listIndex].type == leqsym)
	{
		listIndex++;
		isERROR = expression();
		if (isERROR == ERROR_CODE)
			return ERROR_CODE;
		emit(21, registercounter - 1, registercounter - 1, registercounter); // LEQ
		registercounter--;
	}
	else if (list[listIndex].type == gtrsym)
	{
		listIndex++;
		isERROR = expression();
		if (isERROR == ERROR_CODE)
			return ERROR_CODE;
		emit(22, registercounter - 1, registercounter - 1, registercounter); // GTR
		registercounter--;
	}
	else if (list[listIndex].type == geqsym)
	{
		listIndex++;
		isERROR = expression();
		if (isERROR == ERROR_CODE)
			return ERROR_CODE;
		emit(23, registercounter - 1, registercounter - 1, registercounter); // GEQ
		registercounter--;
	}
	else
	{
		err = 21;
		return ERROR_CODE;
	}
	return 0;
}

int expression()
{
	int isERROR;
	if (list[listIndex].type == subsym)
	{
		listIndex++;
		isERROR = term();
		if (isERROR == ERROR_CODE)
			return ERROR_CODE;
		emit(12, registercounter, 0, registercounter); // NEG
		while (list[listIndex].type == addsym || list[listIndex].type == subsym)
		{
			if (list[listIndex].type == addsym)
			{
				listIndex++;
				isERROR = term();
				if (isERROR == ERROR_CODE)
					return ERROR_CODE;
				emit(13, registercounter - 1, registercounter - 1, registercounter); // ADD
				registercounter--;
			}
			else
			{
				listIndex++;
				isERROR = term();
				if (isERROR == ERROR_CODE)
					return ERROR_CODE;
				emit(14, registercounter - 1, registercounter - 1, registercounter); // SUB
				registercounter--;
			}
		}
	}
	else
	{
		isERROR = term();
		if (isERROR == ERROR_CODE)
			return ERROR_CODE;
	 	while (list[listIndex].type == addsym || list[listIndex].type == subsym)
		{
			if (list[listIndex].type == addsym)
			{
				listIndex++;
				isERROR = term();
				if (isERROR == ERROR_CODE)
					return ERROR_CODE;
				emit(13, registercounter - 1, registercounter - 1, registercounter); // ADD
				registercounter--;
			}
			else
			{
				listIndex++;
				isERROR = term();
				if (isERROR == ERROR_CODE)
					return ERROR_CODE;
				emit(14, registercounter - 1, registercounter - 1, registercounter); // SUB
				registercounter--;
			}
		}
    }
    if (list[listIndex].type == lparenthesissym || list[listIndex].type == identsym || list[listIndex].type == numbersym)
    {
        err = 22;
		return ERROR_CODE;
    }
	return 0;
}

int term()
{
    int isERROR;
	isERROR = factor();
	if (isERROR == ERROR_CODE)
		return ERROR_CODE;
	while (list[listIndex].type == multsym || list[listIndex].type == divsym || list[listIndex].type == modsym)
	{
		if (list[listIndex].type == multsym)
		{
			listIndex++;
			isERROR = factor();
			if (isERROR == ERROR_CODE)
				return ERROR_CODE;
			emit(15, registercounter - 1,  registercounter - 1, registercounter); // MUL
			registercounter--;
		}
		else if (list[listIndex].type == divsym)
		{
			listIndex++;
			isERROR = factor();
			if (isERROR == ERROR_CODE)
				return ERROR_CODE;
			emit(16, registercounter - 1,  registercounter - 1, registercounter); // DIV
			registercounter--;
		}
		else
		{
			listIndex++;
			isERROR = factor();
			if (isERROR == ERROR_CODE)
				return ERROR_CODE;
			emit(17, registercounter - 1,  registercounter - 1, registercounter); // MOD
			registercounter--;
		}
	}
    return 0;
}

int factor()
{
    int isERROR;
    int symidx;
    char symbolname[12];
	int arrayidxreg;
    int varlocreg;
	if (list[listIndex].type == identsym)
	{
		strcpy(symbolname, list[listIndex].name);
		listIndex++;
		if (list[listIndex].type == lbracketsym)
		{
			listIndex++;
			symidx = findsymbol(symbolname, 2);
			if (symidx == -1)
			{
				if (findsymbol(symbolname, 1) != -1)
				{
					err = 11;
					return ERROR_CODE;
				}
				else if (findsymbol(symbolname, 3) != -1)
				{
					err = 9;
					return ERROR_CODE;
				}
				else
				{
					err = 10;
					return ERROR_CODE;
				}
			}
			isERROR = expression();
			if (isERROR == ERROR_CODE)
				return ERROR_CODE;
			arrayidxreg = registercounter;
			if (list[listIndex].type != rbracketsym)
			{
				err = 5;
				return ERROR_CODE;
			}
			listIndex++;
			registercounter++;
			if (registercounter >= 10)
			{
				err = 14;
				return ERROR_CODE;
			}
			emit(1, registercounter, 0, table[symidx].addr); // LIT
			emit(13, arrayidxreg, arrayidxreg, registercounter); // ADD
			registercounter--;
			emit(3, registercounter, level - table[symidx].level, arrayidxreg); // LOD
		}
		else
		{
			symidx = findsymbol(symbolname, 1);
			if (symidx == -1)
			{
				if (findsymbol(symbolname, 2) != -1)
				{
					err = 12;
					return ERROR_CODE;
				}
				else if (findsymbol(symbolname, 3) != -1)
				{
					err = 9;
					return ERROR_CODE;
				}
				else
				{
					err = 10;
					return ERROR_CODE;
				}
			}
			registercounter++;
			if (registercounter >= 10)
			{
				err = 14;
				return ERROR_CODE;
			}
			emit(1, registercounter, 0, table[symidx].addr); // LIT
			varlocreg = registercounter;
			emit(3, registercounter, level - table[symidx].level, varlocreg); // LOD
		}
        return 0;
	}
	else if (list[listIndex].type == numbersym)
	{
		registercounter++;
		if (registercounter >= 10)
		{
			err = 14;
			return ERROR_CODE;
		}
		emit(1, registercounter, 0, list[listIndex].value); // LIT
		listIndex++;
        return 0;
	}
	else if (list[listIndex].type == lparenthesissym)
	{
		listIndex++;
		isERROR = expression();
		if (isERROR == ERROR_CODE)
			return ERROR_CODE;
		if (list[listIndex].type != rparenthesissym)
		{
			err = 23;
			return ERROR_CODE;
		}
		listIndex++;
        return 0;
	}
	else
	{
		err = 24;
		return ERROR_CODE;
	}
}

void emit(int opname, int reg, int level, int mvalue)
{
	code[codeIdx].opcode = opname;
	code[codeIdx].r = reg;
	code[codeIdx].l = level;
	code[codeIdx].m = mvalue;
	codeIdx++;
}

void addToSymbolTable(int k, char n[], int s, int l, int a, int m)
{
	table[tIndex].kind = k;
	strcpy(table[tIndex].name, n);
	table[tIndex].size = s;
	table[tIndex].level = l;
	table[tIndex].addr = a;
	table[tIndex].mark = m;
	tIndex++;
}

void mark()
{
	int i;
	for (i = tIndex - 1; i >= 0; i--)
	{
		if (table[i].mark == 1)
			continue;
		if (table[i].level < level)
			return;
		table[i].mark = 1;
	}
}

int multipledeclarationcheck(char name[])
{
	int i;
	for (i = 0; i < tIndex; i++)
		if (table[i].mark == 0 && table[i].level == level && strcmp(name, table[i].name) == 0)
			return i;
	return -1;
}

int findsymbol(char name[], int kind)
{
	int i;
	int max_idx = -1;
	int max_lvl = -1;
	for (i = 0; i < tIndex; i++)
	{
		if (table[i].mark == 0 && table[i].kind == kind && strcmp(name, table[i].name) == 0)
		{
			if (max_idx == -1 || table[i].level > max_lvl)
			{
				max_idx = i;
				max_lvl = table[i].level;
			}
		}
	}
	return max_idx;
}

void printparseerror(int err_code)
{
	switch (err_code)
	{
		case 1:
			printf("Parser Error: Program must be closed by a period\n");
			break;
		case 2:
			printf("Parser Error: Symbol names must be identifiers\n");
			break;
		case 3:
			printf("Parser Error: Confliciting symbol declarations\n");
			break;
		case 4:
			printf("Parser Error: Array sizes must be given as a single, nonzero number\n");
			break;
		case 5:
			printf("Parser Error: [ must be followed by ]\n");
			break;
		case 6:
			printf("Parser Error: Multiple symbols in var declaration must be separated by commas\n");
			break;
		case 7:
			printf("Parser Error: Symbol declarations should close with a semicolon\n");
			break;
		case 8:
			printf("Parser Error: Procedure declarations should contain a semicolon before the body of the procedure begins\n");
			break;
		case 9:
			printf("Parser Error: Procedures may not be assigned to, read, or used in arithmetic\n");
			break;
		case 10:
			printf("Parser Error: Undeclared identifier\n");
			break;
		case 11:
			printf("Parser Error: Variables cannot be indexed\n");
			break;
		case 12:
			printf("Parserr Error: Arrays must be indexed\n");
			break;
		case 13:
			printf("Parser Error: Assignment operator missing\n");
			break;
		case 14:
			printf("Parser Error: Register Overflow Error\n");
			break;
		case 15:
			printf("Parser Error: call must be followed by a procedure identifier\n");
			break;
		case 16:
			printf("Parser Error: Statements within begin-end must be separated by a semicolon\n");
			break;
		case 17:
			printf("Parser Error: begin must be followed by end\n");
			break;
		case 18:
			printf("Parser Error: if must be followed by ?\n");
			break;
		case 19:
			printf("Parser Error: do must be followed by while\n");
			break;
		case 20:
			printf("Parser Error: read must be followed by a var or array identifier\n");
			break;
		case 21:
			printf("Parser Error: Relational operator missing from condition\n");
			break;
		case 22:
			printf("Parser Error: Bad arithmetic\n");
			break;
		case 23:
			printf("Parser Error: ( must be followed by )\n");
			break;
		case 24:
			printf("Parser Error: Arithmetic expressions may only contain arithmetic operators, numbers, parentheses, and variables\n");
			break;
		default:
			printf("Implementation Error: unrecognized error code\n");
			break;
	}

	free(code);
	free(table);
}

void printsymboltable()
{
	int i;
	printf("Symbol Table:\n");
	printf("Kind | Name        | Size | Level | Address | Mark\n");
	printf("---------------------------------------------------\n");
	for (i = 0; i < tIndex; i++)
		printf("%4d | %11s | %5d | %4d | %5d | %5d\n", table[i].kind, table[i].name, table[i].size, table[i].level, table[i].addr, table[i].mark);

	free(table);
	table = NULL;
}

void printassemblycode()
{
	int i;
	printf("Line\tOP Code\tOP Name\tR\tL\tM\n");
	for (i = 0; i < codeIdx; i++)
	{
		printf("%d\t", i);
		printf("%d\t", code[i].opcode);
		switch (code[i].opcode)
		{
			case 1:
				printf("LIT\t");
				break;
			case 2:
				printf("RET\t");
				break;
			case 3:
				printf("LOD\t");
				break;
			case 4:
				printf("STO\t");
				break;
			case 5:
				printf("CAL\t");
				break;
			case 6:
				printf("INC\t");
				break;
			case 7:
				printf("JMP\t");
				break;
			case 8:
				printf("JPC\t");
				break;
			case 9:
				printf("WRT\t");
				break;
			case 10:
				printf("RED\t");
				break;
			case 11:
				printf("HLT\t");
				break;
			case 12:
				printf("NEG\t");
				break;
			case 13:
				printf("ADD\t");
				break;
			case 14:
				printf("SUB\t");
				break;
			case 15:
				printf("MUL\t");
				break;
			case 16:
				printf("DIV\t");
				break;
			case 17:
				printf("MOD\t");
				break;
			case 18:
				printf("EQL\t");
				break;
			case 19:
				printf("NEQ\t");
				break;
			case 20:
				printf("LSS\t");
				break;
			case 21:
				printf("LEQ\t");
				break;
			case 22:
				printf("GTR\t");
				break;
			case 23:
				printf("GEQ\t");
				break;
			default:
				printf("err\t");
				break;
		}
		printf("%d\t%d\t%d\n", code[i].r, code[i].l, code[i].m);
	}

	if (table != NULL)
		free(table);
}
