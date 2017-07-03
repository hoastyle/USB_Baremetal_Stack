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

#include <stdint.h>
#include <stdbool.h>
#include "usb_pd_config.h"
#include "usb_pd.h"
#include "usb_cmsis_wrapper.h"
#include "Driver_I2C.h"

#if (defined PD_CONFIG_CMSIS_I2C_INTERFACE) && (PD_CONFIG_CMSIS_I2C_INTERFACE)

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define CMSIS_DRIVER_INSTANCE_COUNT (3)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void CMSIS_SignalEvent0(uint32_t event);
void CMSIS_SignalEvent1(uint32_t event);
void CMSIS_SignalEvent2(uint32_t event);
void USB_WEAK BOARD_I2C0_ReleaseBus(void);
void USB_WEAK BOARD_I2C1_ReleaseBus(void);
void USB_WEAK BOARD_I2C2_ReleaseBus(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

extern ARM_DRIVER_I2C USB_WEAK Driver_I2C0;
extern ARM_DRIVER_I2C USB_WEAK Driver_I2C1;
extern ARM_DRIVER_I2C USB_WEAK Driver_I2C2;
static ARM_DRIVER_I2C *s_DriverI2CArray[CMSIS_DRIVER_INSTANCE_COUNT] = {&Driver_I2C0, &Driver_I2C1, &Driver_I2C2};
static cmsis_driver_adapter_t s_CMSISI2CDriverInstance[CMSIS_DRIVER_INSTANCE_COUNT];
const ARM_I2C_SignalEvent_t s_CMSISI2CDriverCallback[CMSIS_DRIVER_INSTANCE_COUNT] = {
    CMSIS_SignalEvent0, CMSIS_SignalEvent1, CMSIS_SignalEvent2};

#if (PD_CONFIG_CMSIS_I2C_INTERFACE > CMSIS_DRIVER_INSTANCE_COUNT)
#error "CMSIS driver error, please increase the instance count"
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

void *CMSIS_GetI2cInterface(uint8_t interface)
{
    if ((interface - kInterface_i2c0 + 1) > CMSIS_DRIVER_INSTANCE_COUNT)
    {
        return NULL;
    }
    else
    {
        return (void *)(s_DriverI2CArray[interface - kInterface_i2c0]);
    }
}

void CMSIS_I2cReleaseBus(uint8_t interface)
{
    switch (interface)
    {
        case kInterface_i2c0:
            BOARD_I2C0_ReleaseBus();
            break;

        case kInterface_i2c1:
            BOARD_I2C1_ReleaseBus();
            break;

        case kInterface_i2c2:
            BOARD_I2C2_ReleaseBus();
            break;

        default:
            break;
    }
}

static void CMSIS_SignalEvent0(uint32_t event)
{
    if (s_CMSISI2CDriverInstance[0].cmsisState != CMSIS_TRANSFERING)
    {
        return;
    }

    switch (event)
    {
        case ARM_I2C_EVENT_TRANSFER_DONE:
            s_CMSISI2CDriverInstance[0].cmsisState = CMSIS_IDLE;
            break;

        case ARM_I2C_EVENT_TRANSFER_INCOMPLETE:
        case ARM_I2C_EVENT_ADDRESS_NACK:
        case ARM_I2C_EVENT_ARBITRATION_LOST:
        case ARM_I2C_EVENT_BUS_ERROR:
        case ARM_I2C_EVENT_BUS_CLEAR:
        case ARM_I2C_EVENT_SLAVE_TRANSMIT:
        case ARM_I2C_EVENT_SLAVE_RECEIVE:
        case ARM_I2C_EVENT_GENERAL_CALL:
        default:
            s_CMSISI2CDriverInstance[0].cmsisState = CMSIS_TRANSFER_ERROR_DONE;
            break;
    }
}

static void CMSIS_SignalEvent1(uint32_t event)
{
    if (s_CMSISI2CDriverInstance[1].cmsisState != CMSIS_TRANSFERING)
    {
        return;
    }

    switch (event)
    {
        case ARM_I2C_EVENT_TRANSFER_DONE:
            s_CMSISI2CDriverInstance[1].cmsisState = CMSIS_IDLE;
            break;

        case ARM_I2C_EVENT_TRANSFER_INCOMPLETE:
        case ARM_I2C_EVENT_ADDRESS_NACK:
        case ARM_I2C_EVENT_ARBITRATION_LOST:
        case ARM_I2C_EVENT_BUS_ERROR:
        case ARM_I2C_EVENT_BUS_CLEAR:
        case ARM_I2C_EVENT_SLAVE_TRANSMIT:
        case ARM_I2C_EVENT_SLAVE_RECEIVE:
        case ARM_I2C_EVENT_GENERAL_CALL:
        default:
            s_CMSISI2CDriverInstance[1].cmsisState = CMSIS_TRANSFER_ERROR_DONE;
            break;
    }
}

static void CMSIS_SignalEvent2(uint32_t event)
{
    if (s_CMSISI2CDriverInstance[2].cmsisState != CMSIS_TRANSFERING)
    {
        return;
    }

    switch (event)
    {
        case ARM_I2C_EVENT_TRANSFER_DONE:
            s_CMSISI2CDriverInstance[2].cmsisState = CMSIS_IDLE;
            break;

        case ARM_I2C_EVENT_TRANSFER_INCOMPLETE:
        case ARM_I2C_EVENT_ADDRESS_NACK:
        case ARM_I2C_EVENT_ARBITRATION_LOST:
        case ARM_I2C_EVENT_BUS_ERROR:
        case ARM_I2C_EVENT_BUS_CLEAR:
        case ARM_I2C_EVENT_SLAVE_TRANSMIT:
        case ARM_I2C_EVENT_SLAVE_RECEIVE:
        case ARM_I2C_EVENT_GENERAL_CALL:
        default:
            s_CMSISI2CDriverInstance[2].cmsisState = CMSIS_TRANSFER_ERROR_DONE;
            break;
    }
}

int32_t CMSIS_I2CInterfaceInit(cmsis_driver_adapter_t **cmsisDriver, uint8_t interface, void *interfaceConfig)
{
    uint8_t index = 0;
    cmsis_driver_adapter_t *cmsis = NULL;
    int32_t status;
    pd_i2c_interface_config_t *i2cConfig = (pd_i2c_interface_config_t *)interfaceConfig;
    void *cmsisInterface = NULL;
    USB_OSA_SR_ALLOC();

    cmsisInterface = CMSIS_GetI2cInterface(interface);
    if (cmsisInterface == NULL)
    {
        return ARM_DRIVER_ERROR;
    }

    USB_OSA_ENTER_CRITICAL();
    for (; index < PD_CONFIG_MAX_PORT; index++)
    {
        if (s_CMSISI2CDriverInstance[index].occupied != 1)
        {
            uint8_t *buffer = (uint8_t *)&s_CMSISI2CDriverInstance[index];
            for (uint32_t j = 0U; j < sizeof(cmsis_driver_adapter_t); j++)
            {
                buffer[j] = 0x00U;
            }
            s_CMSISI2CDriverInstance[index].occupied = 1;
            cmsis = &s_CMSISI2CDriverInstance[index];
            cmsis->callback = (void *)s_CMSISI2CDriverCallback[index];
            break;
        }
    }
    USB_OSA_EXIT_CRITICAL();
    if (cmsis == NULL)
    {
        return ARM_DRIVER_ERROR;
    }

    *cmsisDriver = cmsis;
    cmsis->interface = interface;
    cmsis->i2cAddress = i2cConfig->slaveAddress;
    cmsis->cmsisState = CMSIS_IDLE;
    cmsis->cmsisInterface = cmsisInterface;

    status = ((ARM_DRIVER_I2C *)(cmsis->cmsisInterface))->Initialize((ARM_I2C_SignalEvent_t)cmsis->callback);
    if (status == ARM_DRIVER_OK)
    {
        status = ((ARM_DRIVER_I2C *)(cmsis->cmsisInterface))->PowerControl(ARM_POWER_FULL);
    }
    if (status == ARM_DRIVER_OK)
    {
        /*config transmit speed*/
        status = ((ARM_DRIVER_I2C *)(cmsis->cmsisInterface))->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
    }

    return status;
}

int32_t CMSIS_I2CInterfaceDeinit(cmsis_driver_adapter_t *cmsisDriver)
{
    int32_t status;
    USB_OSA_SR_ALLOC();

    status = ((ARM_DRIVER_I2C *)(cmsisDriver->cmsisInterface))->Uninitialize();
    USB_OSA_ENTER_CRITICAL();
    cmsisDriver->occupied = 0;
    USB_OSA_EXIT_CRITICAL();
    return status;
}

int32_t CMSIS_I2CInterfaceWriteRegister(
    cmsis_driver_adapter_t *cmsisDriver, uint32_t registerAddr, uint8_t registerLen, const uint8_t *data, uint32_t num)
{
    int32_t result;
    uint8_t index;
    uint32_t waitCount;

    cmsisDriver->cmsisTry = CMSIS_TRANSFER_RETRY_COUNT;
    do
    {
        cmsisDriver->cmsisState = CMSIS_TRANSFERING;
        result = ARM_DRIVER_ERROR;
        if (registerLen == 1)
        {
            cmsisDriver->dataBuffer[0] = registerAddr;
            waitCount = 1;
        }
        else
        {
            cmsisDriver->dataBuffer[0] = (registerAddr >> 8);
            cmsisDriver->dataBuffer[1] = registerAddr;
            waitCount = 2;
        }
        for (index = 0; index < num; ++index)
        {
            cmsisDriver->dataBuffer[waitCount + index] = data[index];
        }
        result = ((ARM_DRIVER_I2C *)(cmsisDriver->cmsisInterface))
                     ->MasterTransmit(cmsisDriver->i2cAddress, (const uint8_t *)cmsisDriver->dataBuffer,
                                      registerLen + num, 0);
        if (result == ARM_DRIVER_ERROR_BUSY)
        {
            ((ARM_DRIVER_I2C *)(cmsisDriver->cmsisInterface))->Control(ARM_I2C_ABORT_TRANSFER, 0);
            CMSIS_I2cReleaseBus(cmsisDriver->interface);
            ((ARM_DRIVER_I2C *)(cmsisDriver->cmsisInterface))->Initialize((ARM_I2C_SignalEvent_t)cmsisDriver->callback);
            continue;
        }
        if (result == ARM_DRIVER_OK)
        {
            waitCount = 1000000;
            while (cmsisDriver->cmsisState == CMSIS_TRANSFERING)
            {
                waitCount--;
                if (waitCount == 0)
                {
                    CMSIS_I2cReleaseBus(cmsisDriver->interface);
                    ((ARM_DRIVER_I2C *)(cmsisDriver->cmsisInterface))
                        ->Initialize((ARM_I2C_SignalEvent_t)cmsisDriver->callback);
                    ((ARM_DRIVER_I2C *)(cmsisDriver->cmsisInterface))->Control(ARM_I2C_ABORT_TRANSFER, 0);
                    cmsisDriver->cmsisState = CMSIS_TRANSFER_ERROR_DONE;
                    break;
                }
            }
            if (cmsisDriver->cmsisState == CMSIS_IDLE)
            {
                result = ARM_DRIVER_OK;
            }
            else
            {
                result = ARM_DRIVER_ERROR;
            }
        }
        else
        {
            result = ARM_DRIVER_ERROR;
        }

        if (result == ARM_DRIVER_OK)
        {
            break;
        }
        else
        {
            CMSIS_I2cReleaseBus(cmsisDriver->interface);
            ((ARM_DRIVER_I2C *)(cmsisDriver->cmsisInterface))->Initialize((ARM_I2C_SignalEvent_t)cmsisDriver->callback);
        }
    } while (--cmsisDriver->cmsisTry);

    return result;
}

int32_t CMSIS_I2CInterfaceReadRegister(
    cmsis_driver_adapter_t *cmsisDriver, uint32_t registerAddr, uint8_t registerLen, uint8_t *data, uint32_t num)
{
    int32_t result;
    uint32_t waitCount;

    cmsisDriver->cmsisTry = CMSIS_TRANSFER_RETRY_COUNT;
    do
    {
        cmsisDriver->cmsisState = CMSIS_TRANSFERING;
        result = ARM_DRIVER_ERROR;
        result = ((ARM_DRIVER_I2C *)(cmsisDriver->cmsisInterface))
                     ->MasterTransmit(cmsisDriver->i2cAddress, (const uint8_t *)&registerAddr, registerLen, 1);
        if (result == ARM_DRIVER_ERROR_BUSY)
        {
            ((ARM_DRIVER_I2C *)(cmsisDriver->cmsisInterface))->Control(ARM_I2C_ABORT_TRANSFER, 0);
            CMSIS_I2cReleaseBus(cmsisDriver->interface);
            ((ARM_DRIVER_I2C *)(cmsisDriver->cmsisInterface))->Initialize((ARM_I2C_SignalEvent_t)cmsisDriver->callback);
            continue;
        }
        if (result == ARM_DRIVER_OK)
        {
            waitCount = 1000000;
            while (cmsisDriver->cmsisState == CMSIS_TRANSFERING)
            {
                waitCount--;
                if (waitCount == 0)
                {
                    CMSIS_I2cReleaseBus(cmsisDriver->interface);
                    ((ARM_DRIVER_I2C *)(cmsisDriver->cmsisInterface))
                        ->Initialize((ARM_I2C_SignalEvent_t)cmsisDriver->callback);
                    ((ARM_DRIVER_I2C *)(cmsisDriver->cmsisInterface))->Control(ARM_I2C_ABORT_TRANSFER, 0);
                    cmsisDriver->cmsisState = CMSIS_TRANSFER_ERROR_DONE;
                    break;
                }
            }
            if (cmsisDriver->cmsisState == CMSIS_IDLE)
            {
                cmsisDriver->cmsisState = CMSIS_TRANSFERING;
                result = ((ARM_DRIVER_I2C *)(cmsisDriver->cmsisInterface))
                             ->MasterReceive(cmsisDriver->i2cAddress, data, num, 0);
                if (result == ARM_DRIVER_ERROR_BUSY)
                {
                    ((ARM_DRIVER_I2C *)(cmsisDriver->cmsisInterface))->Control(ARM_I2C_ABORT_TRANSFER, 0);
                    CMSIS_I2cReleaseBus(cmsisDriver->interface);
                    ((ARM_DRIVER_I2C *)(cmsisDriver->cmsisInterface))
                        ->Initialize((ARM_I2C_SignalEvent_t)cmsisDriver->callback);
                    continue;
                }

                if (result == ARM_DRIVER_OK)
                {
                    waitCount = 1000000;
                    while (cmsisDriver->cmsisState == CMSIS_TRANSFERING)
                    {
                        waitCount--;
                        if (waitCount == 0)
                        {
                            CMSIS_I2cReleaseBus(cmsisDriver->interface);
                            ((ARM_DRIVER_I2C *)(cmsisDriver->cmsisInterface))
                                ->Initialize((ARM_I2C_SignalEvent_t)cmsisDriver->callback);
                            ((ARM_DRIVER_I2C *)(cmsisDriver->cmsisInterface))->Control(ARM_I2C_ABORT_TRANSFER, 0);
                            cmsisDriver->cmsisState = CMSIS_TRANSFER_ERROR_DONE;
                            break;
                        }
                    }
                    if (cmsisDriver->cmsisState == CMSIS_IDLE)
                    {
                        result = ARM_DRIVER_OK;
                    }
                    else
                    {
                        result = ARM_DRIVER_ERROR;
                    }
                }
                else
                {
                    result = ARM_DRIVER_ERROR;
                }
            }
            else
            {
                result = ARM_DRIVER_ERROR;
            }
        }
        else
        {
            result = ARM_DRIVER_ERROR;
        }

        if (result == ARM_DRIVER_OK)
        {
            break;
        }
        else
        {
            CMSIS_I2cReleaseBus(cmsisDriver->interface);
            ((ARM_DRIVER_I2C *)(cmsisDriver->cmsisInterface))->Initialize((ARM_I2C_SignalEvent_t)cmsisDriver->callback);
        }
    } while (--cmsisDriver->cmsisTry);

    return result;
}

#endif
