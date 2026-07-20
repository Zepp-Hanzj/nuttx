/****************************************************************************
 * boards/arm/stm32f4/wildfire-stm32f429-challenger-v2/src/stm32_buttons.c
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

#include <nuttx/arch.h>
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

/* Button pin list */

static const uint32_t g_buttons[NUM_BUTTONS] =
{
  GPIO_BTN_KEY1, GPIO_BTN_KEY2
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: board_button_initialize
 *
 * Description:
 *   Initialize the button subsystem.
 *
 ****************************************************************************/

uint32_t board_button_initialize(void)
{
  int i;

  /* Configure button GPIOs for input */

  for (i = 0; i < NUM_BUTTONS; i++)
    {
      stm32_configgpio(g_buttons[i]);
    }

  return 0;
}

/****************************************************************************
 * Name: board_buttons
 *
 * Description:
 *   Return the current state of all buttons.
 *
 * Input Parameters:
 *   None
 *
 * Returned Value:
 *   Bit-encoded button state (see BUTTON_ definitions in board.h)
 *
 ****************************************************************************/

uint32_t board_buttons(void)
{
  uint32_t ret = 0;
  int i;

  for (i = 0; i < NUM_BUTTONS; i++)
    {
      if (stm32_gpioread(g_buttons[i]))
        {
          ret |= (1 << i);
        }
    }

  return ret;
}

/****************************************************************************
 * Name: board_button_irq
 *
 * Description:
 *   Configure button interrupts.
 *
 ****************************************************************************/

#ifdef CONFIG_ARCH_IRQBUTTONS
int board_button_irq(int id, xcpt_t handler, void *arg)
{
  int ret = -EINVAL;

  if ((unsigned)id < NUM_BUTTONS)
    {
      ret = stm32_gpiosetevent(g_buttons[id], true, false, false,
                               handler, arg);
    }

  return ret;
}
#endif
