/*
 * task.c
 *
 * Created: 07.08.2024 05:11:19
 *  Author: tom
 */ 

#include <avr/io.h>
#include <string.h>
#include "rtos.h"


RTOS_static	volatile 	task_handle_t *volatile	__task_sleeping_g;
RTOS_static	volatile 	task_handle_t *volatile	__task_interrupted_g;
RTOS_static volatile	task_handle_t *volatile	__task_ready_g __attribute__((section(".noinit")));


/**********************************************************************************************//**
 * @fn	uint8_t *task_get_dynamic_variables_handle(void)
 *
 * @brief	if the task handle is in heap memory, the function will return the address
 *			of the task's dynamic variables otherwise null.
 *
 * @returns	uint8_t *  address of the deleted item or NULL.
 **************************************************************************************************/

__attribute__ ((noinline)) uint8_t *task_get_dynamic_variables_handle(void)
{
	task_dynamic_handle_t *task = (task_dynamic_handle_t *)__task_ready_g;
	
	return heap_check_if_dynamic_mem(task) ? task->variables : NULL;
}
void __task_get_dynamic_variables_handle(void)
{}


/**********************************************************************************************//**
 * @fn	void task_list_push_front(task_handle_t **head, task_handle_t *new)
 *
 * @brief	the function adds a new item to the front of the doubly linked list.
 *
 * @param		head   		pointer to pointer to the top of the lists.
 * @param		new_task   	new item to add to the list.
 **************************************************************************************************/

void task_list_push_front(task_handle_t **head, task_handle_t *new_task)
{
	if( (new_task == NULL) || (head == NULL))return;
	
	new_task->prev_task	= NULL;
	new_task->next_task	= *head;
	if(*head != NULL)
		(*head)->prev_task = new_task;
	*head = new_task;
}


/**********************************************************************************************//**
 * @fn	void task_list_push_back(task_handle_t **head, task_handle_t *new_task)
 *
 * @brief	the function adds a new item to the end of the doubly linked list.
 *
 * @param		head   		pointer to pointer to the top of the lists.
 * @param		new_task   	new item to add to the list.
 **************************************************************************************************/

void task_list_push_back(task_handle_t **head, task_handle_t *new_task)
{
	task_handle_t *prev = NULL;
	
	if( (head == NULL) || (new_task == NULL) )return;

	while(*head != NULL){
		prev = *head;
		head = &((*head)->next_task);
	}
	*head 				= new_task;	
	new_task->prev_task	= prev;
	new_task->next_task	= NULL;
}


/**********************************************************************************************//**
 * @fn	task_handle_t *task_list_pop_front(task_handle_t **head)
 *
 * @brief	function removes an item from the beginning of a doubly linked list.
 *
 * @param		head   	pointer to pointer to the top of the lists.
 *
 * @returns	task_handle_t *  address of the deleted item or NULL.
 **************************************************************************************************/

task_handle_t *task_list_pop_front(task_handle_t **head)
{
	task_handle_t *task;
	
	if( (head == NULL) || (*head == NULL) )return NULL;
	
	task = *head;
	*head = (*head)->next_task;
	if(*head != NULL)
		(*head)->prev_task = NULL;
	task->next_task = NULL;
	
	return task;
}


/**********************************************************************************************//**
 * @fn	task_handle_t *task_list_pop_back(task_handle_t **head)
 *
 * @brief	function removes an item from the end of a doubly linked list.
 *
 * @param		head   	pointer to pointer to the top of the lists.
 *
 * @returns	task_handle_t *  address of the deleted item or NULL.
 **************************************************************************************************/

task_handle_t *task_list_pop_back(task_handle_t **head)
{
	task_handle_t *task;
	
	if(head == NULL)return NULL;
	
	while((*head)->next_task != NULL){
		head = &((*head)->next_task);
	}
	task			= *head;
	*head 			= NULL;
	task->prev_task	= NULL;
	
	return task;
}


/**********************************************************************************************//**
 * @fn	void task_list_remove_by_item(task_handle_t **head, task_handle_t *item)
 *
 * @brief	function removes a specific item from a doubly linked list.
 *
 * @param		head   	pointer to pointer to the top of the lists.
 *				item	item to remove
 *
 **************************************************************************************************/

void task_list_remove_by_item(task_handle_t **head, task_handle_t *item)
{	
	if( (head == NULL) || (item == NULL) )return;
	
	while(*head != item){
		head = &((*head)->next_task);
		if(*head == NULL)return;
	}
	
	*head = item->next_task;
	if(item->next_task != NULL)
		(item->next_task)->prev_task = item->prev_task;
	item->next_task = NULL;
	item->prev_task = NULL;
}


/**********************************************************************************************//**
 * @fn	void task_init(void)
 *
 * @brief	the function initializes task group pointers in case any of them are in .noinit memory
 *	
 *
 **************************************************************************************************/

void task_init(void)
{
	__task_interrupted_g = NULL;
	__task_ready_g = NULL;
	__task_sleeping_g = NULL;
}


/**********************************************************************************************//**
 * @fn	void task_set_wait_for_semaphore(semaphore_t *sem, task_handle_t *task=task_this())
 *
 * @brief	the function sets the address of the semaphore that the task is waiting for.
 *			in case of reset or deletion of a task, the system must remove the task from
 *			the list of waiting tasks of a given semaphore.
 *
 * @param		task   	pointer to the task, by default this function argument is null
 *						it means call this function for currently running task
 *				sem		pointer to the semaphore
 *
 **************************************************************************************************/

void _task_set_wait_for_semaphore(struct semaphore *sem, task_handle_t *task)
{
	if(task == NULL){
		if(__task_ready_g == NULL)return;
		task = (task_handle_t *)__task_ready_g;
	}
	task->sleep_sema = sem;
}


/**********************************************************************************************//**
 * @fn	struct semaphore *_task_get_wait_for_semaphore(task_handle_t *task=task_this())
 *
 * @brief	the function returns the address of the semaphore that the task is waiting for.
 *			in case of reset or deletion of a task, the system must remove the task from
 *			the list of waiting tasks of a given semaphore.
 *
 * @param		task   	pointer to the task, by default this function argument is null
 *						it means call this function for currently running task
 *
 * @returns		semaphore_t *  address of the semaphore.
 **************************************************************************************************/

struct semaphore *_task_get_wait_for_semaphore(task_handle_t *task)
{	
	if(task == NULL){
		if(__task_ready_g == NULL)return NULL;
		task = (task_handle_t *)__task_ready_g;
	}
	return task->sleep_sema;
}

/**********************************************************************************************//**
 * @fn	void _task_set_first_mutex_in_list(struct semaphore *mutex, task_handle_t *task=task_this())
 *
 * @brief	the function sets a pointer to the first object of the mutex list. 
 *
 * @param		task   	pointer to the task, by default this function argument is null
 *						it means call this function for currently running task
 *				mutex	pointer to the mutex
 *
 **************************************************************************************************/

void _task_set_first_mutex_in_list(struct semaphore *mutex, task_handle_t *task)
{
	if(task == NULL){
		if(__task_ready_g == NULL)return;
		task = (task_handle_t *)__task_ready_g;
	}

	task->head_mutexes_list	= (semaphore_t *)mutex;
}


/**********************************************************************************************//**
 * @fn	struct semaphore *_task_get_first_mutex_from_list(task_handle_t *task=task_this())
 *
 * @brief	the function returns the address of the first object from the mutex list. 
 *
 * @param		task   	pointer to the task, by default this function argument is null
 *						it means call this function for currently running task
 *
 * @returns	semaphore_t *  address of the first item.
 **************************************************************************************************/

struct semaphore *_task_get_first_mutex_from_list(task_handle_t *task)
{
	if(task == NULL){
		if(__task_ready_g == NULL)return NULL;
		task = (task_handle_t *)__task_ready_g;
	}

	return task->head_mutexes_list;
}


/**********************************************************************************************//**
 * @fn	uint8_t task_get_number_of_running_tasks(void)
 *
 * @brief	the function returns the number of currently running tasks. 
 *
 * @returns	uint8_t  number of tasks .
 **************************************************************************************************/

uint8_t task_get_number_of_running_tasks(void)
{
	uint8_t cnt;
	
	if(__task_ready_g == NULL)return 0;
	
	cnt = 1;
	
	for(task_handle_t *task = (task_handle_t *)__task_ready_g; task->next_task != __task_ready_g; cnt++, task = task->next_task);

	return cnt;
}


/**********************************************************************************************//**
 * @fn	uint16_t _task_get_function_address(task_handle_t *task=task_this())
 *
 * @brief	the function returns the address of the task function body. 
 *
 * @param	task   	pointer to the task, by default this function argument is null
 *					it means call this function for currently running task
 *
 * @returns	uint16_t  task function body address .
 **************************************************************************************************/

uint16_t _task_get_function_address(task_handle_t *task)
{	
	if(task == NULL)task = (task_handle_t *)__task_ready_g;

	return task == NULL ? 0 : task->code_addr;
}


/**********************************************************************************************//**
 * @fn	uint16_t _task_get_program_counter(task_handle_t *task=task_this())
 *
 * @brief	the function returns the program counter for a given task. 
 *
 * @param	task   	pointer to the task, by default this function argument is null
 *					it means call this function for currently running task
 *
 * @returns	uint16_t  task program counter.
 **************************************************************************************************/

uint16_t _task_get_program_counter(task_handle_t *task)
{	
	if(task == NULL)task = (task_handle_t *)__task_ready_g;
	
	return task == NULL ? 0 : task->PC;
}


/**********************************************************************************************//**
 * @fn	void __task_set_program_counter(uint16_t pc)
 *
 * @brief	the function update the program counter for a current running task. 
 *
 * @param	pc   program counter value.
 *
 **************************************************************************************************/

__attribute__ ((noinline)) void __task_set_program_counter(uint16_t pc)
{	
	if(__task_ready_g)__task_ready_g->PC = pc;
}


/**********************************************************************************************//**
 * @fn	task_state_t task_get_state(task_handle_t *task)
 *
 * @brief	the function returns the state for a given task. 
 *
 * @param	task   	pointer to the task
 *
 * @returns	task_state_t  task state
 **************************************************************************************************/

task_state_t task_get_state(task_handle_t *task)
{		
	return task == NULL ? STOPPED : task->state;
}


/**********************************************************************************************//**
 * @fn	task_rtos_handle_t *task_this(void)
 *
 * @brief	use this function to get a handle to the currently running task
 *
 * @returns	task_rtos_handle_t	currently running task 
 *
 **************************************************************************************************/

__attribute__ ((noinline)) task_handle_t *task_this(void)
{
	return (task_handle_t *)__task_ready_g;
}


/**********************************************************************************************//**
 * @fn	void __task_switch(void)
 *
 * @brief	this function allows you to switch the currently running task to the next task on the list
 *
 **************************************************************************************************/

void __task_switch(void)
{
	if(__task_ready_g){
		__task_ready_g->state				= READY;
		__task_ready_g						= __task_ready_g->next_task;
		__task_ready_g->state				= RUNNING;
	}
}


/**********************************************************************************************//**
 * @fn	task_rtos_dynamic_handle_t *task_dynamic_this(void)
 *
 * @brief	Use this function to get a handle to the currently running task.
 *			Only for tasks created in dynamic memory.
 *			These tasks are matched to the dynamic memory block size so they have some free memory space
 *			that can be used by tasks.
 *
 * @returns	task_rtos_handle_t	currently running task 
 *
 **************************************************************************************************/

__attribute__ ((noinline)) task_dynamic_handle_t *task_dynamic_this(void)
{
	return (task_dynamic_handle_t *)__task_ready_g;
}


/**********************************************************************************************//**
 * @fn	void task_setup(task_rtos_handle_t *task, void (*task_code_addr)(void), void (*destructor_call_addr)(task_rtos_handle_t *)=NULL)
 *
 * @brief	the function will initialize a new task
 *
 * @param	task					task address
 *			task_code_addr			task program address.
 *			destructor_call_addr	task destructor program address, by default this function argument is null
 *									it means no destructor function
 **************************************************************************************************/

__attribute__ ((noinline)) void _task_setup(task_handle_t *task, void (*task_code_addr)(void), void (*destructor_call_addr)(task_handle_t *))
{
	if( (task == NULL) || (task_code_addr == NULL) )return;
	
	((uint16_t *)task)[1]	= (uint16_t)task_code_addr;
	task->PC 				= (uint16_t)task_code_addr;
	task->state				= STOPPED;
	task->destructor_f		= destructor_call_addr;
}


/**********************************************************************************************//**
 * @fn	void * _task_new(void *(*heap_malloc_f)(uint16_t), void (*task_code_addr)(void), void (*destructor_call_addr)(task_handle_t *) = NULL)
 *
 * @brief	the function will create a new task in dynamic memory. For task call if there is no available memory,
 *			it will add the current task to the waiting queue.
 *
 * @param	heap_malloc_f			malloc function pointer different for task and function call
 *			task_code_addr			task program address.
			destructor_call_addr	task destructor program address, by default this function argument is null
 *									it means no destructor function
 *
 * @returns	void *		task call - task address
 *						function call - task address or NULL if there is no free dynamic memory.
 **************************************************************************************************/

__attribute__ ((noinline)) void * _task_new(void *(*heap_malloc_f)(uint16_t), void (*task_code_addr)(void), void (*destructor_call_addr)(task_handle_t *))
{
	if(task_code_addr == NULL)return NULL;
	
	task_handle_t *NewT = heap_malloc_f(sizeof(task_handle_t));

	if(NewT){
		task_setup(NewT, task_code_addr, destructor_call_addr);
	}	
	return NewT;
}


/**********************************************************************************************//**
 * @fn	task_handle_t * _task_freeze(task_state_t new_task_state, task_handle_t *task=task_this())
 *
 * @brief	This function removes a task from the list of currently working tasks and sets a new state.
 *			After that the task will not be connected to any list
 *
 * @param	task				task to freeze, by default this function argument is null
 *								it means call this function for currently running task
 *			new_task_state		new state for the task.
 *
 * @returns	task_handle_t * 	freezed task address.
 **************************************************************************************************/

__attribute__ ((noinline)) task_handle_t *_task_freeze(task_state_t new_task_state, task_handle_t *task)
{
	if(task == NULL){					//null means call this function for currently running task
		if(__task_ready_g == NULL)	//no currently running task err
			return NULL;
		task = (task_handle_t *)__task_ready_g;
	}
	if(task != __task_ready_g){
		for(task_handle_t *list_task = (__task_ready_g ? __task_ready_g->next_task : NULL); list_task != task; list_task = list_task->next_task){
			if(list_task == __task_ready_g){										//there is no such task in the currently running task list
				if( (task->next_task == NULL) && (task->prev_task == NULL) ){		//the state can only be changed if the task is not attached to any task list
					task->state = new_task_state == (new_task_state == RUNNING) || (new_task_state == READY) ? SLEEP_INFINITE : new_task_state;
				}
				return task;
			}
		}
 
	}else{
		__task_ready_g = (task->prev_task == task) ? NULL : task->prev_task;	//if prev_task/next_task is equal to task it means that this is the only running task
	}
	
	(task->prev_task)->next_task	= task->next_task;
	(task->next_task)->prev_task	= task->prev_task;
	task->next_task					= NULL;
	task->prev_task					= NULL;
	task->state    					= (new_task_state == RUNNING) || (new_task_state == READY) ? SLEEP_INFINITE : new_task_state;
	return task;
}


/**********************************************************************************************//**
 * @fn	void task_unfreeze(task_handle_t *wakeup_task)
 *
 * @brief	This function adds a task to the list of currently working tasks and sets a state to READY.
 *			If a task wakes up after temporarily sleeping or waiting for an interrupt, 
 *			it will be placed at the top of the task queue.
 *			otherwise it will be placed at the end of the queue.
 *
 * @param	wakeup_task		task to wakeup.
 **************************************************************************************************/

__attribute__ ((noinline))void task_unfreeze(task_handle_t *wakeup_task)
{
	task_handle_t *task;
	
	if( (wakeup_task == NULL) ||
		(wakeup_task->code_addr == 0) ||
		(wakeup_task->PC == 0) )
	{
		return;		
	}
	
	if(wakeup_task->state == SLEEP_TIMED){
		wakeup_task->sleep_time = 0;
		task_list_remove_by_item((task_handle_t **)&__task_sleeping_g, wakeup_task);
		task = (task_handle_t *)__task_ready_g;
	
	}else if(wakeup_task->state == INTERRUPT){
		wakeup_task->sleep_irq_num = 0;
		task_list_remove_by_item((task_handle_t **)&__task_interrupted_g, wakeup_task);
		task = (task_handle_t *)__task_ready_g;
		
	}else{
		task = (task_handle_t *)__task_ready_g->prev_task;
	}

	if(__task_ready_g == NULL){	//if there is only one task, it must point to itself as prev_task/next_task
		wakeup_task->prev_task	= wakeup_task;
		wakeup_task->next_task	= wakeup_task;
		__task_ready_g		= wakeup_task;
	
	}else{
		wakeup_task->prev_task			= task;
		wakeup_task->next_task			= task->next_task;
		(task->next_task)->prev_task	= wakeup_task;
		task->next_task					= wakeup_task;
	}
	wakeup_task->state = READY;
}


/**********************************************************************************************//**
 * @fn void __task_wait_for_irq(rtos_peripheral_irq_t irq_nr)
 *
 * @brief	This function will suspend the currently running task until the given irq_nr is reported.
 *
 * @param	irq_nr		irq number to check.
 **************************************************************************************************/

__attribute__ ((noinline)) void __task_wait_for_irq(rtos_peripheral_irq_t irq_nr)
{
	task_handle_t *task = (task_handle_t *)__task_ready_g;
	
	if(task != NULL){	
		task->sleep_irq_num = irq_nr;
		task_freeze(INTERRUPT, task);
		task_list_push_front((task_handle_t **)&__task_interrupted_g, task);
	}
	rtos_back_jump();
}


/**********************************************************************************************//**
 * @fn	void __task_refresh_interrupted(void)
 *
 * @brief	Used by the system to refresh the list of tasks waiting for an interrupt request.
 *
 **************************************************************************************************/

void __task_refresh_interrupted(void)
{
	task_handle_t *irq_wait_task = (task_handle_t *)__task_interrupted_g;
	
	while(irq_wait_task != NULL)
	{
		task_handle_t *next_irq_wait_task = irq_wait_task->next_task;

		if(rtos_irq_get(irq_wait_task->sleep_irq_num)){
			task_unfreeze(irq_wait_task);	
		}
		irq_wait_task = next_irq_wait_task;
	}
}


/**********************************************************************************************//**
 * @fn	uint8_t task_check_if_any_is_waiting_for_irq(void)
 *
 * @brief	the function will check if any task is waiting for ext interrupt.
 *
 * @returns	uint8_t 		TRUE - at least one task is waiting for ext irq.
 *							FALSE - there is no task waiting for ext irq.
 **************************************************************************************************/

uint8_t task_check_if_any_is_waiting_for_irq(void)
{
	return __task_interrupted_g != NULL ? TRUE : FALSE;
}


/**********************************************************************************************//**
 * @fn	void __task_delay(uint16_t time_ms)
 *
 * @brief	The function puts the currently working task to sleep, if the sleep time is equal to 0,
 *			the task will be put to sleep forever.
 *
 * @param	time_ms		time in ms.
 **************************************************************************************************/

__attribute__ ((noinline)) void __task_delay(uint16_t time_ms)
{
	task_handle_t *task = (task_handle_t *)__task_ready_g;
	
	if(task != NULL){
		if(time_ms){
			uint8_t irq_flag = rtos_cli();
			uint16_t current_time = __timer_get_time_ms();
			rtos_sei(irq_flag);
			uint32_t sleep = (uint32_t)__timer_ms_to_ticks_16bits(time_ms) + (uint32_t)current_time;
			
			task->sleep_time = (sleep > 0xFFFF) ? 0xFFFF : (uint16_t)sleep;
			task_freeze(SLEEP_TIMED, task);
			task_list_push_front((task_handle_t **)&__task_sleeping_g, task);
			
		}else{
			task_freeze(SLEEP_INFINITE, task);
		}
	}
	rtos_back_jump();
}


/**********************************************************************************************//**
 * @fn	void __task_refresh_delayed(uint16_t time_ms)
 *
 * @brief	Used by the system to refresh the time in all tasks
 *
 * @param	time	value to subtract from all tasks counters.
 **************************************************************************************************/

__attribute__((noinline))void __task_refresh_delayed(uint16_t time_ms)
{
	task_handle_t *asleep_task = (task_handle_t *)__task_sleeping_g;
	
	while(asleep_task != NULL)
	{
		task_handle_t *next_asleep_task = asleep_task->next_task;

		if(asleep_task->sleep_time <= time_ms){
			task_unfreeze(asleep_task);
			
		}else{
			asleep_task->sleep_time -= time_ms;
		}
		asleep_task = next_asleep_task;
	}
}


/**********************************************************************************************//**
 * @fn	void __task_join(task_handle_t *child_task, uint8_t wait_2_join, uint16_t parent_pc)
 *
 * @brief	The function allows you to start a task 'child_task' and put the currently working
 *			task (parent_task) to sleep while waiting for the task 'child_task' to finish working
 *
 * @param	child_task		child task to join.
			wait_2_join		TRUE - the 'parent_task' will wait to be joined to child_task if for some reason it cannot be joined now
							FALSE - the function will exit if for some reason the 'parent_task' cannot now be joined to 'child_task'
			parent_pc		program counter of the parent task after resuming
 **************************************************************************************************/

__attribute__ ((noinline)) void __task_join(task_handle_t *child_task, uint8_t wait_2_join, uint16_t parent_pc)
{
	task_handle_t *parent_task = (task_handle_t *)__task_ready_g;
	
	if( (child_task != NULL) && 
		(child_task != parent_task) &&
		(parent_task != NULL) )
	{
		if( (child_task->family.parent_task == NULL) ||
			(child_task->family.parent_task == parent_task) )
		{
			child_task->family.parent_task	= parent_task;
			parent_task->family.child_task	= child_task;
			
			if( (child_task->state == STOPPED) ||
				(child_task->state == SLEEP_INFINITE) ||
				(child_task->state == SLEEP_TIMED) )
			{
				task_unfreeze(child_task);
			}
			__task_set_program_counter(parent_pc);
			task_freeze(JOIN, parent_task);
	
		}else if(wait_2_join){
			__task_delay(1);
			
		}else{
			return;
		}
	}
	rtos_back_jump();
}


/**********************************************************************************************//**
 * @fn	void  task_start(task_handle_t *task)
 *
 * @brief	If the current state of the task is 'SLEEP_INFINITE' or 'STOPPED' or 'SLEEP_TIMED',
 *			This function adds the task to the list of currently working tasks and sets a state to RUNNING.
 *			If a task is woken up after a temporary sleep period,
 *			it will be placed at the top of the queue of tasks to run,
 *			otherwise it will be placed at the end of the queue.
 *
 * @param	task	task to start.
 **************************************************************************************************/

__attribute__ ((noinline)) void  task_start(task_handle_t *task)
{
	if( (task != NULL) && ((task->state == SLEEP_INFINITE) || (task->state == STOPPED) || (task->state == SLEEP_TIMED)) ){
		task_unfreeze(task);
	}
}


/**********************************************************************************************//**
 * @fn	void __task_infinite_sleep(uint8_t wake_up)
 *
 * @brief	The function will put the currently running task to infinite sleep,
 *			the woken up task will start executing the program from the first command after
 *			calling the __task_infinite_sleep function. If the 'wake_up' parameter is 'TRUE'
 *			and if the currently running task has a parent that is waiting for the child to finish,
 *			the parent will be woken up.
 *
 * @param	wake_up		TRUE - parent will be woken up.
 *						FALSE - parent will not be woken up.
 **************************************************************************************************/

__attribute__ ((noinline)) void __task_infinite_sleep(uint8_t wake_up)
{
	task_handle_t *task = (task_handle_t *)__task_ready_g;
	
	if(task){
		if( (wake_up)											&& 
			(task->family.parent_task != NULL) 					&&
			((task->family.parent_task)->state != RUNNING) 		&&
			((task->family.parent_task)->state != READY) 		&&
			((task->family.parent_task)->state != WAIT_SEMA)	&&
			((task->family.parent_task)->state != INTERRUPT) )
		{
			task_unfreeze(task->family.parent_task);
		}
		task_freeze(SLEEP_INFINITE, task);
	}
	rtos_back_jump();
}


/**********************************************************************************************//**
 * @fn	void task_erase(uint8_t if_permanent, task_handle_t *task=task_this())
 *
 * @brief	Function will reset all task parameters to default settings.
 *			The task parent will be woken up, the task children will be deleted,
 *			all mutexes will be unlocked, all counters will be stopped, the task destructor will be called.
 *			If if_permanent is TRUE, the dynamic memory occupied by the task will be freed.
 *
 * @param	if_permanent	TRUE - the dynamic memory occupied by the task will be freed
 *							FALSE - the dynamic memory occupied by the task will not be freed
 *			task			task to erase, by default this function argument is null
 *							it means call this function for currently running task
 *
 **************************************************************************************************/

__attribute__ ((noinline)) void _task_erase(uint8_t if_permanent, task_handle_t *task)
{
	uint8_t this_is_currently_running = FALSE;
	
	if(task == NULL){
		if(__task_ready_g == NULL)
			return;
		task = (task_handle_t *)__task_ready_g;
	}
	
	if(task->family.parent_task != NULL)
	{
		task_handle_t *parent_task = task->family.parent_task;
		
		if(parent_task->family.child_task == task){
			parent_task->family.child_task	= NULL;
			task->family.parent_task		= NULL;
			
			if( (parent_task->state == STOPPED) || 
				(parent_task->state == SLEEP_INFINITE) || 
				(parent_task->state == JOIN) ||
				(parent_task->state == SLEEP_TIMED) )
			{
				task_unfreeze(parent_task);
			}
			
		}else{
			rtos_error(0x01, __Err_DeviceSoftware_rtOS_ParentReset);								//err the parent does not have such a child
		}
	}

	while(task->family.child_task != NULL){
		task_erase(TRUE, task->family.child_task);
	}
		
	task->PC = task->code_addr;
		
	if(task == __task_ready_g){
		this_is_currently_running = TRUE;
		task_freeze(STOPPED, task);
		
	}else if(task->state == SLEEP_TIMED){
		task->sleep_time = 0;
		task_list_remove_by_item((task_handle_t **)&__task_sleeping_g, task);
		
	}else if(task->state == INTERRUPT){
		task->sleep_irq_num = 0;
		task_list_remove_by_item((task_handle_t **)&__task_interrupted_g, task);
	
	}else if(task->state == WAIT_SEMA){
		semaphore_remove_from_pending_list(task, task->sleep_sema);
		
	}else if(task->state == READY){
		if(task->prev_task){
			(task->prev_task)->next_task = task->next_task;
		}
		if(task->next_task){
			(task->next_task)->prev_task = task->prev_task;
		}
	}
	task->state      = STOPPED;
	task->next_task  = NULL;
	task->prev_task  = NULL;

	while(task->head_mutexes_list != NULL){
		_mutex_unlock(task->head_mutexes_list, task);
	}
	__timer_stop_all_for_given_task(task);
		
	if(task->destructor_f != NULL){
		task->destructor_f(task);
	}
	if(if_permanent){
		heap_free(task);
	}
	if(this_is_currently_running){
		rtos_back_jump();
	}
}


/**********************************************************************************************//**
 * @fn	uint8_t task_check_relationship(task_handle_t *task_parent, task_rtos_handle_t *task_child)
 *
 * @brief	the function will check if the given task_child is a descendant of the task_parent.
 *
 * @param	task_parent		parent task
 *			task_child		child task to check.
 *
 * @returns	uint8_t 		TRUE - task_child is a offspring of the currently running task.
 *							FALSE - task_child is not a offspring of the currently running task.
 **************************************************************************************************/

__attribute__ ((noinline)) uint8_t task_check_relationship(task_handle_t *task_parent, task_handle_t *task_child)
{
	if( (task_child == NULL) ||
		(task_parent == NULL) ||
		(task_parent->family.child_task != task_child) ||
		(task_child->family.parent_task != task_parent) )
	{
		return FALSE;	
	}
	return TRUE;
}





