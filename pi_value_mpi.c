/* File:     pi_value_mpi.c
 * Author:   Mark Marnell
 * Purpose:  Estimate pi using the the formula  using OpenMP
 *
 *              pi = 4*[1 - 1/3 + 1/5 - 1/7 + 1/9 - . . . ]
 *
 * Compile:  gcc -g -Wall -fopenmp -o pi_value_mpi pi_value_mpi.c -lm              
 * Run:      hw12 <threads> <n>
 *           n is the number of terms of the Maclaurin series to use
 *
 * Input:    none            
 * Output:   The estimate of pi and the value of pi computed by the
 *           arctan function in the math library
 *
 * Notes:
 *    1.  The radius of convergence is only 1.  So the series converges
 *        quite slowly.
 */        

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

//global variable, although could be placed in main here
int thread_count;


void Usage(char* prog_name);

int main(int argc, char* argv[]) {
   long long i, n;
   double factor = 1.0;
   double sum = 0.0;

   //check if initial inputs are valid
   if (argc != 3) Usage(argv[0]);

   //saves the amount of threads
   thread_count = strtol(argv[1], NULL, 10);

   n = strtoll(argv[2], NULL, 10);
   if (n <= 0) Usage(argv[0]);

//declares pragma
#  pragma omp parallel for num_threads(thread_count) \
      reduction (+:sum)  default(none) private(i, factor)\
      shared(n)


   for (i = 0; i < n; i++) {

   //Factor moved into if-else block to run 
   //with multiple thread  s 
   if( i%2 ==0){
         factor = 1.0;
      }else{
         factor = -1.0;
      }



      sum += factor/(2*i+1);
#     ifdef DEBUG
      printf("i = %lld, sum = %.15f\n", i, sum);
#     endif
   }

   sum = 4.0*sum;
   printf("With n = %lld terms,\n", n);
   printf("   Our estimate of pi = %.15f\n", sum);
   printf("                   pi = %.15f\n", 4.0*atan(1.0));
   return 0;
}  /* main */

/*------------------------------------------------------------------
 * Function:  Usage
 * Purpose:   Print a message explaining how to run the program
 * In arg:    prog_name
 */
void Usage(char* prog_name) {
   fprintf(stderr, "usage: %s <threads> <n>\n", prog_name);
   fprintf(stderr, "   n is the number of terms and should be >= 1\n");
   exit(0);
}  /* Usage */

