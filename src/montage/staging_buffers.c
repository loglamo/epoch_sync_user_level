/* staging_buffers.c */
#include "staging_buffers.h"
#include "workers.h"
#include "file_modification.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

/*
#define STAGING_BUF_NUM 8
#define STAGING_BUF_LEN 100
#define STAGING_ENTRY_SIZE 56
*/
//extern staging_buffer_lf staging_buffers[STAGING_BUF_NUM];

//functions of staging buffers
void staging_buffer_init(staging_buffer_lf *sb, int id_staging_buffer, int id_partition, uint64_t capacity) {
        sb->id_staging_buffer = id_staging_buffer;
        sb->id_associated_partition = id_partition;
        sb->buffer = malloc(capacity*(sizeof(s_entry)));
        sb->s_entry_head = sb->LIST;
        sb->s_entry_tail = NULL;
        sb->count = 0;
        sb->capacity = capacity;
        sb->LIST = malloc(sizeof(s_entry));       
} 
//free one staging buffer
void staging_buffer_free(staging_buffer_lf *sb) {
        free(sb->buffer);
        sb->count = 0;
}

void add_s_entry(staging_buffer *sb, entry_data new_data) {
        pthread_mutex_lock(&sb->buffer_mutex);
        s_entry *new_entry = malloc(sizeof(s_entry));
        new_entry->payload = new_data;
        new_entry->next_entry = sb->LIST;
        sb->LIST = new_entry;
       // sb->
       // __sync_fetch_and_add(&sb->count,1);
        pthread_mutex_unlock(&sb->buffer_mutex);
}



void add_s_entry_lf(staging_buffer_lf *sb, entry_data new_data) {
        s_entry *new_entry = malloc(sizeof(s_entry));
        new_entry->payload = new_data;
        s_entry *old_next = malloc(sizeof(s_entry));
        while(1) {
          new_entry->next_entry = sb->LIST->next_entry;
          old_next = sb->LIST->next_entry;
          if (__sync_bool_compare_and_swap(&sb->LIST->next_entry, old_next, new_entry)) {
             printf("add entry sucessfully to staging_buffer %d\n", sb->id_staging_buffer);
             __sync_fetch_and_add(&sb->count,1); 
             break;
           } 
        }
}

void print_info_staging_buffer(staging_buffer_lf *staging_buffer) {
        s_entry *temp = staging_buffer->LIST->next_entry;
        uint64_t staging_buffer_id = staging_buffer->id_staging_buffer;
        printf("\n Elements in staging buffer %lu are:\n", staging_buffer_id);
        while(temp != NULL) {
                printf("Seq - %lu\n", temp->payload.seq);
                printf("Staged epoch - %lu\n", temp->payload.staged_epoch);
                printf("Block id - %lu\n", temp->payload.block_id);
                printf("Offset - %lu\n", temp->payload.offset);
                temp = temp->next_entry;
                printf("_____");
           }
        printf("Count of the staging buffer %lu is %lu \n", staging_buffer_id, staging_buffer->count);
        printf("********************");
}
