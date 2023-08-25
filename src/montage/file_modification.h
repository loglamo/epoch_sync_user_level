/* fle_modification.h */
#ifndef FILE_MODIFICATION_H
#define FILE_MODIFICATION_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "workers.h"


#define MOD_NUM 64
#define BLOCKS_NUM 8000000
#define OFFSET 10
// struct for each file  modification
typedef struct modification {
    int block_id;
    int offset;
    int size;
    char data;
    int order_index;
} f_m;


// args struct for pthread writers
typedef struct writer_args {
    f_m current_file_modification;
    writer current_writer;
} writer_args;


int block_id_generate(int blocks_num);
int block_offset_generate(int offset_num);
int size_generate(int size_num);
f_m* modification_generate(int mod_num, int blocks_num, int offset_num, int size_num);
f_m* modification_generate_v1(int mod_num, int blocks_num, int offset_num);
writer_args* writer_args_generate(int, f_m*, writer*);

#endif 
