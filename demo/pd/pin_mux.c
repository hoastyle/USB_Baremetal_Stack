/*
 * Copyright 2017 NXP
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Pins v3.0
processor: MKL27Z64xxx4
package_id: MKL27Z64VLH4
mcu_data: ksdk2_0
processor_version: 2.0.0
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

#include "fsl_common.h"
#include "fsl_port.h"
#include "pin_mux.h"

/*FUNCTION**********************************************************************
 * 
 * Function Name : BOARD_InitBootPins
 * Description   : Calls initialization functions.
 * 
 *END**************************************************************************/
void BOARD_InitBootPins(void) {
    BOARD_InitPins();
    I2C1_InitPins();
}

#define PIN1_IDX                         1u   /*!< Pin number for pin 1 in a port */
#define PIN2_IDX                         2u   /*!< Pin number for pin 2 in a port */
#define PIN4_IDX                         4u   /*!< Pin number for pin 4 in a port */
#define PIN13_IDX                       13u   /*!< Pin number for pin 13 in a port */
#define PIN31_IDX                       31u   /*!< Pin number for pin 31 in a port */
#define SOPT5_LPUART0RXSRC_LPUART_RX  0x00u   /*!< LPUART0 Receive Data Source Select: LPUART_RX pin */
#define SOPT5_LPUART0TXSRC_LPUART_TX  0x00u   /*!< LPUART0 Transmit Data Source Select: LPUART0_TX pin */

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitPins:
- options: {callFromInitBoot: 'true', coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: '19', peripheral: GPIOE, signal: 'GPIO, 31', pin_signal: PTE31/TPM0_CH4, direction: INPUT, slew_rate: slow, pull_select: up, pull_enable: enable}
  - {pin_num: '29', peripheral: GPIOA, signal: 'GPIO, 13', pin_signal: PTA13/TPM1_CH1}
  - {pin_num: '23', peripheral: LPUART0, signal: RX, pin_signal: PTA1/LPUART0_RX/TPM2_CH0}
  - {pin_num: '24', peripheral: LPUART0, signal: TX, pin_signal: PTA2/LPUART0_TX/TPM2_CH1}
  - {pin_num: '26', peripheral: GPIOA, signal: 'GPIO, 4', pin_signal: PTA4/I2C1_SDA/TPM0_CH1/NMI_b, direction: INPUT, slew_rate: slow, pull_select: up, pull_enable: enable,
    passive_filter: enable}
  - {pin_num: '44', peripheral: GPIOC, signal: 'GPIO, 1', pin_signal: ADC0_SE15/PTC1/LLWU_P6/RTC_CLKIN/I2C1_SCL/TPM0_CH0, direction: INPUT, slew_rate: slow, pull_select: up,
    pull_enable: enable}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/*FUNCTION**********************************************************************
 *
 * Function Name : BOARD_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 *END**************************************************************************/
void BOARD_InitPins(void) {
  CLOCK_EnableClock(kCLOCK_PortA);                           /* Port A Clock Gate Control: Clock enabled */
  CLOCK_EnableClock(kCLOCK_PortC);                           /* Port C Clock Gate Control: Clock enabled */
  CLOCK_EnableClock(kCLOCK_PortE);                           /* Port E Clock Gate Control: Clock enabled */

  PORT_SetPinMux(PORTA, PIN1_IDX, kPORT_MuxAlt2);            /* PORTA1 (pin 23) is configured as LPUART0_RX */
  PORT_SetPinMux(PORTA, PIN13_IDX, kPORT_MuxAsGpio);         /* PORTA13 (pin 29) is configured as PTA13 */
  PORT_SetPinMux(PORTA, PIN2_IDX, kPORT_MuxAlt2);            /* PORTA2 (pin 24) is configured as LPUART0_TX */
  const port_pin_config_t porta4_pin26_config = {
    kPORT_PullUp,                                            /* Internal pull-up resistor is enabled */
    kPORT_SlowSlewRate,                                      /* Slow slew rate is configured */
    kPORT_PassiveFilterEnable,                               /* Passive filter is enabled */
    kPORT_LowDriveStrength,                                  /* Low drive strength is configured */
    kPORT_MuxAsGpio,                                         /* Pin is configured as PTA4 */
  };
  PORT_SetPinConfig(PORTA, PIN4_IDX, &porta4_pin26_config);  /* PORTA4 (pin 26) is configured as PTA4 */
  const port_pin_config_t portc1_pin44_config = {
    kPORT_PullUp,                                            /* Internal pull-up resistor is enabled */
    kPORT_SlowSlewRate,                                      /* Slow slew rate is configured */
    kPORT_PassiveFilterDisable,                              /* Passive filter is disabled */
    kPORT_LowDriveStrength,                                  /* Low drive strength is configured */
    kPORT_MuxAsGpio,                                         /* Pin is configured as PTC1 */
  };
  PORT_SetPinConfig(PORTC, PIN1_IDX, &portc1_pin44_config);  /* PORTC1 (pin 44) is configured as PTC1 */
  const port_pin_config_t porte31_pin19_config = {
    kPORT_PullUp,                                            /* Internal pull-up resistor is enabled */
    kPORT_SlowSlewRate,                                      /* Slow slew rate is configured */
    kPORT_PassiveFilterDisable,                              /* Passive filter is disabled */
    kPORT_LowDriveStrength,                                  /* Low drive strength is configured */
    kPORT_MuxAsGpio,                                         /* Pin is configured as PTE31 */
  };
  PORT_SetPinConfig(PORTE, PIN31_IDX, &porte31_pin19_config); /* PORTE31 (pin 19) is configured as PTE31 */
  SIM->SOPT5 = ((SIM->SOPT5 &
    (~(SIM_SOPT5_LPUART0TXSRC_MASK | SIM_SOPT5_LPUART0RXSRC_MASK))) /* Mask bits to zero which are setting */
      | SIM_SOPT5_LPUART0TXSRC(SOPT5_LPUART0TXSRC_LPUART_TX) /* LPUART0 Transmit Data Source Select: LPUART0_TX pin */
      | SIM_SOPT5_LPUART0RXSRC(SOPT5_LPUART0RXSRC_LPUART_RX) /* LPUART0 Receive Data Source Select: LPUART_RX pin */
    );
}


#define PCR_DSE_HIGH                  0x01u   /*!< Drive Strength Enable: High drive strength is configured on the corresponding pin, if pin is configured as a digital output. */
#define PIN0_IDX                         0u   /*!< Pin number for pin 0 in a port */

#define PIN1_IDX                         1u   /*!< Pin number for pin 1 in a port */

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
I2C0_InitPins:
- options: {coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: '35', peripheral: I2C0, signal: SCL, pin_signal: ADC0_SE8/PTB0/LLWU_P5/I2C0_SCL/TPM1_CH0/SPI1_MOSI/SPI1_MISO, drive_strength: high}
  - {pin_num: '36', peripheral: I2C0, signal: SDA, pin_signal: ADC0_SE9/PTB1/I2C0_SDA/TPM1_CH1/SPI1_MISO/SPI1_MOSI, drive_strength: high}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C0_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 *END**************************************************************************/
void I2C0_InitPins(void) {
  CLOCK_EnableClock(kCLOCK_PortB);                           /* Port B Clock Gate Control: Clock enabled */

  PORT_SetPinMux(PORTB, PIN0_IDX, kPORT_MuxAlt2);            /* PORTB0 (pin 35) is configured as I2C0_SCL */
  PORTB->PCR[0] = ((PORTB->PCR[0] &
    (~(PORT_PCR_DSE_MASK | PORT_PCR_ISF_MASK)))              /* Mask bits to zero which are setting */
      | PORT_PCR_DSE(PCR_DSE_HIGH)                           /* Drive Strength Enable: High drive strength is configured on the corresponding pin, if pin is configured as a digital output. */
    );
  PORT_SetPinMux(PORTB, PIN1_IDX, kPORT_MuxAlt2);            /* PORTB1 (pin 36) is configured as I2C0_SDA */
  PORTB->PCR[1] = ((PORTB->PCR[1] &
    (~(PORT_PCR_DSE_MASK | PORT_PCR_ISF_MASK)))              /* Mask bits to zero which are setting */
      | PORT_PCR_DSE(PCR_DSE_HIGH)                           /* Drive Strength Enable: High drive strength is configured on the corresponding pin, if pin is configured as a digital output. */
    );
}



#define PIN0_IDX                         0u   /*!< Pin number for pin 0 in a port */

#define PIN1_IDX                         1u   /*!< Pin number for pin 1 in a port */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
I2C0_DeinitPins:
- options: {coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: '35', peripheral: ADC0, signal: 'SE, 8', pin_signal: ADC0_SE8/PTB0/LLWU_P5/I2C0_SCL/TPM1_CH0/SPI1_MOSI/SPI1_MISO}
  - {pin_num: '36', peripheral: ADC0, signal: 'SE, 9', pin_signal: ADC0_SE9/PTB1/I2C0_SDA/TPM1_CH1/SPI1_MISO/SPI1_MOSI}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C0_DeinitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 *END**************************************************************************/
void I2C0_DeinitPins(void) {
  CLOCK_EnableClock(kCLOCK_PortB);                           /* Port B Clock Gate Control: Clock enabled */

  PORT_SetPinMux(PORTB, PIN0_IDX, kPORT_PinDisabledOrAnalog); /* PORTB0 (pin 35) is configured as ADC0_SE8 */
  PORT_SetPinMux(PORTB, PIN1_IDX, kPORT_PinDisabledOrAnalog); /* PORTB1 (pin 36) is configured as ADC0_SE9 */
}



#define PCR_DSE_HIGH                  0x01u   /*!< Drive Strength Enable: High drive strength is configured on the corresponding pin, if pin is configured as a digital output. */
#define PIN6_IDX                         6u   /*!< Pin number for pin 6 in a port */
#define PIN7_IDX                         7u   /*!< Pin number for pin 7 in a port */

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
I2C1_InitPins:
- options: {callFromInitBoot: 'true', coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: '63', peripheral: I2C1, signal: SDA, pin_signal: ADC0_SE7b/PTD6/LLWU_P15/SPI1_MOSI/LPUART0_RX/I2C1_SDA/SPI1_MISO/FXIO0_D6, drive_strength: high}
  - {pin_num: '64', peripheral: I2C1, signal: SCL, pin_signal: PTD7/SPI1_MISO/LPUART0_TX/I2C1_SCL/SPI1_MOSI/FXIO0_D7, drive_strength: high}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C1_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 *END**************************************************************************/
void I2C1_InitPins(void) {
  CLOCK_EnableClock(kCLOCK_PortD);                           /* Port D Clock Gate Control: Clock enabled */

  PORT_SetPinMux(PORTD, PIN6_IDX, kPORT_MuxAlt4);            /* PORTD6 (pin 63) is configured as I2C1_SDA */
  PORTD->PCR[6] = ((PORTD->PCR[6] &
    (~(PORT_PCR_DSE_MASK | PORT_PCR_ISF_MASK)))              /* Mask bits to zero which are setting */
      | PORT_PCR_DSE(PCR_DSE_HIGH)                           /* Drive Strength Enable: High drive strength is configured on the corresponding pin, if pin is configured as a digital output. */
    );
  PORT_SetPinMux(PORTD, PIN7_IDX, kPORT_MuxAlt4);            /* PORTD7 (pin 64) is configured as I2C1_SCL */
  PORTD->PCR[7] = ((PORTD->PCR[7] &
    (~(PORT_PCR_DSE_MASK | PORT_PCR_ISF_MASK)))              /* Mask bits to zero which are setting */
      | PORT_PCR_DSE(PCR_DSE_HIGH)                           /* Drive Strength Enable: High drive strength is configured on the corresponding pin, if pin is configured as a digital output. */
    );
}



#define PIN6_IDX                         6u   /*!< Pin number for pin 6 in a port */

#define PIN7_IDX                         7u   /*!< Pin number for pin 7 in a port */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
I2C1_DeinitPins:
- options: {coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: '63', peripheral: ADC0, signal: 'SE, 7b', pin_signal: ADC0_SE7b/PTD6/LLWU_P15/SPI1_MOSI/LPUART0_RX/I2C1_SDA/SPI1_MISO/FXIO0_D6}
  - {pin_num: '64', peripheral: n/a, signal: disabled, pin_signal: PTD7/SPI1_MISO/LPUART0_TX/I2C1_SCL/SPI1_MOSI/FXIO0_D7}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C1_DeinitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 *END**************************************************************************/
void I2C1_DeinitPins(void) {
  CLOCK_EnableClock(kCLOCK_PortD);                           /* Port D Clock Gate Control: Clock enabled */

  PORT_SetPinMux(PORTD, PIN6_IDX, kPORT_PinDisabledOrAnalog); /* PORTD6 (pin 63) is configured as ADC0_SE7b */
  PORT_SetPinMux(PORTD, PIN7_IDX, kPORT_PinDisabledOrAnalog); /* PORTD7 (pin 64) is disabled */
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
