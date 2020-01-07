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
#include "event_groups.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "fsl_gpio.h"
#include "core_cm4.h"
#include "fsl_gint.h"
#include "fsl_inputmux.h"
#include "fsl_pint.h"

#include "pin_mux.h"
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define Task_PRIORITY 0
#define portTICK_RATE_MS portTICK_PERIOD_MS

/*LED*/
#define LED1_PORT BOARD_LED1_GPIO_PORT
#define LED1_PIN BOARD_LED1_GPIO_PIN
#define LED2_PORT BOARD_LED2_GPIO_PORT
#define LED2_PIN BOARD_LED2_GPIO_PIN
#define LED3_PORT BOARD_LED3_GPIO_PORT
#define LED3_PIN BOARD_LED3_GPIO_PIN

#define DEMO_GINT0_PORT kGINT_Port0
#define DEMO_GINT1_PORT kGINT_Port0

/* Select one input, active low for GINT0 */
#define DEMO_GINT0_POL_MASK ~(1U << BOARD_SW2_GPIO_PIN)
#define DEMO_GINT0_ENA_MASK (1U << BOARD_SW2_GPIO_PIN)

/* Select two inputs, active low for GINT1. SW2 & SW3 must be connected to the same port */
#define DEMO_GINT1_POL_MASK ~((1U << BOARD_SW3_GPIO_PIN) | (1U << BOARD_SW4_GPIO_PIN))
#define DEMO_GINT1_ENA_MASK ((1U << BOARD_SW3_GPIO_PIN) | (1U << BOARD_SW4_GPIO_PIN))

#define SW3_PINT kINPUTMUX_GpioPort0Pin5ToPintsel

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void Task1(void *pvParameters);
//static void Task2(void *pvParameters);
//static void Task3(void *pvParameters);
//static void TaskSW(void *pvParameters);
void TaskCreate(void);
void vApplicationIdleBook(void);
void gint0_callback(void);
void PinInt0_Callback(pint_pin_int_t pin, uint32_t pmatch_status);
void SysTick_DelayTicks(uint32_t n);
void SysTick_Handler(void);
void GPIOInit(void);
void InterruptInit(void);

/*******************************************************************************
 * Globals
 ******************************************************************************/
unsigned long ulIdleCycleCount = 0UL;
TaskHandle_t xTask2Handle;
QueueHandle_t xQueue;
volatile uint32_t g_systickCounter;
uint32_t port_state = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL180M();
    BOARD_InitDebugConsole();
    GPIOInit();
    InterruptInit();
    TaskCreate();

    /* Set systick reload value to generate 1ms interrupt */
    if (SysTick_Config(SystemCoreClock / 1000U))
    {
        while (1)
        {
        }
    }

    /* Start scheduling. 启动RTOS调度器*/
    vTaskStartScheduler();
    for (;;)
        ;
}

void SysTick_DelayTicks(uint32_t n)
{
    g_systickCounter = n;
    while (g_systickCounter != 0U)
    {
    }
}

void SysTick_Handler(void)
{
    if (g_systickCounter != 0U)
    {
        g_systickCounter--;
    }
}

void TaskCreate(void)
{
    if (xTaskCreate(Task1,
                    "Task1",
                    100,
                    NULL,
                    Task_PRIORITY + 1,
                    NULL) != pdPASS)
    {
        PRINTF("Create SenderTask1 failed!.\r\n");
        while (1)
            ;
    }
    // if (xTaskCreate(Task3,
    //                 "Task3",
    //                 200,
    //                 NULL,
    //                 Task_PRIORITY + 1,
    //                 NULL) != pdPASS)
    // {
    //     PRINTF("Create Receiver failed!.\r\n");
    //     while (1)
    //         ;
    // }
    // if (xTaskCreate(Task2,
    //                 "Task2",
    //                 700,
    //                 (void *)200,
    //                 Task_PRIORITY + 2,
    //                 NULL) != pdPASS)
    // {
    //     PRINTF("Create Task2 failed!.\r\n");
    //     while (1)
    //         ;
    // }
    // if (xTaskCreate(TaskSW,
    //                 "TaskSW",
    //                 100,
    //                 NULL,
    //                 Task_PRIORITY + 1,
    //                 NULL) != pdPASS)
    // {
    //     PRINTF("Create TaskSW faile~\r\n");
    //     while (1)
    //         ;
    // }
}

/*!
 * @brief Task1 function
 */
static void Task1(void *pvParameters)
{

    while (1)
    {
        SysTick_DelayTicks(250u);
        PRINTF("Task1 is running\r\n");
    }
}

//static void Task2(void *pvParameters)
//{
//    while (1)
//    {
//
//    }
//}

/*!
 * @brief Task3 function
 */
//static void Task3(void *pvParameters)
//{
//    while (1)
//    {
//        // SysTick_DelayTicks(250U);
//        // PRINTF("Tick time\r\n");
//        // GPIO_PortToggle(GPIO, LED1_PORT, 1u << LED1_PIN);
//    }
//}

//static void TaskSW(void *pvParameters)
//{
//    while (1)
//    {
//        /* code */
//    }
//
//}

void gint0_callback(void)
{
    GPIO_PortToggle(GPIO, LED1_PORT, 1u << LED1_PIN);
    PRINTF("LED1 Toggle\r\n");
}

void PinInt0_Callback(pint_pin_int_t pin, uint32_t pmatch_status)
{
    GPIO_PortToggle(GPIO, LED2_PORT, 1u << LED2_PIN);
    PRINTF("LED2 Toggle, current pin is %d, pmatch_status is %d\r\n", pin, pmatch_status);
}

void GPIOInit(void){
    gpio_pin_config_t led_config = {
        kGPIO_DigitalOutput,
        0};

    /* Init output LED GPIO */
    GPIO_PortInit(GPIO, LED1_PORT);
    GPIO_PortInit(GPIO, LED2_PORT);
    GPIO_PortInit(GPIO, LED3_PORT);
    //    GPIO_PortInit(GPIO, SW2_PORT);
    GPIO_PinInit(GPIO, LED1_PORT, LED1_PIN, &led_config);
    GPIO_PinInit(GPIO, LED2_PORT, LED2_PIN, &led_config);
    GPIO_PinInit(GPIO, LED3_PORT, LED3_PIN, &led_config);
    GPIO_PinWrite(GPIO, LED1_PORT, LED1_PIN, 1);
    GPIO_PinWrite(GPIO, LED2_PORT, LED2_PIN, 1);
    GPIO_PinWrite(GPIO, LED3_PORT, LED3_PIN, 1);
}

void InterruptInit(void){

    /* Initialize GINT0 */
    GINT_Init(GINT0);
    /* Setup GINT0 for edge trigger, "OR" mode */
    GINT_SetCtrl(GINT0, kGINT_CombineAnd, kGINT_TrigEdge, gint0_callback);
    /* Select pins & polarity for GINT0 */
    GINT_ConfigPins(GINT0, DEMO_GINT0_PORT, DEMO_GINT0_POL_MASK, DEMO_GINT0_ENA_MASK);
    /* Enable callbacks for GINT0 */
    GINT_EnableCallback(GINT0);

    /* Connect trigger sources to PINT */
    INPUTMUX_Init(INPUTMUX);
    INPUTMUX_AttachSignal(INPUTMUX, kPINT_PinInt0, SW3_PINT);
    /* Turn off clock to inputmux to save power, clock is only needed to make changes */
    INPUTMUX_Deinit(INPUTMUX);

    /* Initialize PINT */
    PINT_Init(PINT);
    /* Setup Pin interrupt 0 for rising dege */
    PINT_PinInterruptConfig(PINT, kPINT_PinInt0, kPINT_PinIntEnableFallEdge, PinInt0_Callback);
    /* Enable callbacks for PINT0 by Index */
    PINT_EnableCallbackByIndex(PINT, kPINT_PinInt0);
}
