/*
 * event.c
 *
 * Created: 02.08.2024 09:02:57
 *  Author: tom
 */ 
#include <avr/io.h>
#include "semaphore.h"
#include "event.h"



/**********************************************************************************************//**
 * @fn	void event_action_init(event_action_t *action)
 *
 * @brief	Action initialization function.
 *			Use the action mechanism to create a system that notifies multiple tasks when a specific action occurs.
 *
 * @param		action			pointer to the action.
  **************************************************************************************************/

void event_action_init(event_action_t *action)
{
	if(action != NULL)
		semaphore_init(action, 2, 0);
}


/**********************************************************************************************//**
 * @fn	void event_action_notify_listeners(event_action_t *sender)
 *
 * @brief	This function will wake up all pending task.
 *			Use the action mechanism to create a system that notifies multiple tasks when a specific action occurs.
 *
 * @param		sender			pointer to the action sender.
  **************************************************************************************************/

void event_action_notify_listeners(event_action_t *sender)
{
	while(semaphore_is_pending_list_empty(sender) == FALSE)
		semaphore_signal(sender);
}


/**********************************************************************************************//**
 * @fn	void __event_wait_signal(uint8_t volatile *sig_src, uint8_t sig_mask, uint16_t time_ms)
 *
 * @brief	This function will suspend the currently running task until 
 *			the following condition is met *sig_src&sig_mask == sig_mask. 
 *			If the previously specified condition is not met and the time_ms parameter is different from zero,
 *			then before the next condition check the task will be additionally suspended 
 *			for a time equal to the time_ms value.
 *
 * @param	sig_src		signal source
 *			sig_mask	signal mask
 *			time_ms		sleep time
 *
 **************************************************************************************************/

void __event_wait_signal(uint8_t volatile *sig_src, uint8_t sig_mask, uint16_t time_ms)
{
	uint8_t signal_src, irq_flag;
	
	irq_flag = rtos_cli();
	signal_src = *sig_src;
	rtos_sei(irq_flag);
	
	if((signal_src&sig_mask) != sig_mask){
		if(time_ms){
			__task_delay(time_ms);
		}
		rtos_back_jump();
	}
}