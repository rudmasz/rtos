#ifndef __RTOS_PERIPHERAL_H_						
#define	__RTOS_PERIPHERAL_H_

#ifndef __RTOS_H_
	#error "Include <rtos.h> instead of this file."
#endif



#ifndef RAMSIZE
	#define RAMSIZE 16384
#endif

typedef enum{
				_IrqReset = 0,		//RESET
				_IrqINT0, 			//External Interrupt Request 0
				_IrqINT1,			//External Interrupt Request 1
				_IrqINT2,			//External Interrupt Request 2
				_IrqPCINT0,			//Pin Change Interrupt Request 0
				_IrqPCINT1,			//Pin Change Interrupt Request 1
				_IrqPCINT2,			//Pin Change Interrupt Request 2
				_IrqPCINT3,			//Pin Change Interrupt Request 3
				_IrqWDT,			//Watchdog Time-out Interrupt
				_IrqTIMER2_COMPA,	//Timer/Counter2 Compare Match A
				_IrqTIMER2_COMPB,	//Timer/Counter2 Compare Match B
				_IrqTIMER2_OVF,		//Timer/Counter2 Overflow
				_IrqTIMER1_CAPT,	//Timer/Counter1 Capture Event
				_IrqTIMER1_COMPA,	//Timer/Counter1 Compare Match A
				_IrqTIMER1_COMPB,	//Timer/Counter1 Compare Match B
				_IrqTIMER1_OVF,		//Timer/Counter1 Overflow
				_IrqTIMER0_COMPA,	//Timer/Counter0 Compare Match A
				_IrqTIMER0_COMPB,	//Timer/Counter0 Compare match B
				_IrqTIMER0_OVF,		//Timer/Counter0 Overflow
				_IrqSPI_STC,		//SPI Serial Transfer Complete
				_IrqUSART0_RX,		//USART0 Rx Complete
				_IrqUSART0_UDRE,	//USART0 Data Register Empty
				_IrqUSART0_TX,		//USART0 Tx Complete
				_IrqANALOG_COMP,	//Analog Comparator
				_IrqADC,			//Conversion Complete
				_IrqEE_READY,		//EEPROM Ready
				_IrqTWI,			//2-wire Serial Interface
				_IrqSPM_READY,		//Store Program Memory Ready
				_IrqUSART1_RX,		//USART1 Rx Complete
				_IrqUSART1_UDRE,	//USART1 Data Register Empty
				_IrqUSART1_TX,		//USART1 Tx Complete
				_IrqTIMER3_CAPT,	//Timer/Counter3 Capture Event
				_IrqTIMER3_COMPA,	//Timer/Counter3 Compare Match A
				_IrqTIMER3_COMPB,	//Timer/Counter3 Compare Match B
				_IrqTIMER3_OVF,		//Timer/Counter3 Overflow

			}rtos_peripheral_irq_t;
			

#define rtos_peripheral_irq_register_t	uint64_t

typedef enum{
				 _ADC = 0,
				 _USART0,
				 _SPI,			
				 _TIMER1,
				 _USART1,
				 _TIMER0,
				 _TIMER2,
				 _TWI,
				 _TIMER3,
				 _AC

			}rtos_peripheral_t;

#define rtos_peripheral_register_t							uint16_t
#define RTOS_peripheral_system_clock_timer					_TIMER2
#define RTOS_peripheral_system_clock_freq					32768		
#define RTOS_peripheral_system_clock_OCRA					32
#define RTOS_peripheral_system_clock_ticks_for_time(time_)	(( ((uint64_t)time_ * (uint64_t)RTOS_peripheral_system_clock_freq) / (RTOS_peripheral_system_clock_OCRA + 1) + 500) / 1000)

#define RTOS_peripheral_system_clock_ticks_for_0x40000000ms		RTOS_peripheral_system_clock_ticks_for_time(0x40000000)
#define RTOS_peripheral_system_clock_ticks_for_0x20000000ms		RTOS_peripheral_system_clock_ticks_for_time(0x20000000)
#define RTOS_peripheral_system_clock_ticks_for_0x10000000ms		RTOS_peripheral_system_clock_ticks_for_time(0x10000000)
#define RTOS_peripheral_system_clock_ticks_for_0x08000000ms		RTOS_peripheral_system_clock_ticks_for_time(0x08000000)
#define RTOS_peripheral_system_clock_ticks_for_0x04000000ms		RTOS_peripheral_system_clock_ticks_for_time(0x04000000)
#define RTOS_peripheral_system_clock_ticks_for_0x02000000ms		RTOS_peripheral_system_clock_ticks_for_time(0x02000000)
#define RTOS_peripheral_system_clock_ticks_for_0x01000000ms		RTOS_peripheral_system_clock_ticks_for_time(0x01000000)
#define RTOS_peripheral_system_clock_ticks_for_0x00800000ms		RTOS_peripheral_system_clock_ticks_for_time(0x00800000)
#define RTOS_peripheral_system_clock_ticks_for_0x00400000ms		RTOS_peripheral_system_clock_ticks_for_time(0x00400000)
#define RTOS_peripheral_system_clock_ticks_for_0x00200000ms		RTOS_peripheral_system_clock_ticks_for_time(0x00200000)
#define RTOS_peripheral_system_clock_ticks_for_0x00100000ms		RTOS_peripheral_system_clock_ticks_for_time(0x00100000)
#define RTOS_peripheral_system_clock_ticks_for_0x00080000ms		RTOS_peripheral_system_clock_ticks_for_time(0x00080000)
#define RTOS_peripheral_system_clock_ticks_for_0x00040000ms		RTOS_peripheral_system_clock_ticks_for_time(0x00040000)
#define RTOS_peripheral_system_clock_ticks_for_0x00020000ms		RTOS_peripheral_system_clock_ticks_for_time(0x00020000)
#define RTOS_peripheral_system_clock_ticks_for_0x00010000ms		RTOS_peripheral_system_clock_ticks_for_time(0x00010000)

#define RTOS_peripheral_system_clock_ticks_for_0x8000ms		RTOS_peripheral_system_clock_ticks_for_time(0x8000)
#define RTOS_peripheral_system_clock_ticks_for_0x4000ms		RTOS_peripheral_system_clock_ticks_for_time(0x4000)
#define RTOS_peripheral_system_clock_ticks_for_0x2000ms		RTOS_peripheral_system_clock_ticks_for_time(0x2000)
#define RTOS_peripheral_system_clock_ticks_for_0x1000ms		RTOS_peripheral_system_clock_ticks_for_time(0x1000)
#define RTOS_peripheral_system_clock_ticks_for_0x0800ms		RTOS_peripheral_system_clock_ticks_for_time(0x0800)
#define RTOS_peripheral_system_clock_ticks_for_0x0400ms		RTOS_peripheral_system_clock_ticks_for_time(0x0400)
#define RTOS_peripheral_system_clock_ticks_for_0x0200ms		RTOS_peripheral_system_clock_ticks_for_time(0x0200)
#define RTOS_peripheral_system_clock_ticks_for_0x0100ms		RTOS_peripheral_system_clock_ticks_for_time(0x0100)
#define RTOS_peripheral_system_clock_ticks_for_0x0080ms		RTOS_peripheral_system_clock_ticks_for_time(0x0080)
#define RTOS_peripheral_system_clock_ticks_for_0x0040ms		RTOS_peripheral_system_clock_ticks_for_time(0x0040)
#define RTOS_peripheral_system_clock_ticks_for_0x0020ms		RTOS_peripheral_system_clock_ticks_for_time(0x0020)
#define RTOS_peripheral_system_clock_ticks_for_0x0010ms		RTOS_peripheral_system_clock_ticks_for_time(0x0010)
#define RTOS_peripheral_system_clock_ticks_for_0x0008ms		RTOS_peripheral_system_clock_ticks_for_time(0x0008)
#define RTOS_peripheral_system_clock_ticks_for_0x0004ms		RTOS_peripheral_system_clock_ticks_for_time(0x0004)
#define RTOS_peripheral_system_clock_ticks_for_0x0002ms		RTOS_peripheral_system_clock_ticks_for_time(0x0002)
#define RTOS_peripheral_system_clock_ticks_for_0x0001ms		RTOS_peripheral_system_clock_ticks_for_time(0x0001)





#ifndef SPDR
#define SPDR	SPDR0
#endif

#ifndef SPCR
#define SPCR	SPCR0
#endif

#ifndef SPSR
#define SPSR	SPSR0
#endif

#ifndef SPIE
#define SPIE	SPIE0
#endif

#ifndef SPE
#define SPE	SPE0
#endif

#ifndef DORD
#define DORD	DORD0
#endif

#ifndef MSTR
#define MSTR	MSTR0
#endif

#ifndef CPOL
#define CPOL	CPOL0
#endif

#ifndef CPHA
#define CPHA	CPHA0
#endif

#ifndef SPR0
#define SPR0	SPR00
#endif

#ifndef SPR1
#define SPR1	SPR01
#endif

#ifndef SPIF
#define SPIF	SPIF0
#endif

#ifndef WCOL
#define WCOL	WCOL0
#endif

#ifndef SPI2X
#define SPI2X	SPI2X0
#endif

#define SPI_PORT	PORTB
#define SPI_DDR		DDRB
#define SPI_PIN		PINB
#define SPI_SS		PB4
#define SPI_MOSI	PB5
#define SPI_MISO	PB6
#define SPI_SCK		PB7


#define TWI_PORT	PORTC
#define TWI_DDR		DDRC
#define TWI_PIN		PINC
#define TWI_SCL		PC0
#define TWI_SDA		PC1

#define RTOS_peripheral_system_timer_vect TIMER2_COMPA_vect

void rtos_peripheral_switch_on(rtos_peripheral_t periph);


#define __rtos_peripheral_on(status_reg, periph)({\
	*status_reg |= _BV(periph) | (periph == _AC ? _BV(_ADC) : 0x00);\
	PRR0 = ~(uint8_t)*status_reg;\
	PRR1 = ~(uint8_t)(*status_reg>>0x08);\
})


#define __rtos_peripheral_off(status_reg, periph)({\
	*status_reg &= ~_BV(periph);\
	PRR0 = ~(uint8_t)*status_reg;\
	PRR1 = ~(uint8_t)(*status_reg>>0x08);\
})


inline void __rtos_peripheral_ports_init(void)
{
	PORTA = 0x00;
	PORTB = 0x00;
	PORTC = 0x00;
	PORTD = 0x0F;				//usart has to be 1

	DDRA = 0xFF;
	DDRB = 0xFF;
	DDRC  = 0x3F;				//rtos clock
	DDRD = 0xFF;
}


inline void __rtos_peripheral_init(void)
{
	ACSR 	= _BV(ACD);						//TURN OFF THE ANALOG COMPARATOR
	PRR0 	= 0xFF;							//TURN OFF ALL PERIPHERALS
	PRR1 	= 0xFF;							//TURN OFF ALL PERIPHERALS
	DIDR0	= 0x00;
	DIDR1	= 0x00;
#ifndef RUN_TESTS
		rtos_peripheral_switch_on(_TIMER2);
	
		TIMSK2	= 0x00;
	#ifndef RUN_SIMULATOR
		#if BOARD_has_external_clock_input == TRUE
			ASSR = _BV(EXCLK);
		#endif
		ASSR |= _BV(AS2);
		TCCR2B	= _BV(CS20);
	#else
		TCCR2B	= _BV(CS21) | _BV(CS20);
	#endif
		TCNT2	= 0x00;
		OCR2A   = RTOS_peripheral_system_clock_OCRA;
		TCCR2A  = _BV(WGM21);				//CTC on OCR0A
		TIFR2	= 0x00;
	#ifndef RUN_SIMULATOR
		while(ASSR & (_BV(TCN2UB) | _BV(OCR2AUB) | _BV(TCR2AUB) | _BV(TCR2BUB)));
	#endif
		TIMSK2	= _BV(OCIE2A);
#endif
}


inline uint8_t volatile *__rtOS_GetPORT(uint8_t PinNum)		
{														
	if( (!PinNum) || (PinNum > 44) )return (void *)0x00;

	if( (PinNum < 4) 	|| (PinNum > 39) )return &PORTB;
	if( (PinNum > 8) 	&& (PinNum < 17) )return &PORTD;
	if( (PinNum > 18) 	&& (PinNum < 27) )return &PORTC;
	if( (PinNum > 29) 	&& (PinNum < 38) )return &PORTA;	

	return (void *)0x00;
}

inline uint8_t volatile *__rtOS_GetPIN(uint8_t PinNum)		
{														
	if( (!PinNum) || (PinNum > 44) )return (void *)0x00;

	if( (PinNum < 4) 	|| (PinNum > 39) )return &PINB;
	if( (PinNum > 8) 	&& (PinNum < 17) )return &PIND;
	if( (PinNum > 18) 	&& (PinNum < 27) )return &PINC;
	if( (PinNum > 29) 	&& (PinNum < 38) )return &PINA;	

	return (void *)0x00;
}

inline uint8_t volatile *__rtOS_GetDDR(uint8_t PinNum)		
{														
	if( (!PinNum) || (PinNum > 44) )return (void *)0x00;

	if( (PinNum < 4) 	|| (PinNum > 39) )return &DDRB;
	if( (PinNum > 8) 	&& (PinNum < 17) )return &DDRD;
	if( (PinNum > 18) 	&& (PinNum < 27) )return &DDRC;
	if( (PinNum > 29) 	&& (PinNum < 38) )return &DDRA;	

	return (void *)0x00;
}

inline uint8_t __rtOS_GetPinPort(uint8_t PinNum)		
{														

	if(!PinNum)return 0x00;

	if(PinNum < 4) return PinNum += 4;
	if( (PinNum > 8) 	&& (PinNum < 17) ) return PinNum - 9;
	if( (PinNum > 18) 	&& (PinNum < 27) ) return PinNum - 19;
	if( (PinNum > 29) 	&& (PinNum < 38) ){
		int8_t pin = PinNum - 37;

		return -pin;
	}
	if(PinNum > 39)						   return PinNum - 40;
		
	return 0;
}

#endif
