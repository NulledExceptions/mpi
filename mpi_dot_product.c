/* File:     mpi_dot_product.c
 * Purpose:  compute a dot product of a vector distributed among
 *           the processes.  Uses a block distribution of the vectors.
 *
 * Compile:  mpicc -g -Wall -o mpi_dot_product mpi_dot_product.c
 * Run:      mpiexec -n <number of processes> ./proj4 <number to search>
 *
 * Input:    n: number to search primes in
 *           p: the number of processors
 *
 * Output:   global_primes: array of all the primes up to the number
 *
 * Notes:  
 *     1.  n, the global order of the vectors must be divisible by p, 
 *         the number of processes.
 *     2.  Result returned by Parallel_dot is valid on all processes
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <mpi.h>




void Print_list(int list[], int n, int my_rank);
int Get_n(int argc, char* argv[]);
int Is_prime(int i);
void Usage (char* prog_name);

int main(int argc, char* argv[]) {
   int my_rank, n;
   int global_primes[];
   int local_primes[];
   int p;
   int local_n = 0;
   

   MPI_Comm comm;


   MPI_Init(&argc, &argv);
   comm = MPI_COMM_WORLD;
   MPI_Comm_size(comm, &p);
   MPI_Comm_rank(comm, &my_rank);

  
   if( my_rank == 0)
   {
      n = Get_n(int argc, char* argv[])
   }

   MPI_Bcast(n, 1, MPI_INT, 0, comm);


   local_primes[] = malloc(((n/(2p))+2)*sizeof(int));

   int j=0;
   int i = (2*my_rank+3);
   for(i; i < n; i=(i + 2*p)){
	int temp_prime = Is_prime(i);
   
	
	if (temp_prime == 1)
	{
		local_primes[j] =temp_prime;
		j=j+1
		local_n=local_n +1;
	}

	}



   free(global_primes);
   free(local_primes);
   MPI_Finalize();
   return 0;
}  /* main */

/*-------------------------------------------------------------------
 * Function:  Print_list
 * Purpose:   Convert a list of ints to a single string before
 *            printing.  This should make it less likely that the
 *            output is interrupted by another process.  This is
 *            mainly intended for debugging purposes.
 * In args:   list:  the ints to be printed
 *            n:  the number of ints
 *            my_rank:  the usual MPI variable
 */
void Print_list(int list[], int n, int my_rank) {
   char string[STRING_MAX];
   char* s_p;
   int i;

   sprintf(string, "Proc %d > ", my_rank);
   // Pointer arithmetic:  make s_p point to the character strlen(string)
   // into string; i.e., make it point at the `\0'
   s_p = string + strlen(string);
   for (i = 0; i < n; i++) {
      sprintf(s_p, "%d ", list[i]);
      s_p = string + strlen(string);
   }

   printf("%s\n", string);
   fflush(stdout);
}  /* Print_list */


/*------------------------------------------------------------------
 * Function:  Usage
 * Purpose:   print a message showing what the command line should
 *            be, and terminate
 * In arg :   prog_name
 */
void Usage (char* prog_name) {
   fprintf(stderr, "usage: -n <number of nodes> %s <number to search>\n", prog_name);
   exit(0);
}  /* Usage */


/*-------------------------------------------------------------------
 * Function:    Get_n
 * Purpose:     Get the input value n
 * Input args:  argc:  number of command line args
 *              argv:  array of command line args
 */
int Get_n(int argc, char* argv[]) {
   int n;

   if (argc != 2) Usage(argv[0]);

   /* Convert string to int */
   n = strtol(argv[1], NULL, 10);

   /* Check for bogus input */
   if (n <= 1) Usage(argv[0]);

   return n;
}  /* Get_n */




/*-------------------------------------------------------------------
 * Function:   Is_prime
 * Purpose:    Determine whether the argument is prime
 * Input arg:  i
 * Return val: true (nonzero) if arg is prime, false (zero) otherwise
 */
int Is_prime(int i) {
   int j;

   for (j = 2; j <= sqrt(i); j++)
      if (i % j == 0)
         return 0;
   return 1;
}  /* Is_prime */


/*---------------------------------------------------------------
 * Function:  Global_sum
 * Purpose:   Compute global sum of values distributed across 
 *            processes
 * Input args:
 *    my_contrib: the calling process' contribution to the global sum
 *    my_rank:    the calling process' rank in the communicator
 *    p:          the number of processes in the communicator
 *    comm:       the communicator used for sends and receives
 *
 * Return val:  the sum of the my_contrib values contributed by
 *    each process.
 *
 * Algorithm:  Use tree structured communication, pairing processes
 *    to communicate.
 *
 * Notes:
 *    1. The value returned by global_sum on processes other
 *       than 0 is meaningless.
 *    2. The pairing of the processes is done using modular
 *       arithmetic.
 */
int Collect_primes(int local, int my_rank, int p, MPI_Comm comm) {
   int sum = my_contrib;
   int temp;
   int divisor = 2;
   int proc_diff = 1;
   int partner;
   int done = 0;
   int i_send;

   while (!done && divisor <= p) {
      i_send = my_rank % divisor;
      if (i_send) {
         partner = my_rank - proc_diff;
         MPI_Send(&sum, 1, MPI_INT, partner, 0, comm); 
         done = 1;
      } else {  /* I'm receiving */
         partner = my_rank + proc_diff;
         MPI_Recv(&temp, 1, MPI_INT, partner, 0, comm, 
               MPI_STATUS_IGNORE);
         sum += temp;
         divisor *= 2;
         proc_diff *= 2;
      }
   }

   /* Valid only on 0 */
   return sum;
}  /* Global_sum */


