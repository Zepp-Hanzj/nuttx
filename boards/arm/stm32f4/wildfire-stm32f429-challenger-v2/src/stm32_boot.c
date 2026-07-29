/****************************************************************************
 * boards/arm/stm32f4/wildfire-stm32f429-challenger-v2/src/stm32_boot.c
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

#include <nuttx/board.h>
#include <arch/board/board.h>

#include "arm_internal.h"
#include "stm32.h"
#include "stm32_gpio.h"

/* SDRAM must be initialized before arm_addregion() adds it to the heap */

#ifdef CONFIG_STM32_FMC
extern void stm32_sdram_initialize(void);
#endif

#ifdef CONFIG_WILDFIRE_CHALLENGER_V2_NAND_FLASH
extern void nand_fmc_init(void);
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_boardinitialize
 *
 * Description:
 *   All STM32 architectures must provide the following entry point.
 *   This entry point is called early in the initialization -- after all
 *   memory has been configured and mapped but before any devices have been
 *   initialized.
 *
 ****************************************************************************/

void stm32_boardinitialize(void)
{
#ifdef CONFIG_ARCH_LEDS
  /* Configure LED GPIOs as outputs HIGH (active-low LEDs OFF) as early as
   * possible.  After reset PH10/11/12 are floating inputs and the LEDs
   * are wired active-low (anode to VCC), so they light during the entire
   * boot window until board_autoled_initialize() runs below.  Configuring
   * them HIGH here kills them from the very first instruction of board
   * init.  stm32_configgpio is safe to call this early (stm32_gpioinit()
   * already ran in __start).
   */

  board_autoled_initialize();
  {
    int i;
    for (i = 0; i < BOARD_NLEDS; i++)
      {
        board_autoled_off(i);
      }
  }
#endif

#ifdef CONFIG_WILDFIRE_CHALLENGER_V2_NAND_FLASH
  /* Configure FMC NAND Bank3 BEFORE SDRAM.  On this chip the FMC
   * controller won't honor NAND Bank3 (0x90000000) accesses if the
   * SDRAM controller (SDCR1) is enabled first -- it bus-faults.
   * NAND control pins (PG9/PD11/PD12/PD4/PD5) are independent of the
   * D0-D7 data bus, so configuring them here is safe.  Actual NAND
   * data access (readid) happens later in bringup, after SDRAM has
   * configured D0-D7 as AF12.
   */
  nand_fmc_init();
#endif

#ifdef CONFIG_STM32_FMC
  /* Wait for SDRAM power stabilization before FMC init */

  {
    volatile int delay;
    for (delay = 0; delay < 500000; delay++);
  }

  stm32_sdram_initialize();
#endif

#ifdef CONFIG_ARCH_LEDS
  /* board_autoled_initialize() already called above at the top of this
   * function to turn LEDs off ASAP; no need to call it again here.
   */
#endif

#ifdef CONFIG_ARCH_BUTTONS
  /* Configure on-board buttons if button support has been selected. */

  board_button_initialize();
#endif
}
