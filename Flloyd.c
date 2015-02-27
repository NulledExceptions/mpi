#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

const int INFINITY = 1000000;

void Read_matrix(int mat[], int n);
void Print_matrix(int mat[], int n);
void Floyd(int mat[], int n);
int min(int x, int y);
void Floyd(int local_mat[], int n, int p, int my_rank, MPI_Comm comm, );

int main(void) {
   int  n;
   int* mat;
   int p, my_rank;
   int * local_mat;
   int * test_mat;
  
   MPI_Comm comm;
   MPI_Init(NULL, NULL);
   comm = MPI_COMM_WORLD;
   MPI_Comm_size(comm, &p);
   MPI_Comm_rank(comm, &my_rank);
  



   if (my_rank == 0){


   	printf("How many vertices?\n");
   	scanf("%d", &n);
   	mat = malloc(n*n*sizeof(int));

  	printf("Enter the matrix\n");
  	Read_matrix(mat, n);

  // Floyd(mat, n);

  // 	printf("The solution is:\n");
  // 	Print_matrix(mat, n);
   }
   MPI_Bcast(&n, 1, MPI_INT, 0, comm);
   local_mat = malloc((n*n/p)*sizeof(int));
   MPI_Scatter(mat, n*n/p, MPI_INT, local_mat, n*n/p, MPI_INT, 0, comm);
   //test
    
 //    printf("My rank is: %d \n", my_rank);
   //  Print_matrix(local_mat, n);
 //  test_mat = malloc(n*n*sizeof(int));
   MPI_Gather(local_mat, n*n/p, MPI_INT, mat, n*n/p, MPI_INT, 0 , comm);

   free(mat);
   free(local_mat);
   MPI_Finalize();

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
 * In arg:      n
 * In/out arg:  mat:  on input, the adjacency matrix, on output
 *              lengths of the shortest paths between each pair of
 *              vertices.
 */
void Floyd(int local_mat[], int n, int p, int my_rank, MPI_Comm comm, ) {
   int int_city, local_city1, local_city2, local_city,  local_int_city, root, j;
   int* row_int_city;

   row_int_city = malloc((n*n/p)*sizeof(int))

   for (int_city = 0; int_city < n; int_city++) {
	root = int_city/(n/p);
	if (my_rank == root){
		local_int_city = int_city % (n/p);
		for(j=0; j<n; j++) {
			row_int_city[j] = local_mat[local_int_city*n+j];
		}
	}      
	MPI_Bcast(row_int_city, n, MPI_INT, root, comm);

	for (local_city1 = 0; local_city1 < n/p; local_city1++){
            for (local_city2 = 0; local_city2 < n; local_city++) {
               local_mat[local_city1*n + local_city2] = min(local_mat[local_city*n+ local_city2], local_mat[local_city1*n + int_city] + row_int_city[local_city2]);
	    }
	}
            
         }
   //  #ifdef SHOW_INT_MATS
     // printf("After int_city = %d\n", int_city);
    //  Print_matrix(mat, n);
   // # endif
   }
}  /* Floyd */




int min(int x, int y){
	if (x >= y){
		return y;
	}
	return x;
}
