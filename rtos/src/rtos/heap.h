/*
 * heap.h
 *
 * Created: 23.05.2024 07:46:17
 *  Author: tom
 */ 


#ifndef HEAP_H_
#define HEAP_H_


void * __heap_malloc(uint16_t bytes_num);


/**********************************************************************************************//**
 * @fn	uint8_t heap_check_if_dynamic_mem(void *mem_addr)
 *
 * @brief	the function checks whether the given memory address comes from the heap address range
 *
 * @param 	mem_addr   	memory address.
 * @returns	TRUE - if in the heap range, FALSE - if outside the heap range.
 **************************************************************************************************/
uint8_t heap_check_if_dynamic_mem(void *mem_addr);


/**********************************************************************************************//**
 * @fn	void heap_init(void)
 *
 * @brief	the function initialize the heap memory
 *
 **************************************************************************************************/
void heap_init(void);


/**********************************************************************************************//**
 * @fn	uint16_t heap_get_size_of_free_memory(void)
 *
 * @brief	the function returns the size of free space on the heap
 *
 * @returns	uint16_t.
 **************************************************************************************************/
uint16_t heap_get_size_of_free_memory(void);


/**********************************************************************************************//**
 * @fn	void * condWait_heap_malloc(uint16_t bytes_num)
 *
 * @brief	the function allocates the requested amount of space in the heap memory.
 *			the function returns the address of available memory or, if it is missing,
 *			it adds the currently running task to the waiting queue.
 *
 * @param		bytes_num   number of bytes.
 * @returns	void *  memory address.
 **************************************************************************************************/
#define condWait_heap_malloc(byte_num)\
			task_update_pc_addr_before_call(__heap_malloc(byte_num))


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
void * heap_malloc(uint16_t bytes_num);

			
/**********************************************************************************************//**
 * @fn	void heap_free(void *memory_addr)
 *
 * @brief	the function frees space in the heap memory.
 *
 * @param		memory_addr   	memory address to free.
 **************************************************************************************************/
void heap_free(void *memory_addr);


#endif /* HEAP_H_ */