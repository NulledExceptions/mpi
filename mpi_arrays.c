/* File:     mpi_arrays.c
 *
 * Author:    Mark Marnell  
 *     
 *
 * Purpose: DAXPY- "Double precision Alpha X Plus Y."
 *           Reads in 2 arrays, multiplies each element of one 
 *           by alpha and then adds it to the second array
 *           
 *       
 *
 * Input:
 *     n: order
 *     y, x: the matrix and the vector to be multiplied
 *
 * Output:
 *     y: the product vector
 *
 * Compile:  gcc -g -Wall -o  mpi_arrays.c -lpthread
 * Usage:
 *     mpi_arrays.c <thread_count>
 *
 * Notes:  
 *     1.  Local storage for x, y is dynamically allocated.
 *     2.  Number of threads (thread_count) should evenly divide both 
 *         m and n.  The program doesn't check for this.
 *     3.  We use a 1-dimensional array for A and compute subscripts
 *         using the formula A[i][j] = A[i*n + j]
 *     4.  Distribution of x, and y is logical:  all three are 
 *         globally shared.
 *
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

const int MAX_THREADS = 64;

/* Global variables */
int     thread_count;
int     n;
double alpha;
double* x;
double* y;

/* Serial functions */
void Usage(char* prog_name);
void Read_vector(char* prompt, double x[], int n);
void Print_vector(char* title, double y[], double m);

/* Parallel function */
void *Daxpy(void* rank);

/*------------------------------------------------------------------*/


int main(int argc, char* argv[]) {
   long       thread;
   pthread_t* thread_handles;

   if (argc != 2) Usage(argv[0]);
   // error checks thread count

   thread_count = atoi(argv[1]);

   // error checks thread count
   if (thread_count <= 0 || thread_count > MAX_THREADS) Usage(argv[0]);

   thread_handles = malloc(thread_count*sizeof(pthread_t));

   //asks for the dimensions of the arrays
   printf("Enter n: \n");
   scanf("%d", &n);


  //allocates storage for arrays
   x = malloc(n*sizeof(double));
   y = malloc(n*sizeof(double));



   // Reads and prints first array
   Read_vector("Enter the x array: ", x, n);
   Print_vector("We read", x, n);

   //Reads and prints second array
   Read_vector("Enter the y array: ", y, n);
   Print_vector("We read", y, n);

   //Reads in alpha
   printf("Enter Alpha: \n");
   scanf("%lf", &alpha);


   //Creates threads and passes Daxpy function 
   for (thread = 0; thread < thread_count; thread++)
      pthread_create(&thread_handles[thread], NULL,
         Daxpy, (void*) thread);

   //Gathers threads
   for (thread = 0; thread < thread_count; thread++)
      pthread_join(thread_handles[thread], NULL);

   //Prints the solution
   Print_vector("The product is", y, n);


   //Frees the allocated memory
   free(x);
   free(y);
   free(thread_handles);

   return 0;
}  /* main */




/*------------------------------------------------------------------
 * Function:       Daxpy
 * Purpose:        Multiply one array by alpha and add
 *                 it to the second array
 * In arg:         rank
 * Global in vars: alpha, x, n, thread_count
 * Global out var: y
 */
void *Daxpy(void* rank) {
   long my_rank = (long) rank;
   int i;
   int local_n = n/thread_count; 
   int my_first_row = my_rank*local_n;


   for (i = my_first_row; i <= n; i = i +0) {
           y[i] += alpha*x[i];

	if( (i + local_n) < n){
		 i+=local_n;
	}else{
		i = n+1;
	}

   }

   return NULL;
}  /*  Daxpys */



/*------------------------------------------------------------------
 * Function:        Read_vector
 * Purpose:         Read in the vector x
 * In arg:          prompt, n
 * Out arg:         x
 */
void Read_vector(char* prompt, double x[], int n) {
   int   i;

   printf("%s\n", prompt);
   for (i = 0; i < n; i++) 
      scanf("%lf", &x[i]);
}  /* Read_vector */




/*------------------------------------------------------------------
 * Function:    Print_vector
 * Purpose:     Print a vector
 * In args:     title, y, m
 */
void Print_vector(char* title, double y[], double m) {
   int   i;

   printf("%s\n", title);
   for (i = 0; i < m; i++)
      printf("%4.1f ", y[i]);
   printf("\n");
}  /* Print_vector */

/*------------------------------------------------------------------
 * Function:  Usage
 * Purpose:   print a message showing what the command line should
 *            be, and terminate
 * In arg :   prog_name
 */
void Usage (char* prog_name) {
   fprintf(stderr, "usage: %s <thread_count>\n", prog_name);
   exit(0);
}  /* Usage */

