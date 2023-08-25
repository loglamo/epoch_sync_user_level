/* workers.c */
#define _GNU_SOURCE
#include "workers.h"
#include "file_modification.h"
#include "staging_buffers.h"

#include <stdint.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sched.h>

//#define CPU_NUM 10
//#define WRITERS_NUM 8
//#define COMMITTERS_NUM 3
//#define IOWORKERS_NUM 8

/*
uint64_t global_epoch = 0;
uint64_t global_committed_epoch = 0;
uint64_t global_checkpointed_epoch = 0;
*/
// convert global epoch variables to atomic types
//atomic_long global_epoch;
//atomic_long global_committed_epoch;
//atomic_long global_checkpointed_epoch;

/*
int writers_status_flags[] = {};
uint64_t writers_epoch_flags[] = {};
int ioworkers_status_flags[] = {};
uint64_t ioworkers_epoch_flags[] = {};
*/
// convert progress flags of workers to atomic types
/*
atomic_bool writers_status_flags[] = {};
atomic_long writers_epoch_flags[] = {};
atomic_bool ioworkers_status_flags[] = {};
uint64_t ioworkers_epoch_flags[] = {};
*/

staging_buffer_lf staging_buffers[STAGING_BUF_NUM];

long global_epoch;
long global_committed_epoch;
long global_checkpointed_epoch;

/*
int writers_status_flags[WRITERS_NUM] = {0,0,0,0,0,0,0,0};
long writers_epoch_flags[WRITERS_NUM] = {0,0,0,0,0,0,0,0};
int ioworkers_status_flags[IOWORKERS_NUM] = {0,0,0,0,0,0,0,0};
long ioworkers_epoch_flags[IOWORKERS_NUM] = {0,0,0,0,0,0,0,0};
*/
int writers_status_flags[WRITERS_NUM] = {};
long writers_epoch_flags[WRITERS_NUM] = {};
int ioworkers_status_flags[IOWORKERS_NUM] = {};
long ioworkers_epoch_flags[IOWORKERS_NUM] = {};



// initialize global variables for per-core flags of writers, ioworkers => maybe not in use
void writers_flags_init(int nr_writers) {
    for (int i=0; i < nr_writers; i++) {
        writers_status_flags[i] = 0;
        writers_epoch_flags[i] = 0;
    }
    printf("writers_status_flags and writers_epoch_flags are initialized\n");
} 

void ioworkers_flags_init(int nr_ioworkers) {// number of ioworkers is equal to number of staging buffers
    for (int i = 0; i < nr_ioworkers; i++) {
        ioworkers_status_flags[i] = 1; // ioworkers initially in running status
        ioworkers_epoch_flags[i] = 0;
    }
    printf("ioworkers_flags are initialized\n");
}

//define functions related to 4 workers
//writer
//initialize a writer 
void writer_init (writer *writer, int id_writer, int cpu_id) {
     writer->id_writer = id_writer;
     writer->cpu_id = cpu_id;
}
// writer updates its flag(current epoch, status writer:1~ON, 0~OFF)
void writer_update(writer *writer, long current_epoch, int status_writer) {
     int id_writer = writer->id_writer;
     atomic_exchange(&writers_epoch_flags[id_writer], current_epoch);
     if (status_writer == 0) {
         atomic_flag_clear(&writers_status_flags[id_writer]);
     }
}

int get_id_staging_buffer(uint64_t block_id) {
    int id_staging_buffer = rand()%STAGING_BUF_NUM;
    return id_staging_buffer;
}

// staging process of a writer
void *writer_staging(void *args) {
    // set cpu workings
    writer_args current_args = *((writer_args *) args);
    int cpu_id = current_args.current_writer.cpu_id; 
    writer current_writer = current_args.current_writer;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    pthread_t current_thread = pthread_self();
    pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);


    // staging process
    f_m local_modification = current_args.current_file_modification;
    int id_writer = current_args.current_writer.id_writer;
    atomic_flag_test_and_set(&writers_status_flags[id_writer]);
    int id_staging_buffer = get_id_staging_buffer(local_modification.block_id);

    long local_epoch = atomic_load(&global_epoch);
    entry_data new_data;
    new_data.block_id = local_modification.block_id;
    new_data.offset = local_modification.offset;

    if (local_epoch == atomic_load(&global_epoch)) {
        new_data.staged_epoch = local_epoch;
        new_data.seq = staging_buffers[id_staging_buffer].count + 1;
    } else {
        long previous_entry_epoch = staging_buffers[id_staging_buffer].LIST->next_entry->payload.staged_epoch;
        if (previous_entry_epoch > local_epoch) 
            new_data.staged_epoch = previous_entry_epoch;
        else new_data.staged_epoch = local_epoch;
      }

    // add entry to the staging buffer with lockfree
    add_s_entry_lf(&staging_buffers[id_staging_buffer], new_data);
    writer_update(&current_writer, local_epoch, 0);
   // printf("flag of current writer is %d\n", atomic_load(&writers_status_flags[id_writer]));
    printf("One entry has already staged to staging buffer %d, current count of the staging buffer is %lu\n", id_staging_buffer, staging_buffers[id_staging_buffer].count);
    printf("\n--------------------\n");
}    

// committer
// initialize a committer
void committer_init(committer *committer, int id_committer) {
    committer->id_committer = id_committer;
    //committer->committing_epoch = 0;
}



// committer updates property as global epoch changes
// main function of committer: process_commit()
void *committer_procedure(void* args) {
    long committing_epoch;
    committing_epoch = atomic_fetch_add(&global_epoch,1);
    printf("Committer is in procedure...\n");
    // check flags of writers
    for (int i = 0; i < WRITERS_NUM; i++) {
        int status_flag_i;
        status_flag_i = atomic_load(&writers_status_flags[i]);
        long epoch_flag_i;
        epoch_flag_i = atomic_load(&writers_epoch_flags[i]);
        //printf("epoch flag of writer is %lu\n", epoch_flag_i);
        while ((status_flag_i == 1) && (epoch_flag_i <= committing_epoch)) {
            status_flag_i = atomic_load(&writers_status_flags[i]);
           // printf("status of writer %d\n", status_flag_i);
            epoch_flag_i = atomic_load(&writers_epoch_flags[i]); 
           // printf("Here 1\n");
        }
    }
    long new_value = committing_epoch - 1;
    // update global_committed_epoch with CAS
    while(atomic_compare_exchange_strong(&global_committed_epoch, &new_value, committing_epoch) != new_value)
       {
           //printf("Here 2\n");
       }
    printf("Committer done, Global_committed epoch is updated to %lu\n", global_committed_epoch);
    printf("Committer done, Global_epoch is updated to %lu\n", global_epoch);
    printf("********************\n");
}

//ioworker
//initialize a ioworker
void ioworker_init(ioworker *ioworker, int assoc_staging_buffer) {
    ioworker->assoc_staging_buffer = assoc_staging_buffer;
    ioworker->id_ioworker = assoc_staging_buffer;
    ioworker->status_ioworker = 1;
    ioworker->committed_epoch = atomic_load(&global_committed_epoch);
}
//ioworker start flushing modifications to their destination as global_committed_epoch changes
//ioworker checks global_committed_epoch and updates its propertie (bool status_ioworker)
void ioworker_update_status(ioworker *ioworker, int current_epoch) {
    int id_ioworker = ioworker->id_ioworker;
    atomic_store(&ioworkers_epoch_flags[id_ioworker], current_epoch);
}

// simulate flushing by sleeping in seconds
void ioworker_flush_to_destination(ioworker *ioworker) {
    sleep(2);
}

// main procedure of an ioworker
void ioworker_process_flushing(int id_ioworker, ioworker *ioworker) {
    int ioworker_status_flag = atomic_load(&ioworkers_status_flags[id_ioworker]);
    long ioworker_epoch_flag = atomic_load(&ioworkers_epoch_flags[id_ioworker]);
    while(ioworker_status_flag == 1) {
        if (ioworker_epoch_flag != atomic_load(&global_committed_epoch)) {
            ioworker_flush_to_destination(ioworker);
        } else continue;
    }
   // printf("********************\n");
}

// main procedure of an ioworker used for pthread, 
// ioworker_args: ioworker_id
void *ioworker_process_flushing_for_pthread(void* args) {
    ioworker_args ioworker_arguments = *((ioworker_args *) args);
    ioworker ioworker = ioworker_arguments.current_ioworker;
    int id_ioworker = ioworker.id_ioworker;
    while(1) {
    int ioworker_status_flag = atomic_load(&ioworkers_status_flags[id_ioworker]);
    long ioworker_epoch_flag = atomic_load(&ioworkers_epoch_flags[id_ioworker]);
    while(ioworker_status_flag == 1) {
        //printf("global committed epoch %lu\n", atomic_load(&global_committed_epoch));
        if (ioworker_epoch_flag != atomic_load(&global_committed_epoch)) {
            printf("+global_committed_epoch changes, io_worker %d starts flushing\n", id_ioworker);
            ioworker_flush_to_destination(&ioworker);
            printf ("Done with flushing of ioworker %d\n", id_ioworker);
            printf ("---\n");
            //ioworker_update_status(&ioworker, );
        } else continue;
     }
  }
}


//monitor
//initialize a monitor
void monitor_init(monitor *monitor) {
    monitor->checkpointed_epoch = atomic_load(&global_checkpointed_epoch);
}

// take the minimum epoch flag of epoch flags from all ioworkers
long monitor_MIN(long ioworkers_epoch_flags[], int len) {
    //uint64_t *convert_ioworkers_epoch_flags = *((uint64_t*) ioworkers_epoch_flags);
    long num = ioworkers_epoch_flags[0];
    for (int i=1; i < len; i++) {
        long tmp = ioworkers_epoch_flags[i];
        if (tmp < num)
            num = tmp;
    }
    return num;
}

// the monitor checks all flushing processes of ioworkers for a global_committed_epoch. If all are done, global_checkpointed_epoch is updated
void *monitor_procedure(void* args) {
     printf("\n The monitor in progress...\n");
     while(1) {
     long local_epoch;
     local_epoch = monitor_MIN(ioworkers_epoch_flags, IOWORKERS_NUM);
     atomic_store(&global_checkpointed_epoch, local_epoch);
     //printf("Checkpointed epoch now is updated with the local epoch %lu\n", global_checkpointed_epoch);
  }
}
