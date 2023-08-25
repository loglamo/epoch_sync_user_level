/* workers.h */
#ifndef WORKERS_H
#define WORKERS_H


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define CPU_NUM 10
#define WRITERS_NUM 8
#define COMMITTERS_NUM 3
#define IOWORKERS_NUM 8

/*
extern long global_epoch;
extern long global_committed_epoch;
extern long global_checkpointed_epoch;

int writers_status_flags[WRITERS_NUM] = {0,0,0,0,0,0,0,0};
long writers_epoch_flags[WRITERS_NUM] = {0,0,0,0,0,0,0,0};
int ioworkers_status_flags[IOWORKERS_NUM] = {0,0,0,0,0,0,0,0};
long ioworkers_epoch_flags[IOWORKERS_NUM] = {0,0,0,0,0,0,0,0};
*/

//define 4 types of workers in MONTAGE
//writer: stores file modifications into a staging buffer

typedef struct writer {
               int assoc_staging_buffer;
               int cpu_id;
               long current_epoch;
               int status_writer;
               int id_writer;

} writer;

//commiter: groups modifications in an epoch
typedef struct committer {
               int id_committer;  //id of committer
               long committing_epoch;   //epoch is being considered as the committing epoch
} committer;

typedef struct committer_args {
               committer current_committer;
} committer_args;
//ioworker: moves blocks in staging buffers to their final storage destination
typedef struct ioworker {
               int id_ioworker;
               int assoc_staging_buffer;
               int status_ioworker;
               long committed_epoch;
} ioworker;
// ioworker arguments for pthread
typedef struct ioworker_args {
               //int current_id_ioworker;
               ioworker current_ioworker;
} ioworker_args;
//pipeline worker: update checkpointed global variable as each I/O worker progress
typedef struct monitor {
               long checkpointed_epoch;
} monitor;


//functions of workers
void writers_flags_init(int);
void ioworkers_flags_init(int);
void writer_init(writer*, int, int);
void writer_update(writer*, long, int);
int get_id_staging_buffer(uint64_t);
void *writer_staging(void*);
void committer_init(committer*, int);
void *committer_procedure(void*);
void ioworker_init(ioworker*, int);
void ioworker_update_status(ioworker*, int);
void ioworker_flush_to_destination(ioworker*);
void ioworker_process_flushing(int, ioworker*);
void *ioworker_process_flushing_for_pthread(void*);
void monitor_init(monitor*);
long monitor_MIN(long*, int);
void *monitor_procedure(void*);

#endif
