/* File:     thread_messaging.c
 * Author:   Mark Marnell
 * Purpose:  Threads send each other messages, by putting a
 *           barrier between the threads' creation of the 
 *           messages and the threads' reading the messages
 *
 * Compile:  gcc -g -Wall -o thread_messaging. thread_messaging.c
 * Run:      ./hw11 <number of threads>
 *
 * Input:    None
 * Output:   Messages received by threads
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_MSG_LEN 100
#define MAX_MSGS 100

int volatile total = 0;
int thread_count;
char* messages[MAX_MSGS];
pthread_mutex_t total_mutex;
pthread_cond_t cond_var;

void Usage(char prog_name[]);
void *Thread_work(void* rank);

int main(int argc, char* argv[]) {
   long i;
   pthread_t* thread_handles; 

   if (argc != 2) Usage(argv[0]);
   thread_count = strtol(argv[1], NULL, 10);
   thread_handles = malloc (thread_count*sizeof(pthread_t));
   pthread_mutex_init(&total_mutex, NULL);
   pthread_cond_init(&cond_var, NULL);

   for (i = 0; i < thread_count; i++) {
      pthread_create(&thread_handles[i], NULL, Thread_work, (void*) i);
   }

   for (i = 0; i < thread_count; i++) 
      pthread_join(thread_handles[i], NULL);

   pthread_mutex_destroy(&total_mutex);
   pthread_cond_destroy(&cond_var);
   for (i = 0; i < thread_count; i++)
      free(messages[i]);
   free(thread_handles);
   return 0;
}  /* main */

/*---------------------------------------------------------------------
 * Function:  Usage
 * Purpose:   Print a message showing how to run program and quit
 * In arg:    prog_name
 */
void Usage(char prog_name[]) {
   fprintf(stderr, "usage: %s <number of threads>\n", prog_name);
   exit(0);
}  /* Usage */

/*---------------------------------------------------------------------
 * Function:        Thread_work
 * Purpose:         Threads send messages to each other by first creating
 *                     the messages, then entering a barrier, and finally
 *                     printing the messages after leaving the barrier.
 * In arg:          rank
 * Globals in:      thread_count
 * Globals in/out:  total_mutex:  mutex protecting total
 *                  total:  number of threads that have entered barrier
 */
void *Thread_work(void* rank) {
   long my_rank = (long) rank;
   int dest = (my_rank + 1) % thread_count;

   /* Create message */
   messages[dest] = malloc(MAX_MSG_LEN*sizeof(char));
   sprintf(messages[dest], "Greetings to %d from %ld", dest, my_rank);

   /* Enter barrier */
   pthread_mutex_lock(&total_mutex);
   total++;
   pthread_mutex_unlock(&total_mutex);

   //Checks if all threads have finished
   if (total == thread_count){
	total =0;       
        pthread_cond_broadcast(&cond_var);	  //awakes sleeping threads
        pthread_mutex_unlock(&total_mutex);  //unlocks mutex for last thread
        


    }else{

       while(pthread_cond_wait(&cond_var, &total_mutex)!=0);  //If not all are 									//finished, wait
       
    }
     pthread_mutex_unlock(&total_mutex);
	

   /* Print message */
   printf("Thread %ld > total = %d, %s\n", my_rank, total, 
         messages[my_rank]);

	
   return NULL;
}  /* Thread_work */

