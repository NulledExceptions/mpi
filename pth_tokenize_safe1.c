/* File:  
 *    pth_tokenize_safe1.c
 *
 * Purpose:
 *    Try to use threads to tokenize text input.  This version
 *    uses the threadsafe function strtok_r.
 *
 *
 * Input:
 *    Lines of text
 * Output:
 *    For each line of input:
 *       the line read by the program, and the tokens identified by 
 *       strtok_r
 *
 * Compile:
 *    gcc -g -Wall -o pth_tokenize_safe1 pth_tokenize_safe1.c -lpthread
 * Usage:
 *    ./pth_tokenize_safe1 <thread_count> < <input>
 *
 * Algorithm:
 *    For each line of input, next thread reads the line and
 *    "tokenizes" it.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

const int MAX = 1000;

int thread_count;
sem_t* sems;

void Usage(char* prog_name);
void *Tokenize(void* rank);  /* Thread function */

/*--------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
   long        thread;
   pthread_t* thread_handles; 

   if (argc != 2) Usage(argv[0]);
   thread_count = strtol(argv[1], NULL, 10);

   thread_handles = malloc (thread_count*sizeof(pthread_t));
   sems = malloc(thread_count*sizeof(sem_t));
   // sems[0] should be unlocked, the others should be locked
   sem_init(&sems[0], 0, 1);
   for (thread = 1; thread < thread_count; thread++);
      sem_init(&sems[thread], 0, 0);

   printf("Enter text\n");
   for (thread = 0; thread < thread_count; thread++)
      pthread_create(&thread_handles[thread], (pthread_attr_t*) NULL,
          Tokenize, (void*) thread);

   for (thread = 0; thread < thread_count; thread++) {
      pthread_join(thread_handles[thread], NULL);
   }

   for (thread=0; thread < thread_count; thread++)
      sem_destroy(&sems[thread]);

   free(sems);
   free(thread_handles);
   return 0;
}  /* main */


/*--------------------------------------------------------------------
 * Function:    Usage
 * Purpose:     Print command line for function and terminate
 * In arg:      prog_name
 */
void Usage(char* prog_name) {

   fprintf(stderr, "usage: %s <number of threads>\n", prog_name);
   exit(0);
}  /* Usage */


/*-------------------------------------------------------------------
 * Function:    Work
 * Purpose:     Tokenize lines of input
 * In arg:      rank
 * Global vars: thread_count (in), sems (in/out)
 * Return val:  Ignored
 */
void *Tokenize(void* rank) {
   long my_rank = (long) rank;
   int count;
   int next = (my_rank + 1) % thread_count;
   char *fg_rv;
   char my_line[MAX+1];
   char *my_string, *save_string;

   /* Force sequential reading of the input */
   sem_wait(&sems[my_rank]);
   fg_rv = fgets(my_line, MAX, stdin);
   sem_post(&sems[next]);
   while (fg_rv != NULL) {
      printf("Thread %ld > my line = %s", my_rank, my_line);

      count = 0;
      my_string = strtok_r(my_line, " \t\n", &save_string);
      while ( my_string != NULL ) {
         count++;
         printf("Thread %ld > string %d = %s\n", my_rank, count, my_string);
         my_string = strtok_r(NULL, " \t\n", &save_string);
      }
      sem_wait(&sems[my_rank]);
      fg_rv = fgets(my_line, MAX, stdin);
      sem_post(&sems[next]);
   }

   return NULL;
}  /* Tokenize */
