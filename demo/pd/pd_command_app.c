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

#include <stdint.h>
#include <stdbool.h>
#include "usb_pd_config.h"
#include "usb_pd.h"
#include "usb_pd_phy.h"
#include "pd_app.h"
#include "fsl_debug_console.h"
#include "pd_power_interface.h"
#include "pd_command_interface.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

pd_status_t PD_DemoFindPDO(pd_app_t *pdAppInstance, pd_rdo_t *rdo, uint32_t *voltage);
uint32_t PD_DemoPrint(pd_app_t *pdAppInstance, const char *format, ...);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

pd_status_t PD_DpmSoftResetCallback(void *callbackParam)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    /* reset soft status */
    PD_DemoPrint(pdAppInstance, "app soft reset\r\n");
    return kStatus_PD_Success;
}

pd_status_t PD_DpmHardResetCallback(void *callbackParam)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    /* reset state, The Sink shall not draw more than iSafe0mA when VBUS is driven to vSafe0V. */
    pdAppInstance->selfHasEnterAlernateMode = 0;
    PD_DemoPrint(pdAppInstance, "hard reset request\r\n");
    return kStatus_PD_Success;
}

pd_status_t PD_DpmPowerRoleSwapRequestCallback(void *callbackParam, uint8_t frSwap, uint8_t *evaluateResult)
{
#if (defined PD_CONFIG_COMPLIANCE_TEST_ENABLE) && (PD_CONFIG_COMPLIANCE_TEST_ENABLE)
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    uint8_t powerRole;

    if (frSwap)
    {
        *evaluateResult = kCommandResult_Accept;
    }
    else
    {
        PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_POWER_ROLE, &powerRole);
        if ((powerRole == kPD_PowerRoleSource) && (pdAppInstance->selfSourcePdo1.fixedPDO.externalPowered))
        {
            if ((pdAppInstance->partnerSourceCapNumber == 0) && (g_PDAppInstance.partnerSinkCapNumber == 0))
            {
                *evaluateResult = kCommandResult_Wait;
                PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_GET_PARTNER_SOURCE_CAPABILITIES, NULL);
            }
            else
            {
                if (pdAppInstance->partnerSourceCapNumber)
                {
                    if (pdAppInstance->partnerSourceCaps[0].fixedPDO.externalPowered == 0)
                    {
                        *evaluateResult = kCommandResult_Reject;
                    }
                    else
                    {
                        *evaluateResult = kCommandResult_Accept;
                    }
                }
                else
                {
                    if (pdAppInstance->partnerSinkCaps[0].fixedPDO.externalPowered == 0)
                    {
                        *evaluateResult = kCommandResult_Reject;
                    }
                    else
                    {
                        *evaluateResult = kCommandResult_Accept;
                    }
                }
            }
        }
        else
        {
            *evaluateResult = kCommandResult_Accept;
        }
    }
#else
    *evaluateResult = kCommandResult_Accept;
#endif
    return kStatus_PD_Success;
}

pd_status_t PD_DpmPowerRoleSwapResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    uint8_t roleInfo;

    if (success)
    {
        PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_POWER_ROLE, &roleInfo);
        if (roleInfo == kPD_PowerRoleSource)
        {
            PD_DemoPrint(pdAppInstance, "enter source\r\n");
        }
        else
        {
            PD_DemoPrint(pdAppInstance, "enter sink\r\n");
        }
    }
    else
    {
        switch (failResultType)
        {
            case kCommandResult_Reject:
                PD_DemoPrint(pdAppInstance, "power role swap result: reject\r\n");
                break;

            case kCommandResult_Wait:
                PD_DemoPrint(pdAppInstance, "power role swap result: wait\r\n");
                break;

            case kCommandResult_Error:
                PD_DemoPrint(pdAppInstance, "power role swap result: fail\r\n");
                break;

            case kCommandResult_NotSupported:
                break;

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmDataRoleSwapRequestCallback(void *callbackParam, uint8_t *evaluateResult)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    *evaluateResult = ((pdAppInstance->drSwapAccept) ? kCommandResult_Accept : kCommandResult_Reject);
    return kStatus_PD_Success;
}

pd_status_t PD_DpmDataRoleSwapResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    uint8_t roleInfo;

    if (success)
    {
        PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_DATA_ROLE, &roleInfo);
        if (roleInfo == kPD_DataRoleDFP)
        {
            PD_DemoPrint(pdAppInstance, "dr swap result: enter dfp\r\n");
        }
        else
        {
            PD_DemoPrint(pdAppInstance, "dr swap result: enter ufp\r\n");
        }
    }
    else
    {
        switch (failResultType)
        {
            case kCommandResult_Error:
                PD_DemoPrint(pdAppInstance, "dr swap result: fail\r\n");
                break;

            case kCommandResult_Reject:
                PD_DemoPrint(pdAppInstance, "dr swap result: reject\r\n");
                break;

            case kCommandResult_Wait:
                PD_DemoPrint(pdAppInstance, "dr swap result: wait\r\n");
                break;

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmVconnSwapRequestCallback(void *callbackParam, uint8_t *evaluateResult)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    *evaluateResult = ((pdAppInstance->vconnSwapAccept) ? kCommandResult_Accept : kCommandResult_Reject);
    return kStatus_PD_Success;
}

pd_status_t PD_DpmVconnSwapResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    uint8_t roleInfo;

    if (success)
    {
        PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_VCONN_ROLE, &roleInfo);
        if (roleInfo == kPD_IsVconnSource)
        {
            PD_DemoPrint(pdAppInstance, "vconn swap result: turn as source\r\n");
        }
        else
        {
            PD_DemoPrint(pdAppInstance, "vconn swap result: not vconn source\r\n");
        }
    }
    else
    {
        switch (failResultType)
        {
            case kCommandResult_Reject:
                PD_DemoPrint(pdAppInstance, "vconn swap result: reject\r\n");
                break;

            case kCommandResult_Wait:
                PD_DemoPrint(pdAppInstance, "vconn swap result: wait\r\n");
                break;

            case kCommandResult_Error:
                PD_DemoPrint(pdAppInstance, "vconn swap result: fail\r\n");
                break;

            case kCommandResult_NotSupported:
                break;

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSrcRDORequestCallback(void *callbackParam, pd_rdo_t rdo, uint8_t *negotiateResult)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    uint8_t accept = 0;

    /* prepare for power supply, The power supply shall be ready to operate at the new power level within
     * tSrcReady */
    if (rdo.bitFields.objectPosition == 1)
    {
        if (rdo.bitFields.operateValue <= pdAppInstance->selfSourcePdo1.fixedPDO.maxCurrent)
        {
            accept = 1;
        }
    }
    else if (rdo.bitFields.objectPosition == 2)
    {
        if (rdo.bitFields.operateValue <= pdAppInstance->selfSourcePdo2.fixedPDO.maxCurrent)
        {
            accept = 1;
        }
    }
    else
    {
    }

    if (accept)
    {
        pdAppInstance->sinkRequestRDO.rdoVal = rdo.rdoVal;
        *negotiateResult = kCommandResult_Accept;
    }
    else
    {
        *negotiateResult = kCommandResult_Reject;
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSrcPreContractStillValidCallback(void *callbackParam, uint8_t *isStillValid)
{
    /* if pre contract exist, return true */
    *isStillValid = 1;
    return kStatus_PD_Success;
}

pd_status_t PD_DpmSrcRDOResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    if (success)
    {
        PD_DemoPrint(pdAppInstance, "partner sink's request %dV success\r\n",
                     (pdAppInstance->sinkRequestRDO.bitFields.objectPosition == 1) ? 5 : 9);
    }
    else
    {
        switch (failResultType)
        {
            case kCommandResult_Error:
                PD_DemoPrint(pdAppInstance, "source has error in power negotiation\r\n");
                break;

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmReceivePartnerSrcCapsCallback(void *callbackParam, pd_capabilities_t *caps)
{
    uint32_t index;
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    pd_source_pdo_t sourcePDO;
    PD_DemoPrint(pdAppInstance, "receive source capabilities:\r\n");
    pdAppInstance->partnerSourceCapNumber = caps->capabilitiesCount;
    for (index = 0; index < pdAppInstance->partnerSourceCapNumber; ++index)
    {
        pdAppInstance->partnerSourceCaps[index].PDOValue = caps->capabilities[index];
        sourcePDO.PDOValue = caps->capabilities[index];
        switch (sourcePDO.commonPDO.pdoType)
        {
            case kPDO_Fixed:
            {
                PD_DemoPrint(pdAppInstance, "%d: fixed PDO; ", index + 1);
                PD_DemoPrint(pdAppInstance, "vol:%dmV, current:%dmA\r\n",
                             sourcePDO.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT,
                             sourcePDO.fixedPDO.maxCurrent * PD_PDO_CURRENT_UNIT);
                break;
            }

            case kPDO_Variable:
            {
                PD_DemoPrint(pdAppInstance, "%d: variable PDO; ", index + 1);
                PD_DemoPrint(pdAppInstance, "vol:%dmV ~ %dmV, current:%dmA\r\n",
                             sourcePDO.variablePDO.maxVoltage * PD_PDO_VOLTAGE_UNIT,
                             sourcePDO.variablePDO.minVoltage * PD_PDO_VOLTAGE_UNIT,
                             sourcePDO.variablePDO.maxCurrent * PD_PDO_CURRENT_UNIT);
                break;
            }

            case kPDO_Battery:
            {
                PD_DemoPrint(pdAppInstance, "%d: battery PDO; ", index + 1);
                PD_DemoPrint(pdAppInstance, "vol:%dmV ~ %dmV, power:%dmW\r\n",
                             sourcePDO.batteryPDO.maxVoltage * PD_PDO_VOLTAGE_UNIT,
                             sourcePDO.batteryPDO.minVoltage * PD_PDO_VOLTAGE_UNIT,
                             sourcePDO.batteryPDO.maxAllowPower * PD_PDO_POWER_UNIT);
            }

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmGetPartnerSrcCapsFailCallback(void *callbackParam, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    switch (failResultType)
    {
        case kCommandResult_Error:
            PD_DemoPrint(pdAppInstance, "get src cap fail");
            break;

        case kCommandResult_NotSupported:
            PD_DemoPrint(pdAppInstance, "get src cap replying not supported");
            break;
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSnkGetRequestRDOCallback(void *callbackParam, pd_rdo_t *rdo)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    uint32_t voltage;

    if (PD_DemoFindPDO(pdAppInstance, rdo, &voltage) != kStatus_PD_Success)
    {
        PD_DemoPrint(pdAppInstance, "cap mismatch\r\n");
    }
    pdAppInstance->sinkRequestRDO = *rdo;
    pdAppInstance->sinkRequestVoltage = voltage;

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSnkRDOResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    if (success)
    {
        PD_DemoPrint(pdAppInstance, "sink request %dV success\r\n", pdAppInstance->sinkRequestVoltage / 1000);
    }
    else
    {
        switch (failResultType)
        {
            case kCommandResult_Error:
                PD_DemoPrint(pdAppInstance, "sink request power result: fail\r\n");
                break;

            case kCommandResult_Reject:
                PD_DemoPrint(pdAppInstance, "sink request power result: reject\r\n");
                break;

            case kCommandResult_Wait:
                PD_DemoPrint(pdAppInstance, "sink request power result: wait\r\n");
                break;

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSrcGotoMinResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    if (success)
    {
        PD_DemoPrint(pdAppInstance, "success\r\n");
    }
    else
    {
        PD_DemoPrint(pdAppInstance, "fail\r\n");
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSnkGotoMinResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    if (success)
    {
        PD_DemoPrint(pdAppInstance, "success\r\n");
    }
    else
    {
        PD_DemoPrint(pdAppInstance, "fail\r\n");
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmReceivePartnerSnkCapsCallback(void *callbackParam, pd_capabilities_t *caps)
{
    uint32_t index;
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    pd_sink_pdo_t sinkPDO;
    PD_DemoPrint(pdAppInstance, "receive sink capabilities:\r\n");
    pdAppInstance->partnerSinkCapNumber = caps->capabilitiesCount;
    for (index = 0; index < pdAppInstance->partnerSinkCapNumber; ++index)
    {
        pdAppInstance->partnerSinkCaps[index].PDOValue = caps->capabilities[index];
        sinkPDO.PDOValue = caps->capabilities[index];
        switch (sinkPDO.commonPDO.pdoType)
        {
            case kPDO_Fixed:
            {
                PD_DemoPrint(pdAppInstance, "%d: fixed PDO; ", index + 1);
                PD_DemoPrint(pdAppInstance, "vol:%dmV, current:%dmA\r\n",
                             sinkPDO.fixedPDO.voltage * PD_PDO_VOLTAGE_UNIT,
                             sinkPDO.fixedPDO.operateCurrent * PD_PDO_CURRENT_UNIT);
                break;
            }

            case kPDO_Variable:
            {
                PD_DemoPrint(pdAppInstance, "%d: variable PDO; ", index + 1);
                PD_DemoPrint(pdAppInstance, "vol:%dmV ~ %dmV, current:%dmA\r\n",
                             sinkPDO.variablePDO.maxVoltage * PD_PDO_VOLTAGE_UNIT,
                             sinkPDO.variablePDO.minVoltage * PD_PDO_VOLTAGE_UNIT,
                             sinkPDO.variablePDO.operateCurrent * PD_PDO_CURRENT_UNIT);
                break;
            }

            case kPDO_Battery:
            {
                PD_DemoPrint(pdAppInstance, "%d: battery PDO; ", index + 1);
                PD_DemoPrint(pdAppInstance, "vol:%dmV ~ %dmV, power:%dmW\r\n",
                             sinkPDO.batteryPDO.maxVoltage * PD_PDO_VOLTAGE_UNIT,
                             sinkPDO.batteryPDO.minVoltage * PD_PDO_VOLTAGE_UNIT,
                             sinkPDO.batteryPDO.operatePower * PD_PDO_POWER_UNIT);
            }

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmGetPartnerSnkCapsFailCallback(void *callbackParam, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    switch (failResultType)
    {
        case kCommandResult_Error:
            PD_DemoPrint(pdAppInstance, "get snk cap fail");
            break;

        case kCommandResult_NotSupported:
            PD_DemoPrint(pdAppInstance, "get snk cap replying not supported");
            break;
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSVDMRequestCallback(void *callbackParam, pd_svdm_command_request_t *svdmRequest)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    /* common process */
    switch (svdmRequest->vdmHeader.bitFields.command)
    {
        case kVDM_DiscoverIdentity:
        case kVDM_DiscoverSVIDs:
            break;

        case kVDM_DiscoverModes:
        case kVDM_EnterMode:
        case kVDM_ExitMode:
            if (svdmRequest->vdmHeader.bitFields.SVID != PD_VENDOR_VID)
            {
                svdmRequest->requestResultStatus = kCommandResult_VDMNAK;
                return kStatus_PD_Success;
            }
            break;

        default:
            break;
    }

    /* ack or nak, no busy */
    /* partner return nak if it is not in the alternate mode */
    switch (svdmRequest->vdmHeader.bitFields.command)
    {
        case kVDM_DiscoverIdentity:
            svdmRequest->vdoData = (uint32_t *)&pdAppInstance->selfVdmIdentity;
            svdmRequest->vdoCount = sizeof(pdAppInstance->selfVdmIdentity) / 4;
            if (pdAppInstance->reqestResponse == kCommandResult_Accept)
            {
                svdmRequest->requestResultStatus = kCommandResult_VDMACK;
            }
            else
            {
                svdmRequest->requestResultStatus = kCommandResult_VDMBUSY;
            }
            break;

        case kVDM_DiscoverSVIDs:
            svdmRequest->vdoData = (uint32_t *)&pdAppInstance->selfVdmSVIDs;
            svdmRequest->vdoCount = 1;
            if (pdAppInstance->reqestResponse == kCommandResult_Accept)
            {
                svdmRequest->requestResultStatus = kCommandResult_VDMACK;
            }
            else
            {
                svdmRequest->requestResultStatus = kCommandResult_VDMBUSY;
            }
            break;

        case kVDM_DiscoverModes:
            svdmRequest->vdoData = (uint32_t *)&pdAppInstance->selfVdmModes;
            svdmRequest->vdoCount = 1;
            if (pdAppInstance->reqestResponse == kCommandResult_Accept)
            {
                svdmRequest->requestResultStatus = kCommandResult_VDMACK;
            }
            else
            {
                svdmRequest->requestResultStatus = kCommandResult_VDMBUSY;
            }
            break;

        case kVDM_EnterMode:
            svdmRequest->vdoData = NULL;
            svdmRequest->vdoCount = 0;
            if (pdAppInstance->reqestResponse == kCommandResult_Accept)
            {
                if (svdmRequest->vdmHeader.bitFields.objPos == 1)
                {
                    svdmRequest->requestResultStatus = kCommandResult_VDMACK;
                    pdAppInstance->selfHasEnterAlernateMode = 1;
                }
                else
                {
                    svdmRequest->requestResultStatus = kCommandResult_VDMNAK;
                    pdAppInstance->selfHasEnterAlernateMode = 1;
                }
            }
            else
            {
                svdmRequest->requestResultStatus = kCommandResult_VDMBUSY;
            }
            break;

        case kVDM_ExitMode:
            svdmRequest->vdoData = NULL;
            svdmRequest->vdoCount = 0;
            if ((pdAppInstance->selfHasEnterAlernateMode == 1) && (svdmRequest->vdmHeader.bitFields.objPos == 1))
            {
                pdAppInstance->selfHasEnterAlernateMode = 0;
                svdmRequest->requestResultStatus = kCommandResult_VDMACK;
            }
            else
            {
                svdmRequest->requestResultStatus = kCommandResult_VDMNAK;
            }
            break;

        case kVDM_Attention:
            PD_DemoPrint(pdAppInstance, "receive attention\r\n");
            PD_DemoPrint(pdAppInstance, "start exit mode command\r\n");
            pdAppInstance->structuredVDMCommandParam.vdmSop = pdAppInstance->msgSop;
            pdAppInstance->structuredVDMCommandParam.vdmHeader.bitFields.objPos =
                svdmRequest->vdmHeader.bitFields.objPos;
            pdAppInstance->structuredVDMCommandParam.vdmHeader.bitFields.SVID = svdmRequest->vdmHeader.bitFields.SVID;

            if (PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_EXIT_MODE,
                           &pdAppInstance->structuredVDMCommandParam) != kStatus_PD_Success)
            {
                PD_DemoPrint(pdAppInstance, "command start fail\r\n");
            }
            break;

        default:
            /* vendor defined structured vdm */
            if (pdAppInstance->reqestResponse == kCommandResult_Accept)
            {
                svdmRequest->requestResultStatus = kCommandResult_VDMACK;
            }
            else
            {
                svdmRequest->requestResultStatus = kCommandResult_VDMBUSY;
            }
            break;
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSVDMResultCallback(void *callbackParam, uint8_t success, pd_svdm_command_result_t *svdmResult)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    if (success)
    {
        switch (svdmResult->vdmCommand)
        {
            case kVDM_DiscoverIdentity:
            {
                pd_id_header_vdo_t idHeaderVDO;
                idHeaderVDO.vdoValue = USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)(svdmResult->vdoData)));
                PD_DemoPrint(pdAppInstance, "vendor:%x, modual support:%d, usb communication capable device:%d",
                             idHeaderVDO.bitFields.usbVendorID, idHeaderVDO.bitFields.modalOperateSupport,
                             idHeaderVDO.bitFields.usbCommunicationCapableAsDevice);
                PD_DemoPrint(pdAppInstance, ", usb communication capable host:%d",
                             idHeaderVDO.bitFields.usbCommunicationCapableAsHost);
                PD_DemoPrint(pdAppInstance, ", product type: (DFP or Cable)");
                if (idHeaderVDO.bitFields.productTypeDFP != 0)
                {
                    switch (idHeaderVDO.bitFields.productTypeDFP)
                    {
                        case 0:
                            PD_DemoPrint(pdAppInstance, "undefined, ");
                            break;

                        case 1:
                            PD_DemoPrint(pdAppInstance, "PDUSB Hub, ");
                            break;

                        case 2:
                            PD_DemoPrint(pdAppInstance, "PDUSB Host, ");
                            break;

                        case 3:
                            PD_DemoPrint(pdAppInstance, "Power Brick, ");
                            break;

                        case 4:
                            PD_DemoPrint(pdAppInstance, "Alternate Mode Controller (AMC), ");
                            break;

                        default:
                            PD_DemoPrint(pdAppInstance, "cannot recognition, ");
                            break;
                    }
                }

                PD_DemoPrint(pdAppInstance, "(UFP)");
                if (idHeaderVDO.bitFields.productTypeUFPOrCablePlug != 0)
                {
                    switch (idHeaderVDO.bitFields.productTypeUFPOrCablePlug)
                    {
                        case 0:
                            PD_DemoPrint(pdAppInstance, "undefined\r\n");
                            break;

                        case 1:
                            PD_DemoPrint(pdAppInstance, "PDUSB Hub\r\n");
                            break;

                        case 2:
                            PD_DemoPrint(pdAppInstance, "PDUSB Peripheral\r\n");
                            break;

                        case 3:
                            PD_DemoPrint(pdAppInstance, "Passive Cable\r\n");
                            break;

                        case 4:
                            PD_DemoPrint(pdAppInstance, "Active Cable\r\n");
                            break;

                        case 5:
                            PD_DemoPrint(pdAppInstance, "Alternate Mode Adapter (AMA)\r\n");
                            break;

                        default:
                            PD_DemoPrint(pdAppInstance, "cannot recognition\r\n");
                            break;
                    }
                }

                PD_DemoPrint(pdAppInstance, "(2) discovery SVIDs\r\n");
                pdAppInstance->structuredVDMCommandParam.vdmSop = pdAppInstance->msgSop;
                if (PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_DISCOVERY_SVIDS,
                               &pdAppInstance->structuredVDMCommandParam) != kStatus_PD_Success)
                {
                    PD_DemoPrint(pdAppInstance, "command fail\r\n");
                }
                break;
            }

            case kVDM_DiscoverSVIDs:
            {
                uint32_t index;
                uint8_t svidIndex = 1;
                uint32_t SVID;
                uint8_t *buffPtr = (uint8_t *)svdmResult->vdoData;
                for (index = 0; index < (svdmResult->vdoCount >> 2); ++index)
                {
                    SVID = USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(buffPtr);
                    buffPtr += 4;
                    if (SVID != 0)
                    {
                        pdAppInstance->partnerSVIDs[index * 2] = SVID >> 16;
                        if (pdAppInstance->partnerSVIDs[index * 2] != 0x0000u)
                        {
                            PD_DemoPrint(pdAppInstance, "SVID%d: %x\r\n", svidIndex,
                                         pdAppInstance->partnerSVIDs[index * 2]);
                            svidIndex++;
                        }

                        pdAppInstance->partnerSVIDs[index * 2 + 1] = (SVID & 0x0000FFFFu);
                        if (pdAppInstance->partnerSVIDs[index * 2 + 1] != 0x0000u)
                        {
                            PD_DemoPrint(pdAppInstance, "SVID%d: %x\r\n", svidIndex,
                                         pdAppInstance->partnerSVIDs[index * 2 + 1]);
                            svidIndex++;
                        }
                    }

                    pdAppInstance->partnerSVIDs[index * 2 + 1] = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(buffPtr);
                }

                if (pdAppInstance->partnerSVIDs[0] == 0x0000u)
                {
                    PD_DemoPrint(pdAppInstance, "SVIDs: none\r\n");
                }
                else
                {
                    PD_DemoPrint(pdAppInstance, "(3) discovery Modes\r\n");
                    pdAppInstance->structuredVDMCommandParam.vdmSop = pdAppInstance->msgSop;
                    pdAppInstance->structuredVDMCommandParam.vdmHeader.bitFields.SVID = pdAppInstance->partnerSVIDs[0];
                    if (PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_DISCOVERY_MODES,
                                   &pdAppInstance->structuredVDMCommandParam) != kStatus_PD_Success)
                    {
                        PD_DemoPrint(pdAppInstance, "command fail\r\n");
                    }
                }
                break;
            }

            case kVDM_DiscoverModes:
            {
                uint32_t index;
                uint8_t *buffPtr = (uint8_t *)svdmResult->vdoData;
                uint8_t dataRole;
                for (index = 0; index < (svdmResult->vdoCount >> 2); ++index)
                {
                    pdAppInstance->partnerModes[index * 2] = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(buffPtr);
                    buffPtr += 2;
                    PD_DemoPrint(pdAppInstance, "Mode%d: %x\r\n", 1 + index * 2,
                                 pdAppInstance->partnerModes[index * 2]);
                    pdAppInstance->partnerModes[index * 2 + 1] = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(buffPtr);
                    buffPtr += 2;
                    PD_DemoPrint(pdAppInstance, "Mode%d: %x\r\n", 1 + index * 2 + 1,
                                 pdAppInstance->partnerModes[index * 2 + 1]);
                }

                PD_Control(pdAppInstance->pdHandle, PD_CONTROL_GET_DATA_ROLE, &dataRole);

                if (dataRole == kPD_DataRoleDFP)
                {
                    PD_DemoPrint(pdAppInstance, "(4) enter mode 1\r\n");
                    pdAppInstance->structuredVDMCommandParam.vdmSop = pdAppInstance->msgSop;
                    pdAppInstance->structuredVDMCommandParam.vdoCount = 0;
                    pdAppInstance->structuredVDMCommandParam.vdoData = NULL;
                    pdAppInstance->structuredVDMCommandParam.vdmHeader.bitFields.objPos = 1;
                    pdAppInstance->structuredVDMCommandParam.vdmHeader.bitFields.SVID = pdAppInstance->partnerSVIDs[0];
                    if (PD_Command(pdAppInstance->pdHandle, PD_DPM_CONTROL_ENTER_MODE,
                                   &pdAppInstance->structuredVDMCommandParam) != kStatus_PD_Success)
                    {
                        PD_DemoPrint(pdAppInstance, "command fail\r\n");
                    }
                }
                break;
            }

            case kVDM_EnterMode:
                PD_DemoPrint(pdAppInstance, "enter mode result: ack\r\n");
                break;

            case kVDM_ExitMode:
                PD_DemoPrint(pdAppInstance, "exit mode result: ack\r\n");
                break;

            case kVDM_Attention:
                PD_DemoPrint(pdAppInstance, "send attention result: success\r\n");
                break;

            default:
            {
                /* process the buffer data and length */
                // pd_svdm_command_result_t *vdmResult = (pd_svdm_command_result_t*)param;
                PD_DemoPrint(pdAppInstance, "vendor structured vdm result: success\r\n");
                break;
            }
        }
    }
    else
    {
        switch (svdmResult->vdmCommandResult)
        {
            case kCommandResult_Error:
                PD_DemoPrint(pdAppInstance, "structured vdm result: fail\r\n");
                break;

            case kCommandResult_VDMNAK:
                PD_DemoPrint(pdAppInstance, "structured vdm result: nak\r\n");
                break;

            case kCommandResult_VDMBUSY:
                PD_DemoPrint(pdAppInstance, "structured vdm result: busy\r\n");
                break;

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmUnstructuredVDMReceivedCallback(void *callbackParam,
                                                  pd_unstructured_vdm_command_param_t *unstructuredVDMParam)
{
    uint32_t index;
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    /* process the unstructured vdm */
    PD_DemoPrint(pdAppInstance, "receive unstructured vdm, sop:%d, vdo count:%d\r\n", unstructuredVDMParam->vdmSop,
                 unstructuredVDMParam->vdmHeaderAndVDOsCount);
    PD_DemoPrint(pdAppInstance, "VDO Header:%x\r\n", unstructuredVDMParam->vdmHeaderAndVDOsData[0]);
    for (index = 1; index < unstructuredVDMParam->vdmHeaderAndVDOsCount; ++index)
    {
        PD_DemoPrint(pdAppInstance, "VDO%d:%d\r\n", index, unstructuredVDMParam->vdmHeaderAndVDOsData[index]);
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmUnstructuredVDMSendResultCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    if (success)
    {
        PD_DemoPrint(pdAppInstance, "send unstructured vdm result: success\r\n");
    }
    else
    {
        PD_DemoPrint(pdAppInstance, "send unstructured vdm result: fail\r\n");
    }

    return kStatus_PD_Success;
}

#if ((defined PD_CONFIG_REVISION) && (PD_CONFIG_REVISION >= PD_SPEC_REVISION_30))
pd_status_t PD_DpmGetInfoRequestCallback(void *callbackParam, uint8_t type, pd_command_data_param_t *dataParam)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    switch (type)
    {
        case kInfoType_SrcExtCap:
            if (pdAppInstance->reqestResponse != kCommandResult_Accept)
            {
                dataParam->resultStatus = kCommandResult_NotSupported;
            }
            else
            {
                dataParam->resultStatus = kCommandResult_Accept;
            }

            dataParam->dataBuffer = (uint8_t *)&pdAppInstance->selfExtCap;
            dataParam->dataLength = sizeof(pdAppInstance->selfExtCap);
            break;

        case kInfoType_Status:
            if (pdAppInstance->reqestResponse != kCommandResult_Accept)
            {
                dataParam->resultStatus = kCommandResult_NotSupported;
            }
            else
            {
                dataParam->resultStatus = kCommandResult_Accept;
            }

            dataParam->dataBuffer = (uint8_t *)&pdAppInstance->selfStatus;
            dataParam->dataLength = sizeof(pdAppInstance->selfStatus);
            break;

        case kInfoType_BatteryCap:
            if (pdAppInstance->reqestResponse != kCommandResult_Accept)
            {
                dataParam->resultStatus = kCommandResult_NotSupported;
            }
            else
            {
                dataParam->resultStatus = kCommandResult_Accept;
            }
            dataParam->dataBuffer = (uint8_t *)&pdAppInstance->selfBatteryCap;
            dataParam->dataLength = sizeof(pdAppInstance->selfBatteryCap);
            break;

        case kInfoType_BatteryStatus:
            if (pdAppInstance->reqestResponse != kCommandResult_Accept)
            {
                dataParam->resultStatus = kCommandResult_NotSupported;
            }
            else
            {
                dataParam->resultStatus = kCommandResult_Accept;
            }
            dataParam->dataBuffer = (uint8_t *)&pdAppInstance->selfBatteryStatus;
            dataParam->dataLength = sizeof(pdAppInstance->selfBatteryStatus);
            break;

        case kInfoType_ManufacturerInfo:
            if (pdAppInstance->reqestResponse != kCommandResult_Accept)
            {
                dataParam->resultStatus = kCommandResult_NotSupported;
            }
            else
            {
                dataParam->resultStatus = kCommandResult_Accept;
            }
            dataParam->dataBuffer = (uint8_t *)&pdAppInstance->selfManufacInfo;
            dataParam->dataLength = 7;
            break;

        default:
            break;
    }
    return kStatus_PD_Success;
}

pd_status_t PD_DpmGetInfoResultCallback(
    void *callbackParam, uint8_t type, uint8_t success, pd_command_data_param_t *successData, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    if (success)
    {
        switch (type)
        {
            case kInfoType_SrcExtCap:
            {
                pd_source_cap_ext_data_block_t *extDataBlock;
                extDataBlock = (pd_source_cap_ext_data_block_t *)successData->dataBuffer;
                PD_DemoPrint(pdAppInstance, "vid:%x, pid:%x, xid:%x, fw version:%d, hw version:%d\r\n",
                             (uint16_t)extDataBlock->vid, (uint16_t)extDataBlock->pid,
                             USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)&(extDataBlock->xid))),
                             (uint8_t)extDataBlock->fwVersion, (uint8_t)extDataBlock->hwVersion);
                break;
            }

            case kInfoType_Status:
            {
                pd_status_data_block_t *status = (pd_status_data_block_t *)(&successData->dataBuffer[0]);
                PD_DemoPrint(pdAppInstance, "Internal Temp:%d\r\n", status->internalTemp);
                break;
            }

            case kInfoType_BatteryCap:
            {
                pd_battery_cap_data_block_t *partnerBatteryCap =
                    (pd_battery_cap_data_block_t *)(successData->dataBuffer);
                PD_DemoPrint(pdAppInstance, "design cap:%dmWH, last full cap:%d\r\n",
                             partnerBatteryCap->batteryDesignCap * 100,
                             partnerBatteryCap->batteryLastFullChargeCap * 100);
                break;
            }

            case kInfoType_BatteryStatus:
            {
                pd_battery_status_data_object_t *partnerBatteryStatus =
                    (pd_battery_status_data_object_t *)(successData->dataBuffer);
                PD_DemoPrint(pdAppInstance, "battery charge state:%dmWH, last full cap:%d\r\n",
                             partnerBatteryStatus->batteryPC * 100);
                break;
            }

            case kInfoType_ManufacturerInfo:
            {
                pd_manufac_info_data_block_t *partnerManufacturerInfo =
                    (pd_manufac_info_data_block_t *)(successData->dataBuffer);
                partnerManufacturerInfo->manufacturerString[successData->dataLength - 4] = 0;
                PD_DemoPrint(pdAppInstance, "manufacturer string:%s\r\n", partnerManufacturerInfo->manufacturerString);
                break;
            }

            default:
                break;
        }
    }
    else
    {
        switch (failResultType)
        {
            case kCommandResult_Error:
                PD_DemoPrint(pdAppInstance, "get info fail\r\n");
                break;

            case kCommandResult_NotSupported:
                PD_DemoPrint(pdAppInstance, "get info is not supported by partner\r\n");
                break;

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmReceiveAlertCallback(void *callbackParam, pd_command_data_param_t *dataParam)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;
    pd_alert_data_object_t alertObj;

    dataParam->resultStatus = kCommandResult_Accept;
    alertObj.alertValue = USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(dataParam->dataBuffer);
    PD_DemoPrint(pdAppInstance, "alert change:%x\r\n", alertObj.bitFields.typeOfAlert);

    PD_DemoPrint(pdAppInstance, "start get status command\r\n");
    if (PD_Command(pdAppInstance->pdHandle, PD_DPM_GET_STATUS, NULL) != kStatus_PD_Success)
    {
        PD_DemoPrint(pdAppInstance, "command start fail\r\n");
    }

    return kStatus_PD_Success;
}

pd_status_t PD_DpmSendAlertCallback(void *callbackParam, uint8_t success, uint8_t failResultType)
{
    pd_app_t *pdAppInstance = (pd_app_t *)callbackParam;

    if (success)
    {
        PD_DemoPrint(pdAppInstance, "send alert result: success\r\n");
    }
    else
    {
        switch (failResultType)
        {
            case kCommandResult_Error:
                PD_DemoPrint(pdAppInstance, "send alert result: fail\r\n");
                break;

            case kCommandResult_NotSupported:
                PD_DemoPrint(pdAppInstance, "send alert result: not supported\r\n");
                break;

            default:
                break;
        }
    }

    return kStatus_PD_Success;
}
#endif
