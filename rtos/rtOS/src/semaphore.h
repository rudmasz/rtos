/*
 * semaphore.h
 *
 * Created: 13.06.2024 15:26:43
 *  Author: tom
 */ 


#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include "rtos.h"
#include "vrg.h"




typedef struct semaphore{
	union{
		uint8_t count;
		struct task_handle *owner;
	};
	union{
		uint8_t max_count;
		uint8_t type;
	};
	struct task_handle	*head_pending_tasks_list;
	struct semaphore	*next;

}semaphore_t, mutex_t;

void __semaphore_wait(semaphore_t *sem);
void __mutex_lock(mutex_t *mutex);


/**********************************************************************************************//**
 * @fn	void semaphore_init(semaphore_t *sem, uint8_t max_count, uint8_t init_count=max_count)
 *
 * @brief	Counting semaphore initialization function.
 *
 * @param		sem			pointer to the semaphore.
 * @param		max_count	The maximum count value that can be reached.
 * @param		init_count	The initial count value assigned to the semaphore (default value is equal max_count).
  **************************************************************************************************/
void _semaphore_init(semaphore_t *sem, uint8_t max_count, uint8_t init_count);
#define semaphore_init(...)								VRG(_semaphore_init, __VA_ARGS__)
#define _semaphore_init2(sem, max_count)				_semaphore_init(sem, max_count, max_count)
#define _semaphore_init3(sem, max_count, init_count)	_semaphore_init(sem, max_count, init_count)


/**********************************************************************************************//**
 * @fn	uint8_t semaphore_get_count(semaphore_t *sem)
 *
 * @brief	the function returns the current count value.
 *
 * @param		sem		pointer to the semaphore.
 * 
 * @returns		uint8_t  current count value.
  **************************************************************************************************/
uint8_t semaphore_get_count(semaphore_t *sem);


/**********************************************************************************************//**
 * @fn	uint8_t semaphore_get_count(semaphore_t *sem)
 *
 * @brief	the function returns the max count value.
 *
 * @param		sem		pointer to the semaphore.
 * 
 * @returns		uint8_t  max count value.
  **************************************************************************************************/
uint8_t semaphore_get_max_count(semaphore_t *sem);


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
uint8_t semaphore_is_pending_list_empty(semaphore_t *sem);


/**********************************************************************************************//**
 * @fn	uint8_t semaphore_wait(semaphore_t *sem)
 *
 * @brief	the function that decrements a semaphore counter, used to access a shared resource.
 *
 * @param		sem		pointer to the semaphore.
 * @returns		uint8_t  TRUE - access granted, FALSE - no available resources.
  **************************************************************************************************/
uint8_t semaphore_wait(semaphore_t *sem);


/**********************************************************************************************//**
 * @fn	int8_t semaphore_signal(semaphore_t *sem)
 *
 * @brief	the function that increments a semaphore counter, used to release access to a shared resource.
 *			for each released resource, the function wakes up the next waiting task.
 *
 * @param		sem		pointer to the semaphore.
 * @returns		int8_t  0 - ok, -1 - attempting to release over the limit.
  **************************************************************************************************/
int8_t semaphore_signal(semaphore_t *sem);


/**********************************************************************************************//**
 * @fn	void semaphore_remove_from_pending_list(task_handle_t *task, semaphore_t *sem)
 *
 * @brief	the function removes the task from the semaphore waiting list. 
 *
 * @param		sem		pointer to the semaphore.
 *				task	pointer to the task.
  **************************************************************************************************/
void semaphore_remove_from_pending_list(struct task_handle *task, semaphore_t *sem);


/**********************************************************************************************//**
 * @fn	void condWait_semaphore_wait(semaphore_t *sem)
 *
 * @brief	The function that decrements a semaphore counter, used by task to access a shared resource.
 *			Use this function if you want to freeze the task until the semaphore is obtained
 *
 * @param		sem		pointer to the semaphore.
  **************************************************************************************************/
#define condWait_semaphore_wait(sem)\
			task_update_pc_addr_after_call(__semaphore_wait(sem))



/**********************************************************************************************//**
 * @fn	void mutex_init(mutex_t *mutex)
 *
 * @brief	Mutex initialization function.
 *
 * @param		mutex	pointer to the mutex.
 *
  **************************************************************************************************/
void mutex_init(mutex_t *mutex);


/**********************************************************************************************//**
 * @fn	uint8_t mutex_is_pending_list_empty(mutex_t *mutex)
 *
 * @brief	the function checks if there is any task waiting for the mutex.
 *
 * @param		mutex		pointer to the mutex.
 * 
 * @returns		uint8_t  TRUE-there is at least one task pending.
 *						 FALSE-there are no pending tasks
  **************************************************************************************************/
#define mutex_is_pending_list_empty(mutex)\
			semaphore_is_pending_list_empty((semaphore_t *)mutex)


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
uint8_t _mutex_check_access(mutex_t *mutex, struct task_handle *task);
#define mutex_check_access(...)				VRG(_mutex_check_access, __VA_ARGS__)
#define _mutex_check_access1(mutex)			_mutex_check_access(mutex, NULL)
#define _mutex_check_access2(mutex, task)	_mutex_check_access(mutex, task)


/**********************************************************************************************//**
 * @fn	void mutex_remove_from_pending_list(task_handle_t *task, mutex_t *mutex)
 *
 * @brief	the function removes the task from the mutex waiting list. 
 *
 * @param		mutex	pointer to the mutex.
 *				task	pointer to the task.
  **************************************************************************************************/
#define mutex_remove_from_pending_list(task, mutex)\
			semaphore_remove_from_pending_list(task, (semaphore_t *)mutex)


/**********************************************************************************************//**
 * @fn	void condWait_mutex_lock(mutex_t *mutex)
 *
 * @brief	the function that lock a mutex, used by task to access a shared resource.
 *			Use this function if you want to freeze the task until the mutex is obtained.
 *
 * @param		mutex	pointer to the mutex.
  **************************************************************************************************/
#define condWait_mutex_lock(mutex)\
			task_update_pc_addr_after_call(__mutex_lock(mutex))


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
int8_t _mutex_unlock(mutex_t *mutex, struct task_handle *task);
#define mutex_unlock(...)				VRG(_mutex_unlock, __VA_ARGS__)
#define _mutex_unlock1(mutex)			_mutex_unlock(mutex, NULL)
#define _mutex_unlock2(mutex, task)		_mutex_unlock(mutex, task)

#endif /* SEMAPHORE_H_ */