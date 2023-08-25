//basic ring buffer with multiple thread accessing the buffer
#define _GNU_SOURCE

#include "bounded_buffer.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <sched.h>

#define BUFLEN 10
#define ITEMSIZE 16
//#define NITERS 10
#define THREAD_NUM 4

/*
typedef struct ring_buffer {
	void *buffer;        //data buffer
	void *buffer_end;    //end of data buffer
	void *data_head;    //pointer to head
	void *data_tail;      //pointer to tail
	uint64_t capacity;   //maximum number of items in buffer
	uint64_t count;      //current number of items
	uint64_t size;       //size of each item
	pthread_mutex_t buffer_mutex;    //a mutex for the buffer
	sem_t full;          //semaphore for full status
	sem_t empty;          //semaphore for empty status
} ring_buffer;

//define item in buffer, each item has its own timestamp
typedef struct buf_data_item {
	time_t timestamp;
	int data_item;
} data_item;
*/


//initialize ring buffer
void rb_init(ring_buffer *rb, uint64_t size, uint64_t capacity) {
	rb->buffer = malloc(capacity*size);
	rb->buffer_end = (char*)rb->buffer + capacity*size;
	rb->capacity = capacity;
	rb->count = 0;
	rb->size = size;
	rb->data_head = rb->buffer;
	rb->data_tail = rb->buffer;
}

//free ring buffer
void rb_free(ring_buffer *rb) {
	pthread_mutex_lock(&rb->buffer_mutex);
	free(rb->buffer);
	rb->count = 0;
	pthread_mutex_unlock(&rb->buffer_mutex);
}

//add item to ring buffer, push item to the back (head)
void rb_push_item(ring_buffer *rb, const void *item) {
	if(rb->count == rb->capacity) {
	     printf("Buffer is full!");
	}
 	//pthread_mutex_lock(&rb->buffer_mutex);
	memcpy(rb->data_head, item, rb->size);
	rb->data_head = (char*)rb->data_head + rb->size;
	if(rb->data_head == rb->buffer_end)
		rb->data_head = rb->buffer;
	rb->count++;
	//pthread_mutex_unlock(&rb->buffer_mutex);

}

//remove item from ring buffer following FIFO, remove item from front of the buffer
void rb_pop_item(ring_buffer *rb, void *item) {
	if(rb->count == 0) {
	     printf("Buffer is empty!");
	}
	//pthread_mutex_lock(&rb->buffer_mutex);
	memcpy(item, rb->data_tail, rb->size);
	rb->data_tail = (char*)rb->data_tail + rb->size;
	if(rb->data_tail == rb->buffer_end)
		rb->data_tail = rb->buffer;
	rb->count--;
	//pthread_mutex_unlock(&rb->buffer_mutex);
}

//other version of pop() without item argument 
int rb_pop_item_v2(ring_buffer *rb) {
	if(rb->count == 0) {	
		return -1;
	}else {
	   pthread_mutex_lock(&rb->buffer_mutex);
	   char*  item = rb->data_tail;
	   printf("value dequeued is %d\n", *item);
	   rb->data_tail = (char*)rb->data_tail + rb->size; //move tail pointer to the new pointer according to size of item
	   if(rb->data_tail == rb->buffer_end)
		   rb->data_tail = rb->buffer;
	   rb->count--;
	   pthread_mutex_unlock(&rb->buffer_mutex);
           printf("Count after then is %ld\n", rb->count);
	   return 1;
	}
}

//define producer and consumer thread
void *producer(ring_buffer rb) {
	pthread_mutex_init(&rb.buffer_mutex, NULL);
	printf("information of thread is %lu\n", pthread_self() );
	while(1) {
	     //produce an item 
	     int item = rand() % 100;
	     sleep(2);
	     sem_wait(&rb.full);
	     //pthread_mutex_lock(&rb.buffer_mutex);
	     //push item to the buffer
	     rb_push_item(&rb, &item);
	     printf("item %d was added to the buffer\n", item);
	     //pthread_mutex_unlock(&rb.buffer_mutex);
	     sem_post(&rb.empty);
	     printf("The number of items now is %ld\n", rb.count);
	     printf("**********\n");
	      }
	printf("Buffer is full!\n");
	//pthread_exit(&rb);
}
void *consumer(ring_buffer rb) {
	    pthread_mutex_init(&rb.buffer_mutex, NULL);
	    printf("information of thread is %lu\n", pthread_self() );
	    while(1) {
		sleep(2);
	    	sem_wait(&rb.empty);
	    	//void* flushed_item = &rb.data_head;
	    	printf("Current count is %ld\n", rb.count); 
	    	//pthread_mutex_lock(&rb.buffer_mutex);
	    	rb_pop_item_v2(&rb);
	    	printf("The first item was dequeued from the buffer\n");
	    	//pthread_mutex_unlock(&rb.buffer_mutex);
	    	sem_post(&rb.full);
	    	printf("The number of items now is %ld\n", rb.count);
	    	printf("**********\n");
	    }
}




int main() {
        //set cpu
	cpu_set_t cpu_setting;
	CPU_ZERO(&cpu_setting);
	CPU_SET(5, &cpu_setting);
	sched_setaffinity(0, sizeof(cpu_set_t), &cpu_setting);


	ring_buffer Nbuffer;
	rb_init(&Nbuffer, ITEMSIZE, BUFLEN);
	//rb_free(&Nbuffer);
	printf("A buffer has been created...\n");
	printf("sched_getcpu = %d\n", sched_getcpu());

       	sem_init(&Nbuffer.full, 0, BUFLEN);
	sem_init(&Nbuffer.empty, 0, 0);

	
        //pthread_t th_prod[THREAD_NUM];
	//pthread_t th_cons[THREAD_NUM];
	
	pthread_mutex_init(&Nbuffer.buffer_mutex, NULL);
	
	int i;
	for (i = 0; i < 9; i++){
		int item = rand()%100;
		rb_push_item(&Nbuffer, &item);
		printf("Adding %d to buffer\n", item);
		sleep(2);
	}
	printf("Done adding test\n");
	printf("Current count is %ld\n", Nbuffer.count);
	printf("Test new version pop function....\n");
        rb_pop_item_v2(&Nbuffer);
        rb_pop_item_v2(&Nbuffer);
	printf("current count is %ld\n", Nbuffer.count);
	return 0;
}
