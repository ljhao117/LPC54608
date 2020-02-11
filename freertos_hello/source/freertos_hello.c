/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"

#include "fsl_gint.h"
#include "fsl_usart_freertos.h"
#include "fsl_usart.h"
#include "fsl_ctimer.h"
#include "fsl_power.h"
#include "fsl_adc.h"
#include "fsl_crc.h"
#include "fsl_inputmux_connections.h"
#include "fsl_pint.h"
#include "fsl_inputmux.h"
#include "fsl_flashiap.h"
#include "fsl_wwdt.h"
#include "fsl_power.h"
#include "fsl_usart_dma.h"
#include "fsl_dma.h"
#include "fsl_spifi_dma.h"
#include "fsl_rtc.h"
#include "fsl_mrt.h"
#include "fsl_otp.h"
#include "fsl_clock.h"
#include "fsl_reset.h"

#include "pin_mux.h"
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Task priorities. */
#define tick_task_PRIORITY (configMAX_PRIORITIES - 4)
/* LED */
#define LED1_PORT 3u
#define LED1_PIN 14u
#define LED2_PORT 3u
#define LED2_PIN 3u
#define LED3_PORT 2u
#define LED3_PIN 2u
/* GINT */
#define GINT0_PORT kGINT_Port0
#define GINT0_POL_MASK ~(1u << BOARD_SW2_GPIO_PIN)
#define GINT0_ENA_MASK (1u << BOARD_SW2_GPIO_PIN)
/* vTaskDelayUntil */
#define portTICK_RATE_MS ((TickType_t)1000 / configTICK_RATE_HZ)
#define portTickType TickType_t
/* pint */
#define SW3_PINT kINPUTMUX_GpioPort0Pin5ToPintsel
/* polling interrupt_transfer */
#define DEMO_USART USART0
#define DEMO_USART_CLK_SRC kCLOCK_Flexcomm0
#define DEMO_USART_CLK_FREQ CLOCK_GetFlexCommClkFreq(0)
#define ECHO_BUFFER_LENGTH 8

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void tick_task(void *pvParameters);
static void GPCHAR_task(void *pvParameters);
void GPIO_Init(void);
void GPIO_Interrupt_Init(void);
void gint0_callback(void);
void PinInt0_Callback(pint_pin_int_t pin, uint32_t pmatch_status);
void Task_Create(void);
/* USART user callback interrupt_transfer */
void USART_UserCallback(USART_Type *base, usart_handle_t *handle, status_t status, void *userData);

/*******************************************************************************
 * Globals Variables
 ******************************************************************************/
/* polling */
uint8_t txbuff[] = "Usart polling example \r\nBoard will send back received characters\r\n";
uint8_t rxbuff[20] = {0};
/* interrupt_transfer */
usart_handle_t g_uartHandle;
uint8_t g_tipString[] = "Usart interrupt\rBoard receives 8 characters then sends themout\r\nNow please input:\r\n";
uint8_t g_txBuffer[ECHO_BUFFER_LENGTH] = {0};
uint8_t g_rxBuffer[ECHO_BUFFER_LENGTH] = {0};
volatile bool rxBufferEmpty = true;
volatile bool txBufferFull = false;
volatile bool txOnGoing = false;
volatile bool rxOnGoing = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Application entry point.
 */
int main(void)
{
    usart_config_t config;
    usart_transfer_t xfer;
    usart_transfer_t sendXfer;
    usart_transfer_t receiveXfer;

    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    /* 180MHz */
    BOARD_BootClockPLL180M();
    BOARD_InitDebugConsole();

    USART_GetDefaultConfig(&config);
    config.baudRate_Bps = BOARD_DEBUG_UART_BAUDRATE;
    config.enableTx = true;
    config.enableRx = true;

    USART_Init(DEMO_USART, &config, DEMO_USART_CLK_FREQ);
    USART_TransferCreateHandle(DEMO_USART, &g_uartHandle, USART_UserCallback, NULL);

    /* Send g_tipString out */
    xfer.data = g_tipString;
    xfer.dataSize = sizeof(g_tipString) - 1;
    txOnGoing = true;
    USART_TransferSendNonBlocking(DEMO_USART, &g_uartHandle, &xfer);

    /* Wait send finished */
    while (txOnGoing)
    {
    }

    /* Start to echo */
    sendXfer.data = g_txBuffer;
    sendXfer.dataSize = sizeof(g_txBuffer);
    receiveXfer.data = g_rxBuffer;
    receiveXfer.dataSize = sizeof(g_rxBuffer);

    GPIO_Init();
    GPIO_Interrupt_Init();
    Task_Create();

    // vTaskStartScheduler();
    PRINTF("Hello World\r\n");
    while (1)
    {
        /* If RX is idle and g_rxBuffer is empty, start to read data to g_rxBuffer */
        if ((!rxOnGoing) && rxBufferEmpty)
        {
            rxOnGoing = true;
            USART_TransferReceiveNonBlocking(DEMO_USART, &g_uartHandle, &receiveXfer, NULL);
        }
        /* If Tx is idle and g_txBufferis full, start to send data */
        if ((!txOnGoing) && txBufferFull)
        {
            txOnGoing = true;
            USART_TransferSendNonBlocking(DEMO_USART, &g_uartHandle, &sendXfer);
        }
        /* If g_txBuffer is empty and g_rxBuffer is full, copy g_rxBuffer to g_txBuffer */
        if ((!rxBufferEmpty) && (!txBufferFull))
        {
            memcpy(g_txBuffer, g_rxBuffer, ECHO_BUFFER_LENGTH);
            rxBufferEmpty = true;
            txBufferFull = true;
        }
    }
}

void GPIO_Init(void)
{
    gpio_pin_config_t led_config = {
        kGPIO_DigitalOutput,
        0,
    };
    GPIO_PortInit(GPIO, LED1_PORT);
    GPIO_PortInit(GPIO, LED2_PORT);
    GPIO_PortInit(GPIO, LED3_PORT);

    GPIO_PinInit(GPIO, LED1_PORT, LED1_PIN, &led_config);
    GPIO_PinInit(GPIO, LED2_PORT, LED2_PIN, &led_config);
    GPIO_PinInit(GPIO, LED3_PORT, LED3_PIN, &led_config);

    GPIO_PinWrite(GPIO, LED1_PORT, LED1_PIN, 1);
    GPIO_PinWrite(GPIO, LED2_PORT, LED2_PIN, 1);
    GPIO_PinWrite(GPIO, LED3_PORT, LED3_PIN, 1);
}

void GPIO_Interrupt_Init(void)
{
    GINT_Init(GINT0);
    GINT_SetCtrl(GINT0, kGINT_CombineOr, kGINT_TrigEdge, gint0_callback);
    GINT_ConfigPins(GINT0, GINT0_PORT, GINT0_POL_MASK, GINT0_ENA_MASK);
    GINT_EnableCallback(GINT0);

    /* Connect trigger sources to PINT */
    INPUTMUX_Init(INPUTMUX);
    INPUTMUX_AttachSignal(INPUTMUX, kPINT_PinInt0, SW3_PINT);
    /* Turn off clock to inputmux to save power,
    clock is only needed to make changes */
    INPUTMUX_Deinit(INPUTMUX);
    /* Initialize PINT */
    PINT_Init(PINT);
    /* Setup pin interrupt 0 for rising edge */
    PINT_PinInterruptConfig(PINT, kPINT_PinInt0,
                            kPINT_PinIntEnableFallEdge, PinInt0_Callback);
    /* Enable callbacks for PINT0 by Index */
    PINT_EnableCallbackByIndex(PINT, kPINT_PinInt0);
}

void Task_Create(void)
{
    if (xTaskCreate(tick_task,
                    "tick_task",
                    configMINIMAL_STACK_SIZE + 100,
                    NULL,
                    tick_task_PRIORITY,
                    NULL) != pdPASS)
    {
        PRINTF("Hello task creation failed!.\r\n");
        while (1)
            ;
    }

    if (xTaskCreate(GPCHAR_task,
                    "GPCHAR_task",
                    configMINIMAL_STACK_SIZE + 100,
                    NULL,
                    tick_task_PRIORITY,
                    NULL) != pdPASS)
    {
        PRINTF("GPCHAR task creation failed!.\r\n");
        while (1)
            ;
    }
}

/*!
 * @brief tick_task
 */
static void tick_task(void *pvParameters)
{
    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    for (;;)
    {
        PRINTF("TickCount is %d\r\n", xLastWakeTime);
        GPIO_PortToggle(GPIO, LED1_PORT, 1u << LED1_PIN);
        vTaskDelayUntil(&xLastWakeTime, (1000 / portTICK_RATE_MS));
    }
}

/**
 * @brief GPCHAR_task
 */
static void GPCHAR_task(void *pvParameters)
{

    PRINTF("Hello World.\r\n");
    char ch;
    while (1)
    {

        ch = GETCHAR();
        PUTCHAR(ch);
    }
}

void gint0_callback(void)
{
    uint32_t PortStatus = GPIO_PortRead(GPIO, LED1_PORT);
    if (PortStatus == 0xffffffff)
    {
        PRINTF("LED1 is on\r\n");
        GPIO_PortToggle(GPIO, LED1_PORT, 1u << LED1_PIN);
    }
    else
    {
        PRINTF("LED1 is off\r\n");
        GPIO_PortToggle(GPIO, LED1_PORT, 1u << LED1_PIN);
    }
}

void PinInt0_Callback(pint_pin_int_t pin, uint32_t pmatch_status)
{
    uint32_t PortStatus = GPIO_PortRead(GPIO, LED2_PORT);
    if (PortStatus == 0xffffffff)
    {
        PRINTF("LED2 is on\r\n");
        GPIO_PortToggle(GPIO, LED2_PORT, 1u << LED2_PIN);
    }
    else
    {
        PRINTF("LED2 is off\r\n");
        GPIO_PortToggle(GPIO, LED2_PORT, 1u << LED2_PIN);
    }
}

/* USART user callback */
void USART_UserCallback(USART_Type *base, usart_handle_t *handle, status_t status, void *userData)
{
    userData = userData;
    if (kStatus_USART_TxIdle == status)
    {
        txBufferFull = false;
        txOnGoing = false;
    }

    if (kStatus_USART_RxIdle == status)
    {
        rxBufferEmpty = false;
        rxOnGoing = false;
    }
}
