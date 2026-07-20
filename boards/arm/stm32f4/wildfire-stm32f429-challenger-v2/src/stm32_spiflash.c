/****************************************************************************
 * boards/arm/stm32f4/wildfire-stm32f429-challenger-v2/src/stm32_spiflash.c
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

#include <debug.h>
#include <errno.h>

#include <nuttx/spi/spi.h>
#include <nuttx/mtd/mtd.h>
#include <nuttx/board.h>

#include <arch/board/board.h>

#include "stm32.h"
#include "stm32_gpio.h"
#include "stm32_spi.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* SPI Flash configuration */

#define SPI_FLASH_PORT    4  /* SPI4 */

/* SPI Flash chip select pin (PE4) */

#define GPIO_CS_SPIFLASH  (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | \
                           GPIO_OUTPUT_SET | GPIO_PORTE | GPIO_PIN4)

/****************************************************************************
 * Private Data ****************************************************************************/

static struct mtd_dev_s *g_mtd_dev = NULL;

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_spiflash_initialize
 *
 * Description:
 *   Initialize the SPI Flash interface.
 *
 ****************************************************************************/

int stm32_spiflash_initialize(void)
{
  struct spi_dev_s *spi;
  int ret;

  /* Configure the SPI Flash chip select pin */

  stm32_configgpio(GPIO_CS_SPIFLASH);
  stm32_gpiowrite(GPIO_CS_SPIFLASH, true);

  /* Initialize the SPI interface */

  spi = stm32_spibus_initialize(SPI_FLASH_PORT);
  if (spi == NULL)
    {
      syslog(LOG_ERR, "ERROR: stm32_spibus_initialize failed\n");
      return -ENODEV;
    }

  /* Initialize the SST25XX SPI Flash driver */

  g_mtd_dev = sst25xx_initialize(spi);
  if (g_mtd_dev == NULL)
    {
      syslog(LOG_ERR, "ERROR: sst25xx_initialize failed\n");
      return -ENODEV;
    }

#ifdef CONFIG_WILDFIRE_CHALLENGER_V2_SPI_FLASH_PART
  /* Create partitions if enabled */

  const char *partlist = CONFIG_WILDFIRE_CHALLENGER_V2_SPI_FLASH_PART_LIST;
  const char *partnames = CONFIG_WILDFIRE_CHALLENGER_V2_SPI_FLASH_PART_NAMES;

  ret = mtd_configure(g_mtd_dev, partlist, partnames);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: mtd_configure failed: %d\n", ret);
      return ret;
    }
#endif

  syslog(LOG_INFO, "SPI Flash initialized successfully\n");
  return OK;
}
