/* File:     parallel_mat_vect.c 
 *
 * Purpose:  Computes a parallel matrix-vector product.  Matrix
 *           is distributed by block rows.  Vectors are distributed 
 *           by blocks.  This version generates a random matrix
 *           and a random vector.
 *
 * Input:
 *     m, n: order of matrix
 *
 * Output:
 *     y:    the product vector
 *
 * Compile:  mpicc -g -Wall -o parallel_mat_vect parallel_mat_vect.c
 * Run:      mpiexec -n <number of processes> parallel_mat_vect
 *
 * Notes:  
 *     1.  Local storage for A, x, and y is dynamically allocated.
 *     2.  Number of processes (p) should evenly divide both m and n.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void Gen_array(float array[], int size, int seed);
void Read_matrix(char* prompt, float local_A[], int local_m, int n,
             int my_rank, int p, MPI_Comm comm);
void Read_vector(char* prompt, float local_x[], int local_n, int my_rank,
             int p, MPI_Comm comm);
void Parallel_matrix_vector_prod(float local_A[], int m, 
             int n, float local_x[], float global_x[], float local_y[],
             int local_m, int local_n, MPI_Comm comm);
void Print_matrix(char* title, float local_A[], int local_m,
             int n, int my_rank, int p, MPI_Comm comm);
void Print_vector(char* title, float local_y[], int local_m, int my_rank,
             int p, MPI_Comm comm);

int main(int argc, char* argv[]) {
    int             my_rank;
    int             p;
    float*          local_A; 
    float*          global_x;
    float*          local_x;
    float*          local_y;
    int             m, n;
    int             local_m, local_n;
    MPI_Comm        comm;

    MPI_Init(&argc, &argv);
    comm = MPI_COMM_WORLD;
    MPI_Comm_size(comm, &p);
    MPI_Comm_rank(comm, &my_rank);

    if (my_rank == 0) {
        printf("Enter the order of the matrix (m x n)\n");
        scanf("%d %d", &m, &n);
    }
    MPI_Bcast(&m, 1, MPI_INT, 0, comm);
    MPI_Bcast(&n, 1, MPI_INT, 0, comm);

    local_m = m/p;
    local_n = n/p;

    local_A = malloc(local_m*n*sizeof(float));
    Gen_array(local_A, local_m*n, my_rank);
//  Print_matrix("We read", local_A, local_m, n, my_rank, p, comm);

    local_x = malloc(local_n*sizeof(float));
    Gen_array(local_x, local_n, 10*my_rank);
//  Print_vector("We read", local_x, local_n, my_rank, p, comm);

    local_y = malloc(local_m*sizeof(float));
    global_x = malloc(n*sizeof(float));

    Parallel_matrix_vector_prod(local_A, m, n, local_x, global_x, 
        local_y, local_m, local_n, comm);
    Print_vector("The product is", local_y, local_m, my_rank, p, comm);

    free(local_A);
    free(local_x); 
    free(local_y); 
    free(global_x);

    MPI_Finalize();

    return 0;
}  /* main */

/*--------------------------------------------------------------------
 * Function:  Gen_array
 * Purpose:   Generate a random array of floats
 * In args:   size:  the number of elements in the array
 *            seed:  seed for the random number generator
 * Out arg:   array:  the array of floats
 */
void Gen_array(float array[], int size, int seed) {
   int i;

   srandom(seed+1);
   for (i = 0; i < size; i++)
      array[i] = random()/((double) RAND_MAX);
}  /* Gen_array */

/*--------------------------------------------------------------------
 * Function:  Read_matrix
 * Purpose:   Read an m x n matrix from stdin and distribute it by
 *            block rows
 * In args:   prompt:  tell user to enter matrix
 *            local_m: number of local rows
 *            n:       number of columns
 *            my_rank, p, comm:  usual MPI variables
 * Out arg:   local_A: block of rows assigned to this process
 */
void Read_matrix(
         char*      prompt    /* in  */, 
         float      local_A[] /* out */, 
         int        local_m   /* in  */, 
         int        n         /* in  */,
         int        my_rank   /* in  */, 
         int        p         /* in  */,
         MPI_Comm   comm      /* in  */) {

    int     i, j;
    float*  temp = NULL;


    if (my_rank == 0) {
        temp = (float*) malloc(local_m*p*n*sizeof(float));
        printf("%s\n", prompt);
        for (i = 0; i < p*local_m; i++) 
            for (j = 0; j < n; j++)
                scanf("%f",&temp[i*n+j]);
        MPI_Scatter(temp, local_m*n, MPI_FLOAT, local_A,
            local_m*n, MPI_FLOAT, 0, comm);
        free(temp);
    } else {
        MPI_Scatter(temp, local_m*n, MPI_FLOAT, local_A,
            local_m*n, MPI_FLOAT, 0, comm);
    }
}  /* Read_matrix */


/*--------------------------------------------------------------------
 * Function:  Read_vector
 * Purpose:   Read a vector from stdin and distribute it by blocks
 * In args:   prompt:  tell the user what to enter
 *            local_n:  the number of components going to each process
 *            my_rank, p, comm:  the usual MPI variables
 * Out arg:   local_x:  the block of the vector assigned to this process
 */
void Read_vector(
         char*    prompt     /* in  */,
         float    local_x[]  /* out */, 
         int      local_n    /* in  */, 
         int      my_rank    /* in  */,
         int      p          /* in  */,
         MPI_Comm comm       /* in  */) {

    int    i;
    float* temp = NULL;

#   ifdef DEBUG
    printf("Proc %d > local_n = %d, p = %d\n",
          my_rank, local_n, p);
    fflush(stdout);
#   endif

    if (my_rank == 0) {
        temp = malloc(local_n*p*sizeof(float));
        printf("%s\n", prompt);
        for (i = 0; i < p*local_n; i++) 
            scanf("%f", &temp[i]);
#       ifdef DEBUG
        printf("Proc 0 > input vector = ");
        for (i = 0; i < p*local_n; i++) 
            printf("%.1f ", temp[i]);
        printf("\n");
#       endif
        MPI_Scatter(temp, local_n, MPI_FLOAT, local_x, local_n, MPI_FLOAT,
            0, comm);
        free(temp);
    } else {
        MPI_Scatter(temp, local_n, MPI_FLOAT, local_x, local_n, MPI_FLOAT,
            0, comm);
    }

}  /* Read_vector */


/*--------------------------------------------------------------------
 * Function:  Parallel_matrix_vector_prod
 * Purpose:   Multiply a matrix distributed by block rows by a vector
 *            distributed by blocks 
 * In args:   local_A:  my rows of the matrix A
 *            m:  the number of rows in the global matrix A
 *            n:  the number of columns in A
 *            local_x:  my components of the vector x
 *            local_m:  the number of rows in my block of A
 *            local_n:  the number of components in my block of x
 *            comm:  communicator for call to MPI_Allgather
 * Out arg:   local_y:  my components of the product vector Ax
 * Scratch:   global_x:  temporary storage for all of vector x
 * Note:      argument m is unused              
 */
void Parallel_matrix_vector_prod(
         float    local_A[]   /* in  */,
         int      m           /* in  */,
         int      n           /* in  */,
         float    local_x[]   /* in  */,
         float    global_x[]  /* in  */,
         float    local_y[]   /* out */,
         int      local_m     /* in  */,
         int      local_n     /* in  */,
         MPI_Comm comm        /* in  */) {

    /* local_m = m/p, local_n = n/p */

    int local_i, j;

    MPI_Allgather(local_x, local_n, MPI_FLOAT,
                   global_x, local_n, MPI_FLOAT,
                   comm);
    for (local_i = 0; local_i < local_m; local_i++) {
        local_y[local_i] = 0.0;
        for (j = 0; j < n; j++)
            local_y[local_i] += local_A[local_i*n+j]*global_x[j];
    }
}  /* Parallel_matrix_vector_prod */


/*--------------------------------------------------------------------*/
void Print_matrix(
         char*      title      /* in */, 
         float      local_A[]  /* in */, 
         int        local_m    /* in */, 
         int        n          /* in */,
         int        my_rank    /* in */,
         int        p          /* in */,
         MPI_Comm   comm       /* in */) {

    int   i, j;
    float* temp = NULL;

    if (my_rank == 0) {
        temp = malloc(local_m*p*n*sizeof(float));
        MPI_Gather(local_A, local_m*n, MPI_FLOAT, temp, 
            local_m*n, MPI_FLOAT, 0, comm);
        printf("%s\n", title);
        for (i = 0; i < p*local_m; i++) {
            for (j = 0; j < n; j++)
                printf("%4.1f ", temp[i*n + j]);
            printf("\n");
        }
        free(temp);
    } else {
        MPI_Gather(local_A, local_m*n, MPI_FLOAT, temp, 
            local_m*n, MPI_FLOAT, 0, comm);
    }
}  /* Print_matrix */


/*--------------------------------------------------------------------*/
void Print_vector(
         char*    title      /* in */, 
         float    local_y[]  /* in */, 
         int      local_m    /* in */, 
         int      my_rank    /* in */,
         int      p          /* in */,
         MPI_Comm comm       /* in */) {

    int    i;
    float* temp = NULL;


    if (my_rank == 0) {
        temp = malloc(local_m*p*sizeof(float));
        MPI_Gather(local_y, local_m, MPI_FLOAT, temp, local_m, MPI_FLOAT,
           0, MPI_COMM_WORLD);
        printf("%s\n", title);
        for (i = 0; i < p*local_m; i++)
            printf("%4.1f ", temp[i]);
        printf("\n");
        free(temp);
    } else {
        MPI_Gather(local_y, local_m, MPI_FLOAT, temp, local_m, MPI_FLOAT,
           0, MPI_COMM_WORLD);
    }
}  /* Print_vector */
