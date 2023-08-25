/* file_modification_generator.c */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "file_modification.h"
#include "workers.h"
#include "staging_buffers.h"
/*
#define BLOCKS_NUM 8000000
#define OFFSET 10
#define SIZE 4096 
#define MOD_NUM 20
*/


//staging_buffer_lf staging_buffers[STAGING_BUF_NUM];
// generate block_id
int  block_id_generate(int  blocks_num) {
     int id;
     id = rand()%blocks_num;
     return id;
    }

// generate block_offset
int block_offset_generate(int offset_num) {
    int id;
    id = rand()%offset_num;
    return id;
    }

// may not in use
// generate size
int size_generate(int size_num) {
    int size;
    size = rand()%size_num;
    return size;
    }

// generate array of modifications
f_m* modification_generate(int mod_num, int blocks_num, int offset_num, int size_num) {
    f_m *file_modifications = malloc(sizeof(f_m)*mod_num);
    for (int i = 0; i < mod_num; i++) {
        int block_id_i = block_id_generate(blocks_num);
        int block_offset_i = block_offset_generate(offset_num);
        int size_i = size_generate(size_num);
        file_modifications[i].block_id = block_id_i;
        file_modifications[i].offset = block_offset_i;
        file_modifications[i].size = size_i;
        file_modifications[i].order_index = i;
        //char data_i = "a";
        //file_modifications[i].data = data_i;
        }
    return file_modifications;
}

// generate array of modification (mod_num, blocks_num, offset_num) without size
f_m* modification_generate_v1(int mod_num, int blocks_num, int offset_num) {
    f_m *file_modifications = malloc(sizeof(f_m)*mod_num);
    for (int i = 0; i < mod_num; i++) {
        int block_id_i = block_id_generate(blocks_num);
        int block_offset_i = block_offset_generate(offset_num);
        file_modifications[i].block_id = block_id_i;
        file_modifications[i].offset = block_offset_i;
        file_modifications[i].order_index = i;
    }
    return file_modifications;
}

// generate arguments for writers 
writer_args* writer_args_generate(int mod_num, f_m *modification, writer *allocated_writer) {
    writer_args *args = malloc(sizeof(writer_args)*mod_num);
    for (int i = 0; i < mod_num; i++) {
           args[i].current_file_modification = modification[i];
           int id_writer = rand()%WRITERS_NUM;
           args[i].current_writer = allocated_writer[id_writer];
    }
    printf("Writer arguments for pthreads have been created...\n");
    return args; 
}
/*
void main() {
    //int id;
    //id = block_id_generate(BLOCKS_NUM);
    //printf("Generated id is %d\n", id);
    f_m *test;
    test = modification_generate_v1(MOD_NUM, BLOCKS_NUM, OFFSET);
    for (int i = 0; i < MOD_NUM; i++) {
        printf("The generation %d with block_id %d, offset %d, size %d\n", i, test[i].block_id, test[i].offset, test[i].size);
    }
}*/
