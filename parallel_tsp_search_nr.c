/* File:     parallel_tsp_search_nr.c
 * Purpose:  Use an iterative depth-first search to solve an 
 *           instance of the travelling salesman problem.
 * Input:    From a user-specified file, the number of cities
 *           followed by the costs of travelling between the
 *           cities organized as a matrix:  the cost of
 *           travelling from city i to city j is the ij entry.
 * Output:   The best tour found by the program and the cost
 *           of the tour.
 * Compile:  gcc -g -Wall -o  -lpthread
 * Usage:    parallel_tsp_search_nr <number of threads> <matrix_file>
 *
 * Notes:
 * 1.  Weights and cities are non-negative ints.
 * 2.  Program assumes the cost of travelling from a city to
 *     itself is zero, and the cost of travelling from one
 *     city to another city is positive.
 * 3.  Note that costs may not be symmetric:  the cost of travelling
 *     from A to B, will, in general, be different from the cost
 *     of travelling from B to A.
 * 4.  Salesperson's home town is 0.
 * 5.  This version uses a linked list for the stack.
 */



#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>




const int INFINITY = 1000000;
const int NO_CITY = -1;
const int FALSE = 0;
const int TRUE = 1;

typedef int city_t;
typedef int weight_t;

typedef struct {
   city_t* cities;
   int count;
   weight_t cost;
} tour_t;

typedef struct stack_struct {
   tour_t* tour_p;               /* Partial tour             */
   city_t  city;                 /* City under consideration */
   weight_t cost;                /* Cost of going to city    */
   struct stack_struct* next_p;  /* Next record on stack     */
} stack_elt_t;


/*------------------------------------------------------------------*/
/* Global variables */
int n;
weight_t* mat;
tour_t best_tour;
pthread_rwlock_t    rwlock;
pthread_mutex_t     count_mutex;
int         thread_count;


/*------------------------------------------------------------------*/
/* Serial functions */

void Usage(char* prog_name);
void Read_mat(FILE* mat_file);
void Print_mat(void);

void Initialize_tour(tour_t* tour_p);
void *Search(void* rank);
void Print_tour(tour_t* tour_p, char* title);
void Check_best_tour(city_t city, tour_t* tour_p, int local_best);
int  Feasible(city_t city, city_t nbr, tour_t* tour_p, int local_best);
int  Visited(city_t nbr, tour_t* tour_p);
void Push(tour_t* tour_p, city_t city, weight_t cost, stack_elt_t** stack_p);
tour_t* Dup_tour(tour_t* tour_p);
void Pop(tour_t** tour_pp, city_t* city_p, weight_t* cost_p, 
    stack_elt_t** stack_p);
int  Empty(stack_elt_t* stack);


/* Parallel function */

void* Search(void* rank);

/*------------------------------------------------------------------*/

int main(int argc, char* argv[]) {
   FILE*  mat_file;
   pthread_t* thread_handles;
   
   long thread;


   if (argc != 3) Usage(argv[0]);
   thread_count = strtol(argv[1],NULL,10);
   mat_file = fopen(argv[2], "r");
   if (mat_file == NULL) {
      fprintf(stderr, "Can't open %s\n", argv[2]);
      Usage(argv[0]);
   }
   Read_mat(mat_file);
   fclose(mat_file);
#  ifdef DEBUG
   Print_mat();
#  endif   

   Initialize_tour(&best_tour);
   best_tour.cost = INFINITY;
  
   thread_handles = malloc(thread_count*sizeof(pthread_t));
   pthread_mutex_init(&count_mutex, NULL);
   pthread_rwlock_init(&rwlock, NULL);


   for (thread = 0; thread < thread_count; thread++)
      pthread_create(&thread_handles[thread], NULL, Search, (void*) thread);

  
   for (thread = 0; thread < thread_count; thread++) 
      pthread_join(thread_handles[thread], NULL); 


   
   Print_tour(&best_tour, "Best tour");
   printf("Cost = %d\n", best_tour.cost);

   free(best_tour.cities);
   free(mat);
   return 0;
}  /* main */

/*------------------------------------------------------------------
 * Function:  Usage
 * Purpose:   Inform user how to start program and exit
 * In arg:    prog_name
 */
void Usage(char* prog_name) {
   fprintf(stderr, "usage: %s <number of threads>  <matrix file>\n", prog_name);
   exit(0);
}  /* Usage */

/*------------------------------------------------------------------
 * Function:         Read_mat
 * Purpose:          Read in the number of cities and the matrix of costs
 * In arg:           mat_file
 * Global vars out:  mat, n
 */
void Read_mat(FILE* mat_file) {
   int i, j;

   fscanf(mat_file, "%d", &n);
   mat = malloc(n*n*sizeof(weight_t));

   for (i = 0; i < n; i++)
      for (j = 0; j < n; j++)
         fscanf(mat_file, "%d", &mat[n*i+j]);
}  /* Read_mat */


/*------------------------------------------------------------------
 * Function:        Print_mat
 * Purpose:         Print the number of cities and the matrix of costs
 * Global vars in:  mat, n
 */
void Print_mat(void) {
   int i, j;

   printf("Order = %d\n", n);
   printf("Matrix = \n");
   for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++)
         printf("%2d ", mat[i*n+j]);
      printf("\n");
   }
   printf("\n");
}  /* Print_mat */


/*------------------------------------------------------------------
 * Function:    Initialize_tour
 * Purpose:     Initialize a tour with no visited cities
 * In/out arg:  tour_p
 */
void Initialize_tour(tour_t* tour_p) {
   int i;

   tour_p->cities = malloc((n+1)*sizeof(city_t));
   for (i = 0; i <= n; i++) {
      tour_p->cities[i] = NO_CITY;
   }
   tour_p->cost = 0;
   tour_p->count = 0;
}  /* Initialize_tour */


/*------------------------------------------------------------------
 * Function:            Search
 * Purpose:             Search for an optimal tour
 * Global vars in:      mat, n
 * Global vars in/out:  best_tour
 */
void* Search(void* rank) {
   int quotient;
   int remainder;
   city_t nbr;
   city_t city;
   weight_t cost;
   tour_t* tour_p;
   stack_elt_t* stack_p;
   long my_rank = (long) rank;
   int partial_tour_count;
   int first_final_city;
   int last_final_city;
   int local_best = INFINITY;


   quotient = (n-1) / thread_count;
   remainder = (n-1) % thread_count;
   if(my_rank<remainder){
	partial_tour_count= quotient+1;
	first_final_city = my_rank*partial_tour_count +1;
   }else{
	partial_tour_count = quotient;
	first_final_city = my_rank*partial_tour_count+remainder+1;
   }
   last_final_city= first_final_city + partial_tour_count -1;



   tour_p = malloc(sizeof(tour_t));
   Initialize_tour(tour_p);
   /* Don't Push the first node, since Push duplicates */
   stack_p = malloc(sizeof(stack_elt_t));
   stack_p->tour_p = tour_p;
   stack_p->city = 0;
   stack_p->cost = 0;
   stack_p->next_p = NULL;

   while (!Empty(stack_p)) {
      Pop(&tour_p, &city, &cost, &stack_p);
      tour_p->cities[tour_p->count] = city;
      tour_p->cost += cost;
      tour_p->count++;
      if (tour_p->count == n) {
         Check_best_tour(city, tour_p, local_best);
         
      } else {
         for (nbr = n-1; nbr > 0; nbr--) 
            if (Feasible(city, nbr, tour_p, local_best)) 
               Push(tour_p, nbr, mat[n*city+nbr], &stack_p);
      }
      /* Push duplicates the tour.  So it needs to be freed */
      free(tour_p->cities);
      free(tour_p);
   }  /* while */
  return NULL;

}  /* Search */


/*------------------------------------------------------------------
 * Function:        Feasible
 * Purpose:         Check whether nbr could possibly lead to a better
 *                  solution if it is added to the current tour.  The
 *                  functions checks whether nbr has already been visited
 *                  in the current tour, and, if not, whether adding the
 *                  edge from the current city to nbr will result in
 *                  a cost less than the current best cost.
 * In args:         All
 * Global vars in:  mat, n, best_tour
 * Return:          TRUE if the nbr can be added to the current tour.
 *                  FALSE otherwise
 */
int Feasible(city_t city, city_t nbr, tour_t* tour_p, int local_best) {
   if (!Visited(nbr, tour_p) &&
        tour_p -> cost + mat[n*city + nbr] < local_best) 
      return TRUE;
   else
      return FALSE;
}  /* Feasible */


/*------------------------------------------------------------------
 * Function:   Visited
 * Purpose:    Use linear search to determine whether nbr has already
 *             bee visited on the current tour.
 * In args:    All
 * Return val: TRUE if nbr has already been visited.  
 *             FALSE otherwise
 */
int Visited(city_t nbr, tour_t* tour_p) {
   int i;

   for (i = 0; i < tour_p->count; i++)
      if ( tour_p->cities[i] == nbr ) return TRUE;
   return FALSE;
}  /* Visited */


/*------------------------------------------------------------------
 * Function:  Print_tour
 * Purpose:   Print a tour
 * In args:   All
 */
void Print_tour(tour_t* tour_p, char* title) {
   int i;

   printf("%s:\n", title);
   for (i = 0; i < tour_p->count; i++)
      printf("%d ", tour_p->cities[i]);
   printf("\n\n");
}  /* Print_tour */


/*------------------------------------------------------------------
 * Function:            Check_best_tour
 * Purpose:             Determine whether the current n-city tour will be
 *                      better than the current best tour.  If so, update
 *                      best_tour
 * In args:             city, tour_p
 * Global vars in:      mat, n
 * Global vars in/out:  best_tour
 */
void Check_best_tour(city_t city, tour_t* tour_p, int local_best) {
   int i;
   if (tour_p->cost + mat[city*n + 0] < local_best){
	local_best=tour_p->cost + mat[city*n + 0];   
   }


   if (tour_p->cost + mat[city*n + 0] < best_tour.cost) {
      for (i = 0; i < tour_p->count; i++)
         best_tour.cities[i] = tour_p->cities[i];
      best_tour.cities[n] = 0;
      best_tour.count = n+1;
      best_tour.cost = tour_p->cost + mat[city*n + 0];
   }
}  /* Check_best_tour */



/*------------------------------------------------------------------
 * Function:    Push
 * Purpose:     Add a new node to the top of the stack
 * In args:     tour_p, city, cost
 * In/out arg:  stack_pp:  on input pointer to current stack
 *                 on output pointer to stack with new top record
 * Note:        The input tour is duplicated before being pushed
 *              so that the existing tour can be used in the 
 *              Search function
 */
void Push(tour_t* tour_p, city_t city, weight_t cost, 
      stack_elt_t** stack_pp) {
   stack_elt_t* temp = malloc(sizeof(stack_elt_t));
   temp->tour_p = Dup_tour(tour_p);
   temp->city = city;
   temp->cost = cost;
   temp->next_p = *stack_pp;
   *stack_pp = temp;
}  /* Push */


/*------------------------------------------------------------------
 * Function:  Dup_tour
 * Purpose:   Create a duplicate of the tour referenced by tour_p:
 *            used by the Push function
 * In arg:    tour_p
 * Ret val:   Pointer to the copy of the tour
 */
tour_t* Dup_tour(tour_t* tour_p) {
   int i;
   tour_t* temp_p = malloc(sizeof(tour_t));
   temp_p->cities = malloc(n*sizeof(city_t));
   for (i = 0; i < n; i++)
      temp_p->cities[i] = tour_p->cities[i];
   temp_p->cost = tour_p->cost;
   temp_p->count = tour_p->count;
   return temp_p;
}  /* Dup_tour */

/*------------------------------------------------------------------
 * Function:    Pop
 * Purpose:     Remove the top node from the stack and return it
 * In/out arg:  stack_pp:  on input the current stack, on output
 *                 the stack with the top record removed
 * Out args:    tour_pp:  the tour in the top stack node
 *              city_p:   the city in the top stack node
 *              cost_p:   the cost of visiting the city
 */
void Pop(tour_t** tour_pp, city_t* city_p, weight_t* cost_p, 
    stack_elt_t** stack_pp) {
   stack_elt_t* stack_p = *stack_pp;
   *tour_pp = stack_p->tour_p;
   *city_p = stack_p->city;
   *cost_p = stack_p->cost;
   *stack_pp = stack_p->next_p;
   free(stack_p);

}  /* Pop */


/*------------------------------------------------------------------
 * Function:  Empty
 * Purpose:   Determine whether the stack is empty
 * In arg:    stack_p
 * Ret val:   TRUE if stack is empty, FALSE otherwise
 */
int  Empty(stack_elt_t* stack_p) {
   if (stack_p == NULL)
      return TRUE;
   else
      return FALSE;
}  /* Empty */


