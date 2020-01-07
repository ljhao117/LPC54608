/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Pins v3.0
processor: LPC54608J512
package_id: LPC54608J512ET180
mcu_data: ksdk2_0
processor_version: 0.0.11
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

#include "fsl_common.h"
#include "fsl_iocon.h"
#include "pin_mux.h"



/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitPins:
- options: {callFromInitBoot: 'true', coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: B13, peripheral: FLEXCOMM0, signal: RXD_SDA_MOSI, pin_signal: PIO0_29/FC0_RXD_SDA_MOSI/CTIMER2_MAT3/SCT0_OUT8/TRACEDATA(2), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: standard, open_drain: disabled}
  - {pin_num: A2, peripheral: FLEXCOMM0, signal: TXD_SCL_MISO, pin_signal: PIO0_30/FC0_TXD_SCL_MISO/CTIMER0_MAT0/SCT0_OUT9/TRACEDATA(1), mode: inactive, invert: disabled,
    glitch_filter: disabled, slew_rate: standard, open_drain: disabled}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
/* Function assigned for the Core #0 (ARM Cortex-M4) */
void BOARD_InitPins(void)
{
    /* Enables the clock for the IOCON block. 0 = Disable; 1 = Enable.: 0x01u */
    CLOCK_EnableClock(kCLOCK_Iocon);

    const uint32_t port0_pin29_config = (/* Pin is configured as FC0_RXD_SDA_MOSI */
                                         IOCON_PIO_FUNC1 |
                                         /* No addition pin function */
                                         IOCON_PIO_MODE_INACT |
                                         /* Input function is not inverted */
                                         IOCON_PIO_INV_DI |
                                         /* Enables digital function */
                                         IOCON_PIO_DIGITAL_EN |
                                         /* Input filter disabled */
                                         IOCON_PIO_INPFILT_OFF |
                                         /* Standard mode, output slew rate control is enabled */
                                         IOCON_PIO_SLEW_STANDARD |
                                         /* Open drain is disabled */
                                         IOCON_PIO_OPENDRAIN_DI);
    /* PORT0 PIN29 (coords: B13) is configured as FC0_RXD_SDA_MOSI */
    IOCON_PinMuxSet(IOCON, 0U, 29U, port0_pin29_config);

    const uint32_t port0_pin30_config = (/* Pin is configured as FC0_TXD_SCL_MISO */
                                         IOCON_PIO_FUNC1 |
                                         /* No addition pin function */
                                         IOCON_PIO_MODE_INACT |
                                         /* Input function is not inverted */
                                         IOCON_PIO_INV_DI |
                                         /* Enables digital function */
                                         IOCON_PIO_DIGITAL_EN |
                                         /* Input filter disabled */
                                         IOCON_PIO_INPFILT_OFF |
                                         /* Standard mode, output slew rate control is enabled */
                                         IOCON_PIO_SLEW_STANDARD |
                                         /* Open drain is disabled */
                                         IOCON_PIO_OPENDRAIN_DI);
    /* PORT0 PIN30 (coords: A2) is configured as FC0_TXD_SCL_MISO */
    IOCON_PinMuxSet(IOCON, 0U, 30U, port0_pin30_config);

    /*LED1*/
    const uint32_t port3_pin14_config = (IOCON_PIO_FUNC0 |         /* Pin is configured as PIO3_14 */
                                         IOCON_PIO_MODE_PULLUP |   /* Selects pull-up function */
                                         IOCON_PIO_INV_DI |        /* Input function is not inverted */
                                         IOCON_PIO_DIGITAL_EN |    /* Enables digital function */
                                         IOCON_PIO_INPFILT_OFF |   /* Input filter disabled */
                                         IOCON_PIO_SLEW_STANDARD | /* Standard mode, output slew rate control is enabled */
                                         IOCON_PIO_OPENDRAIN_DI    /* Open drain is disabled */
    );
    IOCON_PinMuxSet(IOCON, 3U, 14U, port3_pin14_config); /* PORT3 PIN14 (coords: E3) is configured as PIO3_14 */

    /*LED2*/
    const uint32_t port3_pin3_config = (IOCON_PIO_FUNC0 |         /* Pin is configured as PIO3_14 */
                                         IOCON_PIO_MODE_PULLUP |   /* Selects pull-up function */
                                         IOCON_PIO_INV_DI |        /* Input function is not inverted */
                                         IOCON_PIO_DIGITAL_EN |    /* Enables digital function */
                                         IOCON_PIO_INPFILT_OFF |   /* Input filter disabled */
                                         IOCON_PIO_SLEW_STANDARD | /* Standard mode, output slew rate control is enabled */
                                         IOCON_PIO_OPENDRAIN_DI    /* Open drain is disabled */
    );
    IOCON_PinMuxSet(IOCON, 3U, 3U, port3_pin3_config); /* PORT3 PIN3 (coords: E3) is configured as PIO3_3 */

    /*LED3*/
    const uint32_t port2_pin2_config = (IOCON_PIO_FUNC0 |         /* Pin is configured as PIO3_14 */
                                         IOCON_PIO_MODE_PULLUP |   /* Selects pull-up function */
                                         IOCON_PIO_INV_DI |        /* Input function is not inverted */
                                         IOCON_PIO_DIGITAL_EN |    /* Enables digital function */
                                         IOCON_PIO_INPFILT_OFF |   /* Input filter disabled */
                                         IOCON_PIO_SLEW_STANDARD | /* Standard mode, output slew rate control is enabled */
                                         IOCON_PIO_OPENDRAIN_DI    /* Open drain is disabled */
    );
    IOCON_PinMuxSet(IOCON, 2U, 2U, port2_pin2_config); /* PORT2 PIN2 (coords: E3) is configured as PIO2_2 */

    /*SW2*/
    const uint32_t port0_pin6_config = (/* Pin is configured as PIO0_6 */
                                        IOCON_PIO_FUNC0 |
                                        /* Selects pull-up function */
                                        IOCON_PIO_MODE_PULLUP |
                                        /* Input function is not inverted */
                                        IOCON_PIO_INV_DI |
                                        /* Enables digital function */
                                        IOCON_PIO_DIGITAL_EN |
                                        /* Input filter disabled */
                                        IOCON_PIO_INPFILT_OFF |
                                        /* Standard mode, output slew rate control is enabled */
                                        IOCON_PIO_SLEW_STANDARD |
                                        /* Open drain is disabled */
                                        IOCON_PIO_OPENDRAIN_DI);
    /* PORT0 PIN6 (coords: A5) is configured as PIO0_6 */
    IOCON_PinMuxSet(IOCON, 0U, 6U, port0_pin6_config);
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
