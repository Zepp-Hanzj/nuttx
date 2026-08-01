/****************************************************************************
 * boards/arm/stm32f4/wildfire-stm32f429-challenger-v2/src/stm32_touchscreen.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to you under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ****************************************************************************/

/* Board-level glue for the Goodix GT9xx capacitive touch panel.
 *
 * The arch-level driver (drivers/input/gt9xx.c) implements the I2C register
 * protocol, coordinate read, and the standard /dev/input touchscreen char
 * driver. This file only supplies the three board callbacks it needs:
 *   - irq_attach : route the PD13 rising-edge EXTI into the gt9xx ISR
 *   - irq_enable : arm/disarm that EXTI
 *   - set_power  : hold the GT9xx in reset (RST=PI8 low) or release it
 *
 * Pins (per Wildfire BSP bsp_i2c_touch.h, confirmed against schematic):
 *   SCL = PH4 (AF4), SDA = PH5 (AF4), I2C2
 *   RST = PI8 (GPIO output, active-low reset)
 *   INT = PD13 (GPIO input, rising-edge IRQ)
 *
 * The Wildfire BSP drove I2C2 in software-bit-bang mode because its hardware
 * I2C implementation was unreliable on this board.  NuttX's STM32 I2C driver
 * includes F4 errata workarounds, so this port uses hardware I2C2.
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <assert.h>
#include <nuttx/debug.h>
#include <errno.h>
#include <semaphore.h>
#include <nuttx/irq.h>
#include <nuttx/arch.h>
#include <syslog.h>
#include <nuttx/i2c/i2c_master.h>

#include <nuttx/input/gt9xx.h>

#include "stm32_gpio.h"
#include "stm32_i2c.h"

#include <arch/board/board.h>

#ifdef CONFIG_INPUT_GT9XX

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef CONFIG_STM32_I2C2
#  error "GT9xx touch support requires CONFIG_STM32_I2C2"
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* Saved ISR and argument let irq_enable() arm or disarm the EXTI.  The GT9xx
 * driver calls irq_attach() once, then calls irq_enable() as needed.
 */

struct stm32_gt9xx_priv_s
{
  xcpt_t isr;
  void  *arg;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct stm32_gt9xx_priv_s g_gt9xx_priv;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_gt9xx_irq_attach
 *
 * Description:
 *   Save the gt9xx ISR + arg. The EXTI is actually armed later by
 *   stm32_gt9xx_irq_enable(true). Passing isr=NULL detaches.
 *
 ****************************************************************************/

static int stm32_gt9xx_irq_attach(const struct gt9xx_board_s *state,
                                   xcpt_t isr, void *arg)
{
  syslog(LOG_INFO, "Saving isr=%p arg=%p\n", isr, arg);

  if (isr)
    {
      g_gt9xx_priv.isr = isr;
      g_gt9xx_priv.arg = arg;
    }
  else
    {
      /* Detach: disable EXTI first, then clear saved handler */

      stm32_gpiosetevent(GPIO_GT9XX_INT, false, false, false, NULL, NULL);
      g_gt9xx_priv.isr = NULL;
      g_gt9xx_priv.arg = NULL;
    }

  return OK;
}

/****************************************************************************
 * Name: stm32_gt9xx_irq_enable
 *
 * Description:
 *   Arm or disarm the PD13 rising-edge EXTI. Reconfiguration is a multi-
 *   step GPIO setup, so keep IRQs disabled while reconfiguring it.
 *
 ****************************************************************************/

static void stm32_gt9xx_irq_enable(const struct gt9xx_board_s *state,
                                    bool enable)
{
  irqstate_t flags;

  flags = enter_critical_section();

  if (enable)
    {
      /* Rising-edge trigger, event on the saved ISR. stm32_gpiosetevent
       * configures the EXTI line + NVIC route in one call.
       */

      stm32_gpiosetevent(GPIO_GT9XX_INT, true, false, true,
                         g_gt9xx_priv.isr, g_gt9xx_priv.arg);
    }
  else
    {
      stm32_gpiosetevent(GPIO_GT9XX_INT, false, false, false, NULL, NULL);
    }

  leave_critical_section(flags);
}

/****************************************************************************
 * Name: stm32_gt9xx_set_power
 *
 * Description:
 *   Power the GT9xx on/off by controlling the RST pin (PI8).
 *
 *   GT9xx reset is active-low on the RST line itself but the Wildfire BSP
 *   sequence is: RST low > delay > RST high > delay. "on" = release reset
 *   (RST high), "off" = hold reset (RST low). We also drive INT low during
 *   the reset pulse so the GT9xx boots into its 0x5D/0xBA address variant
 *   exactly like the BSP, then release INT back to input.
 *
 *   Note: set_power(on=true) is called by the arch gt9xx driver BEFORE it
 *   probes the I2C device, so the RST/INT boot sequence MUST run here, not
 *   in a separate init. The arch driver then waits 100ms (nxsched_usleep)
 *   before probing — longer than the BSP's ~10ms settle, so no extra delay
 *   needed here.
 *
 ****************************************************************************/

static int stm32_gt9xx_set_power(const struct gt9xx_board_s *state,
                                  bool on)
{
  if (on)
    {
      /* Boot sequence (mirrors BSP I2C_ResetChip):
       *   1. INT becomes output low (selects the 0x5D I2C address)
       *   2. RST goes low
       *   3. wait
       *   4. RST goes high
       *   5. wait
       *   6. INT becomes input so rising-edge EXTI can fire later
       */

      /* Configure PI8 (RST) as output, default low */

      stm32_configgpio(GPIO_GT9XX_RST);
      stm32_gpiowrite(GPIO_GT9XX_RST, false);

      /* Drive INT low during reset pulse */

      stm32_configgpio(GPIO_GT9XX_INT_OUT);
      stm32_gpiowrite(GPIO_GT9XX_INT_OUT, false);

      up_mdelay(10);

      /* Release reset with RST high */

      stm32_gpiowrite(GPIO_GT9XX_RST, true);
      up_mdelay(10);

      /* Release INT back to input (EXTI will be armed by irq_enable) */

      stm32_configgpio(GPIO_GT9XX_INT);
    }
  else
    {
      /* Hold reset low to power the GT9xx down */

      stm32_gpiowrite(GPIO_GT9XX_RST, false);
    }

  return OK;
}

/****************************************************************************
 * Private Data: board callback table
 ****************************************************************************/

static const struct gt9xx_board_s g_gt9xx_board =
{
  .irq_attach = stm32_gt9xx_irq_attach,
  .irq_enable = stm32_gt9xx_irq_enable,
  .set_power  = stm32_gt9xx_set_power,
};

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_touchscreen_initialize
 *
 * Description:
 *   Initialize the GT9xx capacitive touch panel. Configures the I2C2 GPIO
 *   pins, grabs the I2C2 bus from the stm32 I2C driver, and registers the
 *   arch-level gt9xx driver at /dev/input0.
 *
 ****************************************************************************/

int stm32_touchscreen_initialize(void)
{
  FAR struct i2c_master_s *i2c;
  int ret;

  syslog(LOG_INFO, "Initializing GT9xx touch panel\n");

  /* Configure I2C2 GPIO pins (SCL=PH4, SDA=PH5, AF4).
   * stm32_i2cbus_initialize does this internally via its own pin config,
   * but configuring here too is harmless and makes the pin intent explicit
   * in case the I2C driver's pin list ever diverges from board.h.
   */

  stm32_configgpio(GPIO_GT9XX_SCL);
  stm32_configgpio(GPIO_GT9XX_SDA);

  /* Grab the I2C2 master bus. CONFIG_STM32_I2C2 must be enabled in defconfig
   * so the arch driver is compiled and its bus-2 initializer returns a valid
   * i2c_master_s.  Nothing else on this board uses I2C2.
   */

  i2c = stm32_i2cbus_initialize(BOARD_GT9XX_I2C_PORT);
  if (i2c == NULL)
    {
      syslog(LOG_ERR, "ERROR: Failed to initialize I2C%d\n",
             BOARD_GT9XX_I2C_PORT);
      return -ENODEV;
    }

  /* Register the GT9xx driver.  It handles probe, IRQ processing,
   * coordinate reads, and the /dev/inputN character driver.
   */

  ret = gt9xx_register("/dev/input0", i2c, BOARD_GT9XX_I2C_ADDR,
                       &g_gt9xx_board);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: gt9xx_register failed: %d\n", ret);
      stm32_i2cbus_uninitialize(i2c);
      return ret;
    }

  syslog(LOG_INFO, "GT9xx registered at /dev/input0 (I2C%d addr 0x%02X)\n",
         BOARD_GT9XX_I2C_PORT, BOARD_GT9XX_I2C_ADDR);
  return OK;
}

#endif /* CONFIG_INPUT_GT9XX */
