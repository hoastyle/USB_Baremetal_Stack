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

#ifndef __SPI_IIC_ADAPTER_H__
#define __SPI_IIC_ADAPTER_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef enum _cmsis_driver_state
{
    CMSIS_IDLE,
    CMSIS_TRANSFERING,
    CMSIS_TRANSFER_ERROR_DONE,
} cmsis_driver_state_t;

/*!
 * @addtogroup usb_pd_cmsis_wrapper
 * @{
 */

/*! @brief CMSIS I2C/SPI transfer retry count */
#define CMSIS_TRANSFER_RETRY_COUNT (10)

/*! @brief I2C interface configure parameter */
typedef struct _pd_i2c_interface_config
{
    uint8_t slaveAddress;
} pd_i2c_interface_config_t;

/*! @brief SPI interface configure parameter */
typedef struct _pd_spi_interface_config
{
    uint32_t pcs;
} pd_spi_interface_config_t;

typedef struct _cmsis_drier_adapter
{
    usb_osa_mutex_handle cmsisMutex;
    void *cmsisInterface;
    void *callback;
    uint32_t spiPCS;
    uint16_t i2cAddress;
    uint8_t occupied;
    uint8_t interface;
    volatile uint8_t dataBuffer[32];
    volatile uint8_t cmsisState;
    volatile uint8_t cmsisResult;
    volatile uint8_t cmsisTry;
} cmsis_driver_adapter_t;

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @brief Initialize CMSIS driver adapter instance.
 *
 * This function return the #cmsis_driver_adapter_t instance, the other API use this as the parameter.
 *
 * @param[out] cmsisDriver   Return the instance.
 * @param[in] interface      the I2C/SPI interface, see #pd_phy_interface_t
 * @param[in] interfaceConfig it is #pd_i2c_interface_config_t or #pd_spi_interface_config_t
 *
 * @retval ARM_DRIVER_ERROR    initialization success.
 * @retval other value         error code.
 */
int32_t CMSIS_PortControlInterfaceInit(cmsis_driver_adapter_t **cmsisDriver, uint8_t interface, void *interfaceConfig);

/*!
 * @brief De-initialize CMSIS driver adapter instance.
 *
 * @param[in] cmsisDriver    The instance from CMSIS_PortControlInterfaceInit
 *
 * @retval ARM_DRIVER_ERROR    initialization success.
 * @retval other value         error code.
 */
int32_t CMSIS_PortControlInterfaceDeinit(cmsis_driver_adapter_t *cmsisDriver);

/*!
 * @brief Write data to slave.
 *
 * @param[in] cmsisDriver    Return the instance.
 * @param[in] registerAddr   The access register address.
 * @param[in] registerLen    The register addreess's length, normally it is one byte or two bytes.
 * @param[in] data           The data buffer.
 * @param[in] num            The data length.
 *
 * @retval ARM_DRIVER_ERROR    initialization success.
 * @retval other value         error code.
 */
int32_t CMSIS_PortControlInterfaceWriteRegister(
    cmsis_driver_adapter_t *cmsisDriver, uint32_t registerAddr, uint8_t registerLen, const uint8_t *data, uint32_t num);

/*!
 * @brief Read data from slave.
 *
 * @param[in] cmsisDriver    Return the instance.
 * @param[in] registerAddr   The access register address.
 * @param[in] registerLen    The register addreess's length, normally it is one byte or two bytes.
 * @param[in] data           The data buffer.
 * @param[in] num            The data length.
 *
 * @retval ARM_DRIVER_ERROR    initialization success.
 * @retval other value         error code.
 */
int32_t CMSIS_PortControlInterfaceReadRegister(
    cmsis_driver_adapter_t *cmsisDriver, uint32_t registerAddr, uint8_t registerLen, uint8_t *data, uint32_t num);

/*! @}*/

#endif
