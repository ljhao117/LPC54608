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
#include "fsl_power.h"
#include "cdc_vcom.h"
#include "usbd_rom_api.h"
#include "app_usbd_cfg.h"
#include "lpc_types.h"
#include "romapi_5460x.h"
#include "usbd.h"

#include "usbd_core.h"
#include "pin_mux.h"
#include <stdbool.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define Task_PRIORITY 0

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

#define BUFFER_POOL_COUNT (8U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void USBTask(void *pvParameters);
void TaskCreate(void);
void gint0_callback(void);
void PinInt0_Callback(pint_pin_int_t pin, uint32_t pmatch_status);
void SysTick_DelayTicks(uint32_t n);
void SysTick_Handler(void);
void GPIOInit(void);
void InterruptInit(void);
void USBInit(void);
static void USB_DeviceApplicationInit(void);
void USB0_IRQHandler(void);
USB_INTERFACE_DESCRIPTOR *find_IntfDesc(const uint8_t *pDesc, uint32_t intfClass);

/* USB descriptor arrays define *_desc.c file */
extern const uint8_t USB_DeviceDescriptor[];
extern uint8_t USB_FsConfigDescriptor[];
extern const uint8_t USB_StringDescriptor[];
extern const uint8_t USB_DeviceQualifier[];

/*******************************************************************************
 * Globals Variables
 ******************************************************************************/
volatile uint32_t g_systickCounter;
static USBD_HANDLE_T g_hUsb;
ALIGN(2048)
uint8_t g_memUsbStack[USB_STACK_MEM_SIZE];

typedef struct _buffer_node_struct
{
    struct _buffer_node_struct *next;
    uint32_t length;
    uint8_t buffer[64];
} buffer_node_struct_t;

buffer_node_struct_t buffer[BUFFER_POOL_COUNT];

buffer_node_struct_t *bufferCurrent;
buffer_node_struct_t *bufferPoolList;
buffer_node_struct_t *bufferDataList;

const USBD_API_T *g_pUsbApi;

/*******************************************************************************
 * Code
 ******************************************************************************/
void AddNode(buffer_node_struct_t **list, buffer_node_struct_t *node)
{
    buffer_node_struct_t *p = *list;
    __disable_irq();
    *list = node;
    if (p)
    {
        node->next = p;
    }
    else
    {
        node->next = NULL;
    }
    __enable_irq();
}

void AddNodeToEnd(buffer_node_struct_t **list, buffer_node_struct_t *node)
{
    buffer_node_struct_t *p = *list;
    __disable_irq();
    if (p)
    {
        while (p->next)
        {
            p = p->next;
        }
        p->next = node;
    }
    else
    {
        *list = node;
    }
    node->next = NULL;
    __enable_irq();
}

buffer_node_struct_t *GetNode(buffer_node_struct_t **list)
{
    buffer_node_struct_t *p = *list;
    __disable_irq();
    if (p)
    {
        *list = p->next;
    }
    __enable_irq();
    return p;
}

void sendComplete(void)
{
    buffer_node_struct_t *p;
    p = GetNode(&bufferDataList);
    if (p)
    {
        AddNode(&bufferPoolList, p);
    }
    else if (bufferDataList)
    {
        vcom_write(&bufferDataList->buffer[0], bufferDataList->length);
    }
}

/**
 *  @brief Handle interrupt from USB0
 *  @return none
 */
void USB0_IRQHandler(void)
{
    uint32_t *addr = (uint32_t *)USB0->EPLISTSTART;
    if (USB0->DEVCMDSTAT & _BIT(8))
    {
        /* If setup packet is received,
           clear active bit for EP0_IN */
        addr[2] &= ~(0x80000000);
    }
    USBD_API->hw->ISR(g_hUsb);
}

/**
 * @brief Find the address of interface  descriptor for class type
 * @param pDesc     Pointer to configuration descriptor in which the desired class interface
 *                  descriptor to be found
 * @param intfClass Interface class type to be searched
 * @return If found returns the address fo requested interface else returns NULL
 */
USB_INTERFACE_DESCRIPTOR *find_IntfDesc(const uint8_t *pDesc, uint32_t intfClass)
{
    USB_COMMON_DESCRIPTOR *pD;
    USB_INTERFACE_DESCRIPTOR *pIntfDesc = 0;
    uint32_t next_desc_adr;
    pD = (USB_COMMON_DESCRIPTOR *)pDesc;
    next_desc_adr = (uint32_t)pDesc;

    while (pD->bLength)
    {
        /* Is it interface descriptor(?) */
        if (pD->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE)
        {
            pIntfDesc = (USB_INTERFACE_DESCRIPTOR *)pD;
            /* Did we find the reght interface descriptor */
            if (pIntfDesc->bInterfaceClass == intfClass)
            {
                break;
            }
        }
        pIntfDesc = 0;
        next_desc_adr = (uint32_t)pD + pD->bLength;
        pD = (USB_COMMON_DESCRIPTOR *)next_desc_adr;
    }
    return pIntfDesc;
}

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
    USBInit();

    /* Set systick reload value to generate 1ms interrupt */
    // if (SysTick_Config(SystemCoreClock / 1000U))
    // {
    //     while (1)
    //     {
    //     }
    // }

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
    if (xTaskCreate(USBTask,
                    "USBTask",
                    100,
                    NULL,
                    Task_PRIORITY + 1,
                    NULL) != pdPASS)
    {
        PRINTF("Create USBTask failed!.\r\n");
        while (1)
            ;
    }
}

/*!
 * @brief USBTask function
 */
static void USBTask(void *pvParameters)
{
    uint32_t prompt = 0;
    while (1)
    {
        /* Check if Vcom is connected */
        if ((vcom_connected() != 0) && (prompt == 0))
        {
            vcom_write((uint8_t *)"Hello World\r\n", 15);
            prompt = 1;
        }
        /* If VCOM port is opened echo whatever we receive back to host */
        if (prompt)
        {
            if (bufferCurrent == NULL)
            {
                bufferCurrent = GetNode(&bufferPoolList);
            }
            else if (bufferCurrent)
            {
                /* Virtual com port buffered read routine */
                bufferCurrent->length = vcom_bread(&bufferCurrent->buffer[0], 64);
                if (bufferCurrent->length)
                {
                    AddNodeToEnd(&bufferDataList, bufferCurrent);
                    bufferCurrent = NULL;
                }
                /* Disable IRQ Interrupts */
                __disable_irq();
                if (bufferDataList)
                {
                    /* Virtual com port write routine */
                vcom_write(&bufferDataList->buffer[0], bufferDataList->length);
                }
                /* Enable IRQ Interrupts */
                __enable_irq();
            }
        }
    }
}

void gint0_callback(void)
{
    GPIO_PortToggle(GPIO, LED1_PORT, 1u << LED1_PIN);
    vcom_write((uint8_t *)"LED1 Toggle\r\n", 15);
    PRINTF("LED1 Toggle\r\n");
}

void PinInt0_Callback(pint_pin_int_t pin, uint32_t pmatch_status)
{
    GPIO_PortToggle(GPIO, LED2_PORT, 1u << LED2_PIN);
    vcom_write((uint8_t *)"LED2 Toggle, current pin is %d, pmatch_status is %d\r\n", 55);
    PRINTF("LED2 Toggle, current pin is %d, pmatch_status is %d\r\n", pin, pmatch_status);
}

void GPIOInit(void)
{
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

void InterruptInit(void)
{

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

void USBInit(void)
{
    /* Turn on USB Phy */
    POWER_DisablePD(kPDRUNCFG_PD_USB0_PHY);
    /* Setup peripheral clock dividers */
    CLOCK_SetClkDiv(kCLOCK_DivUsb0Clk, 1, false);
    /* Configure the clock selection muxes */
    CLOCK_AttachClk(kFRO_HF_to_USB0_CLK);
    /* Enable USB0 host clock */
    CLOCK_EnableClock(kCLOCK_Usbhsl0);

    /* According to reference mannual, device mode setting has to be set by access usb host register */
    *((uint32_t *)(USBFSH_BASE + 0x5C)) |= USBFSH_PORTMODE_DEV_ENABLE_MASK;
    /* Disable USB0 host clock */
    CLOCK_DisableClock(kCLOCK_Usbhsl0);
    /* Enable USB IP clock */
    CLOCK_EnableUsbfs0DeviceClock(kCLOCK_UsbSrcFro, CLOCK_GetFreq(kCLOCK_FroHf));

    USB_DeviceApplicationInit();
}

static void USB_DeviceApplicationInit(void)
{
    /* USB device stack initialization parameter data structure */
    USBD_API_INIT_PARAM_T usb_param;
    /* USB descriptors data structure */
    USB_CORE_DESCS_T desc;
    /* Error code returned by Boot ROM drivers/library functions */
    ErrorCode_t ret = LPC_OK;

    /* Initialize USBD ROM API pointer */
    g_pUsbApi = (const USBD_API_T *)LPC_ROM_API->usbdApiBase;

    /* Initialize call back structures */
    memset((void *)&usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));

    usb_param.usb_reg_base = USB0_BASE;
    usb_param.max_num_ep = 3 + 1;
    usb_param.mem_base = (uint32_t)&g_memUsbStack;
    usb_param.mem_size = USB_STACK_MEM_SIZE;

    /* Set the USB descriptors */
    desc.device_desc = (uint8_t *)&USB_DeviceDescriptor[0];
    desc.string_desc = (uint8_t *)&USB_StringDescriptor[0];

    /* Note, to pass USBCV test full-speed only devices should have both
       descriptor arrays point to same location and device_qualifier set to 0. */
    desc.high_speed_desc = (uint8_t *)&USB_FsConfigDescriptor[0];
    desc.full_speed_desc = (uint8_t *)&USB_FsConfigDescriptor[0];
    desc.device_qualifier = 0;

    bufferPoolList = &buffer[0];
    bufferDataList = NULL;
    bufferCurrent = NULL;
    for (int i = 1; i < BUFFER_POOL_COUNT; i++)
    {
        bufferPoolList->next = &buffer[i];
    }
    bufferPoolList->next = NULL;

    /* USB Initialize */
    ret = USBD_API->hw->Init(&g_hUsb, &desc, &usb_param);
    if (ret == LPC_OK)
    {
        usb_param.mem_base = (uint32_t)&g_memUsbStack + (USB_STACK_MEM_SIZE - usb_param.mem_size);

        /* Init VCOM interface */
        ret = vcom_init(g_hUsb, &desc, &usb_param);
        if (ret == LPC_OK)
        {
            /* Install isr, set priority, and enable IRQ. */
            NVIC_SetPriority(USB0_IRQn, USB_DEVICE_INTERRUPT_PRIORITY);
            NVIC_EnableIRQ(USB0_IRQn);
            /* Now connect */
            USBD_API->hw->Connect(g_hUsb, 1);
        }
    }

    if (LPC_OK == ret)
    {
        PRINTF("USB CDC class based virtual Comm port example!\r\n");
    }
    else
    {
        PRINTF("USB CDC example initialization faild!\r\n");
        return;
    }
}
