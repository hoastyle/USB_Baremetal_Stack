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
#include "Driver_Common.h"
#include "usb_osa.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if (defined PD_CONFIG_CMSIS_I2C_INTERFACE) && (PD_CONFIG_CMSIS_I2C_INTERFACE)
extern int32_t CMSIS_I2CInterfaceInit(cmsis_driver_adapter_t **cmsisDriver, uint8_t interface, void *interfaceConfig);
extern int32_t CMSIS_I2CInterfaceDeinit(cmsis_driver_adapter_t *cmsisDriver);
extern int32_t CMSIS_I2CInterfaceWriteRegister(
    cmsis_driver_adapter_t *cmsisDriver, uint32_t registerAddr, uint8_t registerLen, const uint8_t *data, uint32_t num);
extern int32_t CMSIS_I2CInterfaceReadRegister(
    cmsis_driver_adapter_t *cmsisDriver, uint32_t registerAddr, uint8_t registerLen, uint8_t *data, uint32_t num);
#endif

#if (defined PD_CONFIG_CMSIS_SPI_INTERFACE) && (PD_CONFIG_CMSIS_SPI_INTERFACE)
extern int32_t CMSIS_SPIInterfaceInit(cmsis_driver_adapter_t **cmsisDriver, uint8_t interface, void *interfaceConfig);
extern int32_t CMSIS_SPIInterfaceDeinit(cmsis_driver_adapter_t *cmsisDriver);
extern int32_t CMSIS_SPIInterfaceWriteRegister(
    cmsis_driver_adapter_t *cmsisDriver, uint32_t registerAddr, uint8_t registerLen, const uint8_t *data, uint32_t num);
extern int32_t CMSIS_SPIInterfaceReadRegister(
    cmsis_driver_adapter_t *cmsisDriver, uint32_t registerAddr, uint8_t registerLen, uint8_t *data, uint32_t num);
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

int32_t CMSIS_PortControlInterfaceInit(cmsis_driver_adapter_t **cmsisDriver, uint8_t interface, void *interfaceConfig)
{
    int32_t status = ARM_DRIVER_ERROR;

    switch (interface)
    {
#if (defined PD_CONFIG_CMSIS_I2C_INTERFACE) && (PD_CONFIG_CMSIS_I2C_INTERFACE)
        case kInterface_i2c0:
        case kInterface_i2c1:
        case kInterface_i2c2:
        case kInterface_i2c3:
            status = CMSIS_I2CInterfaceInit(cmsisDriver, interface, interfaceConfig);
            break;
#endif

#if (defined PD_CONFIG_CMSIS_SPI_INTERFACE) && (PD_CONFIG_CMSIS_SPI_INTERFACE)
        case kInterface_spi0:
        case kInterface_spi1:
        case kInterface_spi2:
        case kInterface_spi3:
            status = CMSIS_SPIInterfaceInit(cmsisDriver, interface, interfaceConfig);
            break;
#endif
    }

    if ((status == ARM_DRIVER_OK) && (*cmsisDriver != NULL))
    {
        if (USB_OsaMutexCreate(&((*cmsisDriver)->cmsisMutex)) != kStatus_USB_OSA_Success)
        {
            (*cmsisDriver)->cmsisMutex = NULL;
            status = ARM_DRIVER_ERROR;
        }
    }

    return status;
}

int32_t CMSIS_PortControlInterfaceDeinit(cmsis_driver_adapter_t *cmsisDriver)
{
    int32_t status = ARM_DRIVER_ERROR;

    switch (cmsisDriver->interface)
    {
#if (defined PD_CONFIG_CMSIS_I2C_INTERFACE) && (PD_CONFIG_CMSIS_I2C_INTERFACE)
        case kInterface_i2c0:
        case kInterface_i2c1:
        case kInterface_i2c2:
        case kInterface_i2c3:
            status = CMSIS_I2CInterfaceDeinit(cmsisDriver);
            break;
#endif

#if (defined PD_CONFIG_CMSIS_SPI_INTERFACE) && (PD_CONFIG_CMSIS_SPI_INTERFACE)
        case kInterface_spi0:
        case kInterface_spi1:
        case kInterface_spi2:
        case kInterface_spi3:
            status = CMSIS_SPIInterfaceDeinit(cmsisDriver);
            break;
#endif
    }

    if (cmsisDriver->cmsisMutex != NULL)
    {
        USB_OsaMutexDestroy(cmsisDriver->cmsisMutex);
        cmsisDriver->cmsisMutex = NULL;
    }

    return status;
}

int32_t CMSIS_PortControlInterfaceWriteRegister(
    cmsis_driver_adapter_t *cmsisDriver, uint32_t registerAddr, uint8_t registerLen, const uint8_t *data, uint32_t num)
{
    int32_t status = ARM_DRIVER_ERROR;

    if (USB_OsaMutexLock(cmsisDriver->cmsisMutex) != kStatus_USB_OSA_Success)
    {
        return status;
    }

    switch (cmsisDriver->interface)
    {
#if (defined PD_CONFIG_CMSIS_I2C_INTERFACE) && (PD_CONFIG_CMSIS_I2C_INTERFACE)
        case kInterface_i2c0:
        case kInterface_i2c1:
        case kInterface_i2c2:
        case kInterface_i2c3:
            status = CMSIS_I2CInterfaceWriteRegister(cmsisDriver, registerAddr, registerLen, data, num);
            break;
#endif

#if (defined PD_CONFIG_CMSIS_SPI_INTERFACE) && (PD_CONFIG_CMSIS_SPI_INTERFACE)
        case kInterface_spi0:
        case kInterface_spi1:
        case kInterface_spi2:
        case kInterface_spi3:
            status = CMSIS_SPIInterfaceWriteRegister(cmsisDriver, registerAddr, registerLen, data, num);
            break;
#endif
    }

    if (USB_OsaMutexUnlock(cmsisDriver->cmsisMutex) != kStatus_USB_OSA_Success)
    {
        return ARM_DRIVER_ERROR;
    }

    return status;
}

int32_t CMSIS_PortControlInterfaceReadRegister(
    cmsis_driver_adapter_t *cmsisDriver, uint32_t registerAddr, uint8_t registerLen, uint8_t *data, uint32_t num)
{
    int32_t status = ARM_DRIVER_ERROR;

    if (USB_OsaMutexLock(cmsisDriver->cmsisMutex) != kStatus_USB_OSA_Success)
    {
        return status;
    }

    switch (cmsisDriver->interface)
    {
#if (defined PD_CONFIG_CMSIS_I2C_INTERFACE) && (PD_CONFIG_CMSIS_I2C_INTERFACE)
        case kInterface_i2c0:
        case kInterface_i2c1:
        case kInterface_i2c2:
        case kInterface_i2c3:
            status = CMSIS_I2CInterfaceReadRegister(cmsisDriver, registerAddr, registerLen, data, num);
            break;
#endif

#if (defined PD_CONFIG_CMSIS_SPI_INTERFACE) && (PD_CONFIG_CMSIS_SPI_INTERFACE)
        case kInterface_spi0:
        case kInterface_spi1:
        case kInterface_spi2:
        case kInterface_spi3:
            status = CMSIS_SPIInterfaceReadRegister(cmsisDriver, registerAddr, registerLen, data, num);
            break;
#endif
    }

    if (USB_OsaMutexUnlock(cmsisDriver->cmsisMutex) != kStatus_USB_OSA_Success)
    {
        return ARM_DRIVER_ERROR;
    }

    return status;
}
