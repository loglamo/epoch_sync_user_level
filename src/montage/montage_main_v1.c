#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
//#include "file_modification_generator.c"
#include "file_modification.h"
#include "workers.h"
#include "staging_buffers.h"

//staging_buffer_lf staging_buffers[STAGING_BUF_NUM];
// define global variables


void main() {
    // initialize components and their flags
    extern staging_buffer_lf staging_buffers[STAGING_BUF_NUM]; 
    writer writers[WRITERS_NUM];
    committer committers[COMMITTERS_NUM];
    ioworker ioworkers[IOWORKERS_NUM];
    monitor monitor; 
    for (int i = 0; i < WRITERS_NUM; i++) {
        writer_init(&writers[i], i, i);
        //printf("Writers are initialized with their id and associated cpu...\n");
    }   
    for (int i = 0; i < COMMITTERS_NUM; i++) {
        committer_init(&committers[i], i);
        //printf("Committers are initialized with their id...\n");
    }
    for (int i = 0; i < IOWORKERS_NUM; i++) {
        ioworker_init(&ioworkers[i], i);
        //printf("IOworkers are initialized with their id and associated staging buffers...\n");
    }
    monitor_init(&monitor);
    writers_flags_init(WRITERS_NUM);
    ioworkers_flags_init(IOWORKERS_NUM);
     
    // generate file modifications
    f_m *modifications;
    modifications = modification_generate_v1(MOD_NUM, BLOCKS_NUM, OFFSET);
    printf("%d Modifications have already been generated...\n", MOD_NUM);
    // generate writer arguments to pthreads
    writer_args *writer_args;
    writer_args = writer_args_generate(MOD_NUM, modifications, writers);
    // initialize staging buffers
    for (int i = 0; i < STAGING_BUF_NUM; i++) {
        staging_buffer_init(&staging_buffers[i], i, i, 200);
        //printf("Staging buffer %d has already been initialized...\n", i);
    }        


    pthread_t th_monitor;
    pthread_t th_ioworkers[IOWORKERS_NUM];
    pthread_t th_committers[COMMITTERS_NUM];
    pthread_t th_writers[WRITERS_NUM];
    // thread for the monitor
    
    pthread_create(&th_monitor, NULL, &monitor_procedure, NULL);
   
    // threads for ioworkers 
    for (int i=0; i < IOWORKERS_NUM; i++) {
        pthread_create(&th_ioworkers[i], NULL, &ioworker_process_flushing_for_pthread, (void *)(&ioworkers[i]));
        printf("\nA thread for ioworker %d in progress\n", i);
    }
        
    for (int i=0; i < WRITERS_NUM; i++) {
        pthread_create(&th_writers[i], NULL, &writer_staging, (void *)(&writer_args[i]));
    }
    // threads for committers
    for (int i=0; i < COMMITTERS_NUM; i++) {
        pthread_create(&th_committers[i], NULL, &committer_procedure, NULL);        
        printf("\nA thread for committer %d in progress\n", i);
    }
    for (int i=0; i < WRITERS_NUM; i++) {
        pthread_create(&th_writers[i], NULL, &writer_staging, (void *)(&writer_args[i]));
   }
    pthread_join(th_monitor, NULL);
}

