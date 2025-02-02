/*
 * task.h
 *
 * Created: 07.08.2024 08:09:14
 *  Author: tom
 */ 


#ifndef TASK_H_
#define TASK_H_

#include "board.h"


#if BOARD_heap_single_block_size < 32
	#error "BOARD_heap_single_block_size must be at least equal to 32"
#endif


/**********************************************************************************************//**
 * @enum	task_state_t
 *
 * @brief	states in which the task can be found
 **************************************************************************************************/

typedef enum{ 
	STOPPED			= 0x00, 	// stopped, all task parameters are set to default values
	READY			= 0x01,		// ready to be running
	RUNNING			= 0x02, 	// currently running
	SLEEP_INFINITE	= 0x04, 	// asleep forever, all task parameters are not set to default values, task is waiting for something
	SLEEP_TIMED		= 0x08, 	// asleep for a certain period of time
	JOIN			= 0x10,		// waiting for another task to finish
	WAIT_SEMA		= 0x20,		// waiting for semaphore or mutex
	INTERRUPT		= 0x40		// waiting for an interrupt to occur from some peripheral device

}task_state_t;


/**********************************************************************************************//**
 * @struct	task_handle
 *
 * @brief	a structure that stores information about a task
 *
 * @author	Tom
 * @date	02.01.2024
 **************************************************************************************************/

typedef struct task_handle{
	uint16_t 		PC;
	const uint16_t 	code_addr;
			
	union{
		volatile uint16_t		sleep_time;
		struct semaphore		*sleep_sema;
		uint8_t					sleep_irq_num;
	};
	   		
	task_state_t	state;

	struct{
		struct task_handle 	*parent_task;
		struct task_handle 	*child_task;
				
	}family;

	struct task_handle 		*next_task;
	struct task_handle 		*prev_task;

	struct semaphore 		*head_mutexes_list;

	void (*destructor_f)(struct task_handle *task);

}task_handle_t;


#define TASK_number_of_dynamic_variables	(BOARD_heap_single_block_size - sizeof(task_handle_t))
#define TASK_del_all_tasks					NULL

#ifndef RUN_TESTS
	#define TASK_my_task_t					__attribute__((OS_task, noinline)) void
#else
	#define TASK_my_task_t					__attribute__((noinline)) void
#endif


/**********************************************************************************************//**
 * use 'TASK_loop' and 'TASK_do' macros to loop the main body of the task
 * task should looks like this
 *
 *		TASK_my_task_t your_task_function_name(void)
 *		{
 *			//some initialization
 *			DDRB |= _BV(PB0);
 *
 *			TASK_do{	
 *				PORTB ^= _BV(PB0);
 *				condWait_task_delay(5000);
 *	
 *			}TASK_loop();
 *		}
 *
 **************************************************************************************************/
#define TASK_loop()\
			asm volatile(\
				"rjmp 1000b	\n\t"\
			::)

#define TASK_do\
			asm volatile(\
				"1000:		\n\t"\
			::);
		
		
/**********************************************************************************************//**
 * @struct	task_dynamic_handle_t
 *
 * @brief	a structure that stores information about a dynamic task
 *
 * @author	Tom
 * @date	02.01.2024
 **************************************************************************************************/

typedef struct{
	const task_handle_t __do_not_use_task_handle;

	uint8_t variables[TASK_number_of_dynamic_variables];

}task_dynamic_handle_t;



void __task_delay(uint16_t time_ms);
void __task_join(task_handle_t *child_task, uint8_t wait_2_join, uint16_t parent_pc);
void __task_infinite_sleep(uint8_t wake_up);
void * __task_new(void (*task_code_addr)(void), void (*destructor_call_addr)(task_handle_t *));
void __task_refresh_delayed(uint16_t time_ms);
void __task_refresh_interrupted(void);
void __task_wait_for_irq(uint8_t irq_nr);
void __task_set_program_counter(uint16_t pc);
void __task_switch(void);
void * _task_new(void *(*heap_malloc_f)(uint16_t), void (*task_code_addr)(void), void (*destructor_call_addr)(task_handle_t *));




/**********************************************************************************************//**
 * @fn	uint8_t *task_get_dynamic_variables_handle(void)
 *
 * @brief	if the task handle is in heap memory, the function will return the address
 *			of the task's dynamic variables otherwise null.
 *
 * @returns	uint8_t *  address of the deleted item or NULL.
 **************************************************************************************************/
uint8_t *task_get_dynamic_variables_handle(void);
void __task_get_dynamic_variables_handle(void);

#define TASK_tamplate_get_dynamic_variables_handle(type, ptr)\
	void __task_get_dynamic_variables_handle(void)\
	{\
		ptr = (volatile struct type *)task_get_dynamic_variables_handle();\
	}\

#define TASK_check_dynamic_variables(size) ((void)sizeof(char[1-2*(size > TASK_number_of_dynamic_variables)]))

#define TASK_init_dynamic_variables_pointer(struct_t, name)\
	volatile struct struct_t *volatile name;\
	TASK_tamplate_get_dynamic_variables_handle(struct_t, name)\
	TASK_check_dynamic_variables(sizeof(struct struct_t));\
	__task_get_dynamic_variables_handle()
	
	
/**********************************************************************************************//**
 * @fn	void task_update_pc_addr_before_call(f_call)
 *
 * @brief	the function saves the program counter for the currently running task 
 *			with address before the function f_call call.
 *
 * @param	f_call   	a complete function call with all required arguments.
 **************************************************************************************************/
#define task_update_pc_addr_before_call(f_call)({\
			__label__ _before_call;\
			__task_set_program_counter((uint16_t)&&_before_call);\
			_before_call:\
			__task_get_dynamic_variables_handle();\
			f_call;\
		})


/**********************************************************************************************//**
 * @fn	void task_update_pc_addr_after_call(f_call)
 *
 * @brief	the function saves the program counter for the currently running task 
 *			with address just after the function f_call return.
 *
 * @param	f_call   	a complete function call with all required arguments.
 **************************************************************************************************/
#define task_update_pc_addr_after_call(f_call)({\
			__label__ _after_call;\
			__task_set_program_counter((uint16_t)&&_after_call);\
			f_call;\
			_after_call:\
			__task_get_dynamic_variables_handle();\
		})
		
			
/**********************************************************************************************//**
 * @fn	void task_list_remove_by_item(task_handle_t **head, task_handle_t *item)
 *
 * @brief	function removes a specific item from a doubly linked list.
 *
 * @param		head   	pointer to pointer to the top of the lists.
 *				item	item to remove
 *
 **************************************************************************************************/
void task_list_remove_by_item(task_handle_t **head, task_handle_t *item);


/**********************************************************************************************//**
 * @fn	task_handle_t *task_rtos_list_pop_front(task_handle_t **head)
 *
 * @brief	function removes an item from the beginning of a doubly linked list.
 *
 * @param		head   	pointer to pointer to the top of the lists.
 *
 * @returns	task_handle_t *  address of the deleted item or NULL.
 **************************************************************************************************/
task_handle_t *task_list_pop_front(task_handle_t **head);


/**********************************************************************************************//**
 * @fn	task_handle_t *task_list_pop_back(task_handle_t **head)
 *
 * @brief	function removes an item from the end of a doubly linked list.
 *
 * @param		head   	pointer to pointer to the top of the lists.
 *
 * @returns	task_handle_t *  address of the deleted item or NULL.
 **************************************************************************************************/
task_handle_t *task_list_pop_back(task_handle_t **head);


/**********************************************************************************************//**
 * @fn	void task_rtos_list_push_front(task_handle_t **head, task_handle_t *new)
 *
 * @brief	the function adds a new item to the front of the doubly linked list.
 *
 * @param		head   		pointer to pointer to the top of the lists.
 * @param		new_task   	new item to add to the list.
 **************************************************************************************************/
void task_list_push_front(task_handle_t **head, task_handle_t *new_task);


/**********************************************************************************************//**
 * @fn	void task_rtos_list_push_back(task_handle_t **head, task_handle_t *new)
 *
 * @brief	the function adds a new item to the end of the doubly linked list.
 *
 * @param		head   	pointer to pointer to the top of the lists.
 * @param		new   	new item to add to the list.
 **************************************************************************************************/
void task_list_push_back(task_handle_t **head, task_handle_t *new);


/**********************************************************************************************//**
 * @fn	void task_init(void)
 *
 * @brief	the function initializes task group pointers in case any of them are in .noinit memory
 *	
 *
 **************************************************************************************************/
void task_init(void);


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
void _task_set_wait_for_semaphore(struct semaphore *sem, task_handle_t *task);
#define task_set_wait_for_semaphore(...)			VRG(_task_set_wait_for_semaphore, __VA_ARGS__)
#define _task_set_wait_for_semaphore1(sem)			_task_set_wait_for_semaphore(sem, NULL)
#define _task_set_wait_for_semaphore2(sem, task)	_task_set_wait_for_semaphore(sem, task)


/**********************************************************************************************//**
 * @fn	struct semaphore *task_get_wait_for_semaphore(task_handle_t *task=task_this())
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
struct semaphore *_task_get_wait_for_semaphore(task_handle_t *task);
#define task_get_wait_for_semaphore(...)		VRG(_task_get_wait_for_semaphore, __VA_ARGS__)
#define _task_get_wait_for_semaphore0()			_task_get_wait_for_semaphore(NULL)
#define _task_get_wait_for_semaphore1(task)		_task_get_wait_for_semaphore(task)

/**********************************************************************************************//**
 * @fn	void task_rtos_set_head_mutexes_list(struct semaphore *mutex, task_handle_t *task=task_this())
 *
 * @brief	the function sets a pointer to the first object of the mutex list. 
 *
 * @param		task   	pointer to the task, by default this function argument is null
 *						it means call this function for currently running task
 *				mutex	pointer to the mutex
 *
 **************************************************************************************************/
void _task_set_first_mutex_in_list(struct semaphore *mutex, task_handle_t *task);
#define task_set_first_mutex_in_list(...)				VRG(_task_set_first_mutex_in_list, __VA_ARGS__)
#define _task_set_first_mutex_in_list1(mutex)			_task_set_first_mutex_in_list(mutex, NULL)
#define _task_set_first_mutex_in_list2(mutex, task)		_task_set_first_mutex_in_list(mutex, task)


/**********************************************************************************************//**
 * @fn	struct semaphore *task_rtos_get_head_mutexes_list(task_handle_t *task=task_this())
 *
 * @brief	the function returns the address of the first object from the mutex list. 
 *
 * @param		task   	pointer to the task, by default this function argument is null
 *						it means call this function for currently running task
 *
 * @returns	semaphore_t *  address of the first item.
 **************************************************************************************************/
struct semaphore *_task_get_first_mutex_from_list(task_handle_t *task);
#define task_get_first_mutex_from_list(...)			VRG(_task_get_first_mutex_from_list, __VA_ARGS__)
#define _task_get_first_mutex_from_list0()			_task_get_first_mutex_from_list(NULL)
#define _task_get_first_mutex_from_list1(task)		_task_get_first_mutex_from_list(task)


/**********************************************************************************************//**
 * @fn	uint8_t task_get_number_of_running_tasks(void)
 *
 * @brief	the function returns the number of currently running tasks. 
 *
 * @returns	uint8_t  number of tasks .
 **************************************************************************************************/
uint8_t task_get_number_of_running_tasks(void);


/**********************************************************************************************//**
 * @fn	uint16_t task_get_function_address(task_handle_t *task=task_this())
 *
 * @brief	the function returns the address of the task function body. 
 *
 * @param	task   	pointer to the task, by default this function argument is null
 *					it means call this function for currently running task
 *
 * @returns	uint16_t  task function body address .
 **************************************************************************************************/
uint16_t _task_get_function_address(task_handle_t *task);
#define task_get_function_address(...)			VRG(_task_get_function_address, __VA_ARGS__)
#define _task_get_function_address0()			_task_get_function_address(NULL)
#define _task_get_function_address1(task)		_task_get_function_address(task)


/**********************************************************************************************//**
 * @fn	uint16_t task_get_program_counter(task_handle_t *task=task_this())
 *
 * @brief	the function returns the program counter for a given task. 
 *
 * @param	task   	pointer to the task, by default this function argument is null
 *					it means call this function for currently running task
 *
 * @returns	uint16_t  task program counter.
 **************************************************************************************************/
uint16_t _task_get_program_counter(task_handle_t *task);
#define task_get_program_counter(...)			VRG(_task_get_program_counter, __VA_ARGS__)
#define _task_get_program_counter0()			_task_get_program_counter(NULL)
#define _task_get_program_counter1(task)		_task_get_program_counter(task)


/**********************************************************************************************//**
 * @fn	task_state_t task_get_state(task_handle_t *task)
 *
 * @brief	the function returns the state for a given task. 
 *
 * @param	task   	pointer to the task
 *
 * @returns	task_state_t  task state
 **************************************************************************************************/
task_state_t task_get_state(task_handle_t *task);


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
void _task_setup(task_handle_t *task, void (*task_code_addr)(void), void (*destructor_call_addr)(task_handle_t *));
#define task_setup(...)												VRG(_task_setup, __VA_ARGS__)
#define _task_setup2(task, task_code_addr)							_task_setup(task, task_code_addr, NULL)
#define _task_setup3(task, task_code_addr, destructor_call_addr)	_task_setup(task, task_code_addr, destructor_call_addr)


/**********************************************************************************************//**
 * @fn	void * task_new(void (*task_code_addr)(void), void (*destructor_call_addr)(task_handle_t *) = NULL)
 *
 * @brief	the function will create a new task in dynamic memory
 *
 * @param	task_code_addr			task program address.
			destructor_call_addr	task destructor program address, by default this function argument is null
 *									it means no destructor function
 *
 * @returns	void *		task address or NULL if there is no free dynamic memory.
 **************************************************************************************************/
#define task_new(...)												VRG(_task_new, __VA_ARGS__)
#define _task_new1(task_code_addr)							_task_new(heap_malloc, task_code_addr, NULL)
#define _task_new2(task_code_addr, destructor_call_addr)	_task_new(heap_malloc, task_code_addr, destructor_call_addr)


/**********************************************************************************************//**
 * @fn	void * condWait_task_new(void (*task_code_addr)(void), void (*destructor_call_addr)(task_handle_t *))
 *
 * @brief	the function will create a new task in dynamic memory. If there is no available memory,
 *			it will add the current task to the waiting queue.
 *
 * @param	task_code_addr			task program address.
			destructor_call_addr	task destructor program address.
 *
 * @returns	void *		task address.
 **************************************************************************************************/
#define condWait_task_new(...)								task_update_pc_addr_before_call(VRG(__task_new, __VA_ARGS__))
#define __task_new1(task_code_addr)							_task_new(__heap_malloc, task_code_addr, NULL)
#define __task_new2(task_code_addr, destructor_call_addr)	_task_new(__heap_malloc, task_code_addr, destructor_call_addr)
		

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
task_handle_t * _task_freeze(task_state_t new_task_state, task_handle_t *task);
#define task_freeze(...)					VRG(_task_freeze, __VA_ARGS__)
#define _task_freeze1(new_task_state)		_task_freeze(new_task_state, NULL)
#define _task_freeze2(new_task_state, task)	_task_freeze(new_task_state, task)


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
void task_unfreeze(task_handle_t *wakeup_task);


/**********************************************************************************************//**
 * @fn	void condWait_task_wait_irq(rtos_irq_t irq_nr)
 *
 * @brief	This function will suspend the currently running task until the given irq_nr is reported.
 *
 * @param	irq_nr		irq number to check.
 *
 **************************************************************************************************/
#define condWait_task_wait_irq(irq_nr)\
			task_update_pc_addr_after_call(__task_wait_for_irq(irq_nr))
			

/**********************************************************************************************//**
 * @fn	uint8_t task_check_if_any_is_waiting_for_irq(void)
 *
 * @brief	the function will check if any task is waiting for ext interrupt.
 *
 * @returns	uint8_t 		TRUE - at least one task is waiting for ext irq.
 *							FALSE - there is no task waiting for ext irq.
 **************************************************************************************************/
uint8_t task_check_if_any_is_waiting_for_irq(void);

			
/**********************************************************************************************//**
 * @fn	void condWait_task_delay(uint16_t time_ms)
 *
 * @brief	The function puts the currently working task to sleep, if the sleep time is equal to 0,
 *			the task will be put to sleep forever.
 *
 * @param	time_ms		time in ms.
 **************************************************************************************************/
#define condWait_task_delay(time_ms)\
			task_update_pc_addr_after_call(__task_delay(time_ms))


/**********************************************************************************************//**
 * @fn	void condWait_task_join(task_handle_t *child_task)
 *
 * @brief	The function allows you to start a task 'task_to_join' and put the current one
 *			to sleep while waiting for the task 'task_to_join' to finish working.
 *			the current task will wait to be joined to child_task if for some reason it cannot be joined now
 *
 * @param	child_task		task to join/resume.
 **************************************************************************************************/

#define condWait_task_join(child_task)({\
			__label__ _after_join;\
			task_update_pc_addr_before_call(__task_join(child_task, TRUE, (uint16_t)&&_after_join));\
			_after_join:\
			__task_get_dynamic_variables_handle();\
		})


/**********************************************************************************************//**
 * @fn	void task_try_to_join(task_handle_t *child_task)
 *
 * @brief	The function allows you to start a task 'child_task' and put the current one
 *			to sleep while waiting for the 'child_task' to finish working
 *			the function will exit if for some reason the current task cannot now be joined to 'child_task'
 *
 * @param	child_task		task to join/resume.
 **************************************************************************************************/

#define condWait_task_try_to_join(child_task)({\
			__label__ _after_join;\
			__task_join(child_task, FALSE, (uint16_t)&&_after_join);\
			_after_join:\
			__task_get_dynamic_variables_handle();\
		})			

/**********************************************************************************************//**
 * @fn	void  task_start(task_handle_t *task)
 *
 * @brief	If the current state of the task is 'SLEEP_INFINITE' or 'STOPPED' or 'SLEEP_TIMED',
 *			This function adds the task to the list of currently working tasks and sets a state to READY.
 *			If a task is woken up after a temporary sleep period,
 *			it will be placed at the top of the queue of tasks to run,
 *			otherwise it will be placed at the end of the queue.
 *
 * @param	task	task to start.
 **************************************************************************************************/
void  task_start(task_handle_t *task);


/**********************************************************************************************//**
 * @fn	void condWait_task_infinite_sleep()
 *
 * @brief	The function will stop the currently running task. The task will be permanently put to sleep,
 *			the woken up task will start executing the program from the first command after
 *			calling the condWait_task_infinite_sleep function.
 *			The parent task will be not woken up. The relationship between the parent and child tasks will be preserved.
 *
 **************************************************************************************************/
#define condWait_task_infinite_sleep()\
			task_update_pc_addr_after_call(__task_infinite_sleep(FALSE))


/**********************************************************************************************//**
 * @fn	void condWait_task_infinite_sleep_wup()
 *
 * @brief	The function will stop the currently running task. The task will be permanently put to sleep,
 *			the woken up task will start executing the program from the first command after
 *			calling the condWait_task_infinite_sleep function.
 *			The parent task will be woken up. The relationship between the parent and child tasks will be preserved.
 *
 **************************************************************************************************/
#define condWait_task_infinite_sleep_wup()\
			task_update_pc_addr_after_call(__task_infinite_sleep(TRUE))
			

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
void _task_erase(uint8_t if_permanent, task_handle_t *task);
#define task_erase(...)						VRG(_task_erase, __VA_ARGS__)
#define _task_erase1(if_permanent)			_task_erase(if_permanent, NULL)
#define _task_erase2(if_permanent, task)	_task_erase(if_permanent, task)


/**********************************************************************************************//**
 * @fn	void task_delete(task_handle_t *task=task_this())
 *
 * @brief	Task will be deleted.
 *			The task parent will be woken up, the task children will be deleted,
 *			all mutexes will be unlocked, all counters will be stopped, the task destructor will be called.
 *			The task will be removed from the parent family tree
 *			and the dynamic memory occupied by the task will be freed.
 *
 * @param	task			task to delete, by default this function argument is null
 *							it means call this function for currently running task
 **************************************************************************************************/
#define task_delete(...)	VRG(_task_delete, __VA_ARGS__)
#define _task_delete0()		_task_erase(TRUE, NULL)
#define _task_delete1(task)	_task_erase(TRUE, task)


/**********************************************************************************************//**
 * @fn	void task_stop(task_handle_t *task=task_this())
 *
 * @brief	Function will reset all task parameters to default settings.
 *			The task parent will be woken up, the task children will be deleted,
 *			all mutexes will be unlocked, all counters will be stopped, the task destructor will be called.
 *			The task will be removed from the parent family tree.
 *			The dynamic memory occupied by the task will not be freed.
 *
 * @param	task			task to reset.
 **************************************************************************************************/
#define task_stop(...)	VRG(_task_stop, __VA_ARGS__)
#define _task_stop0()		_task_erase(FALSE, NULL)
#define _task_stop1(task)	_task_erase(FALSE, task)


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
uint8_t task_check_relationship(task_handle_t *task_parent, task_handle_t *task_child);


/**********************************************************************************************//**
 * @fn	task_rtos_handle_t *task_this(void)
 *
 * @brief	use this function to get a handle to the currently running task
 *
 * @returns	task_rtos_handle_t	currently running task 
 *
 **************************************************************************************************/
task_handle_t *task_this(void);


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
task_dynamic_handle_t *task_dynamic_this(void);




#endif /* TASK_H_ */


