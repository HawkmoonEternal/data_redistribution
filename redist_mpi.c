#include "redist_mpi.h"
#include "redist.h"
#include <stdlib.h>
#include <string.h>

#define FIXED 0
#define EXPANDING 1
#define SHRINKING 2

int init_array_1D_block(MPI_Comm comm, int array_size, double **array, int *partition_size){
    int num_procs, rank;
    MPI_Comm_size(comm, &num_procs);
    MPI_Comm_rank(comm, &rank);

    create_distributed_array_1D_block(array, array_size, num_procs, rank, partition_size);

    return 0;
}

int redistribute_1D_block(MPI_Comm old_comm, int num_procs_old, MPI_Comm new_comm, int num_procs_new, int array_size, double *old_partition, double **new_partition, int *new_size){
    int rank_old = -1, rank_new = -1, ret, i;
    int new_part_size, new_start_index, new_end_index, old_start_index, old_end_index;
    int op;
    rank_list_t *sources, *targets;
    rank_list_item_t *item;


    if(MPI_COMM_NULL != old_comm){ 
        MPI_Comm_rank(old_comm, &rank_old);
    }
    
    if(MPI_COMM_NULL != new_comm){
        MPI_Comm_rank(new_comm, &rank_new);

    }

    op =    (num_procs_new == num_procs_old) ?  FIXED :
            (num_procs_new > num_procs_old)  ?  EXPANDING :
                                                SHRINKING;
    
    new_part_size = get_part_size_1D_block(array_size,  num_procs_new, rank_new);
    *new_partition = malloc(new_part_size * sizeof(double));

    
    /* If the number of processes remains the same, 
     * just copy partition and we are done
     */
    if(op == FIXED){
        memcpy(*new_partition, old_partition, new_part_size * sizeof(double));
        *new_size = new_part_size;
        return 0;
    }

    if(0 != (ret = get_target_info_1D_block(&targets, array_size, num_procs_old, num_procs_new, rank_old, rank_new))){
        return ret;
    }

    if(0 != (ret = get_source_info_1D_block(&sources, array_size, num_procs_old, num_procs_new, rank_old, rank_new))){
        return ret;
    }

    MPI_Request requests[sources->length + targets->length];
    i = 0;
    /* Send data to targets */
    if(targets->length > 0){
        old_start_index = get_start_index_1D_block(array_size, num_procs_old, rank_old);
        RANK_LIST_FOREACH(item, targets){
            if(op == EXPANDING){
                MPI_Isend(old_partition + (item->start_index - old_start_index), (item->end_index - item->start_index), MPI_DOUBLE, item->rank, 0, new_comm, &requests[i]);
            }else if (op == SHRINKING){
                MPI_Isend(old_partition + (item->start_index - old_start_index), (item->end_index - item->start_index), MPI_DOUBLE, item->rank, 0, old_comm, &requests[i]);
            }
            i++;
        }
    }
    /* Receive data from sources */
    if(sources->length > 0){
        new_start_index = get_start_index_1D_block(array_size, num_procs_new, rank_new);
        RANK_LIST_FOREACH(item, sources){
            if(op == EXPANDING){
                printf("rank_new %d: recv from %d\n", rank_new, item->rank);
                MPI_Irecv(*new_partition + (item->start_index - new_start_index), (item->end_index - item->start_index), MPI_DOUBLE, item->rank, 0, new_comm, &requests[i]);
            }else if (op == SHRINKING){
                MPI_Irecv(*new_partition + (item->start_index - new_start_index), (item->end_index - item->start_index), MPI_DOUBLE, item->rank, 0, old_comm, &requests[i]);
            }
            i++;
        }
    }
    MPI_Waitall(sources->length + targets->length, requests, MPI_STATUSES_IGNORE);
    *new_size = new_part_size;

    return 0;
}

