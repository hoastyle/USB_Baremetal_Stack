/*
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
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
#include "fsl_port.h"
#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "board.h"
#include "pd_power_interface.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define PD_PTN5110_GPIO_INTERRUPT_PRIORITY (4)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

pd_status_t PD_PowerBoardReset(void);
pd_status_t PD_DpmAppCommandCallback(void *callbackParam, uint32_t event, void *param);

/*******************************************************************************
 * Variables
 ******************************************************************************/
pd_app_t g_PDAppInstance;

pd_power_port_config_t g_PowerPortConfig = {
    NULL,                    /* source caps */
    NULL,                    /* self sink caps */
    0,                       /* source cap count */
    0,                       /* sink cap count */
    kPowerConfig_SourceOnly, /* typec role */
    kCurrent_1A5,            /* source: Rp current level */
    kTypecTry_None,          /* drp try function */
    kDataConfig_DFP,         /* data function */
    1,                       /* support vconn */
    0,                       /* reserved */
    NULL,
    NULL,
    NULL,
};

pd_instance_config_t g_PDConfig = {
    kDeviceType_NormalPowerPort,  /* normal power port */
    PORTB_PORTC_PORTD_PORTE_IRQn, /* interrupt number */
    kPD_PhyPTN5110,
    kInterface_i2c1,
    0x50u,
    &g_PowerPortConfig,
};

pd_power_handle_callback_t callbackFunctions = {
    PD_PowerSrcTurnOnDefaultVbus,
    PD_PowerSrcTurnOnRequestVbus,
    PD_PowerSrcTurnOffVbus,
    PD_PowerSrcGotoMinReducePower,
    NULL,
    NULL,
    NULL,
    NULL,
    PD_PowerControlVconn,
};

/*******************************************************************************
 * Code
 ******************************************************************************/

void PORTB_PORTC_PORTD_PORTE_IRQHandler(void)
{
    if (BOARD_PTN5110_GPIO_PORT->ISFR & (1U << BOARD_PTN5110_GPIO_PIN))
    {
        BOARD_PTN5110_GPIO_PORT->ISFR = 1U << BOARD_PTN5110_GPIO_PIN;
        if (!(BOARD_PTN5110_GPIO->PDIR & (1U << BOARD_PTN5110_GPIO_PIN)))
        {
            PD_PTN5110IsrFunction(g_PDAppInstance.pdHandle);
        }
    }
}

pd_status_t PD_DpmConnectCallback(void *callbackParam, uint32_t event, void *param)
{
    pd_status_t status = kStatus_PD_Error;
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    switch (event)
    {
        case PD_DISCONNECTED:
            PD_PowerBoardReset();
            PRINTF("disconnect\r\n");
            status = kStatus_PD_Success;
            break;

        case PD_CONNECT_ROLE_CHANGE:
        case PD_CONNECTED:
        {
            uint8_t roleInfo;

            PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_POWER_ROLE, &roleInfo);
            if (roleInfo == kPD_PowerRoleSource)
            {
                PRINTF("provide default 5V vbus\r\n");
            }

            status = kStatus_PD_Success;
            break;
        }

        default:
            break;
    }

    return status;
}

pd_status_t PD_DpmDemoAppCallback(void *callbackParam, uint32_t event, void *param)
{
    pd_status_t status = kStatus_PD_Error;

    switch (event)
    {
        case PD_FUNCTION_DISABLED:
            /* need hard or software reset */
            status = kStatus_PD_Success;
            break;

        case PD_CONNECTED:
        case PD_CONNECT_ROLE_CHANGE:
        case PD_DISCONNECTED:
            status = PD_DpmConnectCallback(callbackParam, event, param);
            break;

        default:
            status = PD_DpmAppCommandCallback(callbackParam, event, param);
    }
    return status;
}

void PD_AppInit(void)
{
    g_PowerPortConfig.sourceCaps = (uint32_t *)&g_PDAppInstance.selfSourcePdo1, /* source caps */
        g_PowerPortConfig.sourceCapCount = 2;
    g_PowerPortConfig.sinkCaps = NULL, /* self sink caps */
        g_PowerPortConfig.sinkCapCount = 0;
    if (PD_InstanceInit(&g_PDAppInstance.pdHandle, PD_DpmDemoAppCallback, &callbackFunctions, &g_PDAppInstance,
                        &g_PDConfig) != kStatus_PD_Success)
    {
        PRINTF("pd init fail\r\n");
    }

    /* initialize port interrupt */
    PORT_SetPinInterruptConfig(BOARD_PTN5110_GPIO_PORT, BOARD_PTN5110_GPIO_PIN, kPORT_InterruptLogicZero);
    BOARD_PTN5110_GPIO->PDDR &= ~(1U << BOARD_PTN5110_GPIO_PIN);    /* gpio set as input */
    PD_EXTRA_EN_SRC_GPIO->PDDR |= (1U << PD_EXTRA_EN_SRC_GPIO_PIN); /* as output */

    NVIC_SetPriority((IRQn_Type)BOARD_PTN5110_GPIO_IRQ, PD_PTN5110_GPIO_INTERRUPT_PRIORITY);

    PD_PowerBoardReset();

    g_PDAppInstance.msgSop = kPD_MsgSOP;
    /* PDO1: fixed supply: dual-role power; Externally Powered; no USB communication; dual-role data; 5V; 2.7A */
    g_PDAppInstance.selfSourcePdo1.PDOValue = 0;
    g_PDAppInstance.selfSourcePdo1.fixedPDO.dualRoleData = 0;
    g_PDAppInstance.selfSourcePdo1.fixedPDO.dualRolePower = 0;
    g_PDAppInstance.selfSourcePdo1.fixedPDO.externalPowered = 1;
    g_PDAppInstance.selfSourcePdo1.fixedPDO.fixedSupply = kPDO_Fixed;
    g_PDAppInstance.selfSourcePdo1.fixedPDO.maxCurrent = (27 * 10);
    g_PDAppInstance.selfSourcePdo1.fixedPDO.peakCurrent = 0;
#if ((defined PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30))
    g_PDAppInstance.selfSourcePdo1.fixedPDO.unchunkedSupported = 1;
#endif
    g_PDAppInstance.selfSourcePdo1.fixedPDO.usbCommunicationsCapable = 0;
    g_PDAppInstance.selfSourcePdo1.fixedPDO.usbSuspendSupported = 0;
    g_PDAppInstance.selfSourcePdo1.fixedPDO.voltage = (5 * 1000 / 50);
    /* PDO2: fixed Supply: 9V - 1.5A */
    g_PDAppInstance.selfSourcePdo2.PDOValue = 0;
    g_PDAppInstance.selfSourcePdo2.fixedPDO.fixedSupply = kPDO_Fixed;
    g_PDAppInstance.selfSourcePdo2.fixedPDO.maxCurrent = (15 * 10);
    g_PDAppInstance.selfSourcePdo2.fixedPDO.peakCurrent = 0;
#if ((defined PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30))
    g_PDAppInstance.selfSourcePdo2.fixedPDO.unchunkedSupported = 1;
#endif
    g_PDAppInstance.selfSourcePdo2.fixedPDO.usbCommunicationsCapable = 0;
    g_PDAppInstance.selfSourcePdo2.fixedPDO.usbSuspendSupported = 0;
    g_PDAppInstance.selfSourcePdo2.fixedPDO.voltage = (9 * 1000 / 50);
    g_PDAppInstance.selfSouceCapNumber = 2;
}
