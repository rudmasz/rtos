#ifndef __RTOS_H_						
#define	__RTOS_H_

#include "task.h"
#include "heap.h"
#include "semaphore.h"
#include "timers.h"
#include "board.h"
#include "errCode.h"
#include "event.h"

#ifndef RUN_TESTS
	#define RTOS_static	static
#else
	#include "test.h"
	#define RTOS_static
#endif

#ifndef NULL
#define	NULL	(void *)0x00
#endif

#ifndef TRUE
#define TRUE	1
#endif

#ifndef	FALSE
#define FALSE	0
#endif

#ifndef HIGH
#define HIGH	1
#endif

#ifndef LOW
#define	LOW		0
#endif

#ifndef ON
#define ON		1
#endif

#ifndef OFF
#define	OFF		0
#endif


#define RTOS_stack_overflow_tag_size		2
#define RTOS_stack_overflow_tag				0xCA
#define RTOS_err_out_of_dynamic_mem			0x80

#ifndef F_CPU
	#define F_CPU	BOARD_cpu_clock
#else
	#if (F_CPU != BOARD_cpu_clock)
		#undef F_CPU
		#define F_CPU	BOARD_cpu_clock
	#endif
#endif

#if defined(__AVR_ATmega164__)
#include "rtos_peripheral_at164.h"

#elif defined(__AVR_ATmega324__)
#include "rtos_peripheral_at324.h"

#elif defined(__AVR_ATmega644__)
#include "rtos_peripheral_at644.h"

#elif defined(__AVR_ATmega128__)
#include "rtos_peripheral_at128.h"

#elif defined(__AVR_ATmega1284__)
#include "rtos_peripheral_at1284.h"

#elif defined(__AVR_ATmega32__)
#include "rtos_peripheral_at32.h"

#elif defined(__AVR_ATmega1280__)
#include "rtos_peripheral_at1280.h"

#else
#error You have to specify the device type _for example '#define __AVR_ATmega1284__'
#endif

void __rtos_wait_irq_val(rtos_peripheral_irq_t irq_nr);



/**********************************************************************************************//**
 * @fn	uint8_t rtos_cnt_bits_uint8_t(uint8_t v)
 *
 * @brief	This macro will count the number of high state bits in a given byte variable.
 *
 **************************************************************************************************/

#define rtos_cnt_bits_uint8_t(v)	((v&0x80?1:0)+(v&0x40?1:0)+(v&0x20?1:0)+(v&0x10?1:0)+(v&0x08?1:0)+(v&0x04?1:0)+(v&0x02?1:0)+(v&0x01))


/**********************************************************************************************//**
 * @fn	void rtos_load_uint8_t_from_ptr(uint8_t variable, uint8_t *ptr)
 *
 * @brief	This macro instruction will load one byte of data from the address
 *			pointed to by the pointer ptr into the variable.
 *
 **************************************************************************************************/
#define rtos_load_uint8_t_from_ptr(variable, ptr)\
			asm volatile (\
				"ld %[var], %a1	\n\t"\
				:[var] "=a" (variable)\
				:"e" (ptr)\
			)


/**********************************************************************************************//**
 * @fn	void rtos_load_uint8_t(uint8_t variable, uint8_t )
 *
 * @brief	This macro will load a single-byte constant into a variable.
 *
 **************************************************************************************************/
#define rtos_load_uint8_t(variable, value)\
			asm volatile (\
				"ldi %[var], %[val]	\n\t"\
				:[var] "=a" (variable)\
				: [val]"M" (value)\
			)


/**********************************************************************************************//**
 * @fn	void rtos_load_uint16_t_from_ptr(uint16_t variable, uint16_t *ptr)
 *
 * @brief	This macro instruction will load two bytes of data from the address
 *			pointed to by the pointer ptr into the variable.
 *
 **************************************************************************************************/
#define rtos_load_uint16_t_from_ptr(variable, ptr)\
			asm volatile (\
				"ld %A[var], %a1+	\n\t"\
				"ld %B[var], %a1	\n\t"\
				:[var] "=a" (variable)\
				:"e" (ptr)\
			)


/**********************************************************************************************//**
 * @fn	void rtos_load_uint16_t(uint16_t variable, uint16_t )
 *
 * @brief	This macro will load a two-byte constant into a variable.
 *
 **************************************************************************************************/			
#define rtos_load_uint16_t(variable, value)\
			asm volatile (\
				"ldi %A[var], lo8((%[val]))	\n\t"\
				"ldi %B[var], hi8((%[val]))	\n\t"\
				:[var] "=a" (variable)\
				: [val]"i" (value)\
			)


/**********************************************************************************************//**
 * @fn	void rtos_load_uint32_t(uint32_t variable, uint32_t )
 *
 * @brief	This macro will load a four-byte constant into a variable.
 *
 **************************************************************************************************/
#define rtos_load_uint32_t(variable, value)\
			asm volatile (\
				"ldi %A[var], lo8((%[val]))	\n\t"\
				"ldi %B[var], hi8((%[val]))	\n\t"\
				"ldi %C[var], hlo8((%[val]))\n\t"\
				"ldi %D[var], hhi8((%[val]))\n\t"\
				:[var] "=a" (variable)\
				: [val]"i" (value)\
			)


/**********************************************************************************************//**
 * @fn	uint32_t rtos_get_system_time_ms(void)
 *
 * @brief	the function returns the value of the system ms counter. 
 *
 * @returns	uint32_t.
 **************************************************************************************************/
uint32_t rtos_get_system_time_ms(void);


/**********************************************************************************************//**
 * @fn	uint8_t rtos_get_global_interrupt_state(void)
 *
 * @brief	the function checks if the global interrupt bit is set returns TRUE if set and FALSE if not. 
 *
 * @returns	uint8_t.
 **************************************************************************************************/
uint8_t rtos_get_global_interrupt_state(void);


/**********************************************************************************************//**
 * @fn	uint8_t rtos_cli(void)
 *
 * @brief	the function reads the global interrupt flag, clears it if set, and returns the read value. 
 *			this function should be used with rtos_set() to clear and set the global interrupt
 *			flag instead of the cli() and sei() functions.
 *
 * @returns	uint8_t.
 **************************************************************************************************/
uint8_t rtos_cli(void);


/**********************************************************************************************//**
 * @fn	void rtos_sei(uint8_t irq_flag)
 *
 * @brief	the function enables the global interrupt if irq_flag is non-zero. 
 *			this function should be used with rtos_cli() to clear and set the global interrupt
 *			flag instead of the cli() and sei() functions.
 *
 * @param	irq_flag	if non-zero the global interrupt will be enabled.
 **************************************************************************************************/
void rtos_sei(uint8_t irq_flag);


/**********************************************************************************************//**
 * @fn	void rtos_peripheral_switch_on(rtos_peripheral_t periph)
 *
 * @brief	Use this function to enable specific peripherals. 
 *			The function must be called before writing to any of the registers of a given peripheral.
 *
 * @param	periph	peripheral to run.
 **************************************************************************************************/
void rtos_peripheral_switch_on(rtos_peripheral_t periph);


/**********************************************************************************************//**
 * @fn	void rtos_peripheral_switch_off(rtos_peripheral_t periph)
 *
 * @brief	Use this function to disable specific peripheral, to reduce power consumption.
 *
 * @param	periph	peripheral to stop.
 **************************************************************************************************/
void rtos_peripheral_switch_off(rtos_peripheral_t periph);


/**********************************************************************************************//**
 * @fn	void rtos_peripheral_get_state(rtos_peripheral_t periph)
 *
 * @brief	Use this function to get the current state of a specific peripheral.
 *
 * @param	periph	peripheral to check.
 **************************************************************************************************/
__attribute__ ((noinline)) uint8_t rtos_peripheral_get_state(rtos_peripheral_t periph);


/**********************************************************************************************//**
 * @fn	void rtos_irq_report(rtos_peripheral_irq_t irq)
 *
 * @brief	Call this function in the IRQ handler to report an IRQ occurrence to the system.
 *
 * @param	irq		irq number to report.
 **************************************************************************************************/
void rtos_irq_report(rtos_peripheral_irq_t irq);


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
uint8_t rtos_irq_get(rtos_peripheral_irq_t irq);


/**********************************************************************************************//**
 * @fn	void rtos_back_jump(void)
 *
 * @brief	This function allows you to jump back from the task program to the scheduler program.
 *
 **************************************************************************************************/
#ifndef RUN_TESTS
	void rtos_back_jump(void);
#else
	void __rtos_back_jump(void);
#endif


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
void rtos_error(int8_t sign, uint32_t __ErrCode);




#endif





