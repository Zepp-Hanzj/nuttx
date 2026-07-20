/****************************************************************************
 * boards/arm/stm32f4/wildfire-stm32f429-challenger-v2/src/stm32_autoleds.c
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

#include <nuttx/board.h>
#include <arch/board/board.h>

#include "chip.h"
#include "arm_internal.h"
#include "stm32_gpio.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* LED pin list */

static const uint32_t g_ledlist[BOARD_NLEDS] =
{
  GPIO_LED1, GPIO_LED2, GPIO_LED3, GPIO_LED4
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: board_autoled_initialize
 *
 * Description:
 *   Initialize the LED subsystem.
 *
 ****************************************************************************/

void board_autoled_initialize(void)
{
  int i;

  /* Configure LED GPIOs for output */

  for (i = 0; i < BOARD_NLEDS; i++)
    {
      stm32_configgpio(g_ledlist[i]);
    }
}

/****************************************************************************
 * Name: board_autoled_on
 *
 * Description:
 *   Turn on the LED specified by the LED argument.
 *
 * Input Parameters:
 *   led - LED number (see BOARD_LED_ definitions in board.h)
 *
 ****************************************************************************/

void board_autoled_on(int led)
{
  if ((unsigned)led < BOARD_NLEDS)
    {
      /* Active low: setting the pin LOW turns the LED on */

      stm32_gpiowrite(g_ledlist[led], false);
    }
}

/****************************************************************************
 * Name: board_autoled_off
 *
 * Description:
 *   Turn off the LED specified by the LED argument.
 *
 * Input Parameters:
 *   led - LED number (see BOARD_LED_ definitions in board.h)
 *
 ****************************************************************************/

void board_autoled_off(int led)
{
  if ((unsigned)led < BOARD_NLEDS)
    {
      /* Active low: setting the pin HIGH turns the LED off */

      stm32_gpiowrite(g_ledlist[led], true);
    }
}
