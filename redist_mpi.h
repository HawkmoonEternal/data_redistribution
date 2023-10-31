#include "mpi.h"
#include <stdio.h>

/* For a given communicator and array size creates a distributed array where each rank has a certain partition */
int init_array_1D_block(MPI_Comm comm, int array_size, double **partition, int *partition_size);

/* Redistributes a distributed array from origin_comm to destination_comm 
 * If a process is not included in the origin_comm / destination_comm pass MPI_COMM_NULL respectively
 */
int redistribute_1D_block(MPI_Comm origin_comm, int origin_size, MPI_Comm destination_comm, int destination_size, int array_size, double *old_partition, double **new_parition, int *new_partition_size);