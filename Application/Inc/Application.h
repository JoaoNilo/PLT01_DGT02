//==============================================================================
/**
 * @file Application.cpp
 * @title Digit Controller Module - Placar Tenis PLT-01
 * @author Joao Nilo Rodrigues  -  nilo@pobox.com
 */
//------------------------------------------------------------------------------
#ifndef APPLICATION_H
#define __APPLICATION_H

#include <stdio.h>

#include "NLed.h"
#include "NSwitch.h"
#include "NSerial.h"
#include "NTimer.h"
#include "NTinyOutput.h"
#include "NAdc.h"
#include "NFilter.h"
#include "NAnalogParameter.h"

#include "NDataLink.h"
#include "NSerialProtocol.h"

//------------------------------------------------------------------------------
#define BUS_PORT		USART1, seStandard
#define BLE_PORT		USART2, seStandard
#define MEM_PORT		I2C1
#define TIMEBASE		TIM2


#define LED				GPIOA, (uint32_t)1

#define ADDR0			GPIOA, (uint32_t)3
#define ADDR1			GPIOA, (uint32_t)4
#define ADDR2			GPIOA, (uint32_t)5
#define ADDR3			GPIOA, (uint32_t)6
#define ADDR4			GPIOA, (uint32_t)7

#define USART1_CTS		GPIOA,  (uint32_t)11
#define USART1_RTS		GPIOA,  (uint32_t)12

#define SEGMENT_A		GPIOB, (uint32_t)0
#define SEGMENT_B		GPIOB, (uint32_t)1
#define SEGMENT_C		GPIOB, (uint32_t)2
#define SEGMENT_D		GPIOB, (uint32_t)3
#define SEGMENT_E		GPIOB, (uint32_t)4
#define SEGMENT_F		GPIOB, (uint32_t)5
#define SEGMENT_G		GPIOB, (uint32_t)6
#define SEGMENT_H		GPIOB, (uint32_t)7

#define DRV_VR			GPIOB, (uint32_t)12
#define DRV_HR			GPIOC, (uint32_t)14

//------------------------------------------------------------------------------
#define FSM_IDLE		0
#define FSM_GETSTATUS	1
#define FSM_SETDATA		2

//------------------------------------------------------------------------------
#define BUS_NODES		10
#define PLAY1_TENS		((uint8_t) 0x1E)
#define PLAY1_UNITS		((uint8_t) 0x1D)
#define PLAY1_SET1		((uint8_t) 0x1C)
#define PLAY1_SET2		((uint8_t) 0x1B)
#define PLAY1_SET3		((uint8_t) 0x1A)
#define PLAY2_TENS		((uint8_t) 0x19)
#define PLAY2_UNITS		((uint8_t) 0x18)
#define PLAY2_SET1		((uint8_t) 0x17)
#define PLAY2_SET2		((uint8_t) 0x16)
#define PLAY2_SET3		((uint8_t) 0x15)
#define MATCH_SECONDS	((uint8_t) 0x14)
#define MATCH_MINUTES	((uint8_t) 0x13)
#define MATCH_HOURS		((uint8_t) 0x12)

//------------------------------------------------------------------------------
#define INDEX_PLAY1_TENS		((uint8_t) 0x00)
#define INDEX_PLAY1_UNITS		((uint8_t) 0x01)
#define INDEX_PLAY1_SET1		((uint8_t) 0x02)
#define INDEX_PLAY1_SET2		((uint8_t) 0x03)
#define INDEX_PLAY1_SET3		((uint8_t) 0x04)
#define INDEX_PLAY2_TENS		((uint8_t) 0x05)
#define INDEX_PLAY2_UNITS		((uint8_t) 0x06)
#define INDEX_PLAY2_SET1		((uint8_t) 0x07)
#define INDEX_PLAY2_SET2		((uint8_t) 0x08)
#define INDEX_PLAY2_SET3		((uint8_t) 0x09)

//------------------------------------------------------------------------------
#define DELAY_TENS		((uint16_t) 0)
#define DELAY_UNITS		((uint16_t) 500)
#define DELAY_SET1		((uint16_t) 1000)
#define DELAY_SET2		((uint16_t) 1500)
#define DELAY_SET3		((uint16_t) 2000)

//------------------------------------------------------------------------------
/*
#define FLAG_STATUS_PLAY1_TENS		((uint16_t) 0x0001)
#define FLAG_STATUS_PLAY1_UNITS		((uint16_t) 0x0002)
#define FLAG_STATUS_PLAY1_SET1		((uint16_t) 0x0004)
#define FLAG_STATUS_PLAY1_SET2		((uint16_t) 0x0008)
#define FLAG_STATUS_PLAY1_SET3		((uint16_t) 0x0010)
#define FLAG_STATUS_PLAY2_TENS		((uint16_t) 0x0020)
#define FLAG_STATUS_PLAY2_UNITS		((uint16_t) 0x0040)
#define FLAG_STATUS_PLAY2_SET1		((uint16_t) 0x0080)
#define FLAG_STATUS_PLAY2_SET2		((uint16_t) 0x0100)
#define FLAG_STATUS_PLAY2_SET3		((uint16_t) 0x0200)
#define FLAG_STATUS_CALIBRATING		((uint16_t) 0x8000)
*/

#endif /* __APPLICATION_H */
//==============================================================================
