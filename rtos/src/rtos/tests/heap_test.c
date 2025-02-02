/*
 * heap_test.c
 *
 * Created: 13.11.2024 11:05:32
 *  Author: tom
 */ 
#ifdef RUN_TESTS
#include <avr/io.h>
#include <string.h>
#include <avr/interrupt.h>
#include "test.h"
#include "rtos.h"
#include "event.h"


uint8_t * volatile task_mem;

static void test_task(void)
{
	task_mem = condWait_heap_malloc(BOARD_heap_single_block_size*2);
}


void heap_test(void)
{
	#define MEM_PTR_SIZE 10
	#if MEM_PTR_SIZE > BOARD_heap_number_of_blocks
		#error "for testing purposes, set BOARD_heap_number_of_blocks to a value of at least 10"
	#endif
	#define check_proper_byte_allocation(num_byte, free_mem_after_alloc)\
				mem_ptr[0] = heap_malloc(num_byte);\
				TEST(((num_byte) ? mem_ptr[0] != NULL : mem_ptr[0] == NULL));\
				TEST(heap_get_size_of_free_memory() == free_mem_after_alloc);\
				heap_free(mem_ptr[0]);\
				mem_ptr[0] = NULL\
	
	uint8_t *volatile mem_ptr[MEM_PTR_SIZE];
	uint16_t free_mem_size = BOARD_heap_number_of_blocks * BOARD_heap_single_block_size;
	uint8_t *volatile ptr;

	void malloc_all_memory(void)
	{
		//allocate the entire memory according to the following method: MEM_PTR_SIZE - 1 allocations of one block size and one allocation occupying the rest of the free memory
		for(uint8_t i=0; i<MEM_PTR_SIZE - 1; i++){
			if(mem_ptr[i] == NULL){
				mem_ptr[i] = heap_malloc(BOARD_heap_single_block_size);
				free_mem_size -= BOARD_heap_single_block_size;
			}
		}
		if(mem_ptr[MEM_PTR_SIZE - 1] == NULL){
			mem_ptr[MEM_PTR_SIZE-1] = heap_malloc(BOARD_heap_single_block_size * (BOARD_heap_number_of_blocks - MEM_PTR_SIZE + 1));
			free_mem_size -= BOARD_heap_single_block_size * (BOARD_heap_number_of_blocks - MEM_PTR_SIZE + 1);
		}
	}

	void free_memory(uint8_t id)
	{
		//free memory based on the given index, check the comment in malloc_all_memory()
		if( (id < MEM_PTR_SIZE) && (mem_ptr[id] != NULL) ){
			heap_free(mem_ptr[id]);
			free_mem_size += (id < MEM_PTR_SIZE - 1) ? BOARD_heap_single_block_size : BOARD_heap_single_block_size * (BOARD_heap_number_of_blocks - MEM_PTR_SIZE + 1);
			mem_ptr[id] = NULL;
		}
	}	
	void check_task_malloc(void)
	{		
		malloc_all_memory();
		
		//run task and try to allocate two blocks of memory size, the task should be added to waiting tasks queue
		test_rtos_add_task_to_scheduler(0, test_task);
		test_rtos_task_call(0, FALSE);
		TEST(test_rtos_task_handle(0)->state == WAIT_SEMA);
		
		//free one block
		free_memory(0);
		
		//the task should run once memory is freed
		TEST(test_rtos_task_handle(0)->state == READY);

		//there is not enough free memory so the task should be added to waiting tasks queue
		test_rtos_task_call(0, FALSE);
		TEST(test_rtos_task_handle(0)->state == WAIT_SEMA);
		
		//free one block
		free_memory(4);
		
		//the task should run once memory is freed
		TEST(test_rtos_task_handle(0)->state == READY);

		//there is not enough free memory so the task should be added to waiting tasks queue
		test_rtos_task_call(0, FALSE);
		TEST(test_rtos_task_handle(0)->state == WAIT_SEMA);
		
		//save the memory address to check if the task receives it after the next steps
		ptr = mem_ptr[3];
		//free one block
		free_memory(3);
		
		//the task should run once memory is freed
		TEST(test_rtos_task_handle(0)->state == READY);

		//there is enough free memory so the task should obtain a memory address
		//this address should be equal to ptr
		test_rtos_task_call(0, FALSE);
		TEST(task_mem == ptr);
		
		//free all memory allocated to the task
		heap_free(task_mem);
		test_rtos_remove_task_from_scheduler(0);
	}
	
	memset((void *)mem_ptr, 0, (MEM_PTR_SIZE * 2));
	
	/****** HEAP INIT ******/
	TEST(heap_get_size_of_free_memory() == free_mem_size);
	
	/****** MALLOC AND FREE ******/
	//check 0 byte memory allocation
	check_proper_byte_allocation(0, free_mem_size);
	
	//check 1 byte memory allocation
	check_proper_byte_allocation(1, free_mem_size - BOARD_heap_single_block_size);

	
	//check single block memory allocation
	check_proper_byte_allocation(BOARD_heap_single_block_size, free_mem_size - BOARD_heap_single_block_size);
	
	//check one block plus one byte memory allocation
	check_proper_byte_allocation(BOARD_heap_single_block_size + 1, free_mem_size - BOARD_heap_single_block_size * 2);
	
	//check memory allocation via task interface
	check_task_malloc();
	
	malloc_all_memory();
	free_memory(0);
	free_memory(1);
	
	free_memory(3);
	
	
	
	free_memory(7);
	free_memory(8);
	TEST(heap_malloc(BOARD_heap_single_block_size*5) == NULL);
	
	ptr = mem_ptr[6];
	free_memory(2);
	free_memory(6);
	TEST(heap_malloc(BOARD_heap_single_block_size*5) == NULL);

	free_memory(9);
	TEST(heap_malloc(BOARD_heap_single_block_size*5) == ptr);
	
	heap_free(ptr);	
	free_memory(4);
	free_memory(5);
	
	#undef MEM_PTR_SIZE
	#undef check_proper_byte_allocation
}

#endif