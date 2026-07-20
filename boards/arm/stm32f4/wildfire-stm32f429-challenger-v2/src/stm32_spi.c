/****************************************************************************
 * boards/arm/stm32f4/wildfire-stm32f429-challenger-v2/src/stm32_spi.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to you under the Apache License, Version
 * 2.0 (the "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.  See the License for the specific language governing
 * permissions and limitations under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <stdbool.h>
#include <debug.h>

#include <nuttx/spi/spi.h>

#include "arm_internal.h"
#include "stm32_gpio.h"
#include "stm32_spi.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* SPI Flash chip select pin (PE4) */

#define GPIO_CS_SPIFLASH  (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | \
                           GPIO_OUTPUT_SET | GPIO_PORTE | GPIO_PIN4)

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_spi4select
 *
 * Description:
 *   Select or deselect the SPI4 device.
 *
 ****************************************************************************/

void stm32_spi4select(struct spi_dev_s *dev, uint32_t devid,
                      bool selected)
{
  spiinfo("devid: %d CS: %s\n", (int)devid,
          selected ? "assert" : "de-assert");

  /* Active low chip select for SPI Flash */

  stm32_gpiowrite(GPIO_CS_SPIFLASH, !selected);
}

/****************************************************************************
 * Name: stm32_spi4status
 *
 * Description:
 *   Return the status of the SPI4 device.
 *
 ****************************************************************************/

uint8_t stm32_spi4status(struct spi_dev_s *dev, uint32_t devid)
{
  return SPI_STATUS_PRESENT;
}

/****************************************************************************
 * Name: stm32_spi4cmddata
 *
 * Description:
 *   Set the command/data pin for SPI4.
 *
 ****************************************************************************/

int stm32_spi4cmddata(struct spi_dev_s *dev, uint32_t devid,
                      bool cmd)
{
  /* Not used for SPI Flash */

  return -ENODEV;
}

/****************************************************************************
 * Name: stm32_spi4register
 *
 * Description:
 *   Register SPI4 device.
 *
 ****************************************************************************/

int stm32_spi4register(struct spi_dev_s *dev, spi_mediachange_t callback,
                       void *arg)
{
  /* Nothing to do - SPI4 is initialized on demand */

  return OK;
}
