#include "redist_mpi.h"
#include <stdlib.h>


int ARRAY_SIZE = 32;


int main(int argc, char *argv[]){
    MPI_Comm origin_comm, destination_comm;
    int origin_size, origin_rank, destination_size, destination_rank;
    
    int rank_split, color; 
    int old_partition_size, new_partition_size;
    int world_rank, world_size;

    double *partition_old, *partition_new;
    
    rank_split = 1;

    if(argc > 1){
        rank_split = atoi(argv[1]);
    }
    if(argc > 2){
        ARRAY_SIZE = atoi(argv[2]);
    }

    MPI_Init(NULL, NULL);

    /* Origin comm is MPI_COMM_WORLD*/
    origin_comm = MPI_COMM_WORLD;
    MPI_Comm_rank(origin_comm, &origin_rank);
    MPI_Comm_size(origin_comm, &origin_size);

    /* Destination comm includes all processes with rank < rank_split in destination_comm */
    color = origin_rank < rank_split ? 0 : 1;
    MPI_Comm_split(origin_comm, color, origin_rank, &destination_comm);
    MPI_Comm_size(destination_comm, &destination_size);
    MPI_Comm_rank(destination_comm, &destination_rank);

    /* Create a distributed array across all processes */
    if(0 != init_array_1D_block(origin_comm, ARRAY_SIZE, &partition_old, &old_partition_size)){
        printf("ERROR!\n");
        return -1;
    }

    for(int i = 0; i < old_partition_size; i++){
        printf("Rank %d: old_partition[%d]: %f\n", origin_rank, i, partition_old[i]);
    }

    if(origin_rank < rank_split ) {
        
        if(0 != redistribute_1D_block(origin_comm, origin_size, destination_comm, destination_size, ARRAY_SIZE, partition_old, &partition_new, &new_partition_size)){
            printf("Rank %d: ERROR in redist!\n", destination_rank);
            return -1;
        }

        for(int i = 0; i < new_partition_size; i++){
            printf("Rank %d: new_partr[%d]: %f\n", destination_rank, i, partition_new[i]);
        }
        
        free(partition_new);
    }else{
        if(0 != redistribute_1D_block(origin_comm, origin_size, MPI_COMM_NULL, origin_size - destination_size, ARRAY_SIZE, partition_old, &partition_new, &new_partition_size)){
            printf("ERROR in redist!\n");
            return -1;
        }
        for(int i = 0; i < new_partition_size; i++){
            printf("Rank %d: new_partr[%d]: %f\n", destination_rank, i, partition_new[i]);
        }

    }
    MPI_Comm_free(&destination_comm);
    
    free(partition_old);
    
    MPI_Finalize();

    return 0;
}