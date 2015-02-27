/* File:     Floyd.c
 *
 * Author:   Mark Marnell 
 *
 * Purpose:  Implement Floyd's algorithm for solving the all-pairs shortest
 *           path problem using multiple processors.  It finds the length of the shortest path 
 *           between each pair of vertices in a directed graph. It splits the matrix into multiple
 *           matrix's, each proc getting its own local matrix.
 *
 * Input:    n, the number of vertices in the digraph
 *           mat, the adjacency matrix of the digraph
 * Output:   A matrix showing the costs of the shortest paths
 *
 * Compile:  mpicc -g -Wall -o Floyd Floyd.c
 * 
 * Run:      mpiexec -n <number of processes> ./p3 < <matrix file>
 *
 *           (See note 7)
 *           
 *
 *           For large matrices, put the matrix into a file with n as
 *           the first line and run with ./floyd < large_matrix
 *
 * Notes:
 * 1.  The input matrix is overwritten by the matrix of lengths of shortest
 *     paths.
 * 2.  Edge lengths should be nonnegative.
 * 3.  If there is no edge between two vertices, the length is the constant
 *     INFINITY.  So input edge length should be substantially less than
 *     this constant.
 * 4.  The cost of travelling from a vertex to itself is 0.  So the adjacency
 *     matrix has zeroes on the main diagonal.
 * 5.  No error checking is done on the input.
 * 6.  The adjacency matrix is stored as a 1-dimensional array and subscripts
 *     are computed using the formula:  the entry in the ith row and jth
 *     column is mat[i*n + j]
 * 7.  Use the compile flag -DSHOW_INT_MATS to print the matrix after its
 *     been updated with each intermediate city.
 */








#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

//Set constants
const int INFINITY = 1000000;


//Function Declaration
void Read_matrix(int mat[], int n);
void Print_matrix(int mat[], int n);
int min(int x, int y);
void Floyd(int local_mat[], int n, int p, int my_rank, MPI_Comm comm );


/* Start main*/
int main(void) {
   //Delcare variables
   int  n;
   int* mat;
   int p, my_rank;
   int * local_mat;
   //int * test_mat;
   
   //intialize and/Or declare MPI variables
   MPI_Comm comm;
   MPI_Init(NULL, NULL);
   comm = MPI_COMM_WORLD;
   MPI_Comm_size(comm, &p);
   MPI_Comm_rank(comm, &my_rank);
  


   //If rank is 0, gets input from user
   if (my_rank == 0){


   	printf("How many vertices?\n");
   	scanf("%d", &n);
   	mat = malloc(n*n*sizeof(int));  //allocates space depending on vertices input

  	printf("Enter the matrix\n");  //Reads in the matrix
  	Read_matrix(mat, n);
	
   }

   MPI_Bcast(&n, 1, MPI_INT, 0, comm);
   local_mat = malloc((n*n/p)*sizeof(int));  //allocates storage for the local matrix's
   MPI_Scatter(mat, n*n/p, MPI_INT, local_mat, n*n/p, MPI_INT, 0, comm);  //splits the matrix to each proc
   Floyd(local_mat, n, p, my_rank, comm);  //calls Floyd function
    
    //test
   //    printf("My rank is: %d \n", my_rank);
  //  Print_matrix(local_mat, n);
 //  test_mat = malloc(n*n*sizeof(int));
//

   MPI_Gather(local_mat, n*n/p, MPI_INT, mat, n*n/p, MPI_INT, 0 , comm);  //Gathers the local mat into final mat

   //if proc 0, prints the final mat
   if (my_rank ==0){
   

   	printf("The solution is:\n");
   	Print_matrix(mat, n);
   }
   free(mat);   //frees the matrix
   free(local_mat);  //frees the local matrices
   MPI_Finalize();  //closes MPI

   return 0;
}  /* main */

/*-------------------------------------------------------------------
 * Function:  Read_matrix
 * Purpose:   Read in the adjacency matrix
 * In arg:    n
 * Out arg:   mat
 */
void Read_matrix(int mat[], int n) {
   int i, j;

   for (i = 0; i < n; i++)
      for (j = 0; j < n; j++)
         scanf("%d", &mat[i*n+j]);
}  /* Read_matrix */

/*-------------------------------------------------------------------
 * Function:  Print_matrix
 * Purpose:   Print the contents of the matrix
 * In args:   mat, n
 */
void Print_matrix(int mat[], int n) {
   int i, j;

   for (i = 0; i < n; i++) {
      for (j = 0; j < n; j++)
         if (mat[i*n+j] == INFINITY)
            printf("i ");
         else
            printf("%d ", mat[i*n+j]);
      printf("\n");
   }
}  /* Print_matrix */

/*-------------------------------------------------------------------
 * Function:    Floyd
 * Purpose:     Apply Floyd's algorithm to the matrix mat
 * In arg:      n, local_mat[], p, my_rank,  comm
 * In/out arg:  mat:  on input, the adjacency matrix, on output
 *              lengths of the shortest paths between each pair of
 *              vertices.
 */
void Floyd(int local_mat[], int n, int p, int my_rank, MPI_Comm comm ) {
   int int_city, local_city1, local_city2,  local_int_city, root, j;
   int* row_int_city;

   row_int_city = malloc((n*n/p)*sizeof(int));  //allocates space for the row 
   for (int_city = 0; int_city < n; int_city++) {  //loops through int city
	root = int_city/(n/p);       //sets root (changes throughout for loop)
	if (my_rank == root){  
		local_int_city = int_city % (n/p);  
		for(j=0; j<n; j++) {
			row_int_city[j] = local_mat[local_int_city*n+j];  //sets row
		}
	}      
	MPI_Bcast(row_int_city, n, MPI_INT, root, comm);    //broadcasts this rank's row

	for (local_city1 = 0; local_city1 < n/p; local_city1++){
            for (local_city2 = 0; local_city2 < n; local_city2++) {
               local_mat[local_city1*n + local_city2] = min(local_mat[local_city1*n+ local_city2], 
		local_mat[local_city1*n + int_city] + row_int_city[local_city2]);
	    }
	}
            
         }
#ifdef SHOW_INT_MATS
     printf("After int_city = %d\n", int_city);
     Print_matrix(mat, n);
# endif
   }
  /* Floyd */



/*-------------------------------------------------------------------
 * Function:    min
 * Purpose:     Finds the mininimum value of two integers
 * In arg:      x, y
 * In/out arg:  returns x
 *              
 *              
 */
int min(int x, int y){
	if (x >= y){
		return y;
	}
	return x;
}
 /* min */
