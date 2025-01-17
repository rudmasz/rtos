/*
 * event.h
 *
 * Created: 02.08.2024 09:03:24
 *  Author: tom
 */ 


#ifndef EVENT_H_
#define EVENT_H_

#define event_action_t	semaphore_t


void __event_wait_signal(uint8_t volatile *sig_src, uint8_t sig_mask, uint16_t time_ms);



/**********************************************************************************************//**
 * @fn	void event_action_init(event_action_t *action)
 *
 * @brief	Action initialization function.
 *			Use the action mechanism to create a system that notifies multiple tasks when a specific action occurs.
 *
 * @param		action			pointer to the action.
  **************************************************************************************************/
void event_action_init(event_action_t *action);


/**********************************************************************************************//**
 * @fn	void event_action_notify_listeners(event_action_t *sender)
 *
 * @brief	This function will wake up all pending task.
 *			Use the action mechanism to create a system that notifies multiple tasks when a specific action occurs.
 *
 * @param		sender			pointer to the action sender.
  **************************************************************************************************/
void event_action_notify_listeners(event_action_t *sender);


/**********************************************************************************************//**
 * @fn	void condWait_event_action_wait(event_action_t *sender)
 *
 * @brief	This function will suspend the currently running task and
 *			add it to the list of tasks waiting for some action to occur.
 *			Use the action mechanism to create a system that notifies multiple tasks when a specific action occurs.
 *
 * @param		sender			pointer to the action sender.
  **************************************************************************************************/
#define condWait_event_action_wait(sender)\
			condWait_semaphore_wait(sender)


/**********************************************************************************************//**
 * @fn	void condWait_event_wait_signal(uint8_t volatile *sig_src, uint8_t sig_mask, uint16_t time_ms)
 *
 * @brief	This function will suspend the currently running task until 
 *			the following condition is met *sig_src&sig_mask == sig_mask. 
 *			If the previously specified condition is not met and the time_ms parameter is different from zero,
 *			then before the next condition check the task will be additionally suspended 
 *			for a time equal to the time_ms value.
 *
 * @param	sig_src		signal source pointer
 *			sig_mask	signal mask
 *			time_ms		sleep time
 *
 **************************************************************************************************/

#define condWait_event_wait_signal(sig_src, sig_mask, time_ms)\
			task_update_pc_addr_before_call(__event_wait_signal(sig_src, sig_mask, time_ms))			
			
#endif /* EVENT_H_ */