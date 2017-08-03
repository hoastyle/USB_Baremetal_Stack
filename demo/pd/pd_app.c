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

#define PD_COMPLIANCE_TEST_DRP (1)
#define PD_COMPLIANCE_TEST_DRP_TRY_SNK (0)
#define PD_COMPLIANCE_TEST_DRP_TRY_SRC (0)
#define PD_COMPLIANCE_TEST_CONSUMER_PROVIDER (0)
#define PD_COMPLIANCE_TEST_PROVIDER_CONSUMER (0)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

pd_status_t PD_PowerBoardReset(void);
pd_status_t PD_DpmAppCommandCallback(void *callbackParam, uint32_t event, void *param);

/*******************************************************************************
 * Variables
 ******************************************************************************/
pd_app_t g_PDAppInstance;

#if (defined PD_COMPLIANCE_TEST_DRP) && (PD_COMPLIANCE_TEST_DRP)
pd_power_port_config_t g_PowerPortConfig = {
    NULL,                     /* source caps */
    NULL,                     /* self sink caps */
    0,                        /* source cap count */
    0,                        /* sink cap count */
    kPowerConfig_DRPToggling, /* typec role */
    kCurrent_1A5,             /* source: Rp current level */
    kTypecTry_None,           /* drp try function */
    kDataConfig_DRD,          /* data function */
    1,                        /* support vconn */
    0,                        /* reserved */
    NULL,
    NULL,
    NULL,
};
#endif

#if (defined PD_COMPLIANCE_TEST_DRP_TRY_SNK) && (PD_COMPLIANCE_TEST_DRP_TRY_SNK)
pd_power_port_config_t g_PowerPortConfig = {
    NULL,                     /* source caps */
    NULL,                     /* self sink caps */
    0,                        /* source cap count */
    0,                        /* sink cap count */
    kPowerConfig_DRPToggling, /* typec role */
    kCurrent_1A5,             /* source: Rp current level */
    kTypecTry_Snk,            /* drp try function */
    kDataConfig_DRD,          /* data function */
    1,                        /* support vconn */
    0,                        /* reserved */
    NULL,
    NULL,
    NULL,
};
#endif

#if (defined PD_COMPLIANCE_TEST_DRP_TRY_SRC) && (PD_COMPLIANCE_TEST_DRP_TRY_SRC)
pd_power_port_config_t g_PowerPortConfig = {
    NULL,                     /* source caps */
    NULL,                     /* self sink caps */
    0,                        /* source cap count */
    0,                        /* sink cap count */
    kPowerConfig_DRPToggling, /* typec role */
    kCurrent_1A5,             /* source: Rp current level */
    kTypecTry_Src,            /* drp try function */
    kDataConfig_DRD,          /* data function */
    1,                        /* support vconn */
    0,                        /* reserved */
    NULL,
    NULL,
    NULL,
};
#endif

#if (defined PD_COMPLIANCE_TEST_CONSUMER_PROVIDER) && (PD_COMPLIANCE_TEST_CONSUMER_PROVIDER)
pd_power_port_config_t g_PowerPortConfig = {
    NULL,                     /* source caps */
    NULL,                     /* self sink caps */
    0,                        /* source cap count */
    0,                        /* sink cap count */
    kPowerConfig_SinkDefault, /* typec role */
    kCurrent_1A5,             /* source: Rp current level */
    kTypecTry_None,           /* drp try function */
    kDataConfig_DRD,          /* data function */
    1,                        /* support vconn */
    0,                        /* reserved */
    NULL,
    NULL,
    NULL,
};
#endif

#if (defined PD_COMPLIANCE_TEST_PROVIDER_CONSUMER) && (PD_COMPLIANCE_TEST_PROVIDER_CONSUMER)
pd_power_port_config_t g_PowerPortConfig = {
    NULL,                       /* source caps */
    NULL,                       /* self sink caps */
    0,                          /* source cap count */
    0,                          /* sink cap count */
    kPowerConfig_SourceDefault, /* typec role */
    kCurrent_1A5,               /* source: Rp current level */
    kTypecTry_None,             /* drp try function */
    kDataConfig_DRD,            /* data function */
    1,                          /* support vconn */
    0,                          /* reserved */
    NULL,
    NULL,
    NULL,
};
#endif

//instace board configuration
// device type
// phy interrupt num
// phy type
// phy interface, i2c or spi
// phy interface, slave address
// device config which depends on device type 
pd_instance_config_t g_PDConfig = {
    kDeviceType_NormalPowerPort,  /* normal power port */
    PORTB_PORTC_PORTD_PORTE_IRQn, /* interrupt number */
    kPD_PhyPTN5110,
    kInterface_i2c1,
    0x50u,
    &g_PowerPortConfig,
};

// 在pd_power_interface.c中定义, pd_power_interface.h中声明, 赋值给pd_instance_t.callbackFns
pd_power_handle_callback_t callbackFunctions = {
    PD_PowerSrcTurnOnDefaultVbus,  PD_PowerSrcTurnOnRequestVbus,  PD_PowerSrcTurnOffVbus,
    PD_PowerSrcGotoMinReducePower, PD_PowerSnkDrawTypeCVbus,      PD_PowerSnkDrawRequestVbus,
    PD_PowerSnkStopDrawVbus,       PD_PowerSnkGotoMinReducePower, PD_PowerControlVconn,
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
            pdAppInstance->selfHasEnterAlernateMode = 0;
            PRINTF("disconnect\r\n");
            status = kStatus_PD_Success;
            break;

        case PD_CONNECT_ROLE_CHANGE:
        case PD_CONNECTED:
        {
            uint8_t getInfo;

            pdAppInstance->selfHasEnterAlernateMode = 0;
            g_PDAppInstance.partnerSourceCapNumber = 0;
            g_PDAppInstance.partnerSinkCapNumber = 0;
            PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_POWER_ROLE, &getInfo);

            PRINTF((event == PD_CONNECTED) ? "connected," : "connect change,");
            PRINTF(" power role:%s,", (getInfo == kPD_PowerRoleSource) ? "Source" : "Sink");
            PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_DATA_ROLE, &getInfo);
            PRINTF(" data role:%s,", (getInfo == kPD_DataRoleDFP) ? "DFP" : "UFP");
            PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_VCONN_ROLE, &getInfo);
            PRINTF(" vconn source:%s\r\n", (getInfo == kPD_IsVconnSource) ? "yes" : "no");
            status = kStatus_PD_Success;
            break;
        }

        default:
            break;
    }

    return status;
}

// DPM Event Callback
// Question: 初步看下来只是为了根据操作情况输出相关信息，提升用户体验而已？
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
	/* port config */
	//因为给的是指针，所以sourceCaps 指向 selfSourcePdo1
	// Question: g_PowerPortConfig的作用是什么？
    g_PowerPortConfig.sourceCaps = (uint32_t *)&g_PDAppInstance.selfSourcePdo1, /* source caps */
        g_PowerPortConfig.sourceCapCount = 2;
    g_PowerPortConfig.sinkCaps = (uint32_t *)&g_PDAppInstance.selfSinkPdo1, /* self sink caps */
        g_PowerPortConfig.sinkCapCount = 2;
	// initialize pd_instance, pd_app_t.pdHandle = pd_instance_t
    if (PD_InstanceInit(&g_PDAppInstance.pdHandle, PD_DpmDemoAppCallback, &callbackFunctions, &g_PDAppInstance,
                        &g_PDConfig) != kStatus_PD_Success)
    {
        PRINTF("pd init fail\r\n");
    }

    /* initialize port interrupt */
	//设置MCU分配给PTN5110的GPIO pin
    PORT_SetPinInterruptConfig(BOARD_PTN5110_GPIO_PORT, BOARD_PTN5110_GPIO_PIN, kPORT_InterruptLogicZero);
    BOARD_PTN5110_GPIO->PDDR &= ~(1U << BOARD_PTN5110_GPIO_PIN);    /* gpio set as input */
    PD_EXTRA_EN_SRC_GPIO->PDDR |= (1U << PD_EXTRA_EN_SRC_GPIO_PIN); /* as output */

	//设置PTN5110 GPIO Interrupt的优先级
    NVIC_SetPriority((IRQn_Type)BOARD_PTN5110_GPIO_IRQ, PD_PTN5110_GPIO_INTERRUPT_PRIORITY);

    PD_PowerBoardReset();

	// pd_app_t type
	// Question: pd_app_t 的作用是什么？
    g_PDAppInstance.msgSop = kPD_MsgSOP;
    g_PDAppInstance.partnerSourceCapNumber = 0;
    g_PDAppInstance.partnerSinkCapNumber = 0;
    g_PDAppInstance.reqestResponse = kCommandResult_Accept;
    /* PDO1: fixed supply: dual-role power; Externally Powered; no USB communication; dual-role data; 5V; 2.7A */
    g_PDAppInstance.selfSourcePdo1.PDOValue = 0;
    g_PDAppInstance.selfSourcePdo1.fixedPDO.dualRoleData = 1;
    g_PDAppInstance.selfSourcePdo1.fixedPDO.dualRolePower = 1;
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
    g_PDAppInstance.selfSourcePdo2.fixedPDO.voltage = (9 * 1000 / 50);
    g_PDAppInstance.selfSouceCapNumber = 2;
    /* PDO1: fixed:5.0V, 2.7A */
    g_PDAppInstance.selfSinkPdo1.PDOValue = 0;
    g_PDAppInstance.selfSinkPdo1.fixedPDO.fixedSupply = kPDO_Fixed;
    g_PDAppInstance.selfSinkPdo1.fixedPDO.dualRoleData = 1;
    g_PDAppInstance.selfSinkPdo1.fixedPDO.dualRolePower = 1;
    g_PDAppInstance.selfSinkPdo1.fixedPDO.externalPowered = 1;
#if ((defined PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30))
    g_PDAppInstance.selfSinkPdo1.fixedPDO.frSwapRequiredCurrent = kFRSwap_CurrentDefaultUSB;
#endif
    g_PDAppInstance.selfSinkPdo1.fixedPDO.higherCapability = 1;
    g_PDAppInstance.selfSinkPdo1.fixedPDO.usbCommunicationsCapable = 0;
    g_PDAppInstance.selfSinkPdo1.fixedPDO.voltage = (5 * 1000 / 50);
    g_PDAppInstance.selfSinkPdo1.fixedPDO.operateCurrent = (27 * 10);
    /* PDO2: fixed: 9V, 1.5A */
    g_PDAppInstance.selfSinkPdo2.PDOValue = 0;
    g_PDAppInstance.selfSinkPdo2.fixedPDO.fixedSupply = kPDO_Fixed;
    g_PDAppInstance.selfSinkPdo2.fixedPDO.voltage = (9 * 1000 / 50);
    g_PDAppInstance.selfSinkPdo2.fixedPDO.operateCurrent = (15 * 10);
    g_PDAppInstance.selfSinkCapNumber = 2;
    /* default SVDM command header */
    g_PDAppInstance.defaultSVDMCommandHeader.bitFields.objPos = 0;
    g_PDAppInstance.defaultSVDMCommandHeader.bitFields.vdmVersion = PD_CONFIG_STRUCTURED_VDM_VERSION;
    g_PDAppInstance.defaultSVDMCommandHeader.bitFields.vdmType = 1;
    g_PDAppInstance.defaultSVDMCommandHeader.bitFields.SVID = PD_STANDARD_ID;
    /* self ext cap */
    g_PDAppInstance.selfExtCap.vid = PD_VENDOR_VID;
    g_PDAppInstance.selfExtCap.pid = PD_CONFIG_PID;
    g_PDAppInstance.selfExtCap.xid = PD_CONFIG_XID;
    g_PDAppInstance.selfExtCap.fwVersion = PD_CONFIG_FW_VER;
    g_PDAppInstance.selfExtCap.hwVersion = PD_CONFIG_HW_VER;
    /* self alert */
    g_PDAppInstance.selfAlert.alertValue = 0u;
    /* self battery */
    g_PDAppInstance.selfBatteryCap.batteryDesignCap = 10;
    g_PDAppInstance.selfBatteryCap.batteryLastFullChargeCap = 10;
    g_PDAppInstance.selfBatteryCap.batteryType = 0;
    g_PDAppInstance.selfBatteryCap.pid = PD_CONFIG_PID;
    g_PDAppInstance.selfBatteryCap.vid = PD_VENDOR_VID;
    g_PDAppInstance.selfBatteryStatus.batterInfo = 0;
    g_PDAppInstance.selfBatteryStatus.batteryPC = 10;
    /* manufacturer string */
    g_PDAppInstance.selfManufacInfo.vid = PD_VENDOR_VID;
    g_PDAppInstance.selfManufacInfo.pid = PD_CONFIG_PID;
    g_PDAppInstance.selfManufacInfo.manufacturerString[0] = 'N';
    g_PDAppInstance.selfManufacInfo.manufacturerString[1] = 'X';
    g_PDAppInstance.selfManufacInfo.manufacturerString[2] = 'P';
    /* alternate mode (VDM) */
    g_PDAppInstance.selfVdmIdentity.idHeaderVDO.vdoValue = 0;
    g_PDAppInstance.selfVdmIdentity.idHeaderVDO.bitFields.modalOperateSupport = 1;
#if ((defined PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30))
    g_PDAppInstance.selfVdmIdentity.idHeaderVDO.bitFields.productTypeDFP = 2; /* PDUSB Host */
#endif
    g_PDAppInstance.selfVdmIdentity.idHeaderVDO.bitFields.productTypeUFPOrCablePlug = 2; /* PDUSB Peripheral */
    g_PDAppInstance.selfVdmIdentity.idHeaderVDO.bitFields.usbCommunicationCapableAsDevice = 0;
    g_PDAppInstance.selfVdmIdentity.idHeaderVDO.bitFields.usbCommunicationCapableAsHost = 0;
    g_PDAppInstance.selfVdmIdentity.idHeaderVDO.bitFields.usbVendorID = PD_VENDOR_VID;
    g_PDAppInstance.selfVdmIdentity.pid = PD_CONFIG_PID;
    g_PDAppInstance.selfVdmIdentity.certStatVDO = PD_CONFIG_XID;
    g_PDAppInstance.selfVdmIdentity.bcdDevice = PD_CONFIG_BCD_DEVICE;
    g_PDAppInstance.selfVdmSVIDs = ((uint32_t)PD_VENDOR_VID << 16); /* only one SVID (display port) */
    g_PDAppInstance.selfVdmModes = PD_CONFIG_APP_MODE;              /* only one Mode */
    /* evaluate result */
    g_PDAppInstance.prSwapAccept = 1;
    g_PDAppInstance.drSwapAccept = 1;
    g_PDAppInstance.vconnSwapAccept = 1;
    /* other */
    g_PDAppInstance.selfHasEnterAlernateMode = 0;
}
