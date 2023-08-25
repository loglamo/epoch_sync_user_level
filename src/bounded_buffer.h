#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>

typedef struct ring_buffer {
	void *buffer;
	void *buffer_end;
	void *data_head;
	void *data_tail;
	uint64_t capacity;
	uint64_t count;
	uint64_t size;
	pthread_mutex_t buffer_mutex;
	sem_t full;
	sem_t empty;
} ring_buffer;

typedef struct buf_data_item {
	time_t timestamp;
	char data_item;
	int global_epoch;
	int local_epoch;
	int CPU_id;
} data_item;

//functions related to the buffer
void rb_init(ring_buffer *rb, uint64_t size, uint64_t capacity);
void rb_free(ring_buffer *rb);
void rb_push_item(ring_buffer *rb, const void *item);
void rb_pop_item(ring_buffer *rb, void *item);
int rb_pop_item_v2(ring_buffer *rb);
