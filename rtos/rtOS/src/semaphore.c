/*
 * semaphore.c
 *
 * Created: 24.05.2024 09:22:48
 *  Author: tom
 */ 
#include <avr/io.h>
#include "rtos.h"
#include "semaphore.h"

#define THIS_IS_MUTEX	0


static void _init(semaphore_t volatile *sem, uint8_t max_count, uint8_t init_count)
{
	sem->max_count					= max_count;
	sem->count						= init_count;
	sem->head_pending_tasks_list	= NULL;
	sem->next						= NULL;
}


/**********************************************************************************************//**
 * @fn	void _semaphore_init(semaphore_t *sem, uint8_t max_count, uint8_t init_count)
 *
 * @brief	Semaphore initialization function.
 *
 * @param		sem			pointer to the semaphore.
 * @param		max_count	The maximum count value that can be reached.
 * @param		init_count	The initial count value assigned to the semaphore .
  **************************************************************************************************/

void _semaphore_init(semaphore_t *sem, uint8_t max_count, uint8_t init_count)
{
	if ( (sem != NULL) && (max_count > 0) )
		_init(sem, max_count, init_count);
}


/**********************************************************************************************//**
 * @fn	uint8_t semaphore_get_count(semaphore_t *sem)
 *
 * @brief	the function returns the current count value.
 *
 * @param		sem		pointer to the semaphore.
 * 
 * @returns		uint8_t  current count value.
  **************************************************************************************************/

uint8_t semaphore_get_count(semaphore_t *sem)
{
	if(sem == NULL)return 0;
	
	return sem->count;
}


/**********************************************************************************************//**
 * @fn	uint8_t semaphore_get_count(semaphore_t *sem)
 *
 * @brief	the function returns the max count value.
 *
 * @param		sem		pointer to the semaphore.
 * 
 * @returns		uint8_t  max count value.
  **************************************************************************************************/

uint8_t semaphore_get_max_count(semaphore_t *sem)
{
	if(sem == NULL)return 0;
	
	return sem->max_count;
}


/**********************************************************************************************//**
 * @fn	uint8_t semaphore_is_pending_list_empty(semaphore_t *sem)
 *
 * @brief	the function checks if there is any task waiting for the semaphore.
 *
 * @param		sem		pointer to the semaphore.
 * 
 * @returns		uint8_t  TRUE-there is at least one task pending.
 *						 FALSE-there are no pending tasks
  **************************************************************************************************/

uint8_t semaphore_is_pending_list_empty(semaphore_t *sem)
{
	if( (sem == NULL) || (sem->head_pending_tasks_list == NULL) )return TRUE;
	
	return FALSE;
}


/**********************************************************************************************//**
 * @fn	uint8_t semaphore_wait(semaphore_t *sem)
 *
 * @brief	the function that decrements a semaphore counter, used to access a shared resource.
 *
 * @param		sem		pointer to the semaphore.
 * @returns		uint8_t  TRUE - access granted, FALSE - no available resources.
  **************************************************************************************************/

uint8_t semaphore_wait(semaphore_t *sem)
{
	uint8_t irq_flag;
	
	if(sem == NULL)return FALSE;

	irq_flag = rtos_cli();

	if( (sem->count > 0) && (sem->max_count > 0) ){
		sem->count--;
		rtos_sei(irq_flag);

		return TRUE;
	}
	rtos_sei(irq_flag);
	return FALSE;
}


/**********************************************************************************************//**
 * @fn	int8_t semaphore_signal(semaphore_t *sem)
 *
 * @brief	the function that increments a semaphore counter, used to release access to a shared resource.
 *			for each released resource, the function wakes up the next waiting task.
 *
 * @param		sem		pointer to the semaphore.
 * @returns		int8_t  0 - ok, -1 - attempting to release over the limit.
  **************************************************************************************************/

int8_t semaphore_signal(semaphore_t *sem)
{
	if(sem == NULL)return -1;
	
	if(sem->head_pending_tasks_list != NULL){
		task_handle_t *pending_task = task_list_pop_front(&sem->head_pending_tasks_list);

		task_set_wait_for_semaphore(NULL, pending_task);
		task_unfreeze(pending_task);

	}else if(sem->count < sem->max_count){
		uint8_t irq = rtos_cli();
		sem->count++;
		rtos_sei(irq);

	}else{
		rtos_error(0x01, __Err_DeviceSoftware_rtOS_SemaphoreCount);
		return -1;
	}
	return 0;
}


/**********************************************************************************************//**
 * @fn	void semaphore_remove_from_pending_list(task_handle_t *task, semaphore_t *sem)
 *
 * @brief	the function removes the task from the semaphore waiting list. 
 *
 * @param		sem		pointer to the semaphore.
 *				task	pointer to the task.
  **************************************************************************************************/

void semaphore_remove_from_pending_list(task_handle_t *task, semaphore_t *sem)
{
	if( (task == NULL) || (sem == NULL) )return;
	if(task_get_wait_for_semaphore(task) == sem){
		task_list_remove_by_item(&sem->head_pending_tasks_list, task);
		task_set_wait_for_semaphore(NULL, task);
		task_freeze(SLEEP_INFINITE, task);
	}
}


/**********************************************************************************************//**
 * @fn	void __semaphore_wait(semaphore_t *sem)
 *
 * @brief	The function that decrements a semaphore counter, used by task to access a shared resource.
 *			Using this function without properly updating the task's program counter may result in unexpected program behavior.
 *			Use the task_semaphore_wait() macro instead of this function.
 *
 * @param		sem		pointer to the semaphore.
  **************************************************************************************************/

void __semaphore_wait(semaphore_t *sem)
{
	if(sem == NULL)return;

	if(!semaphore_wait(sem)){
		task_set_wait_for_semaphore(sem);
		task_list_push_back((task_handle_t **)&sem->head_pending_tasks_list, task_freeze(WAIT_SEMA));
		rtos_back_jump();
	}
}


/**********************************************************************************************//**
 * @fn	void mutex_init(mutex_t *mutex)
 *
 * @brief	Mutex initialization function.
 *
 * @param		mutex			pointer to the mutex.
  **************************************************************************************************/

void mutex_init(mutex_t *mutex)
{
	if(mutex != NULL)	
		_init(mutex, THIS_IS_MUTEX, 0);
}


/**********************************************************************************************//**
 * @fn	uint8_t _mutex_check_access(mutex_t *mutex, task_handle_t *task=task_this())
 *
 * @brief	the function checks whether the given task owns the given mutex
 *
 * @param		mutex		mutex pointer.
 *				task		task pointer, by default this function argument is null
 *							it means call this function for currently running task
 *
 * @returns		uint8_t		FALSE - the task doesn't own the mutex, TRUE - the task owns the mutex.
  **************************************************************************************************/

uint8_t _mutex_check_access(mutex_t *mutex, task_handle_t *task)
{
	if(task == NULL)task = task_this();
	
	return ( (mutex == NULL) || (task == NULL) || (mutex->owner != task) ) ? FALSE : TRUE; 
}


/**********************************************************************************************//**
 * @fn	void __mutex_lock(semaphore_t *mutex)
 *
 * @brief	the function that lock a mutex, used by task to access a shared resource.
 *			Using this function without properly updating the task's program counter may result in unexpected program behavior.
 *			Use the task_semaphore_mutex_lock() macro instead of this function. 
 *
 * @param		mutex	pointer to the mutex.
  **************************************************************************************************/

void __mutex_lock(semaphore_t *mutex)
{
	task_handle_t *task = task_this();
	
	if( (task == NULL) ||						//err
		(mutex == NULL) ||						//err
		(mutex->type != THIS_IS_MUTEX) ||		//err
		(mutex->owner == task)					//the task already owns the mutex
	)return;

	uint8_t irq = rtos_cli();
	
	if(mutex->owner != NULL){
		rtos_sei(irq);
		task_set_wait_for_semaphore(mutex);
		task_list_push_back((task_handle_t **)&mutex->head_pending_tasks_list, task_freeze(WAIT_SEMA));
		rtos_back_jump();
	
	}else{
		mutex->owner = task;
		rtos_sei(irq);
		mutex->next	= task_get_first_mutex_from_list();
		task_set_first_mutex_in_list(mutex);
	}
}


/**********************************************************************************************//**
 * @fn	int8_t _mutex_unlock(mutex_t *mutex, task_handle_t *task=task_this())
 *
 * @brief	use this function to release access to a shared resource locked by a given task.
 *			the function wakes up the next waiting task.
 *
 * @param		mutex	pointer to the mutex.
 *				task	pointer to the task, by default this function argument is null
 *						it means call this function for currently running task
 * @returns		int8_t  0 - ok, -1 - no mutex or mutex unlocked, -2 - the given task does not own the given mutex.
  **************************************************************************************************/

int8_t _mutex_unlock(mutex_t *mutex, task_handle_t *task)
{
	mutex_t *ptr_first_item;
	task_handle_t *new_owner = NULL;
	uint8_t irq;
	
	if(task == NULL)
		task = task_this();
		
	if( (task == NULL) ||
		(mutex == NULL) || 
		(mutex->type != THIS_IS_MUTEX) || 
		(mutex->owner == NULL) 
	)return -1;
	
	if(mutex->owner != task){
		rtos_error(0x01, __Err_DeviceSoftware_rtOS_SemaphoreOwner);
		return -2;
	}
	
	ptr_first_item	= task_get_first_mutex_from_list(task);
	
	if(ptr_first_item == mutex){
		task_set_first_mutex_in_list(ptr_first_item->next, task);
	
	}else{
		for(mutex_t *ptr_mux = ptr_first_item; ptr_mux->next != NULL; ptr_mux = ptr_mux->next){
			if(ptr_mux->next == mutex){
				ptr_mux->next = mutex->next;
				break;
			}
		}
	}
	
	if(mutex->head_pending_tasks_list != NULL){
		task_handle_t *pending_task = task_list_pop_front(&mutex->head_pending_tasks_list);

		task_set_wait_for_semaphore(NULL, pending_task);
		task_unfreeze(pending_task);
		mutex->next = task_get_first_mutex_from_list(pending_task);
		task_set_first_mutex_in_list(mutex, pending_task);
		new_owner = pending_task;
	}
	
	irq = rtos_cli();
	mutex->owner = new_owner;
	rtos_sei(irq);
	
	return 0;
}








