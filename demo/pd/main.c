/*
 * Copyright 2016 - 2017 NXP
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

#include "usb_pd_config.h"
#include "usb_pd.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "pd_app.h"
#include "fsl_device_registers.h"
#include "fsl_lptmr.h"
#include "fsl_debug_console.h"

#include "pin_mux.h"
#include "board.h"
#include "clock_config.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define LPTMR_USEC_COUNT 1000U
/* Get source clock for LPTMR driver */
#define LPTMR_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_LpoClk)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void PD_AppInit(void);
void PD_DemoInit(pd_app_t *pdAppInstance);
void PD_DemoTaskFun(pd_app_t *pdAppInstance);
void BOARD_InitHardware(void);
void PD_Demo1msIsrProcessSW(pd_app_t *pdAppInstance);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_I2C1_ReleaseBus(void);
void BOARD_I2C0_ReleaseBus(void);
#define I2C_RELEASE_BUS_COUNT 100U
#define I2C0_SCL (0U)
#define I2C0_SDA (1U)


uint32_t I2C0_GetFreq(void)
{
    return CLOCK_GetFreq(I2C0_CLK_SRC);
}

uint32_t I2C1_GetFreq(void)
{
    return CLOCK_GetFreq(I2C1_CLK_SRC);
}

static void i2c_release_bus_delay(void)
{
    uint32_t i = 0;
    for (i = 0; i < I2C_RELEASE_BUS_COUNT; i++)
    {
        __NOP();
    }
}

void BOARD_I2C0_ReleaseBus(void)
{
    uint8_t i = 0;
    gpio_pin_config_t pin_config;
    port_pin_config_t i2c_pin_config = {0};

    /* Config pin mux as gpio */
    i2c_pin_config.pullSelect = kPORT_PullUp;
    i2c_pin_config.mux = kPORT_MuxAsGpio;

    pin_config.pinDirection = kGPIO_DigitalOutput;
    pin_config.outputLogic = 1U;
    CLOCK_EnableClock(kCLOCK_PortB);
    PORT_SetPinConfig(PORTB, I2C0_SCL, &i2c_pin_config);
    PORT_SetPinConfig(PORTB, I2C0_SDA, &i2c_pin_config);

    GPIO_PinInit(GPIOB, I2C0_SCL, &pin_config);
    GPIO_PinInit(GPIOB, I2C0_SDA, &pin_config);

    /* Drive SDA low first to simulate a start */
    GPIO_WritePinOutput(GPIOB, I2C0_SDA, 0U);
    i2c_release_bus_delay();

    /* Send 9 pulses on SCL and keep SDA high */
    for (i = 0; i < 9; i++)
    {
        GPIO_WritePinOutput(GPIOB, I2C0_SCL, 0U);
        i2c_release_bus_delay();

        GPIO_WritePinOutput(GPIOB, I2C0_SDA, 1U);
        i2c_release_bus_delay();

        GPIO_WritePinOutput(GPIOB, I2C0_SCL, 1U);
        i2c_release_bus_delay();
        i2c_release_bus_delay();
    }

    /* Send stop */
    GPIO_WritePinOutput(GPIOB, I2C0_SCL, 0U);
    i2c_release_bus_delay();

    GPIO_WritePinOutput(GPIOB, I2C0_SDA, 0U);
    i2c_release_bus_delay();

    GPIO_WritePinOutput(GPIOB, I2C0_SCL, 1U);
    i2c_release_bus_delay();

    GPIO_WritePinOutput(GPIOB, I2C0_SDA, 1U);
    i2c_release_bus_delay();
}

void BOARD_I2C1_ReleaseBus(void)
{
    uint8_t i = 0;
    gpio_pin_config_t pin_config;
    port_pin_config_t i2c_pin_config = {0};

    /* Config pin mux as gpio */
    i2c_pin_config.pullSelect = kPORT_PullUp;
    i2c_pin_config.mux = kPORT_MuxAsGpio;

    pin_config.pinDirection = kGPIO_DigitalOutput;
    pin_config.outputLogic = 1U;
    CLOCK_EnableClock(kCLOCK_PortD);
    PORT_SetPinConfig(PORTD, 7u, &i2c_pin_config);
    PORT_SetPinConfig(PORTD, 6u, &i2c_pin_config);

    GPIO_PinInit(GPIOD, 7u, &pin_config);
    GPIO_PinInit(GPIOD, 6u, &pin_config);

    /* Drive SDA low first to simulate a start */
    GPIO_WritePinOutput(GPIOD, 6u, 0U);
    i2c_release_bus_delay();

    /* Send 9 pulses on SCL and keep SDA high */
    for (i = 0; i < 9; i++)
    {
        GPIO_WritePinOutput(GPIOD, 7u, 0U);
        i2c_release_bus_delay();

        GPIO_WritePinOutput(GPIOD, 6u, 1U);
        i2c_release_bus_delay();

        GPIO_WritePinOutput(GPIOD, 7u, 1U);
        i2c_release_bus_delay();
        i2c_release_bus_delay();
    }

    /* Send stop */
    GPIO_WritePinOutput(GPIOD, 7u, 0U);
    i2c_release_bus_delay();

    GPIO_WritePinOutput(GPIOD, 6u, 0U);
    i2c_release_bus_delay();

    GPIO_WritePinOutput(GPIOD, 7u, 1U);
    i2c_release_bus_delay();

    GPIO_WritePinOutput(GPIOD, 6u, 1U);
    i2c_release_bus_delay();
}

/* 1ms timer interrupt handler
 * Call from startup.s, will be called whenever 1ms timer timeout
 * Including button handler and timer timeout handler
 */
void LPTMR0_IRQHandler(void)
{
    LPTMR_ClearStatusFlags(LPTMR0, kLPTMR_TimerCompareFlag);
    /* Callback into timer service */
    PD_TimerIsrFunction(g_PDAppInstance.pdHandle);
    PD_Demo1msIsrProcessSW(&g_PDAppInstance);
}

/* 1ms low power timer */
static void PD_1mstimerInit(void)
{
    lptmr_config_t lptmrConfig;

    /* Configure LPTMR */
    /*
     * lptmrConfig.timerMode = kLPTMR_TimerModeTimeCounter;
     * lptmrConfig.pinSelect = kLPTMR_PinSelectInput_0;
     * lptmrConfig.pinPolarity = kLPTMR_PinPolarityActiveHigh;
     * lptmrConfig.enableFreeRunning = false;
     * lptmrConfig.bypassPrescaler = true;
     * lptmrConfig.prescalerClockSource = kLPTMR_PrescalerClock_1;
     * lptmrConfig.value = kLPTMR_Prescale_Glitch_0;
     */
    LPTMR_GetDefaultConfig(&lptmrConfig);

    /* Initialize the LPTMR */
    LPTMR_Init(LPTMR0, &lptmrConfig);

    /*
     * Set timer period.
     * Note : the parameter "ticks" of LPTMR_SetTimerPeriod should be equal or greater than 1.
    */
    LPTMR_SetTimerPeriod(LPTMR0, USEC_TO_COUNT(LPTMR_USEC_COUNT, LPTMR_SOURCE_CLOCK));

    /* Enable timer interrupt */
    LPTMR_EnableInterrupts(LPTMR0, kLPTMR_TimerInterruptEnable);

    /* Enable at the NVIC */
    EnableIRQ(LPTMR0_IRQn);

    /* Start counting */
    LPTMR_StartTimer(LPTMR0);
}

static void pd_port_task(void *arg)
{
    EnableIRQ((IRQn_Type)BOARD_PTN5110_GPIO_IRQ);
    while (1)
    {
		// 将不停进入StackStateMachine
        PD_InstanceTask(g_PDAppInstance.pdHandle);
    }
}

static void pd_demo_task(void *arg)
{
    g_PDAppInstance.pdHandle = NULL;
	/* Set and enable interrupt of I2C */
    NVIC_SetPriority(PD_I2C_MASTER_IRQn, 1);
    NVIC_EnableIRQ(PD_I2C_MASTER_IRQn);

    PD_1mstimerInit();
    PD_AppInit();
    PRINTF("pd init success\r\n");

	// state machine task
    if (xTaskCreate(pd_port_task, "port", (1024u + 512u) / sizeof(portSTACK_TYPE), NULL, 5, NULL) != pdPASS)
    {
        PRINTF("create pd task error\r\n");
    }

    PD_DemoInit(&g_PDAppInstance);
    while (1)
    {
        PD_DemoTaskFun(&g_PDAppInstance);
    }
}

int main(void)
{
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_I2C0_ReleaseBus();
    BOARD_InitDebugConsole();

    if (xTaskCreate(pd_demo_task, "demo", 512 / sizeof(portSTACK_TYPE), NULL, 4, NULL) != pdPASS)
    {
        PRINTF("create demo task error\r\n");
    }

    vTaskStartScheduler();

    while (1)
    {
        ;
    }
}
