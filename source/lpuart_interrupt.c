/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_lpuart.h"

#include "fsl_clock.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LIN_COM0

#ifdef LIN_COM0
#define DEMO_LIN            LPUART0
#define DEMO_LIN_CLK_FREQ   12000000U
#define DEMO_LIN_IRQn       LP_FLEXCOMM0_IRQn
#define DEMO_LIN_IRQHandler LP_FLEXCOMM0_IRQHandler
#define DisableLinBreak LPUART0->STAT &= ~(LPUART_STAT_LBKDE_MASK);
#define EnableLinBreak  LPUART0->STAT |= LPUART_STAT_LBKDE_MASK;
#endif

#ifdef LIN_COM2
#define DEMO_LIN            LPUART2
#define DEMO_LIN_CLK_FREQ   12000000U
#define DEMO_LIN_IRQn       LP_FLEXCOMM2_IRQn
#define DEMO_LIN_IRQHandler LP_FLEXCOMM2_IRQHandler
#define DisableLinBreak LPUART2->STAT &= ~(LPUART_STAT_LBKDE_MASK);
#define EnableLinBreak  LPUART2->STAT |= LPUART_STAT_LBKDE_MASK;
#endif
/*! @brief Ring buffer size (Unit: Byte). */
#define DEMO_RING_BUFFER_SIZE 16



#define IDLE                0x00          /**< IDLE state */
#define SEND_BREAK          0x01          /**< Send break field state */
#define SEND_PID            0x02          /**< send PID state */
#define RECV_SYN            0x03          /**< receive synchronize state */
#define RECV_PID            0x04          /**< receive PID state */
#define IGNORE_DATA         0x05          /**< ignore data state */
#define RECV_DATA           0x06          /**< receive data state */
#define SEND_DATA           0x07          /**< send data state */
#define SEND_DATA_COMPLETED 0x08          /**< send data completed state */
#define PROC_CALLBACK       0x09          /**< proceduce callback state */
#define SLEEP_MODE          0x0A          /**< sleep mode state */
#define UNINIT              0xFF          /**< uninitialize state */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*
  Ring buffer for data input and output, in this example, input data are saved
  to ring buffer in IRQ handler. The main function polls the ring buffer status,
  if there are new data, then send them out.
  Ring buffer full: (((rxIndex + 1) % DEMO_RING_BUFFER_SIZE) == txIndex)
  Ring buffer empty: (rxIndex == txIndex)
*/
uint8_t demoRingBuffer[DEMO_RING_BUFFER_SIZE];
volatile uint16_t txIndex; /* Index of the data to send out. */
volatile uint16_t rxIndex; /* Index of the memory to save new arrived data. */
uint8_t rxbuff[20] = {0};
uint16_t cnt=0, recdatacnt=0;
uint8_t Lin_BKflag=0;
static uint8_t          state = UNINIT;

/*******************************************************************************
 * Code
 ******************************************************************************/

void DEMO_LIN_IRQHandler(void)
{
    uint8_t data;
    uint16_t tmprxIndex = rxIndex;
    uint16_t tmptxIndex = txIndex;

    if (DEMO_LIN->STAT & LPUART_STAT_LBKDIF_MASK)
    {
    	DEMO_LIN->STAT |= LPUART_STAT_LBKDIF_MASK;// clear the bit
        Lin_BKflag = 1;
        cnt = 0;
        state = RECV_SYN;
        DisableLinBreak;
    }
    if (DEMO_LIN->STAT & LPUART_STAT_RDRF_MASK)
    {
      	 rxbuff[cnt] = (uint8_t)((DEMO_LIN->DATA) & 0xff);
		switch(state)
		{
		   case RECV_SYN:
			 if(0x55 == rxbuff[cnt])
			 {
				 state = RECV_PID;
			 }
			 else
			 {
				 state = IDLE;
				 DisableLinBreak;
			 }
			 break;
		   case RECV_PID:
			 if(0xAD == rxbuff[cnt])
			 {
				 state = RECV_DATA;
			 }
			 else if(0xEC == rxbuff[cnt])
			 {
				 state = SEND_DATA;
			 }
			 else
			 {
				 state = IDLE;
				 DisableLinBreak;
			 }
			 break;
		   case RECV_DATA:
			 recdatacnt++;
			 if(recdatacnt >= 4) // 3 Bytes data + 1 Bytes checksum
			 {
				 recdatacnt=0;
				 state = IDLE;
				 EnableLinBreak;
			 }
			 break;
		   default:break;
		}
		cnt++;

    }

    SDK_ISR_EXIT_BARRIER;
}

void uart_LIN_break( LPUART_Type *base )
{
	base->CTRL &= ~(LPUART_CTRL_TE_MASK | LPUART_CTRL_RE_MASK);   //Disable UART0 first
	base->STAT |= LPUART_STAT_BRK13_MASK; //13 bit times
	base->STAT |= LPUART_STAT_LBKDE_MASK;//LIN break detection enable
	base->BAUD |= LPUART_BAUD_LBKDIE_MASK;

	base->CTRL |= (LPUART_CTRL_TE_MASK | LPUART_CTRL_RE_MASK);
	base->CTRL |= LPUART_CTRL_RIE_MASK;
	EnableIRQ(DEMO_LIN_IRQn);
}
uint8_t LINCheckSum(uint8_t PID, uint8_t *buf, uint8_t lens);
uint8_t LINCalcParity(uint8_t id);
uint8_t sendbuffer[3] = {0x01, 0x02, 0x10};
/*!
 * @brief Main function
 */
int main(void)
{
    lpuart_config_t config;
    uint16_t tmprxIndex = rxIndex;
    uint16_t tmptxIndex = txIndex;
	uint8_t PID;
    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);
    /* attach FRO 12M to FLEXCOMM3 (LIN bus) */
#ifdef LIN_COM0
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM0);
#endif
#ifdef LIN_COM2
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 1u);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);
#endif
    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
#ifdef LIN_COM0

#ifdef LIN_MASTER
    GPIO_PortSet(BOARD_INITPINS_LIN1_M_GPIO, BOARD_INITPINS_LIN1_M_GPIO_PIN_MASK);
#else
    GPIO_PortClear(BOARD_INITPINS_LIN1_M_GPIO, BOARD_INITPINS_LIN1_M_GPIO_PIN_MASK);
#endif

#endif

#ifdef LIN_COM2

#ifdef LIN_MASTER
    GPIO_PortSet(BOARD_INITPINS_LIN0_M_GPIO, BOARD_INITPINS_LIN0_M_GPIO_PIN_MASK);
#else
    GPIO_PortClear(BOARD_INITPINS_LIN0_M_GPIO, BOARD_INITPINS_LIN0_M_GPIO_PIN_MASK);
#endif

#endif
    /*
     * config.baudRate_Bps = 115200U;
     * config.parityMode = kLPUART_ParityDisabled;
     * config.stopBitCount = kLPUART_OneStopBit;
     * config.txFifoWatermark = 0;
     * config.rxFifoWatermark = 0;
     * config.enableTx = false;
     * config.enableRx = false;
     */
    LPUART_GetDefaultConfig(&config);
    config.baudRate_Bps = 19200;
    config.enableTx     = true;
    config.enableRx     = true;

    LPUART_Init(DEMO_LIN, &config, DEMO_LIN_CLK_FREQ);
    uart_LIN_break( DEMO_LIN );

    while (1)
    {
        if(state == SEND_DATA)
        {
        	PID = LINCalcParity(0x2c);
        	sendbuffer[2] = LINCheckSum( PID, sendbuffer, 2);
        	LPUART_WriteBlocking(DEMO_LIN, sendbuffer, 3);
        	sendbuffer[0]++;
        	sendbuffer[1]++;
           recdatacnt=0;
           state = IDLE;
           EnableLinBreak;
        }
    }
}

uint8_t LINCheckSum(uint8_t PID, uint8_t *buf, uint8_t lens)
{
    uint8_t i, ckm = 0;
    uint16_t chm1 = PID;//Enhanced
//    uint16_t chm1 = 0;//Classic
    for(i = 0; i < lens; i++)
    {
        chm1 += *(buf++);
    }
    ckm = chm1 / 256;
    ckm = ckm + chm1 % 256;
    ckm = 0xFF - ckm;

    return ckm;
}

uint8_t LINCalcParity(uint8_t id)
{
    uint8_t parity, p0,p1;
    parity=id;

    p0=((id & 0x01)^((id & 0x02) >> 1)^((id & 0x04) >> 2)^((id & 0x10) >> 4)) << 6;
    p1=(!((id & 0x02) >> 1)^((id & 0x08) >> 3)^((id & 0x10) >> 4)^((id & 0x20) >> 5)) << 7;
    parity|=(p0|p1);

    return parity;
}
