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
#include "pd_app.h"
#include "fsl_gpio.h"
#include "pd_power_interface.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

uint32_t *PD_PowerBoardGetPartnerSourceCaps(void *callbackParam)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    return &(pdAppInstance->partnerSourceCaps[0].PDOValue);
}

uint32_t *PD_PowerBoardGetSelfSourceCaps(void *callbackParam)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    return (uint32_t *)&(pdAppInstance->selfSourcePdo1);
}

// 对应PD_PowerBoardSourceEnableVbusPower
pd_status_t PD_PowerBoardReset(void)
{
    pd_ptn5110_ctrl_pin_t phyPowerPinCtrl;

	//Question: reset的时候设置为0，什么时候会设置为其他值？
    g_PDAppInstance.sourceVbusVoltage = 0u;
    phyPowerPinCtrl.enSRC = 0;
    phyPowerPinCtrl.enSNK1 = 0;
	// (base, pin, output), 设置output为0
    GPIO_WritePinOutput(PD_EXTRA_EN_SRC_GPIO, PD_EXTRA_EN_SRC_GPIO_PIN, 0);
	//g_PDAppInstance.pdHandle指向PD_Instance
	//PD_Control中，大部分case将获得的结果存进第三个参数变量
    PD_Control(g_PDAppInstance.pdHandle, PD_CONTROL_PHY_POWER_PIN, &phyPowerPinCtrl);
    PD_Control(g_PDAppInstance.pdHandle, PD_CONTROL_DISCHARGE_VBUS, NULL);
    return kStatus_PD_Success;
}

/*...............source need implement follow vbus power related functions..................*/

/* voltage: the unit is 50mV */
// Question: 这边的硬件做了什么？
pd_status_t PD_PowerBoardSourceEnableVbusPower(pd_vbus_power_t vbusPower)
{
    uint32_t delay = 1000000;
    pd_ptn5110_ctrl_pin_t phyPowerPinCtrl;

    phyPowerPinCtrl.enSNK1 = 0;
    if (vbusPower.minVoltage >= VSAFE5V_IN_50MV)
    {
        phyPowerPinCtrl.enSRC = 1;
    }
    else
    {
        phyPowerPinCtrl.enSRC = 0;
    }
    PD_Control(g_PDAppInstance.pdHandle, PD_CONTROL_PHY_POWER_PIN, &phyPowerPinCtrl);

    if (vbusPower.minVoltage > VSAFE5V_IN_50MV)
    {
		//(basic, pin number, logic), GPIOA + 13
        GPIO_WritePinOutput(PD_EXTRA_EN_SRC_GPIO, PD_EXTRA_EN_SRC_GPIO_PIN, 1);
        /* when return, the power is ready. */
        while (--delay)
        {
            __ASM("nop");
        }
    }
    else
    {
        GPIO_WritePinOutput(PD_EXTRA_EN_SRC_GPIO, PD_EXTRA_EN_SRC_GPIO_PIN, 0);
        if (g_PDAppInstance.sourceVbusVoltage > VSAFE5V_IN_50MV)
        {
            while (--delay)
            {
                __ASM("nop");
            }
        }
    }
    g_PDAppInstance.sourceVbusVoltage = vbusPower.minVoltage;

    return kStatus_PD_Success;
}

/*...............sink need implement follow vbus power related functions..................*/

pd_status_t PD_PowerBoardSinkEnableVbusPower(pd_vbus_power_t vbusPower)
{
    pd_ptn5110_ctrl_pin_t phyPowerPinCtrl;

    phyPowerPinCtrl.enSRC = 0;
    phyPowerPinCtrl.enSNK1 = 1;
    PD_Control(g_PDAppInstance.pdHandle, PD_CONTROL_PHY_POWER_PIN, &phyPowerPinCtrl);
    return kStatus_PD_Success;
}

/*................ if support vconn, need implement the follow related functions..........*/
pd_status_t PD_PowerBoardControlVconn(uint8_t on)
{
    uint8_t controlVal;
    controlVal = (on ? 1 : 0);
    PD_Control(g_PDAppInstance.pdHandle, PD_CONTROL_VCONN, &controlVal);
    return kStatus_PD_Success;
}
