#include <stdlib.h>
#include "redist.h"
#include <stdio.h>

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))


void rank_list_item_create(rank_list_item_t **item){
    *item = malloc(sizeof(rank_list_item_t));
    (*item)->rank = -1;
    (*item)->next = NULL;
}

void rank_list_item_free(rank_list_item_t **item){
    free(*item);
}

void rank_list_create(rank_list_t ** list){
    *list = malloc(sizeof(rank_list_t));
    (*list)->first = NULL;
    (*list)->last = NULL;

    (*list)->length = 0;
}

void rank_list_free(rank_list_t ** list){
    if(NULL == (*list)->first)return;
    
    rank_list_item_t *prev, *item = (*list)->first;

    while(NULL != item){
        prev = item;
        item = (rank_list_item_t *) item->next;
        
        free(prev);
        (*list)->length --;
    }

    free(*list);
}

void rank_list_append(rank_list_t *list, rank_list_item_t *item){
    
    if(list->last != NULL){
        list->last->next = item;
    }
    if(NULL == list->first){
        list->first = item;
    }
    list->last = item; 
}

void rank_list_add_rank(rank_list_t *list, int rank, int start_index, int end_index){
    rank_list_item_t * item;
    rank_list_item_create(&item);
    item->rank = rank;
    item->start_index = start_index;
    item->end_index = end_index;
    rank_list_append(list, item);
    list->length++;
}

void create_distributed_array_1D_block(double **array, int size, int num_procs, int rank, int *partition_size){
    int start_index;

    if(rank == num_procs - 1){
        *partition_size = size - rank * (size / num_procs);
    }else{
        *partition_size = size / num_procs;
    }

    start_index = get_start_index_1D_block(size, num_procs, rank);
    *array = malloc(*partition_size * sizeof(double));
    for(int i = 0; i < *partition_size; i++){
        (*array)[i] = (double) (start_index + i);
    }
}

// start index (inclusive)
int get_start_index_1D_block(int array_size, int num_procs, int rank){
    return array_size / num_procs * rank;
}

// end index (non-inclusive) 
int get_end_index_1D_block(int array_size, int num_procs, int rank){
    return rank == num_procs - 1 ? array_size : (array_size / num_procs) * (rank + 1);
}

int get_part_size_1D_block(int array_size, int num_procs, int rank){

    if(0 == num_procs && rank < 0){
        return 0;
    }

    return 
        get_end_index_1D_block(array_size, num_procs, rank) - 
        get_start_index_1D_block(array_size, num_procs, rank);
}

int get_rank_for_index_1D_block(int array_size, int num_procs, int index){
    return min(num_procs - 1, index / (array_size / num_procs));
}

int get_target_info_1D_block(rank_list_t **targets, int array_size, int n_old, int n_new, int rank_old, int rank_new){

    if(     array_size < 1  || n_old > array_size   || n_new > array_size    ||
            rank_new > n_new - 1 || rank_old > n_old - 1 
    ){
        return -1;
    }

    rank_list_create(targets);

    if(n_old == 0 || rank_old < 0){
        return 0;
    }

    int old_start_index = get_start_index_1D_block(array_size, n_old, rank_old);
    int old_end_index = get_end_index_1D_block(array_size, n_old, rank_old);

    /* Targets */
    int target = get_rank_for_index_1D_block(array_size, n_new, old_start_index);
    int cur_start_index = old_start_index;
    int cur_end_index = get_end_index_1D_block(array_size, n_new, target);
    
    rank_list_add_rank(*targets, target, old_start_index, min(cur_end_index, old_end_index));

    while(cur_end_index < old_end_index){
        target++;
        cur_start_index = get_start_index_1D_block(array_size, n_new, target);
        cur_end_index = get_end_index_1D_block(array_size, n_new, target);
        rank_list_add_rank(*targets, target, cur_start_index, min(cur_end_index, old_end_index));
    }

    return 0;

}

int get_source_info_1D_block(rank_list_t **sources, int array_size, int n_old, int n_new, int rank_old, int rank_new){

    if(     array_size < 1  || n_old > array_size   || n_new > array_size   ||
            rank_new > n_new - 1 || rank_old > n_old - 1 
    ){
        return -1;
    }

    rank_list_create(sources);

    if(n_new == 0 || rank_new < 0){
        return 0;
    }

    int new_start_index = get_start_index_1D_block(array_size, n_new, rank_new);
    int new_end_index = get_end_index_1D_block(array_size, n_new, rank_new);

    /* Sources */
    int source = get_rank_for_index_1D_block(array_size, n_old, new_start_index);
    int cur_start_index = new_start_index;
    int cur_end_index = get_end_index_1D_block(array_size, n_old, source);
    rank_list_create(sources);
    rank_list_add_rank(*sources, source, new_start_index, min(cur_end_index, new_end_index));
    while(cur_end_index < new_end_index){
        source++;
        cur_start_index = get_start_index_1D_block(array_size, n_old, source);
        cur_end_index = get_end_index_1D_block(array_size, n_old, source);
        rank_list_add_rank(*sources, source, cur_start_index, min(cur_end_index, new_end_index));
    }
    return 0;
}