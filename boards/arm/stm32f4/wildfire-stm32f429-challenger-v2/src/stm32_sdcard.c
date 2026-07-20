/****************************************************************************
 * boards/arm/stm32f4/wildfire-stm32f429-challenger-v2/src/stm32_sdcard.c
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

#include <nuttx/mmcsd.h>
#include <nuttx/board.h>

#include <arch/board/board.h>

#include "stm32.h"
#include "stm32_gpio.h"
#include "stm32_sdio.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_sdcard_initialize
 *
 * Description:
 *   Initialize the SD card interface and mount the SD card.
 *
 ****************************************************************************/

int stm32_sdcard_initialize(void)
{
  struct sdio_dev_s *sdio;
  int ret;

  /* Initialize the SDIO interface */

  sdio = sdio_initialize(0);
  if (sdio == NULL)
    {
      syslog(LOG_ERR, "ERROR: sdio_initialize failed\n");
      return -ENODEV;
    }

  /* Mount the SD card */

  ret = mmcsd_slotinitialize(0, sdio);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: mmcsd_slotinitialize failed: %d\n", ret);
      return ret;
    }

  /* Detect if an SD card is present */

  sdio_mediachange(sdio, true);

  syslog(LOG_INFO, "SD card initialized successfully\n");
  return OK;
}
