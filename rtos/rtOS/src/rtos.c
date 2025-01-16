#include <avr/io.h>
#include <avr/interrupt.h>
#ifndef RUN_TESTS
	#include <avr/sleep.h>
	#include <avr/wdt.h>
#endif
#include <string.h>
#include "rtos.h"
#include <util/delay.h>
#include "timers.h"
#include "heap.h"
#include "semaphore.h"


void idle_task(void);




#define RTOS_stack_addr					(__stack + BOARD_stack_size - RTOS_stack_overflow_tag_size - BOARD_local_variable_stack_size - 1)


#define RTOS_call_task()\
			asm volatile(\
				"clr __zero_reg__					\n\t"\
				"ldi r28, lo8((%[stack]))			\n\t"\
				"ldi r29, hi8((%[stack]))			\n\t"\
				"cli								\n\t"\
				"out __SP_L__, r28					\n\t"\
				"out __SP_H__, r29					\n\t"\
				"sei								\n\t"\
				"mov r30, %A[pc]				\n\t"\
				"mov r31, %B[pc]				\n\t"\
				"ijmp								\n\t"\
				"BackFromTaskLabel:					\n\t"\
				"clr __zero_reg__					\n\t"\
				::										\
					[stack] "i" (RTOS_stack_addr), [pc] "r" (task_this()->PC)\
			)



volatile uint8_t MCUCSR_saved_val				__attribute__((section(".noinit")));

RTOS_static				task_handle_t			__idle_task;
RTOS_static	volatile	uint8_t					__stack[BOARD_stack_size] __attribute__((section(".noinit")));
RTOS_static	volatile	uint32_t				__rtos_system_time;
					void						(*rtos_response_on_brownout_reset)(void); 
					void						(*rtos_response_on_power_on_reset)(void);
					void						(*rtos_response_on_jtag_reset)(void);
					void						(*rtos_response_on_external_reset)(void);
					void						(*rtos_initialize_avr_device)(void);
					void						(*rtos_response_on_watchdog_reset)(task_handle_t *current_task);
					void						(*rtos_response_on_error)(int8_t sign, uint32_t err_code);



RTOS_static volatile		rtos_peripheral_irq_register_t	__rtos_irq_reg;
RTOS_static volatile		rtos_peripheral_register_t		__rtos_peripherals;




/**********************************************************************************************//**
 * @fn	void __rtos_stac_setup(void)
 *
 * @brief	the function initializes the stack.
 *
 **************************************************************************************************/


void __attribute__ ((naked)) __attribute__ ((section (".init2"))) __rtos_stac_setup(void)
{
#if BOARD_startup_time_ms != 0x0000
	for(uint16_t i=0x00; i<BOARD_startup_time_ms ; i++){
		_delay_us(1000);
	}
#endif
	MCUCSR_saved_val 	= MCUSR;
	MCUSR				= 0x00;

	wdt_disable();

	__stack[0] = RTOS_stack_overflow_tag;
	__stack[1] = RTOS_stack_overflow_tag;
	__stack[BOARD_stack_size - 2] = RTOS_stack_overflow_tag;
	__stack[BOARD_stack_size - 1] = RTOS_stack_overflow_tag;
	

	asm volatile(
		"ldi r28, lo8((%[Stack]))	\n\t"
		"ldi r29, hi8((%[Stack]))	\n\t"
		"out __SP_L__, r28			\n\t"
		"out __SP_H__, r29			\n\t"
		::
		[Stack]"i"(RTOS_stack_addr)
	);
}


/**********************************************************************************************//**
 * @fn	void __rtos_ports_setup(void)
 *
 * @brief	the function initializes the mcu ports.
 *
 **************************************************************************************************/

void __attribute__ ((naked)) __attribute__ ((section (".init3"))) __rtos_ports_setup(void)
{
	__rtos_peripheral_ports_init();
}


/**********************************************************************************************//**
 * @fn	void idle_task(void)
 *
 * @brief	puts the CPU into the appropriate sleep state depending on whether,
 *			there are any active tasks and peripherals
 *
 *			any_irq_pending == TRUE -> run mode
 *
 *			any_peripheral		tasks_in_scheduler
 *				FALSE				FALSE				-> SLEEP_MODE_EXT_STANDBY
 *				TRUE				FALSE				-> SLEEP_MODE_IDLE
 *				FALSE				TRUE				-> run mode
 *				TRUE				TRUE				-> run mode
 *
 **************************************************************************************************/

TASK_my_task_t idle_task(void)
{
	uint8_t tasks_in_scheduler;			//check if there is more than just an idle task on the schedule
	uint8_t any_peripheral;				//check if more peripherals than just the system clock are enabled
	uint8_t any_irq_pending;			//check if any interrupt has been reported.
	
	tasks_in_scheduler	= (task_get_number_of_running_tasks() > 1) ? TRUE : FALSE;
	any_peripheral		= (__rtos_peripherals == _BV(RTOS_peripheral_system_clock_timer)) ? FALSE : TRUE;
	
	cli();
	any_irq_pending		= (__rtos_irq_reg != 0 ? TRUE : FALSE);

	if( (any_irq_pending == FALSE) && (tasks_in_scheduler == FALSE) )
	{
		wdt_disable();
		if(any_peripheral == TRUE){
			
			set_sleep_mode(SLEEP_MODE_IDLE);
		
		}else{
			set_sleep_mode(SLEEP_MODE_EXT_STANDBY);
		}

		sleep_enable();
		sei();
		sleep_cpu();
		sleep_disable();
		wdt_reset();
		wdt_enable(BOARD_watch_dog_time);			//WATCHDOG ENABLE

	}else{
		sei();
	}

	rtos_back_jump();
}


/**********************************************************************************************//**
 * @fn	uint32_t rtos_get_system_time_ms(void)
 *
 * @brief	the function returns the value of the system ms counter. 
 *
 * @returns	uint32_t.
 **************************************************************************************************/

uint32_t rtos_get_system_time_ms(void)
{
	uint8_t irq_state = rtos_cli();
	uint32_t Time = __timer_get_time_ms();
	
	rtos_sei(irq_state);

	return Time + __rtos_system_time;
}


/**********************************************************************************************//**
 * @fn	uint8_t rtos_get_global_interrupt_state(void)
 *
 * @brief	the function checks if the global interrupt bit is set returns TRUE if set and FALSE if not. 
 *
 * @returns	uint8_t.
 **************************************************************************************************/

//__attribute__ ((noinline)) 
uint8_t rtos_get_global_interrupt_state(void)
{
	register uint8_t irqFlag asm("r24");
	
	asm volatile (
		"ldi %[var], 0x00	\n\t"
		"brid L_dl1%=		\n\t"
		"ldi %[var], 0x01	\n\t"
		"L_dl1%=:			\n\t"
		:[var] "=r" (irqFlag)
		:
	);
		
	return irqFlag;
}


/**********************************************************************************************//**
 * @fn	uint8_t rtos_cli(void)
 *
 * @brief	the function reads the global interrupt flag, clears it if set, and returns the read value. 
 *			this function should be used with rtos_set() to clear and set the global interrupt
 *			flag instead of the cli() and sei() functions.
 *
 * @returns	uint8_t.
 **************************************************************************************************/

uint8_t rtos_cli(void)
{
	uint8_t irqFlag = rtos_get_global_interrupt_state();
	if(irqFlag)cli();
	return irqFlag;
}


/**********************************************************************************************//**
 * @fn	void rtos_sei(uint8_t irq_flag)
 *
 * @brief	the function enables the global interrupt if irq_flag is non-zero. 
 *			this function should be used with rtos_cli() to clear and set the global interrupt
 *			flag instead of the cli() and sei() functions.
 *
 * @param	irq_flag	if non-zero the global interrupt will be enabled.
 **************************************************************************************************/

void rtos_sei(uint8_t irq_flag)
{
	if(irq_flag)sei();
}


/**********************************************************************************************//**
 * @fn	void rtos_peripheral_switch_on(rtos_peripheral_t periph)
 *
 * @brief	Use this function to enable specific peripherals. 
 *			The function must be called before writing to any of the registers of a given peripheral.
 *
 * @param	periph	peripheral to run.
 **************************************************************************************************/

__attribute__ ((noinline)) void rtos_peripheral_switch_on(rtos_peripheral_t periph)
{
	__rtos_peripheral_on(&__rtos_peripherals, periph);
}


/**********************************************************************************************//**
 * @fn	void rtos_peripheral_switch_off(rtos_peripheral_t periph)
 *
 * @brief	Use this function to disable specific peripherals, to reduce power consumption.
 *
 * @param	periph	peripheral to stop.
 **************************************************************************************************/

__attribute__ ((noinline)) void rtos_peripheral_switch_off(rtos_peripheral_t periph)
{
	__rtos_peripheral_off(&__rtos_peripherals, periph);
}


/**********************************************************************************************//**
 * @fn	void rtos_peripheral_get_state(rtos_peripheral_t periph)
 *
 * @brief	Use this function to get the current state of a specific peripheral.
 *
 * @param	periph	peripheral to check.
 **************************************************************************************************/

__attribute__ ((noinline)) uint8_t rtos_peripheral_get_state(rtos_peripheral_t periph)
{
	return __rtos_peripherals & _BV(periph) ? ON : OFF;
}


/**********************************************************************************************//**
 * @fn	void rtos_irq_report(rtos_peripheral_irq_t irq)
 *
 * @brief	Call this function in the IRQ handler to report an IRQ occurrence to the system.
 *
 * @param	irq		irq number to report.
 **************************************************************************************************/

__attribute__ ((noinline)) void rtos_irq_report(rtos_peripheral_irq_t irq)
{
	uint8_t *irQ = ((uint8_t *)&__rtos_irq_reg) + (irq>>0x03);
	uint8_t irq_flag = rtos_cli();
	
	*irQ |= _BV((irq&0x07));
	rtos_sei(irq_flag);
}


/**********************************************************************************************//**
 * @fn	uint8_t rtos_irq_get(rtos_peripheral_irq_t irq)
 *
 * @brief	Use this function to check whether a given interrupt has been reported in the system.
 *			Reading automatically resets the flag of the interrupt being checked.
 *
 * @param	irq		irq number to check.
 *
 * @returns	uint8_t	TRUE - irq has been reported, FALSE - irq has not been reported.
 **************************************************************************************************/

__attribute__ ((noinline)) uint8_t rtos_irq_get(rtos_peripheral_irq_t irq)
{	
	uint8_t irQmask = _BV((irq&0x07));
	uint8_t volatile *irQreg = ((uint8_t volatile*)&__rtos_irq_reg) + (irq>>3);
	uint8_t irq_flag, state;
	
	irq_flag = rtos_cli();
	
	if(*irQreg & irQmask){
		*irQreg &= ~irQmask;
		state = HIGH;
	
	}else{
		state = LOW;
	}
	rtos_sei(irq_flag);
	return state;
}


/**********************************************************************************************//**
 * @fn	void rtos_back_jump(void)
 *
 * @brief	This function allows you to jump back from the task program to the scheduler program.
 *
 **************************************************************************************************/

#ifndef RUN_TESTS
	__attribute__((noinline)) __attribute__((naked))  void rtos_back_jump(void)
#else
	__attribute__((noinline)) __attribute__((naked))  void __rtos_back_jump(void)
#endif
{
	asm volatile(									
				"rjmp BackFromTaskLabel	\n\t"
				::);
}


/**********************************************************************************************//**
 * @fn	void rtos_error(int8_t sign, uint32_t __ErrCode)
 *
 * @brief	All errors in the system are reported by calling this function.
 *			The sign parameter should be used to determine whether a given error has occurred (value +1)
 *			or has disappeared (value -1).
 *			The basic list of error codes can be found in the errCode.h file.
 *			Error codes are 32-bit numbers. If the 16 least significant bits in the error code
 *			have a value equal to zero, the function will perform the logical sum of the error code
 *			and the address of the currently running task. Reported errors are not saved in the system,
 *			to capture their value, you should write your own function and set the pointer
 *			'void (*rtos_response_on_error)(int8_t sign, uint32_t err_code)' to this function.
 *
 * @param	sign		+1 error has occurred.
 *						-1 error has disappeared.
 *
 **************************************************************************************************/

void rtos_error(int8_t sign, uint32_t __ErrCode)
{
	if(rtos_response_on_error != NULL){
		if((uint16_t)__ErrCode == 0x0000){
			__ErrCode |= task_get_function_address(task_this());
		}
		rtos_response_on_error(sign, __ErrCode);
	}
}


/**********************************************************************************************//**
 * @fn	void __rtos_scheduler(void)
 *
 * @brief	Rtos scheduler.
 *
 **************************************************************************************************/

__attribute__((naked)) void __rtos_scheduler(void)
{
	uint16_t time;

	TASK_do
	{
		RTOS_call_task();

		if(	(__stack[0] != RTOS_stack_overflow_tag) || 
			(__stack[1] != RTOS_stack_overflow_tag) )
		{
			rtos_error(0x01, __Err_DeviceSoftware_rtOS_StackOverflowDown);
			while(1);
		}
		
		if(	(__stack[(BOARD_stack_size - 1)] != RTOS_stack_overflow_tag) || 
			(__stack[(BOARD_stack_size - 2)] != RTOS_stack_overflow_tag) )
		{
			rtos_error(0x01, __Err_DeviceSoftware_rtOS_StackOverflowUp);
			while(1);
		}
		cli();
		time = __timer_get_time_ms();
		__timer_clear_time_ms();
		sei();
		
		if(time != 0){
			__timer_refresh_timers(time);
			__task_refresh_delayed(time);
		}
		__task_refresh_interrupted();
		__task_switch();
		wdt_reset();

	}TASK_loop();
	
}


/**********************************************************************************************//**
 * @fn	int main(void)
 *
 * @brief	Main function.
 *
 **************************************************************************************************/


__attribute__((naked)) int main()
{	
	wdt_disable();
	MCUSR = 0x00;
	
	if(MCUCSR_saved_val & _BV(BORF)){				//RESET ON BROWNOUT
		if(rtos_response_on_brownout_reset != NULL){
			rtos_response_on_brownout_reset();
		}
	
	}else if(MCUCSR_saved_val & _BV(JTRF)){		//RESET ON JTAG
		if(rtos_response_on_jtag_reset != NULL){
			rtos_response_on_jtag_reset();
		}
	
	}else if(MCUCSR_saved_val & _BV(WDRF)){		//RESET ON WATCHDOG
		if(rtos_response_on_watchdog_reset != NULL){
			rtos_response_on_watchdog_reset((task_handle_t *)task_this());
		}
	
	}else if(MCUCSR_saved_val & _BV(EXTRF)){		//RESET ON EXTERNAL
		if(rtos_response_on_external_reset != NULL){
			rtos_response_on_external_reset();
		}
	
	}else{										//RESET ON POWER ON
		if(rtos_response_on_power_on_reset != NULL){
			rtos_response_on_power_on_reset();
		}
	}
	
	task_init();
	heap_init();
	__rtos_peripheral_init();
	task_setup(&__idle_task, idle_task, NULL);
	task_start(&__idle_task);

	if(rtos_initialize_avr_device != NULL){
		rtos_initialize_avr_device();
	}
	wdt_reset();
	wdt_enable(BOARD_watch_dog_time);			//WATCHDOG ENABLE

	sei();
	asm volatile("rjmp __rtos_scheduler		\n\t"		
				::);

}


