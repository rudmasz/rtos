#ifndef __ERRCODE_H_						
#define	__ERRCODE_H_

/**************************************************************************
						Error definitions 	
**************************************************************************/

/*		Emergency			Meaning
		Error Code
									
		50xxxxxx			Device Hardware
		60xxxxxx			Device Software
		80xxxxxx			Monitoring
		FFxxxxxx			Device specific

		NOTE!!!! THE TWO LEAST SIGNIFICANT BYTES ARE SEEN BY THE RTOS AS A PLACE TO ADD THE ADDRESS OF THE CURRENT TASK (RESPONSIBLE FOR THE ERROR THAT OCCURRED)
		THUS, ERROR CODES THAT ARE NOT RELATED TO A FAULTY TASK OPERATION SHOULD HAVE SOME VALUE IN THIS POSITION, E.G.
					
		#define __Err_Current_DevInputS							0x20000001	- THIS ERROR IS NOT CAUSED BY AN INCORRECT TASK OPERATION. VALUE ONE IN THE FOUR YOUNGER BYTES
		#define __Err_DeviceSoftware_rtOS_SemaphoreOwner		0x60000000	- THE AUTHOR OF THE ERROR IS TASK, THE SYSTEM WILL ADD ITS ADDRESS IN THE POSITION OF THE FOUR LEAST SIGNIFICANT BYTES
*/


/**************************************************************************
					Device Software		
**************************************************************************/
#define __Err_DeviceSoftware		0x0000000
	
	#define __Err_DeviceSoftware_rtOS			0x00000000		//System Error


 		#define __Err_DeviceSoftware_rtOS_SemaphoreOwner		0x00000000	//Task is not the owner of the semaphore
 		#define __Err_DeviceSoftware_rtOS_SemaphoreCount		0x00010000	//Semaphore was release over the state 
 		#define __Err_DeviceSoftware_rtOS_ParentReset			0x00020000	//Task is not parent of the reset task
 		#define __Err_DeviceSoftware_rtOS_DynamicMemory			0x00030000	//No free memory
 		#define __Err_DeviceSoftware_rtOS_DynamicMemoryBlockS	0x00040000	//Too big block
 		#define __Err_DeviceSoftware_rtOS_StackOverflowUp		0x00050000	//Stack overflow by task too many variables in task definition
 		#define __Err_DeviceSoftware_rtOS_StackOverflowDown		0x00060000	//Stack overflow by functions called from task too many variables in a functions definition or to many references to the function


	#define __Err_DeviceSoftware_				0x00800000

/**************************************************************************
					Device Hardware		
**************************************************************************/
#define __Err_DeviceHardware		0xC0000000
	
	#define __Err_DeviceHardware_DS				0xC0000000		//ONE WIRE MAXIM DEVICES

		#define __Err_DeviceHardware_DS_CRC					0xC0000001	//Crc error
		#define __Err_DeviceHardware_DS_NoDevFound			0xC0000002	//No Device found
		#define __Err_DeviceHardware_DS_Line				0xC0000003	//short circuit on the line
		#define __Err_DeviceHardware_DS_NoAccess			0xC0000004	//access denied task is not the owner of the 1wire semaphore
		#define __Err_DeviceHardware_DS_LineOff				0xC0000005	//1wire has been turned off
		#define __Err_DeviceHardware_DS_BufferExc			0xC0000005	//internal buffer has been exceeded
		#define __Err_DeviceHardware_DS_TooManyDevFound		0xC0000100	//Too many Device found
		#define __Err_DeviceHardware_DS_AllGroupFail		0xC0000200	//Group Device fail


	#define __Err_DeviceHardware_FlashMem		0xC0001000		//Flash memory

		#define __Err_DeviceHardware_FlashMem_Communication		0xC0001000	//No communication with flash memory
		#define __Err_DeviceHardware_FlashMem_BadID				0xC0001001	//Not recognize Flash memory 
		#define __Err_DeviceHardware_FlashMem_Format			0xC0001002	//Flash memory is not properly formatted
		#define __Err_DeviceHardware_FlashMem_NoAccess			0xC0001003	//Attempt to access a special area without permission
		#define __Err_DeviceHardware_FlashMem_WriteFailed		0xC0001004	//Attempt to write failed

	#define __Err_DeviceHardware_TWI							0xC0001020	//Two wire serial interface


	#define __Err_DeviceHardware_ENC28J60						0xC0001040	//Stand-Alone Ethernet Controller ENC28J60

		#define __Err_DeviceHardware_ENC28J60_SiliconRevision	0xC0001040	//Unsupported Silicon Revision or device connection error
		#define __Err_DeviceHardware_ENC28J60_PhyIdentifier		0xC0001041	//PHY identifier not recognized


/**************************************************************************
					Monitoring		
**************************************************************************/
#define __Err_Monitoring			0xF0000000

	#define __Err_Monitoring_Modbus				0xF0000100
		#define __Err_Monitoring_Modbus_Communication		0xF0001000		//Communication
		
			#define __Err_Monitoring_Modbus_Communication_Baudrate			0xF0001000	//Baudrate error
			#define __Err_Monitoring_Modbus_Communication_CRC16				0xF0001001	//Cyclic redundancy check
			#define __Err_Monitoring_Modbus_Communication_Overun			0xF0001002	//1 Overrun (Objects lost)
			#define __Err_Monitoring_Modbus_Communication_Parity			0xF0001003	//Parity error
			#define __Err_Monitoring_Modbus_Communication_StopBit			0xF0001004	//Stop bit error
			#define __Err_Monitoring_Modbus_Communication_RxBufferOverflow	0xF0001005	//Receive Buffer overflow


	#define __Err_Monitoring_Modbus_Protocol				0xF0001800		//Protocol Error
			
			#define __Err_Monitoring_Modbus_Protocol_Stack					0xF0001800
		    #define __Err_Monitoring_Modbus_Protocol_IllegalFunction		0xF0001801
		    #define __Err_Monitoring_Modbus_Protocol_IllegalDataAddress		0xF0001802
		    #define __Err_Monitoring_Modbus_Protocol_IllegalDataValue		0xF0001803
		    #define __Err_Monitoring_Modbus_Protocol_SlaveDeviceFailure		0xF0001804
		    #define __Err_Monitoring_Modbus_Protocol_Acknowledge			0xF0001805
		    #define __Err_Monitoring_Modbus_Protocol_SlaveBusy				0xF0001806
		    #define __Err_Monitoring_Modbus_Protocol_MemoryParityError		0xF0001808
		    #define __Err_Monitoring_Modbus_Protocol_GatewayPathFailed		0xF000180A
		    #define __Err_Monitoring_Modbus_Protocol_GstewayTgtFailed		0xF000180B
		    #define __Err_Monitoring_Modbus_Protocol_SlaveTimeout			0xF000180C
		    #define __Err_Monitoring_Modbus_Protocol_UnexpectedPDU			0xF000180D


	#define __Err_Monitoring_Ethernet			0xF0002000
		#define __Err_Monitoring_Ethernet_Communication		0xF0002000		//Communication

			#define __Err_Monitoring_Ethernet_Communication_TxStatusVector_0		0xF0002000		//Transmit Status Vector			
			#define __Err_Monitoring_Ethernet_Communication_TxStatusVector_1		0xF0002100		//Transmit Status Vector			
			#define __Err_Monitoring_Ethernet_Communication_TxStatusVector_2		0xF0002200		//Transmit Status Vector			
			#define __Err_Monitoring_Ethernet_Communication_TxStatusVector_3		0xF0002300		//Transmit Status Vector			
			#define __Err_Monitoring_Ethernet_Communication_TxStatusVector_4		0xF0002400		//Transmit Status Vector			
			#define __Err_Monitoring_Ethernet_Communication_TxStatusVector_5		0xF0002500		//Transmit Status Vector			
			#define __Err_Monitoring_Ethernet_Communication_TxStatusVector_6		0xF0002600		//Transmit Status Vector			
			#define __Err_Monitoring_Ethernet_Communication_RxStatusVector_0		0xF0002700		//Receive Status Vector			
			#define __Err_Monitoring_Ethernet_Communication_RxStatusVector_1		0xF0002800		//Receive Status Vector			
			#define __Err_Monitoring_Ethernet_Communication_RxStatusVector_2		0xF0002900		//Receive Status Vector			
			#define __Err_Monitoring_Ethernet_Communication_RxStatusVector_3		0xF0002A00		//Receive Status Vector			
			#define __Err_Monitoring_Ethernet_Communication_RxBufferOverflow		0xF0002B00		//Receive Buffer overflow
			#define __Err_Monitoring_Ethernet_Communication_LinkStatus				0xF0002B01		//Link Status error
			#define __Err_Monitoring_Ethernet_Communication_TxTimeout				0xF0002B02		//Transmit Timeout			
			#define __Err_Monitoring_Ethernet_Communication_Reset					0xF0002B03		//ETH Reset


		#define __Err_Monitoring_Ethernet_Network_Interface	0xF0003000		//Network Interface

			#define __Err_Monitoring_Ethernet_Network_Interface_Dynamic_Memory		0xF0003000		//No dynamic memory packet dropped


		#define __Err_Monitoring_Ethernet_Protocol			0xF0004000		//Protocol Error

			#define __Err_Monitoring_Ethernet_Protocol_ETH							0xF0004000
				#define __Err_Monitoring_Ethernet_Protocol_ETH_Length				0xF0004001		//ETH packet length
				#define __Err_Monitoring_Ethernet_Protocol_ETH_Type					0xF0004002		//ETH unsupported protocol type
		
			#define __Err_Monitoring_Ethernet_Protocol_ARP							0xF0004020
				#define __Err_Monitoring_Ethernet_Protocol_ARP_Length				0xF0004020		//ARP packet length
				#define __Err_Monitoring_Ethernet_Protocol_ARP_Packet				0xF0004021		//ARP packet format unsupported protocol type or hardware type or protocol addr length or hardware addr length
				#define __Err_Monitoring_Ethernet_Protocol_ARP_Type					0xF0004022		//ARP unsupported type operation
			
	//		#define ..		

			#define __Err_Monitoring_Ethernet_Protocol_ICMP							0xF0004100
				#define __Err_Monitoring_Ethernet_Protocol_ICMP_UnsupportedType		0xF0004100		//ICMP UNSUPPORTED TYPE OF MESSAGE, TYPE IS ORED WITH THIS ERR CODE SO ERR CODE COULD BE FROM 0xF0004100 TO 0xF00041FF
				#define __Err_Monitoring_Ethernet_Protocol_ICMP_HeaderSizeErr		0xF0004200		//ICMP HEADER SIZE TOO SMALL
				#define __Err_Monitoring_Ethernet_Protocol_ICMP_MultiBroadcast		0xF0004201		//ICMP MESSAGE CAN NOT BE SENT ON BROADCAST OR MULTICAST ADDRESS
				#define __Err_Monitoring_Ethernet_Protocol_ICMP_Checksum			0xF0004202		//ICMP CHECKSUM ERROR


/**************************************************************************
					Device specific		
**************************************************************************/
#define __Err_DeviceSpecific		0xFF000000






#endif
