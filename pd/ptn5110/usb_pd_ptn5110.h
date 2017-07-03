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

#ifndef __PD_PTN5110_H__
#define __PD_PTN5110_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define USBPD_ENABLE_PTN5110_A0R1_WORKAROUNDS (0)
#define USBPD_ENABLE_PTN5110_A0R2_WORKAROUNDS (1)

#define TCPC_TX_BUF_HEADER_LEN 2
#define TCPC_TRANSMIT_BYTE_COUNT_LEN 1
/* For TCPC the registers are not mapped to local memory, we need to use an intermediate buffer uint8_t msg_buf[32]; */
/* When sending TRANSMIT_BUFFER through I2c, the TRANSMIT_BYTE_COUNT and TX_BUF_HEADER_BYTE_0/1 must been sent together
 */
/* with TX_BUF_OBJn_BYTE as per USB-Port Conroller Specification 4.3.5 Writing the TRANSMIT_BUFFER */
/* So define this array to contain all buf content. */
/* When transmiting, 0-reserved, 1-TRANSMIT_BYTE_COUNT, 2-TX_BUF_HEADER_BYTE_0, 3-TX_BUF_HEADER_BYTE_1, */
/* 4~31-TX_BUF_OBJn_BYTE */
/* When receiving, 0-RECEIVE_BYTE_COUNT, 1-RX_BUF_FRAME_TYPE, 2-RX_BUF_HEADER_BYTE_0, 3-RX_BUF_HEADER_BYTE_1, */
/* 4~31-RX_BUF_OBJn_BYTE */
#define TCPC_TRANSMIT_BYTE_COUNT_POS 1
#define TCPC_TX_BUF_HEADER_BYTE_POS 2

#define TCPC_TX_BUF_AVAILABLE 0xFF /* If TCPC allows message to be sent, set to this state */
/* If a message is transmitting, no other message execept for Hard_Reset can be send through TCPC. Set to this state */
#define TCPC_TX_BUF_UNAVAILABLE 0xEE

/* VBus definition */
/* Unit: 25mV, Real value is 800mV as per USBPD R3 V0.91 Table 7-18 Common Source/Sink Electrical Parameters */
#define VBUS_VSAFE0V_MAX_THRESHOLD 32
/* Unit: 25mV, Real value is 400mV as per USBPD R3 V0.91 Table 7-18 Common Source/Sink Electrical Parameters */
#define VBUS_VSAFE0V_MIN_THRESHOLD 16
/* Unit: 25mV, Real value is 4000mV as per USB3.1 Table 11-2. DC Electrical Characteristics when working as a  upstream
 * connector */
#define VBUS_VSAFE5V_MIN_THRESHOLD 160
/* 5.5V */
#define VBUS_VSAFE5V_MAX_THRESHOLD 220

#define TX_ABORT_MASK (TCPC_ALERT_TRANSMIT_SOP_MESSAGE_FAILED_MASK | TCPC_ALERT_TRANSMIT_SOP_MESSAGE_DISCARDED_MASK)
#define TX_DONE_MASK (TX_ABORT_MASK | TCPC_ALERT_TRANSMIT_SOP_MESSAGE_SUCCESSFUL_MASK)
#define TX_HARDRESET_MASK \
    (TCPC_ALERT_TRANSMIT_SOP_MESSAGE_FAILED_MASK | TCPC_ALERT_TRANSMIT_SOP_MESSAGE_SUCCESSFUL_MASK)

#define PRODUCT_ID_PTN5110 0x5110
#define DEVICE_ID_PTN5110_A0R1 0x0000
#define DEVICE_ID_PTN5110_A0R2 0x0001
/* DEVICE_ID value 2 is intentionally skipped to align nomenclature */
#define DEVICE_ID_PTN5110_A1R3 0x0003
#define DEVICE_ID_PTN5110_A1R4 0x0004

/* defined - use PDPTN5110_VbusDischarge() below */
#define HAL_SUPPORTS_VBUS_DISCHARGE 1

#define TcpcRegMask(REG_NAME, FIELD_NAME) (TCPC_##REG_NAME##_##FIELD_NAME##_MASK)
#define TcpcRegLsb(REG_NAME, FIELD_NAME) (TCPC_##REG_NAME##_##FIELD_NAME##_LSB)
#define TcpcReadField(VALUE, REG_NAME, FIELD_NAME) \
    (((VALUE)&TcpcRegMask(REG_NAME, FIELD_NAME)) >> TcpcRegLsb(REG_NAME, FIELD_NAME))

/* NOTE: masks should be defined in the device specific header */
typedef enum
{
    kRegModule_Intc = 1 << 0,
    kRegModule_Mask = 1 << 1,
    kRegModule_MsgRX = 1 << 2,
    kRegModule_ConfStdOut = 1 << 3,
    kRegModule_Control = 1 << 4,
    kRegModule_MsgTX = 1 << 5,
    kRegModule_Command = 1 << 6,
    kRegModule_Capability = 1 << 7,
    kRegModule_Vbus = 1 << 8,
    kRegModule_CCStatus = 1 << 9,
    kRegModule_PowerStatus = 1 << 10,
    kRegModule_FaultStatus = 1 << 11,
    kRegModule_All = 0xFFFF,
} RegModuleMaskTcpc_t;

typedef struct
{
    struct
    {
        uint16_t vendor_id;
        uint16_t product_id;
        uint16_t device_id;
        uint16_t usbtypec_rev;
        uint16_t usbpd_rev_ver;
        uint16_t pd_interface_rev;
    } GLOBAL;
    struct
    {
        uint16_t alert;
    } INTC;
    struct
    {
        uint16_t ext_alert;
    } EXT_INTC;
    struct
    {
        uint16_t alert_mask;
        uint16_t ext_alert_mask;
        uint8_t power_status_mask;
        uint8_t fault_status_mask;
    } MASK;
    struct
    {
        uint8_t message_header_info;
        uint8_t receive_detect;
        /* The following bytes must be contiguous, as they are read in a burst */
        uint8_t receive_byte_count;
        uint8_t rx_buf_frame_type;
        uint16_t rx_buf_header;
    } MSG_RX;
    struct
    {
        uint8_t transmit;
    } MSG_TX;
    struct
    {
        uint8_t config_standard_output;
    } CONF_STD_OUT;
    struct
    {
        uint8_t tcpc_control;
        uint8_t role_control;
        uint8_t fault_control;
        uint8_t power_control;
    } CONTROL;
    struct
    {
        uint8_t cc_status;
    } CC_STATUS;
    struct
    {
        uint8_t power_status;
    } POWER_STATUS;
    struct
    {
        uint8_t fault_status;
    } FAULT_STATUS;
    struct
    {
        uint16_t device_capabilities_1;
        uint16_t device_capabilities_2;
        uint8_t standard_input_capabilities;
        uint8_t standard_output_capabilities;
    } CAPABILITY;
    struct
    {
        uint16_t vbus_voltage;
        uint16_t vbus_sink_disconnect_threshold;
        uint16_t vbus_stop_discharge_threshold;
        uint16_t vbus_voltage_alarm_hi_cfg;
        uint16_t vbus_voltage_alarm_lo_cfg;
    } VBUS;
    struct
    {
        uint8_t pmu_lowpower; /* just for compatible with Victoria code */
    } PMU;
    struct
    {
        uint16_t ext_status;
    } EXT_STATUS;
    struct
    {
        uint16_t ext_config;
        uint8_t ext_control;
    } EXT_CONTROL;
} pd_phy_TCPC_reg_cache_t;

typedef struct __pd_phy_tcpc_instance_
{
    pd_handle pdHandle;
    uint8_t *msgTxBuf;
    uint8_t msgRxCacheBuf[32];
    pd_phy_TCPC_reg_cache_t tcpcRegCache;
    cmsis_driver_adapter_t *cmsisAdapter;
    uint8_t *rxDataBuffer;
    uint32_t rxDataLength;
    volatile pd_phy_rx_result_t cacheRxResult;
    volatile uint8_t cacheRxValid;
    uint8_t occupied;
    uint8_t pdPhyId;
    volatile uint8_t frSwapSourceOK;

    volatile uint8_t msgTxBufState; /* Current tx buffer, 0xFF for none, cleared to 0xFF on tx_done assertion,
                                          original usage: msg_tcpc.c, current usage: only internal */
    uint8_t prevPwrStat;
    uint8_t initialCcStatus;
    uint8_t halWaitVsafe0V;
    uint8_t halVsafe5VYes;
    uint8_t halVbusYes;
    volatile uint16_t cacheEna;
    volatile uint16_t intcLastStatus;
    volatile uint8_t usedCC;
    volatile uint8_t roleControlUpdated;
    uint8_t phyPowerRole;
    uint8_t phyDataRole;
    uint8_t amsSinkTxOK;
    volatile uint8_t currentStable;
    volatile uint8_t pwrInProgress;
    uint8_t lowPowerState;
    uint8_t txAbort;
    uint8_t msgId[8];              /* 1 per SOP */
    uint8_t rcvMsgId[8];           /* 1 per SOP, use for detecting duplicate received messages */
    uint8_t firstMsgAfterReset[8]; /* 1 per SOP, use for detecting duplicate received */
    uint8_t msgRxSopMask;
    uint8_t msgTxSop;
    uint8_t msgAMSState;
    volatile uint8_t txHave;
    volatile uint8_t rxHave;
    volatile uint8_t revision;
#if 0
    volatile uint8_t ccStatusNoChangeTime;
    volatile uint8_t ccPrevStatus;
#endif
} pd_phy_ptn5110_instance_t;

/*******************************************************************************
 * API
 ******************************************************************************/

static inline void REG_SET_REG_REF(pd_phy_ptn5110_instance_t *ptn5110Instance, uint32_t reg, uint8_t data)
{
    CMSIS_PortControlInterfaceWriteRegister((ptn5110Instance)->cmsisAdapter, (reg), 1, &data, 1);
}

static inline uint8_t REG_GET_REG_REF(pd_phy_ptn5110_instance_t *ptn5110Instance, uint32_t reg)
{
    uint8_t dataTmp;
    CMSIS_PortControlInterfaceReadRegister(ptn5110Instance->cmsisAdapter, reg, 1, &dataTmp, 1);
    return dataTmp;
}

#define REG_GET_BLOCK_REF(ptn5110Instance, reg, length, dst) \
    CMSIS_PortControlInterfaceReadRegister((ptn5110Instance)->cmsisAdapter, (reg), 1, (dst), (length))

#define REG_SET_BLOCK_REF(ptn5110Instance, src, length, reg) \
    CMSIS_PortControlInterfaceWriteRegister((ptn5110Instance)->cmsisAdapter, (reg), 1, (src), (length))

static inline void REG_SET_FIELD(pd_phy_ptn5110_instance_t *ptn5110Instance, uint32_t reg, uint8_t mask, uint8_t val)
{
    uint8_t regVal;

    CMSIS_PortControlInterfaceReadRegister(ptn5110Instance->cmsisAdapter, reg, 1, &regVal, 1);
    regVal = (val & mask) | (regVal & ~mask);
    CMSIS_PortControlInterfaceWriteRegister(ptn5110Instance->cmsisAdapter, reg, 1, &regVal, 1);
}

static inline uint8_t REG_GET_FIELD(pd_phy_ptn5110_instance_t *ptn5110Instance, uint32_t reg, uint8_t field)
{
    uint8_t regVal;

    CMSIS_PortControlInterfaceReadRegister(ptn5110Instance->cmsisAdapter, reg, 1, &regVal, 1);
    return (regVal & field);
}

#define REG_ADDR(REG) (ADDR_##REG)

#define RegCacheRead(ptn5110Instance, MODULE, REGISTER) ((ptn5110Instance)->tcpcRegCache.MODULE.REGISTER)
#define RegCacheWrite(ptn5110Instance, MODULE, REGISTER, VALUE) \
    ((ptn5110Instance)->tcpcRegCache.MODULE.REGISTER) = (VALUE)

#define RegCacheReadField(ptn5110Instances, MODULE, REGISTER, MASK) \
    ((ptn5110Instances->tcpcRegCache.MODULE.REGISTER) & (MASK))

#define RegCacheWriteField(ptn5110Instances, MODULE, REGISTER, MASK, UPDATE_VALUE) \
    (ptn5110Instances->tcpcRegCache.MODULE.REGISTER) =                             \
        (((ptn5110Instances->tcpcRegCache.MODULE.REGISTER) & ~(MASK)) | ((UPDATE_VALUE) & (MASK)))

#define REG_CLR_BIT(ptn5110Instance, reg, field) REG_CLR_ALL_BITS(ptn5110Instance, reg, field)
static inline void REG_CLR_ALL_BITS(pd_phy_ptn5110_instance_t *ptn5110Instance, uint32_t reg, uint8_t field)
{
    uint8_t regVal;

    CMSIS_PortControlInterfaceReadRegister(ptn5110Instance->cmsisAdapter, reg, 1, &regVal, 1);
    regVal &= (~field);
    CMSIS_PortControlInterfaceWriteRegister(ptn5110Instance->cmsisAdapter, reg, 1, &regVal, 1);
}

#define REG_SET_BIT(port, reg, field) REG_SET_ALL_BITS(port, reg, field)
static inline void REG_SET_ALL_BITS(pd_phy_ptn5110_instance_t *ptn5110Instance, uint32_t reg, uint8_t field)
{
    uint8_t regVal;

    CMSIS_PortControlInterfaceReadRegister(ptn5110Instance->cmsisAdapter, reg, 1, &regVal, 1);
    regVal |= (field);
    CMSIS_PortControlInterfaceWriteRegister(ptn5110Instance->cmsisAdapter, reg, 1, &regVal, 1);
}

#define REG_CACHE_MODIFY_WORD_FIELD(ptn5110Instance, MODULE, REGISTER, MASK, UPDATE_VALUE)               \
    PDPTN5110_RegCacheModIfyWordField(ptn5110Instance, &(ptn5110Instance->tcpcRegCache.MODULE.REGISTER), \
                                      REG_ADDR(REGISTER), MASK, UPDATE_VALUE)

#define REG_CACHE_MODIFY_FIELD(ptn5110Instance, MODULE, REGISTER, MASK, UPDATE_VALUE)                \
    PDPTN5110_RegCacheModIfyField(ptn5110Instance, &(ptn5110Instance->tcpcRegCache.MODULE.REGISTER), \
                                  REG_ADDR(REGISTER), MASK, UPDATE_VALUE)

#define RoleControlIsToggleSrcFirst(roleControl)                                                               \
    ((roleControl & (TCPC_ROLE_CONTROL_DRP_MASK | TCPC_ROLE_CONTROL_CC2_MASK | TCPC_ROLE_CONTROL_CC1_MASK)) == \
     (TCPC_ROLE_CONTROL_DRP_MASK | ROLE_CONTROL_CC2_RP | ROLE_CONTROL_CC1_RP))

#define RoleControlIsBothRp(roleControl)                                          \
    ((roleControl & (TCPC_ROLE_CONTROL_CC2_MASK | TCPC_ROLE_CONTROL_CC1_MASK)) == \
     (ROLE_CONTROL_CC2_RP | ROLE_CONTROL_CC1_RP))

#define RoleControlIsToggleSnkFirst(roleControl)                                                               \
    ((roleControl & (TCPC_ROLE_CONTROL_DRP_MASK | TCPC_ROLE_CONTROL_CC2_MASK | TCPC_ROLE_CONTROL_CC1_MASK)) == \
     (TCPC_ROLE_CONTROL_DRP_MASK | ROLE_CONTROL_CC2_RD | ROLE_CONTROL_CC1_RD))

#define RoleControlIsBothRd(roleControl)                                          \
    ((roleControl & (TCPC_ROLE_CONTROL_CC2_MASK | TCPC_ROLE_CONTROL_CC1_MASK)) == \
     (ROLE_CONTROL_CC2_RD | ROLE_CONTROL_CC1_RD))

extern void PDPTN5110_RegCacheModIfyWordField(
    pd_phy_ptn5110_instance_t *ptn5110Instance, uint16_t *reg, uint16_t addr, uint16_t mask, uint16_t update_value);

extern void PDPTN5110_RegCacheModIfyField(
    pd_phy_ptn5110_instance_t *ptn5110Instance, uint8_t *reg, uint16_t addr, uint8_t mask, uint8_t update_value);

void PDPTN5110_ConnectINTcTypeCCurrentStable(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_ConnectInTcChecKDetacHDurIngHardReset(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_ConnectSWitchCCComms(pd_phy_ptn5110_instance_t *ptn5110Instance, uint8_t enable, uint8_t cc);
void PDPTN5110_ConnectInit(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_ConnectEnableTypeCConnection(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_ConnectSync(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_ConnectAssertRpUnattached(pd_phy_ptn5110_instance_t *ptn5110Instance, uint8_t srcRp);
void PDPTN5110_ConnectAssertRpAttached(pd_phy_ptn5110_instance_t *ptn5110Instance, uint8_t usedCC, uint8_t rpCfg);
void PDPTN5110_ConnectAssertRpDbgAccSrc(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_ConnectAssertRdDbgAccSnk(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_ConnectAssertRdAttached(pd_phy_ptn5110_instance_t *ptn5110Instance, uint8_t usedCC, uint8_t detectWay);
void PDPTN5110_ConnectAssertRdUnattached(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_ConnectAssertDrpUnattached(pd_phy_ptn5110_instance_t *ptn5110Instance, uint8_t startSrc, uint8_t srcRp);
void PDPTN5110_ConnectDbgAccSnkAttached(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_ConnectDbgAccSrcAttached(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_ConnectAudioAccessoryAttached(pd_phy_ptn5110_instance_t *ptn5110Instance, uint8_t cc);
void PDPTN5110_ConnectReleasePowerControl(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_ConnectSetInProgress(pd_phy_ptn5110_instance_t *ptn5110Instance, uint8_t state);
void PDPTN5110_ConnectDisableComparators(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_ConnectDisableAll(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_ConnectDetachOrDisableCallback(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_ConnectSetCC(pd_phy_ptn5110_instance_t *ptn5110Instance, uint8_t usedCC);
void PDPTN5110_ConnectRawVbusDischarge(pd_phy_ptn5110_instance_t *ptn5110Instance, uint8_t enable);
void PDPTN5110_ConnectSwitchVConn(pd_phy_ptn5110_instance_t *ptn5110Instance, uint8_t enable);
void PDPTN5110_ConnectSrcSetTypecCurrent(pd_phy_ptn5110_instance_t *ptn5110Instance, uint8_t usedCC, uint8_t val);
uint8_t PDPTN5110_ConnectGetTypeCCurrent(pd_phy_ptn5110_instance_t *ptn5110Instance, uint8_t usedCC);
void PDPTN5110_ConnectGetCC(pd_phy_ptn5110_instance_t *ptn5110Instance, pd_phy_get_cc_state_t *ccStates);
int8_t PDPTN5110_ConnectCompareCcVoltage(pd_phy_ptn5110_instance_t *ptn5110Instance);

void PDPTN5110_MsgHalStopBist(pd_phy_ptn5110_instance_t *ptn5110Instance, pd_bist_mst_t mode);
void PDPTN5110_MsgInterruptCallbackMsgRcvd(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_MsgInterruptCallbackHardReset(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_MsgHalSendAbortIsrProcess(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_MsgHalSendSuccessIsrProcess(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_MsgResetAllMsgId(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_MsgHalProtocolLayerResetAndPowerUp(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_MsgHalClearPendingAndAbort(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_MsgHalDisableMessageRx(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_MsgSendCompLete(pd_phy_ptn5110_instance_t *ptn5110Instance, uint8_t sendResult);
void PDPTN5110_MsgReceiveCompLete(pd_phy_ptn5110_instance_t *ptn5110Instance, uint8_t rxState);
void PDPTN5110_MsgHalPowerDownRx(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_MsgHalPowerDownTx(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_MsgHalSendReset(pd_phy_ptn5110_instance_t *ptn5110Instance, uint8_t hardResetOrCableReset);
void PDPTN5110_MsgHalSetPortRole(pd_phy_ptn5110_instance_t *ptn5110Instance,
                                 uint8_t revision,
                                 uint8_t powerRole,
                                 uint8_t dataRole);
uint8_t PDPTN5110_MsgHalSendControl(pd_phy_ptn5110_instance_t *ptn5110Instance, uint8_t sop, uint8_t type);
uint8_t PDPTN5110_MsgHalSendData(pd_phy_ptn5110_instance_t *ptn5110Instance, uint8_t sop, uint8_t type, uint8_t count);
void PDPTN5110_MsgHalSetRxSopEnable(pd_phy_ptn5110_instance_t *ptn5110Instance, uint8_t sop_mask);
uint8_t PDPTN5110_MsgHalSendUnchunked(pd_phy_ptn5110_instance_t *ptn5110Instance,
                                      uint8_t sop,
                                      uint8_t type,
                                      uint16_t dataSize);
uint8_t PDPTN5110_MsgHalSendChunked(pd_phy_ptn5110_instance_t *ptn5110Instance,
                                    uint8_t sop,
                                    uint8_t type,
                                    uint8_t count);
void PDPTN5110_MsgHalSetReceiveDetect(pd_phy_ptn5110_instance_t *ptn5110Instance, uint8_t value);
static void PDPTN5110_MsgHalEnableRxAfterReset(pd_phy_ptn5110_instance_t *ptn5110Instance);

void PDPTN5110_HalInit(pd_phy_handle pdPhyHandle);
void PDPTN5110_PwrWaitForPOrCOmpletEAndEnableClocks(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_IntcIrqClearAndEnable(pd_phy_ptn5110_instance_t *ptn5110Instance, uint16_t mask);
void PDPTN5110_VbusDisableMonitorAndDetect(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_EnableVSafe0VComparator(pd_phy_ptn5110_instance_t *ptn5110Instance, bool discharge);
bool PDPTN5110_PwrDoApplyCurState(pd_phy_ptn5110_instance_t *ptn5110Instances, uint8_t pwrState);
void PDPTN5110_RegCacheSynC(pd_phy_ptn5110_instance_t *ptn5110Instance, uint16_t mask);
uint16_t PDPTN5110_IntcTxDoneSeen(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_IntcIntNCallback(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_VbusEnableMonitorAndDetect(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_VbusDisableMonitorAndDetect(pd_phy_ptn5110_instance_t *ptn5110Instance);
bool PDPTN5110_PwrDoApplyCurState(pd_phy_ptn5110_instance_t *ptn5110Instances, uint8_t pwrState);
void PDPTN5110_SetFetControl(pd_phy_ptn5110_instance_t *ptn5110Instance, pd_ptn5110_ctrl_pin_t *set);
uint32_t PDPTN5110_GetVbusVoltage(pd_phy_ptn5110_instance_t *ptn5110Instance);
uint8_t PDPTN5110_VsysIsPresent(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_IntcIrqClearAndEnable(pd_phy_ptn5110_instance_t *ptn5110Instance, uint16_t mask);
void PDPTN5110_IntcProcessIntAll(pd_phy_handle pdPhyHandle);
void PDPTN5110_IntcTypecChipInit(pd_phy_ptn5110_instance_t *ptn5110Instance);
void PDPTN5110_DisableFRSwap(pd_phy_ptn5110_instance_t *ptn5110Instance);

#endif
