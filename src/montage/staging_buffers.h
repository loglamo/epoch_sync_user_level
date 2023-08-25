/* staging_buffers.h */
#ifndef STAGING_BUFFERS_H
#define STAGING_BUFFERS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>


#define STAGING_BUF_NUM 8
#define STAGING_BUF_LEN 100
#define STAGING_ENTRY_SIZE 56




//define data structures of staging_buffers
//s_entry: each entry in one staging_buffer
//
typedef struct entry_data {
    uint64_t seq;
    uint64_t staged_epoch;
    uint64_t block_id;
    uint64_t offset;
    uint64_t length;
    uint64_t data;
} entry_data;


//typedef struct s_entry entry;
typedef struct s_entry {
        struct s_entry *next_entry;
        entry_data payload;
} s_entry;

typedef struct list_head_s_entry {
        struct list_head *next, *prev;
} list_head_s_entry;

typedef struct s_entry_v1 {
         entry_data payload;
         list_head_s_entry entry;
} s_entry_v1;

//s_list: each s_list in one staging buffer
typedef struct s_list {
        uint64_t id_staging_buffer;
        uint64_t id_block;
        void *head_s_list;
        void *tail_s_list;
} s_list;



//staging_buffer: define structure of one staging_buffer
typedef struct staging_buffer {
        uint64_t id_staging_buffer;
        uint64_t id_associated_partition;
        void *buffer;
        void *buffer_end;
        s_entry *s_entry_head;
        s_entry *s_entry_tail;
        uint64_t capacity;
        uint64_t count;
        uint64_t s_entry_size;
        s_entry *LIST;
        pthread_mutex_t buffer_mutex;
} staging_buffer;

// staging buffer without locking
typedef struct staging_buffer_lf {
        int id_staging_buffer;
        int id_associated_partition;
        uint64_t capacity;
        void *buffer;
        void *buffer_end;
        void *s_entry_head;
        void *s_entry_tail;
        uint64_t count;
        s_entry *LIST;
} staging_buffer_lf;


//staging_buffer_lf staging_buffers[STAGING_BUF_NUM];
void staging_buffer_init(staging_buffer_lf*, int, int, uint64_t);
void staging_buffer_free(staging_buffer_lf*);
void add_s_entry(staging_buffer*, entry_data);
void add_s_entry_lf(staging_buffer_lf*, entry_data);
void print_info_staging_buffer(staging_buffer_lf*);

#endif
