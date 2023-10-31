typedef struct{
    void *next;
    int rank;
    int start_index;
    int end_index;
}rank_list_item_t; 

#define RANK_LIST_FOREACH(item, list)                           \
    for (item = ( rank_list_item_t *) (list)->first;                  \
         item != NULL;                                          \
         item = (rank_list_item_t *) ((rank_list_item_t *) (item))->next)

typedef struct{
    rank_list_item_t *first;
    rank_list_item_t *last;
    int length;
}rank_list_t;


// start index (inclusive)
int get_start_index_1D_block(int array_size, int num_procs, int rank);
// end index (non-inclusive)
int get_end_index_1D_block(int array_size, int num_procs, int rank);

int get_part_size_1D_block(int array_size, int num_procs, int rank);

int get_target_info_1D_block(rank_list_t **targets, int array_size, int n_old, int n_new, int rank_old, int rank_new);
int get_source_info_1D_block(rank_list_t **sources, int array_size, int n_old, int n_new, int rank_old, int rank_new);

/* Initializes a "distibuted" array*/
void create_distributed_array_1D_block(double **array, int size, int num_procs, int rank, int *partition_size);