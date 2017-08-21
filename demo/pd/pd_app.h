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

#ifndef __DPM_EXTERNAL_H__
#define __DPM_EXTERNAL_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define PD_CONFIG_APP_MODE (0x00000001u)
#define PD_CONFIG_PID (0x0100)
#define PD_VENDOR_VID (0x1FC9u)
#define PD_CONFIG_XID (0x0000u)
#define PD_CONFIG_FW_VER (0x01)
#define PD_CONFIG_HW_VER (0x01)
#define PD_CONFIG_BCD_DEVICE (0x0100)

#define PD_I2C_MASTER_IRQn (I2C1_IRQn)

#define BOARD_PTN5110_GPIO_PIN (31)
#define BOARD_PTN5110_GPIO_PORT (PORTE)
#define BOARD_PTN5110_GPIO (GPIOE)
#define BOARD_PTN5110_GPIO_IRQ (PORTB_PORTC_PORTD_PORTE_IRQn)

#define PD_EXTRA_EN_SRC_GPIO (GPIOA)
#define PD_EXTRA_EN_SRC_GPIO_PIN (13)
#define PD_EXTRA_EN_SRC_GPIO_PORT (PORTA)

#define PD_DEMO_EXPECTED_VOLTAGE (5 * 1000) /* 5V */
#define PD_DEMO_EXPECTED_CURRENT (2700)     /* 2.7A */

#define PD_DEMO_PRINT_BUFFER_COUNT (15)

/* Source capabilities extended
 * 3 bytes extended header + 24 bytes Data Block
 */
typedef struct _pd_source_cap_ext_data_block
{
    uint16_t vid;
    uint16_t pid;
    uint32_t xid;
    uint8_t fwVersion;
    uint8_t hwVersion;
    uint8_t voltageRegulation;
    uint8_t holdupTime;
    uint8_t compliance;
    uint8_t touchCurrent;
    uint16_t peakCurrent1;
    uint16_t peakCurrent2;
    uint16_t peakCurrent3;
    uint8_t touchTemp;
    uint8_t sourceInputs;
    uint8_t batteries;
} pd_source_cap_ext_data_block_t;

/* status message，but lack two bytes:event flags and temperature status */
typedef struct _pd_status_data_block
{
    uint8_t internalTemp;
    uint8_t presentInput;
    uint8_t presentBatteryInput;
} pd_status_data_block_t;

/* 6.4.6 Alert message
 * Alert message will be followed by get status or get battery status
 */
typedef struct _pd_alert_data_object
{
    union
    {
        uint32_t alertValue;
        struct
        {
            uint32_t reserved : 16;
            uint32_t hostSwappableBatteries : 4;
            uint32_t fixedBatteries : 4;
            uint32_t typeOfAlert : 8;
        } bitFields;
    };
} pd_alert_data_object_t;

/* 6.5.5 Battery capabilities message is sent in response to get_battery_cap */
typedef struct _pd_battery_cap_data_block
{
    uint16_t vid;
    uint16_t pid;
    uint16_t batteryDesignCap;
    uint16_t batteryLastFullChargeCap;
    uint8_t batteryType;
} pd_battery_cap_data_block_t;

/* 6.4.5 battery status message */
typedef struct _pd_battery_status_data_object
{
    uint8_t reserved;
	//battery status
    uint8_t batterInfo;
	//battery capacity
    uint16_t batteryPC;
} pd_battery_status_data_object_t;

/* 6.5.7 Manufacturer info message */
typedef struct _pd_manufac_info_data_block
{
    uint16_t vid;
    uint16_t pid;
    uint8_t manufacturerString[22];
} pd_manufac_info_data_block_t;

/* id header VDO of discover identity response*/
// Question: 这里为什么使用union而不直接使用struct bitFields
typedef struct _pd_id_heaer_vdo
{
    union
    {
        uint32_t vdoValue;
        struct
        {
            uint32_t usbVendorID : 16;
            uint32_t reserved : 7;
            uint32_t productTypeDFP : 3;
            uint32_t modalOperateSupport : 1;
            uint32_t productTypeUFPOrCablePlug : 3;
            uint32_t usbCommunicationCapableAsDevice : 1;
            uint32_t usbCommunicationCapableAsHost : 1;
        } bitFields;
    };
} pd_id_header_vdo_t;

/* 6.4.4.3.1 discover identity response */
// 这个struct不全，应该无法支持多个AMA?
typedef struct _pd_vdm_identity_data
{
    pd_id_header_vdo_t idHeaderVDO;
	// cert VDO
    uint32_t certStatVDO;
	// product VDO
    uint16_t bcdDevice;
    uint16_t pid;
} pd_vdm_identity_data_t;

typedef enum _pd_demo_sw_state
{
    kDEMO_SWIdle = 0,
    kDEMO_SWIsrTrigger,
    kDEMO_SWPending,
    kDEMO_SWShortPress,
    kDEMO_SWLongPress,
    kDEMO_SWProcessed,
} pd_demo_sw_state_t;

//app表示一种应用场景？ 存储当前设备的信息，包括source/sink capabilities, infomation and so on
typedef struct _pd_app
{
	/* PD Instance */
    pd_handle pdHandle;

	/* Source Power Supply */
    pd_source_pdo_t selfSourcePdo1;
    pd_source_pdo_t selfSourcePdo2;
    pd_sink_pdo_t selfSinkPdo1;
    pd_sink_pdo_t selfSinkPdo2;

	/* VDM Info Structure, including Identity, SVID, Modes */
    pd_vdm_identity_data_t selfVdmIdentity;
    uint32_t selfVdmSVIDs;
    uint32_t selfVdmModes;

	/* Sink Request */
    pd_rdo_t sinkRequestRDO; /* sink - the self requested RDO; source - the partner sink requested RDO */


    pd_svdm_command_param_t structuredVDMCommandParam;

	/* VDM Header */
	/* structured vdm header */
    pd_structured_vdm_header_t defaultSVDMCommandHeader;
	/* unstructured vdm header */
    pd_unstructured_vdm_header_t unstructuredVDMCommandHeader;
	/* source extended data message */
    pd_source_cap_ext_data_block_t selfExtCap;
	/* status message*/
    pd_status_data_block_t selfStatus;
	/* Alert message */
    pd_alert_data_object_t selfAlert;
	/* Battery capabilities */
    pd_battery_cap_data_block_t selfBatteryCap;
	/* Battery status */
    pd_battery_status_data_object_t selfBatteryStatus;
	/* manufacturer info */
    pd_manufac_info_data_block_t selfManufacInfo;

    uint32_t sourceVbusVoltage;
    uint32_t sinkRequestVoltage;
    uint8_t commonData[8];
    uint16_t partnerSVIDs[8];
    uint16_t partnerModes[8];
    pd_source_pdo_t partnerSourceCaps[7];
    pd_source_pdo_t partnerSinkCaps[7];
    uint8_t partnerSourceCapNumber; /* partner */
    uint8_t partnerSinkCapNumber;   /* partner */
    uint8_t getBatteryCapDataBlock;
    uint8_t selfSouceCapNumber;
    uint8_t selfSinkCapNumber;
    uint8_t msgSop;
    uint8_t reqestResponse;
    uint8_t appPowerRole;
    uint8_t appDataRole;
    uint8_t connectionRole;
    volatile uint8_t sw1State;
    volatile uint8_t sw3State;
    volatile uint16_t sw1Time;
    volatile uint16_t sw3Time;
    volatile uint8_t selfHasEnterAlernateMode;
    volatile uint8_t printConsumer;
    volatile uint8_t printProvider;
    volatile uint8_t printFull;
    uint8_t printBuffers[PD_DEMO_PRINT_BUFFER_COUNT][70];

    /* application can maintain these values for command evaluation */
    volatile uint32_t prSwapAccept : 1;    /* pr_swap and fr_swap */
    volatile uint32_t drSwapAccept : 1;    /* dr_swap */
    volatile uint32_t vconnSwapAccept : 1; /* vconn_swap */
} pd_app_t;

extern pd_app_t g_PDAppInstance;

/*******************************************************************************
 * API
 ******************************************************************************/

#endif
