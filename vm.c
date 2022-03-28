/*
KYLE MAUTER
DAVID QUALLS
JENNA BUSCH
*/

/*
	You can use these two print statements for the errors:
		printf("Virtual Machine Error: Stack Overflow Error\n");
		printf("Virtual Machine Error: Out of Bounds Access Error\n");
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#define REG_FILE_SIZE 10
#define MAX_STACK_LENGTH 100

/* From the header file

	typedef struct instruction {
		int opcode;
		int r;
		int l;
		int m;
	} instruction;

*/

void print_execution(int line, char *opname, instruction IR, int PC, int BP, int SP, int *stack, int *RF)
{
	int i;
	// print out instruction and registers
	printf("%2d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t\t", line, opname, IR.r, IR.l, IR.m, PC, SP, BP);

	// print register file
	for (i = 0; i < REG_FILE_SIZE; i++)
		printf("%d ", RF[i]);
	printf("\n");

	// print stack
	printf("stack:\t");
	for (i = MAX_STACK_LENGTH - 1; i >= SP; i--)
		printf("%d ", stack[i]);
	printf("\n");
}

int base(int L, int BP, int *stack)
{
	int ctr = L;
	int rtn = BP;
	while (ctr > 0)
	{
		rtn = stack[rtn];
		ctr--;
	}
	return rtn;
}

void execute_program(instruction *code, int printFlag)
{
	/// Iniitial values
	int BP = MAX_STACK_LENGTH - 1;
	int SP = BP + 1;
	int PC = 0;
	instruction IR;
	int * stack = calloc(MAX_STACK_LENGTH, sizeof(int));
	int * RF = calloc(REG_FILE_SIZE, sizeof(int));
	int halt = 0;

	/// Useful info
	char opnames[23][4] = {"LIT", "RET", "LOD", "STO", "CAL", "INC", "JMP", "JPC", "WRT", "RED", "HLT", "NEG", "ADD", "SUB", "MUL", "DIV", "MOD", "EQL", "NEQ", "LSS", "LEQ", "GTR", "GEQ"};

	// keep this
	if (printFlag)
	{
		printf("\t\t\t\t\tPC\tSP\tBP\n");
		printf("Initial values:\t\t\t\t%d\t%d\t%d\n", PC, SP, BP);
    		fflush(stdout);
	}

	/// P-machine instruction cycle
	while (!halt)
	{
		/// Fetch Cycle (gets the next instruction)
		IR = code[PC++];

		/// Execute Cycle
		/// Appendix A of hw instructions indicates 23 different operations

		/// There's some stuff going on I don't understand, am gonna stop for now till we all can work on it together
		switch (IR.opcode)
		{
			/// LIT
			case 1:
				RF[IR.r] = IR.m;
				if (printFlag)
					print_execution(PC - 1, opnames[1-1], IR, PC, BP, SP, stack, RF);
        			break;
        
			/// RET
			case 2:
		      	{
				int prevPC = PC;
				SP = BP + 1;
				PC = stack[SP - 3];
				BP = stack[SP - 2];
				if (printFlag)
					print_execution(prevPC - 1, opnames[2-1], IR, PC, BP, SP, stack, RF);
				break;
		      	}

			/// LOD
			case 3:
		        {
				/// Out of bounds check
				int base_L = base(IR.l, BP, stack);
				if (base_L - IR.m < 0 || base_L >= MAX_STACK_LENGTH)
				{
					printf("Virtual Machine Error: Out of Bounds Access Error\n");
				  	halt = 1;
				  	break;
				}

				RF[IR.r] = stack[base_L - RF[IR.m]];

				if (printFlag)
					print_execution(PC - 1, opnames[3 - 1], IR, PC, BP, SP, stack, RF);        

				break;
		        }
        
			/// STO
			case 4:
		        {
				/// Out of bounds check
				int base_L = base(IR.l, BP, stack);
				if (base_L - IR.m < 0 || base_L >= MAX_STACK_LENGTH)
				{
					printf("Virtual Machine Error: Out of Bounds Access Error\n");
				  	halt = 1;
				  	break;
				}

				stack[base_L - RF[IR.m]] = RF[IR.r];

				if (printFlag)
					print_execution(PC - 1, opnames[4 - 1], IR, PC, BP, SP, stack, RF);        

				break;
		        }
        
			/// CAL
			case 5:
		      	{
				int prevPC = PC;
				stack[SP-1] = base(IR.l, BP, stack);
				stack[SP-2] = BP;
				stack[SP-3] = PC;

				BP = SP-1;
				PC = IR.m;

				if (printFlag)
					print_execution(prevPC - 1, opnames[5 - 1], IR, PC, BP, SP, stack, RF);
				break;
		      	}
        
			/// INC
			case 6:
				SP -= IR.m;
				if (SP < 0)
				{
					printf("Virtual Machine Error: Stack Overflow Error\n");
					halt = 1;
				}

				if (printFlag && (!halt || IR.opcode == 11))
					print_execution(PC - 1, opnames[6 - 1], IR, PC, BP, SP, stack, RF);
        			break;
        
			/// JMP
			case 7:
      			{
				int prevPC = PC;
				PC = IR.m;
				if (printFlag)
					print_execution(prevPC-1, opnames[7 - 1], IR, PC, BP, SP, stack, RF);
        			break;
	      		}
				
			/// JPC
			case 8:
				if (RF[IR.r] == 0)
				{
				  	int prevPC = PC;
				  	PC = IR.m;
					if (printFlag)
						print_execution(prevPC-1, opnames[8 - 1], IR, PC, BP, SP, stack, RF);
				}
				else
				{
					if (printFlag)
						print_execution(PC - 1, opnames[8 - 1], IR, PC, BP, SP, stack, RF);           
				}

				break;
        
			/// WRT
			case 9:
        			printf("Write Value: %d\n", RF[IR.r]);
				if (printFlag)
					print_execution(PC - 1, opnames[9 - 1], IR, PC, BP, SP, stack, RF);
        			break;
        
			/// RED
			case 10:
        			printf("Please Enter a Value: \n");
        			fscanf(stdin, "%d", &RF[IR.r]);
				if (printFlag)
					print_execution(PC - 1, opnames[10 - 1], IR, PC, BP, SP, stack, RF);
        			break;
        
			/// HLT
			case 11:
				halt = 1;
				if (printFlag)
					print_execution(PC - 1, opnames[11 - 1], IR, PC, BP, SP, stack, RF);
        			break;
        
			/// NEG
			case 12:
        			RF[IR.r] = -RF[IR.r];
        			if (printFlag)
          				print_execution(PC - 1, opnames[12 - 1], IR, PC, BP, SP, stack, RF);        
        			break;
        
			/// ADD
			case 13:
        			RF[IR.r] = RF[IR.l] + RF[IR.m];
        			if (printFlag)
          				print_execution(PC - 1, opnames[13 - 1], IR, PC, BP, SP, stack, RF);
        			break;
        
			/// SUB
			case 14:
        			RF[IR.r] = RF[IR.l] - RF[IR.m];
        			if (printFlag)
          				print_execution(PC - 1, opnames[14 - 1], IR, PC, BP, SP, stack, RF);
        			break;
        
			/// MUL
			case 15:
				RF[IR.r] = RF[IR.l] * RF[IR.m];
				if (printFlag)
				  print_execution(PC - 1, opnames[15 - 1], IR, PC, BP, SP, stack, RF);
				break;
        
			/// DIV
			case 16:
				RF[IR.r] = RF[IR.l] / RF[IR.m];
				if (printFlag)
				  print_execution(PC - 1, opnames[16 - 1], IR, PC, BP, SP, stack, RF);
				break;
        
			/// MOD
			case 17:
				RF[IR.r] = RF[IR.l] % RF[IR.m];
				if (printFlag)
				  print_execution(PC - 1, opnames[17 - 1], IR, PC, BP, SP, stack, RF);
				break;
        
			/// EQL
			case 18:
				if (RF[IR.l] != RF[IR.m])
				  RF[IR.r] = 0;
				else
				  RF[IR.r] = 1;
          
				if (printFlag)
					print_execution(PC - 1, opnames[18 - 1], IR, PC, BP, SP, stack, RF);  
        			break;
        
			/// NEQ
			case 19:
				if (RF[IR.l] == RF[IR.m])
				  RF[IR.r] = 0;
				else
				  RF[IR.r] = 1;
          
				if (printFlag)
					print_execution(PC - 1, opnames[19 - 1], IR, PC, BP, SP, stack, RF);   
				break;
        
			/// LSS
			case 20:
				if (RF[IR.l] < RF[IR.m])
				  RF[IR.r] = 1; 
				else 
				  RF[IR.r] = 0; 
          
				if (printFlag)
					print_execution(PC - 1, opnames[20 - 1], IR, PC, BP, SP, stack, RF);  
				break;
        
			/// LEQ
			case 21:
				if (RF[IR.l] <= RF[IR.m])
				  RF[IR.r] = 1; 
				else 
				  RF[IR.r] = 0; 
          
				if (printFlag)
					print_execution(PC - 1, opnames[21 - 1], IR, PC, BP, SP, stack, RF);  
				break;
        
			/// GTR
			case 22:
				if (RF[IR.l] > RF[IR.m])
				  RF[IR.r] = 1; 
				else 
				  RF[IR.r] = 0; 
          
				if (printFlag)
					print_execution(PC - 1, opnames[22 - 1], IR, PC, BP, SP, stack, RF);             
				break;
        
			/// GEQ
			case 23:
				if (RF[IR.l] >= RF[IR.m])
				  RF[IR.r] = 1; 
				else 
				  RF[IR.r] = 0; 
          
				if (printFlag)
					print_execution(PC - 1, opnames[23 - 1], IR, PC, BP, SP, stack, RF); 
				break;

		}
	}
}
