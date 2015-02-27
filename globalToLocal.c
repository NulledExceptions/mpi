/*
 * Pupose:  program that will convert global row numbers to local row numbers
 * 	 		and vice-versa.
 * File:    globalToLocal.c
 * Input:   three ints and a char
 * Output:  local or global row number
 * Compile: -g -Wall -o  globalToLocal globalToLocal.c
 * Run:     ./globalToLocal
 */
 
#include <stdio.h>;
#include <stdlib.h>;

//Main starts
int main (){	
	//Variable dicalration
	int numProc;
	int numRow;
	int rowToConvert;
	char letter;
	int locGlobProcess;
	int rowPProc;
	int conversion;
	

	//Prompting and scanning for variables
	printf("Please enter number of Processes:");
	scanf("%d", &numProc);
	
	printf("Please enter number of Rows:");
	scanf("%d", &numRow);
	
	printf("Please enter the row number to convert: ");
	scanf("%d", &rowToConvert);
	
	printf("Please enter g for global or l for local:");
	scanf(" %c", &letter);
	
	
	//Checking if it is local or global input
	if ((letter =='l') || (letter =='L'))
	{
		printf("Please enter the process to which the local row belongs");
		scanf("%d", &locGlobProcess);	
		rowPProc =(numRow)/(numProc);
		conversion = (locGlobProcess * rowPProc) + rowToConvert;
		printf("Converted row is: %d\n", conversion);
	}
	else if ((letter =='g') || (letter == 'G')){
		
		rowPProc =(numRow)/(numProc);
		conversion = rowToConvert % rowPProc;
		printf("Converted row is: %d\n", conversion);
	}
	
	
return 0;
	
}
//Main ends

