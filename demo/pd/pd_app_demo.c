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

#include <stdarg.h>
#include <stdio.h>
#include "usb_pd_config.h"
#include "usb_pd.h"
#include "board.h"
#include "pd_app.h"
#include "fsl_debug_console.h"
#include "fsl_port.h"
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

static void USB_PDDemoPrintMenu(pd_app_t *pdAppInstance)
{
    uint8_t powerRole;
    uint8_t dataRole;

    PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_POWER_ROLE, &powerRole);
    PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_DATA_ROLE, &dataRole);

    if (powerRole == kPD_PowerRoleSource)
    {
        /* source role */
        PRINTF("The menu is as follow for source:\r\n");
        PRINTF("0. print menu\r\n");
        PRINTF("a. source power change\r\n");
        PRINTF("b. goto min\r\n");
#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
        PRINTF("c. fast role swap\r\n");
#endif
    }
    else
    {
        /* sink role */
        PRINTF("The menu is as follow for sink:\r\n");
        PRINTF("0. print menu\r\n");
        PRINTF("a. get partner source capabilities\r\n");
    }

    PRINTF("e. soft reset\r\n");
    PRINTF("f. data role swap\r\n");
    PRINTF("g. vconn swap\r\n");
    PRINTF("h. get partner sink capabilities\r\n");
    PRINTF("i. standard structured VDM test (only DFP can send enter mode)\r\n");
    if (dataRole == kPD_DataRoleDFP)
    {
        PRINTF("j. exit mode (only DFP)\r\n");
    }
    PRINTF("k. send attention\r\n");
    PRINTF("l. test vendor structured VDM\r\n");
    PRINTF("m. test unstructured VDM\r\n");
#if ((defined PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30))
    PRINTF("n. get source extended capabilities\r\n");
    PRINTF("o. get status\r\n");
    PRINTF("p. alert\r\n");
    PRINTF("q. get battery capabilities\r\n");
    PRINTF("r. get battery status\r\n");
    PRINTF("s. get manufacturer info\r\n");
#endif
    PRINTF("t. cable reset  (not supported yet)\r\n");
}

static pd_status_t PD_DemoConsoleTryReadChar(char *value)
{
#if (BOARD_DEBUG_UART_TYPE == DEBUG_CONSOLE_DEVICE_TYPE_UART)
    UART_Type *base = (UART_Type *)BOARD_DEBUG_UART_BASEADDR;
#if defined(FSL_FEATURE_UART_HAS_FIFO) && FSL_FEATURE_UART_HAS_FIFO
    if (base->RCFIFO)
#else
    if (base->S1 & UART_S1_RDRF_MASK)
#endif
    {
        *value = (char)base->D;
        return kStatus_PD_Success;
    }
    else
    {
        return kStatus_PD_Error;
    }

#elif(BOARD_DEBUG_UART_TYPE == DEBUG_CONSOLE_DEVICE_TYPE_LPUART)
    LPUART_Type *base = (LPUART_Type *)BOARD_DEBUG_UART_BASEADDR;
#if defined(FSL_FEATURE_LPUART_HAS_FIFO) && FSL_FEATURE_LPUART_HAS_FIFO
    if ((base->WATER & LPUART_WATER_RXCOUNT_MASK) >> LPUART_WATER_RXCOUNT_SHIFT)
#else
    if (base->STAT & LPUART_STAT_RDRF_MASK)
#endif
    {
        *value = (char)base->DATA;
        return kStatus_PD_Success;
    }
    else
    {
        if (base->STAT & LPUART_STAT_OR_MASK)
        {
            base->STAT &= LPUART_STAT_OR_MASK;
        }
        return kStatus_PD_Error;
    }

#elif(BOARD_DEBUG_UART_TYPE == DEBUG_CONSOLE_DEVICE_TYPE_LPSCI)
    UART0_Type *base = (UART0_Type *)BOARD_DEBUG_UART_BASEADDR;
#if defined(FSL_FEATURE_LPSCI_HAS_FIFO) && FSL_FEATURE_LPSCI_HAS_FIFO
    if (base->RCFIFO)
#else
    if (base->S1 & UART0_S1_RDRF_MASK)
#endif
    {
        *value = (char)base->DATA;
        return kStatus_PD_Success;
    }
    else
    {
        return kStatus_PD_Error;
    }
#endif
}

static void PD_DemoConsoleClearInputCache(void)
{
    volatile uint8_t discardValue;

#if (BOARD_DEBUG_UART_TYPE == DEBUG_CONSOLE_DEVICE_TYPE_UART)
    UART_Type *base = (UART_Type *)BOARD_DEBUG_UART_BASEADDR;
#if defined(FSL_FEATURE_UART_HAS_FIFO) && FSL_FEATURE_UART_HAS_FIFO
    while (base->RCFIFO)
#else
    while (base->S1 & UART_S1_RDRF_MASK)
#endif
    {
        discardValue = base->D;
    }

#elif(BOARD_DEBUG_UART_TYPE == DEBUG_CONSOLE_DEVICE_TYPE_LPUART)
    LPUART_Type *base = (LPUART_Type *)BOARD_DEBUG_UART_BASEADDR;
#if defined(FSL_FEATURE_LPUART_HAS_FIFO) && FSL_FEATURE_LPUART_HAS_FIFO
    while ((base->WATER & LPUART_WATER_RXCOUNT_MASK) >> LPUART_WATER_RXCOUNT_SHIFT)
#else
    while (base->STAT & LPUART_STAT_RDRF_MASK)
#endif
    {
        discardValue = base->DATA;
    }

#elif(BOARD_DEBUG_UART_TYPE == DEBUG_CONSOLE_DEVICE_TYPE_LPSCI)
    UART0_Type *base = (UART0_Type *)BOARD_DEBUG_UART_BASEADDR;
#if defined(FSL_FEATURE_LPSCI_HAS_FIFO) && FSL_FEATURE_LPSCI_HAS_FIFO
    while (base->RCFIFO)
#else
    while (base->S1 & UART0_S1_RDRF_MASK)
#endif
    {
        discardValue = base->DATA;
    }
#endif
}

static void USB_PDDemoProcessMenu(pd_app_t *pdAppInstance, char ch)
{
    uint8_t powerRole;
    uint8_t dataRole;
    uint32_t commandValid;
    void *commandParam;
    pd_unstructured_vdm_command_param_t unstructuredVDMCommandParam;
#if ((defined PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30))
    pd_command_data_param_t extCommandParam;
#endif

    PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_POWER_ROLE, &powerRole);
    /* function codes */
    if ((ch >= '0') && (ch <= '9'))
    {
        commandValid = 0;
        switch (ch)
        {
            case '0':
                USB_PDDemoPrintMenu(pdAppInstance);
                break;

            default:
                break;
        }
    }
    else if ((ch >= 'a') && (ch <= 'z'))
    {
        PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_DATA_ROLE, &dataRole);
        commandValid = 0;

        switch (ch)
        {
            case 'a':
                if (powerRole == kPD_PowerRoleSource)
                {
                    PRINTF("a. source power change\r\n");
                    commandValid = PD_DPM_CONTROL_POWER_NEGOTIATION;
                    commandParam = NULL;
                }
                else
                {
                    PRINTF("a. get partner source capabilities\r\n");
                    commandValid = PD_DPM_CONTROL_GET_PARTNER_SOURCE_CAPABILITIES;
                    commandParam = NULL;
                }
                break;

            case 'b':
                if (powerRole == kPD_PowerRoleSource)
                {
                    PRINTF("b. goto min\r\n");
                    if (!(pdAppInstance->sinkRequestRDO.bitFields.giveBack))
                    {
                        PRINTF("warning: the GiveBack flag is not set\r\n");
                    }
                    commandValid = PD_DPM_CONTROL_GOTO_MIN;
                    commandParam = NULL;
                }
                break;

#if defined(PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE) && (PD_CONFIG_PD3_FAST_ROLE_SWAP_ENABLE)
            case 'c':
                if (powerRole == kPD_PowerRoleSource)
                {
                    PRINTF("c. fast role swap\r\n");
                    commandValid = PD_DPM_FAST_ROLE_SWAP;
                    commandParam = NULL;
                }
                break;
#endif

            case 'e':
                PRINTF("e. soft reset\r\n");
                commandValid = PD_DPM_CONTROL_SOFT_RESET;
                commandParam = &pdAppInstance->msgSop;
                break;

            case 'f':
                PRINTF("f. data role swap\r\n");
                PRINTF("warning: hard reset will occur if in alternate mode (menu 'i')\r\n");
                commandValid = PD_DPM_CONTROL_DR_SWAP;
                commandParam = NULL;
                break;

            case 'g':
                PRINTF("g. vconn swap\r\n");
                commandValid = PD_DPM_CONTROL_VCONN_SWAP;
                commandParam = NULL;
                break;

            case 'h':
                PRINTF("h. get partner sink capabilities\r\n");
                commandValid = PD_DPM_CONTROL_GET_PARTNER_SINK_CAPABILITIES;
                commandParam = NULL;
                break;

            case 'i':
                PRINTF("i. standard structured VDM test (only DFP can send enter mode)\r\n");
                PRINTF("(1) discovery identity\r\n");
                commandValid = PD_DPM_CONTROL_DISCOVERY_IDENTITY;
                pdAppInstance->structuredVDMCommandParam.vdmSop = pdAppInstance->msgSop;
                commandParam = &pdAppInstance->structuredVDMCommandParam;
                break;

            case 'j':
                if (dataRole == kPD_DataRoleDFP)
                {
                    PRINTF("j. exit mode (only DFP)\r\n");
                    commandValid = PD_DPM_CONTROL_EXIT_MODE;
                    pdAppInstance->structuredVDMCommandParam.vdmSop = pdAppInstance->msgSop;
                    pdAppInstance->structuredVDMCommandParam.vdmHeader.bitFields.objPos = 1;
                    pdAppInstance->structuredVDMCommandParam.vdmHeader.bitFields.SVID = pdAppInstance->partnerSVIDs[0];
                    commandParam = &pdAppInstance->structuredVDMCommandParam;
                }
                break;

            case 'k':
                PRINTF("k. send attention\r\n");
                commandValid = PD_DPM_CONTROL_SEND_ATTENTION;
                pdAppInstance->structuredVDMCommandParam.vdmSop = pdAppInstance->msgSop;
                pdAppInstance->structuredVDMCommandParam.vdoCount = 0;
                pdAppInstance->structuredVDMCommandParam.vdoData = NULL;
                pdAppInstance->structuredVDMCommandParam.vdmHeader.bitFields.objPos = 1;
                pdAppInstance->structuredVDMCommandParam.vdmHeader.bitFields.SVID = pdAppInstance->partnerSVIDs[0];
                commandParam = &pdAppInstance->structuredVDMCommandParam;
                break;

            case 'l':
                PRINTF("l. test vendor structured VDM\r\n");
                commandValid = PD_DPM_SEND_VENDOR_STRUCTURED_VDM;
                pdAppInstance->structuredVDMCommandParam.vdmSop = pdAppInstance->msgSop;
                pdAppInstance->structuredVDMCommandParam.vdoCount = 0;
                pdAppInstance->structuredVDMCommandParam.vdoData = NULL;
                pdAppInstance->structuredVDMCommandParam.vdmHeader.bitFields.SVID = PD_VENDOR_VID;
                pdAppInstance->structuredVDMCommandParam.vdmHeader.bitFields.command = 16;
                pdAppInstance->structuredVDMCommandParam.vendorVDMNeedResponse = 1;
                commandParam = &pdAppInstance->structuredVDMCommandParam;
                break;

            case 'm':
            {
                PRINTF("m. test unstructured VDM\r\n");
                commandValid = PD_DPM_SEND_UNSTRUCTURED_VDM;
                pdAppInstance->unstructuredVDMCommandHeader.bitFields.SVID = PD_VENDOR_VID;
                pdAppInstance->unstructuredVDMCommandHeader.bitFields.vdmType = 0;
                unstructuredVDMCommandParam.vdmSop = pdAppInstance->msgSop;
                unstructuredVDMCommandParam.vdmHeaderAndVDOsCount = 1;
                unstructuredVDMCommandParam.vdmHeaderAndVDOsData =
                    (uint32_t *)&pdAppInstance->unstructuredVDMCommandHeader;
                commandParam = &unstructuredVDMCommandParam;
                break;
            }

#if ((defined PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30))
            case 'n':
            {
                PRINTF("n. get source extended capabilities\r\n");
                commandValid = PD_DPM_GET_SRC_EXT_CAP;
                commandParam = NULL;
                break;
            }

            case 'o':
            {
                PRINTF("o. get status\r\n");
                commandValid = PD_DPM_GET_STATUS;
                commandParam = NULL;
                break;
            }

            case 'p':
            {
                PRINTF("p. alert\r\n");
                commandValid = PD_DPM_ALERT;
                pdAppInstance->selfAlert.bitFields.typeOfAlert = 0x02u; /* battery status change */
                commandParam = &pdAppInstance->selfAlert;
                break;
            }

            case 'q':
            {
                PRINTF("q. get battery capabilities\r\n");
                commandValid = PD_DPM_GET_BATTERY_CAP;
                pdAppInstance->getBatteryCapDataBlock = 0x01u; /* get battery 1 cap */
                commandParam = &pdAppInstance->getBatteryCapDataBlock;
                break;
            }

            case 'r':
            {
                PRINTF("r. get battery status\r\n");
                commandValid = PD_DPM_GET_BATTERY_STATUS;
                pdAppInstance->getBatteryCapDataBlock = 0x01u; /* get battery 1 cap */
                commandParam = &pdAppInstance->getBatteryCapDataBlock;
                break;
            }

            case 's':
            {
                PRINTF("s. get manufacturer info\r\n");
                pdAppInstance->commonData[0] = 1;
                pdAppInstance->commonData[1] = 0; /* battery zero */
                extCommandParam.dataBuffer = &pdAppInstance->commonData[0];
                extCommandParam.dataLength = 2;
                extCommandParam.sop = pdAppInstance->msgSop;
                commandValid = PD_DPM_GET_MANUFACTURER_INFO;
                commandParam = &extCommandParam;
                break;
            }
#endif

#if 0
        case 't':
            PRINTF("t. cable reset\r\n");
            commandValid = PD_DPM_CONTROL_CABLE_RESET;
            commandParam = NULL;
            break;
#endif

            default:
                break;
        }

        if (commandValid)
        {
            if (PD_Command(pdAppInstance->pdHandle, commandValid, commandParam) != kStatus_PD_Success)
            {
                PRINTF("command fail\r\n");
            }
            else
            {
                if (commandValid == PD_DPM_FAST_ROLE_SWAP)
                {
                    PD_PowerSrcTurnOffVbus(pdAppInstance, kVbusPower_Stable);
                }
            }
        }
    }
    else
    {
    }
}

void PD_DemoInit(pd_app_t *pdAppInstance)
{
    g_PDAppInstance.sw1State = kDEMO_SWIdle;
    g_PDAppInstance.sw3State = kDEMO_SWIdle;
    /* button */
    gpio_pin_config_t sw_config = {
        kGPIO_DigitalInput, 0,
    };
    /* Init input switch GPIO. */
    GPIO_PinInit(BOARD_SW3_GPIO, BOARD_SW3_GPIO_PIN, &sw_config);

    GPIO_PinInit(BOARD_SW1_GPIO, BOARD_SW1_GPIO_PIN, &sw_config);

    /* clear debug console input cache */
    PD_DemoConsoleClearInputCache();

    pdAppInstance->printProvider = 0;
    pdAppInstance->printConsumer = 0;
    pdAppInstance->printFull = 0;
}

void PD_Demo1msIsrProcessSW(pd_app_t *pdAppInstance)
{
    if (pdAppInstance->sw1State == kDEMO_SWIsrTrigger)
    {
        /* 10ms as short press, 700ms as long press */
        if (GPIO_ReadPinInput(BOARD_SW1_GPIO, BOARD_SW1_GPIO_PIN) == 0)
        {
            pdAppInstance->sw1Time++;
            if (pdAppInstance->sw1Time > 700)
            {
                pdAppInstance->sw1State = kDEMO_SWLongPress;
            }
        }
        else
        {
            if (pdAppInstance->sw1Time > 10)
            {
                pdAppInstance->sw1State = kDEMO_SWShortPress;
            }
            else
            {
                pdAppInstance->sw1State = kDEMO_SWIdle;
            }
        }
    }
    else if (pdAppInstance->sw1State == kDEMO_SWProcessed)
    {
        if (GPIO_ReadPinInput(BOARD_SW1_GPIO, BOARD_SW1_GPIO_PIN) == 1)
        {
            pdAppInstance->sw1State = kDEMO_SWIdle;
        }
    }
    else if (pdAppInstance->sw1State == kDEMO_SWIdle)
    {
        if (GPIO_ReadPinInput(BOARD_SW1_GPIO, BOARD_SW1_GPIO_PIN) == 0)
        {
            pdAppInstance->sw1Time = 0u;
            pdAppInstance->sw1State = kDEMO_SWIsrTrigger;
        }
    }
    else
    {
    }

    if (pdAppInstance->sw3State == kDEMO_SWIsrTrigger)
    {
        /* 10ms as short press, 700ms as long press */
        if (GPIO_ReadPinInput(BOARD_SW3_GPIO, BOARD_SW3_GPIO_PIN) == 0)
        {
            pdAppInstance->sw3Time++;
            if (pdAppInstance->sw3Time > 700)
            {
                pdAppInstance->sw3State = kDEMO_SWLongPress;
            }
        }
        else
        {
            if (pdAppInstance->sw3Time > 10)
            {
                pdAppInstance->sw3State = kDEMO_SWShortPress;
            }
            else
            {
                pdAppInstance->sw3State = kDEMO_SWIdle;
            }
        }
    }
    else if (pdAppInstance->sw3State == kDEMO_SWProcessed)
    {
        if (GPIO_ReadPinInput(BOARD_SW3_GPIO, BOARD_SW3_GPIO_PIN) == 1)
        {
            pdAppInstance->sw3State = kDEMO_SWIdle;
        }
    }
    else if (pdAppInstance->sw3State == kDEMO_SWIdle)
    {
        if (GPIO_ReadPinInput(BOARD_SW3_GPIO, BOARD_SW3_GPIO_PIN) == 0)
        {
            pdAppInstance->sw3Time = 0u;
            pdAppInstance->sw3State = kDEMO_SWIsrTrigger;
        }
    }
    else
    {
    }
}

void USB_PDDemoProcessSW(pd_app_t *pdAppInstance)
{
    uint8_t powerRole;

    PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_POWER_ROLE, &powerRole);

    if (pdAppInstance->sw1State == kDEMO_SWShortPress)
    {
        pdAppInstance->sw1State = kDEMO_SWProcessed;
        if (powerRole == kPD_PowerRoleSink)
        {
            PRINTF("request 5V\r\n");

            pdAppInstance->sinkRequestVoltage = 5000;
            pdAppInstance->sinkRequestRDO.bitFields.objectPosition = 1;
            pdAppInstance->sinkRequestRDO.bitFields.giveBack = 0;
            pdAppInstance->sinkRequestRDO.bitFields.capabilityMismatch = 0;
            pdAppInstance->sinkRequestRDO.bitFields.usbCommunicationsCapable = 0;
            pdAppInstance->sinkRequestRDO.bitFields.noUsbSuspend = 1;
            pdAppInstance->sinkRequestRDO.bitFields.operateValue = 270; /* 2.7A */
            pdAppInstance->sinkRequestRDO.bitFields.maxOrMinOperateValue = 270;

            if (PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_REQUEST, &pdAppInstance->sinkRequestRDO) !=
                kStatus_PD_Success)
            {
                PRINTF("request call fail\r\n");
            }
        }
    }
    else if (pdAppInstance->sw1State == kDEMO_SWLongPress)
    {
        pdAppInstance->sw1State = kDEMO_SWProcessed;
        if (powerRole == kPD_PowerRoleSink)
        {
            PRINTF("request 9V\r\n");

            pdAppInstance->sinkRequestVoltage = 9000;
            pdAppInstance->sinkRequestRDO.bitFields.objectPosition = 2;
            pdAppInstance->sinkRequestRDO.bitFields.giveBack = 1;
            pdAppInstance->sinkRequestRDO.bitFields.capabilityMismatch = 0;
            pdAppInstance->sinkRequestRDO.bitFields.usbCommunicationsCapable = 0;
            pdAppInstance->sinkRequestRDO.bitFields.noUsbSuspend = 1;
            pdAppInstance->sinkRequestRDO.bitFields.operateValue = 150;         /* 1.5A */
            pdAppInstance->sinkRequestRDO.bitFields.maxOrMinOperateValue = 100; /* 1A */

            if (PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_REQUEST, &pdAppInstance->sinkRequestRDO) !=
                kStatus_PD_Success)
            {
                PRINTF("request call fail\r\n");
            }
        }
    }
    else
    {
    }

    if (pdAppInstance->sw3State == kDEMO_SWShortPress)
    {
        PRINTF("request power role swap\r\n");
        pdAppInstance->sw3State = kDEMO_SWProcessed;
        if (PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_PR_SWAP, NULL) != kStatus_PD_Success)
        {
            PRINTF("request call fail\r\n");
        }
    }
    else if (pdAppInstance->sw3State == kDEMO_SWLongPress)
    {
        PRINTF("hard reset\r\n");
        pdAppInstance->sw3State = kDEMO_SWProcessed;
        if (PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_HARD_RESET, NULL) != kStatus_PD_Success)
        {
            PRINTF("request call fail\r\n");
        }
    }
    else
    {
    }
}

pd_status_t PD_DemoFindPDO(pd_app_t *pdAppInstance, pd_rdo_t *rdo, uint32_t *voltage)
{
    uint32_t index;
    pd_source_pdo_t sourcePDO;
    volatile uint32_t requestVoltage;
    volatile uint32_t requestCurrent;
    uint8_t findSourceCap = 0;

    if (pdAppInstance->partnerSourceCapNumber == 0)
    {
        return kStatus_PD_Error;
    }

    /* default rdo as 5V - 0.5A or less */
    *voltage = 5000;
    rdo->bitFields.objectPosition = 1;
    rdo->bitFields.giveBack = 0;
    rdo->bitFields.capabilityMismatch = 0;
    rdo->bitFields.usbCommunicationsCapable = 0;
    rdo->bitFields.noUsbSuspend = 1;
    rdo->bitFields.operateValue = 500 / PD_PDO_CURRENT_UNIT;
    if (rdo->bitFields.operateValue > pdAppInstance->partnerSourceCaps[0].fixedPDO.maxCurrent)
    {
        rdo->bitFields.operateValue = pdAppInstance->partnerSourceCaps[0].fixedPDO.maxCurrent;
    }
    rdo->bitFields.maxOrMinOperateValue = rdo->bitFields.operateValue;

    requestVoltage = PD_DEMO_EXPECTED_VOLTAGE;
    requestCurrent = PD_DEMO_EXPECTED_CURRENT;

    for (index = 0; index < pdAppInstance->partnerSourceCapNumber; ++index)
    {
        sourcePDO.PDOValue = pdAppInstance->partnerSourceCaps[index].PDOValue;
        switch (sourcePDO.commonPDO.pdoType)
        {
            case kPDO_Fixed:
            {
                if ((sourcePDO.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT == requestVoltage) &&
                    (sourcePDO.fixedPDO.maxCurrent * PD_PDO_CURRENT_UNIT >= requestCurrent))
                {
                    *voltage = sourcePDO.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT;
                    rdo->bitFields.objectPosition = (index + 1);
                    rdo->bitFields.operateValue = requestCurrent / PD_PDO_CURRENT_UNIT;
                    rdo->bitFields.maxOrMinOperateValue = rdo->bitFields.operateValue;
                    findSourceCap = 1;
                }
                break;
            }

            case kPDO_Variable:
            {
                if ((sourcePDO.variablePDO.minVoltage * PD_PDO_VOLTAGE_UNIT <= requestVoltage) &&
                    (sourcePDO.variablePDO.maxVoltage * PD_PDO_VOLTAGE_UNIT >= requestVoltage) &&
                    (sourcePDO.variablePDO.maxCurrent * PD_PDO_CURRENT_UNIT >= requestCurrent))
                {
                    *voltage = sourcePDO.variablePDO.minVoltage * PD_PDO_VOLTAGE_UNIT;
                    rdo->bitFields.objectPosition = (index + 1);
                    rdo->bitFields.operateValue = requestCurrent / PD_PDO_CURRENT_UNIT;
                    rdo->bitFields.maxOrMinOperateValue = rdo->bitFields.operateValue;
                    findSourceCap = 1;
                }
                break;
            }

            case kPDO_Battery:
            {
                if ((sourcePDO.batteryPDO.minVoltage * PD_PDO_VOLTAGE_UNIT <= requestVoltage) &&
                    (sourcePDO.batteryPDO.maxVoltage * PD_PDO_VOLTAGE_UNIT >= requestVoltage) &&
                    (sourcePDO.batteryPDO.maxAllowPower * PD_PDO_POWER_UNIT >=
                     (requestVoltage * requestCurrent / 1000)))
                {
                    *voltage = sourcePDO.batteryPDO.minVoltage * PD_PDO_VOLTAGE_UNIT;
                    rdo->bitFields.objectPosition = (index + 1);
                    rdo->bitFields.operateValue = (requestVoltage * requestCurrent) / 1000 / PD_PDO_POWER_UNIT;
                    rdo->bitFields.maxOrMinOperateValue = rdo->bitFields.operateValue;
                    findSourceCap = 1;
                }
                break;
            }

            default:
                break;
        }

        if (findSourceCap)
        {
            break;
        }
    }

    if (findSourceCap)
    {
        return kStatus_PD_Success;
    }
    return kStatus_PD_Error;
}

uint32_t PD_DemoPrint(pd_app_t *pdAppInstance, const char *format, ...)
{
    va_list arg;
    uint32_t cnt;

    if (pdAppInstance->printFull)
    {
        return 0;
    }
    va_start(arg, format);
    cnt = (uint32_t)vsprintf(pdAppInstance->printBuffers[pdAppInstance->printProvider], format, arg);
    va_end(arg);

    pdAppInstance->printProvider = (pdAppInstance->printProvider + 1) % PD_DEMO_PRINT_BUFFER_COUNT;
    if (pdAppInstance->printProvider == pdAppInstance->printConsumer)
    {
        pdAppInstance->printFull = 1;
    }

    return cnt;
}

void PD_DemoProcessPrint(pd_app_t *pdAppInstance)
{
    if ((pdAppInstance->printProvider != pdAppInstance->printConsumer) || (pdAppInstance->printFull))
    {
        pdAppInstance->printFull = 0;
        PRINTF("%s", pdAppInstance->printBuffers[pdAppInstance->printConsumer]);
        pdAppInstance->printConsumer = (pdAppInstance->printConsumer + 1) % PD_DEMO_PRINT_BUFFER_COUNT;
    }
}

void PD_DemoTaskFun(pd_app_t *pdAppInstance)
{
    char uartData;

    PD_DemoProcessPrint(pdAppInstance);
    USB_PDDemoProcessSW(pdAppInstance);
    if (PD_DemoConsoleTryReadChar(&uartData) == kStatus_PD_Success)
    {
        USB_PDDemoProcessMenu(pdAppInstance, uartData);
    }
}
