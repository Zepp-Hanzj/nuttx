/****************************************************************************
 * boards/arm/stm32f4/wildfire-stm32f429-challenger-v2/src/stm32_bringup.c
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

#include <sys/types.h>
#include <sys/mount.h>
#include <syslog.h>
#include <errno.h>

#include <nuttx/board.h>

#ifdef CONFIG_USERLED
#  include <nuttx/leds/userled.h>
#endif

#ifdef CONFIG_INPUT_BUTTONS
#  include <nuttx/input/buttons.h>
#endif

#ifdef CONFIG_WILDFIRE_CHALLENGER_V2_LCD
#  include <nuttx/video/fb.h>
#endif

#ifdef CONFIG_WILDFIRE_CHALLENGER_V2_SD_CARD
#  include <nuttx/mmcsd.h>
#endif

#ifdef CONFIG_WILDFIRE_CHALLENGER_V2_SPI_FLASH
#  include <nuttx/mtd/mtd.h>
#endif

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

int stm32_bringup(void);
void stm32_sdram_initialize(void);

#ifdef CONFIG_WILDFIRE_CHALLENGER_V2_LCD
int stm32_lcdinitialize(void);
#endif

#ifdef CONFIG_WILDFIRE_CHALLENGER_V2_SD_CARD
int stm32_sdcard_initialize(void);
#endif

#ifdef CONFIG_WILDFIRE_CHALLENGER_V2_SPI_FLASH
int stm32_spiflash_initialize(void);
#endif

/* LED test - directly toggle GPIO for debugging */
#include "stm32_gpio.h"
#include <arch/board/board.h>

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: board_late_initialize
 *
 * Description:
 *   If CONFIG_BOARD_LATE_INITIALIZE is selected, then an additional
 *   initialization call will be performed in the boot-up sequence to a
 *   function called board_late_initialize().  board_late_initialize() will
 *   be called immediately after up_initialize() is called and just before
 *   the initial application is started.  This additional initialization
 *   phase may be used, for example, to initialize board-specific device
 *   drivers.
 *
 ****************************************************************************/

#ifdef CONFIG_BOARD_LATE_INITIALIZE
void board_late_initialize(void)
{
  /* Perform board-specific initialization */

  stm32_bringup();
}
#endif

/****************************************************************************
 * Name: stm32_bringup
 *
 * Description:
 *   Bring up board features
 *
 ****************************************************************************/

int stm32_bringup(void)
{
  int ret;

#ifdef CONFIG_FS_PROCFS
  /* Mount the procfs file system */

  ret = mount(NULL, "/proc", "procfs", 0, NULL);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: Failed to mount /proc: %d\n", errno);
    }
#endif

#ifdef CONFIG_STM32_FMC
  /* Test SDRAM at offset 1MB (avoid heap sentinel at base) */

  {
    volatile uint32_t *sdram = (volatile uint32_t *)(0xD0100000);
    size_t count = 4096;
    size_t i;
    int pass = 1;

    for (i = 0; i < count; i++)
      {
        sdram[i] = (uint32_t)i ^ 0xDEADBEEF;
      }

    for (i = 0; i < count; i++)
      {
        if (sdram[i] != ((uint32_t)i ^ 0xDEADBEEF))
          {
            syslog(LOG_ERR, "SDRAM FAIL at offset %lu: read 0x%08lx\n",
                   (unsigned long)i, (unsigned long)sdram[i]);
            pass = 0;
            break;
          }
      }

    if (pass)
      {
        syslog(LOG_INFO, "SDRAM test PASSED (16KB at 0xD0100000)\n");
      }
  }
#endif

#ifdef CONFIG_USERLED
  /* Register the LED driver */

  ret = userled_lower_initialize("/dev/userleds");
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: userled_lower_initialize failed: %d\n", ret);
    }
#endif

#ifdef CONFIG_INPUT_BUTTONS
  /* Register the BUTTON driver */

  ret = btn_lower_initialize("/dev/buttons");
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: btn_lower_initialize failed: %d\n", ret);
    }
#endif

#ifdef CONFIG_WILDFIRE_CHALLENGER_V2_LCD
  /* Initialize LCD */

  ret = stm32_lcdinitialize();
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: stm32_lcdinitialize failed: %d\n", ret);
    }
#endif

#ifdef CONFIG_WILDFIRE_CHALLENGER_V2_SD_CARD
  /* Initialize SD card */

  ret = stm32_sdcard_initialize();
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: stm32_sdcard_initialize failed: %d\n", ret);
    }
#endif

#ifdef CONFIG_WILDFIRE_CHALLENGER_V2_SPI_FLASH
  /* Initialize SPI Flash */

  ret = stm32_spiflash_initialize();
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: stm32_spiflash_initialize failed: %d\n", ret);
    }
#endif

  UNUSED(ret);
  return OK;
}
