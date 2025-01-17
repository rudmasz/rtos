/*
 * heap.c
 *
 * Created: 23.05.2024 07:44:45
 *  Author: tom
 */ 
#include <avr/io.h>
#include <string.h>
#include "rtos.h"
#include "semaphore.h"

#define FREE_BLOCK_MARKER	0

#if BOARD_heap_number_of_blocks > 255
	#error The number of blocks can not exceed 255
#endif


struct heap{
	uint8_t			mem_space[BOARD_heap_number_of_blocks][BOARD_heap_single_block_size];
	uint8_t			mem_allocation_markers[BOARD_heap_number_of_blocks];
	uint8_t			mem_free_blocks;
	semaphore_t		mem_guard;
};

RTOS_static volatile struct heap __heap;//	__attribute__((section(".noinit")));


/**********************************************************************************************//**
 * @fn	void __heap_wake_up_next_waiting_task(void)
 *
 * @brief	the function checks if there is any task waiting for free dynamic memory, if so, the next waiting task will be woken up
 *
 **************************************************************************************************/

static void __heap_wake_up_next_waiting_task(void)
{
	semaphore_signal((semaphore_t *)&__heap.mem_guard);
	semaphore_wait((semaphore_t *)&__heap.mem_guard);
}


/**********************************************************************************************//**
 * @fn	uint8_t heap_check_if_dynamic_mem(void *mem_addr)
 *
 * @brief	the function checks whether the given memory address comes from the heap address range
 *
 * @param 	mem_addr   	memory address.
 * @returns	TRUE - if in the heap range, FALSE - if outside the heap range.
 **************************************************************************************************/

uint8_t heap_check_if_dynamic_mem(void *mem_addr)
{
	if( (mem_addr >= (void *)__heap.mem_space[0x00]) &&
		(mem_addr <= (void *)__heap.mem_space[(BOARD_heap_number_of_blocks - 0x01)])
	){
		return TRUE;
	}
	return FALSE;
}


/**********************************************************************************************//**
 * @fn	void heap_init(void)
 *
 * @brief	the function initialize the heap memory
 *
 **************************************************************************************************/

void heap_init(void)
{
	__heap.mem_free_blocks = BOARD_heap_number_of_blocks;
	memset((uint8_t *)__heap.mem_allocation_markers, FREE_BLOCK_MARKER, BOARD_heap_number_of_blocks);
	semaphore_init((semaphore_t *)&__heap.mem_guard, 1, 0);
}


/**********************************************************************************************//**
 * @fn	uint16_t heap_get_size_of_free_memory(void)
 *
 * @brief	the function returns the size of free space on the heap
 *
 * @returns	uint16_t.
 **************************************************************************************************/

uint16_t heap_get_size_of_free_memory(void)
{
	return ((uint16_t)__heap.mem_free_blocks * (uint16_t)BOARD_heap_single_block_size);
}


/**********************************************************************************************//**
 * @fn	void * heap_malloc(uint16_t bytes_num)
 *
 * @brief	the function allocates the requested amount of space in the heap memory.
 *			the function returns the address of available memory or, if it is missing, 
 *			it returns null.
 *
 * @param		bytes_num   number of bytes.
 * @returns	void *  memory address or NULL.
 **************************************************************************************************/

void * heap_malloc(uint16_t bytes_num)
{
	uint8_t blocks_num, irq_flag, tot_free_block;
	
	if(bytes_num == 0)return NULL;
	
	blocks_num		= (bytes_num + BOARD_heap_single_block_size - 0x01)/BOARD_heap_single_block_size;
	irq_flag		= rtos_cli();
	tot_free_block	= __heap.mem_free_blocks;
	
	for(uint8_t free_block_cnt=0x00, block_index=0x00; 
		(block_index < BOARD_heap_number_of_blocks) && (tot_free_block >= blocks_num); 
		block_index++)
	{
		if(__heap.mem_allocation_markers[block_index] == FREE_BLOCK_MARKER){
			if(++free_block_cnt == blocks_num){
				uint8_t occupied_block_marker;
					
				block_index				-= (blocks_num - 1);
				occupied_block_marker	= block_index + 1;
				__heap.mem_free_blocks	-= blocks_num;
					
				memset((uint8_t *)&__heap.mem_allocation_markers[block_index], occupied_block_marker, blocks_num);
				rtos_sei(irq_flag);
				memset((uint8_t *)__heap.mem_space[block_index], 0x00, (BOARD_heap_single_block_size * blocks_num));
				return 	(void *)__heap.mem_space[block_index];
			}

		}else{
			tot_free_block -= free_block_cnt;
			free_block_cnt = 0x00;
		}
	}
	
	rtos_sei(irq_flag);
	rtos_error(0x01, __Err_DeviceSoftware_rtOS_DynamicMemory);

	return NULL;
}


/**********************************************************************************************//**
 * @fn	void * __heap_malloc(uint16_t bytes_num)
 *
 * @brief	the function allocates the requested amount of space in the heap memory.
 *			the function returns the address of available memory or, if it is missing, 
 *			it will add the task to the waiting queue.
 *
 * @param		bytes_num   	number of bytes.
 *
 * @returns	void *  memory address.
 **************************************************************************************************/

void * __heap_malloc(uint16_t bytes_num)
{
	void *ptr_mem = heap_malloc(bytes_num);
	
	if(ptr_mem == NULL){
		__semaphore_wait((semaphore_t *)&__heap.mem_guard);
	}
	return ptr_mem;
}


/**********************************************************************************************//**
 * @fn	void heap_free(void *memory_addr)
 *
 * @brief	the function frees space in the heap memory.
 *
 * @param		memory_addr   	memory address to free.
 **************************************************************************************************/

void heap_free(void *memory_addr)
{
	if(heap_check_if_dynamic_mem(memory_addr) == TRUE)
	{	
		uint8_t block_index, occupied_block_marker;
		uint8_t irq_flag;

		block_index				= (uint8_t)(((uint16_t)memory_addr - (uint16_t)__heap.mem_space[0x00]) / (uint16_t)BOARD_heap_single_block_size);
		occupied_block_marker	= block_index + 1;
		
		irq_flag = rtos_cli();
		
		for(; (block_index < BOARD_heap_number_of_blocks) && (__heap.mem_allocation_markers[block_index] == occupied_block_marker); block_index++)
		{
			__heap.mem_allocation_markers[block_index] = FREE_BLOCK_MARKER;
			__heap.mem_free_blocks++;
			__heap_wake_up_next_waiting_task();
		}
		rtos_sei(irq_flag);
	}
}