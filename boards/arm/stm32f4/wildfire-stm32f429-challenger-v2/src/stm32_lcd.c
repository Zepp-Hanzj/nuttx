/****************************************************************************
 * boards/arm/stm32f4/wildfire-stm32f429-challenger-v2/src/stm32_lcd.c
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

#include <stdbool.h>
#include <errno.h>
#include <nuttx/debug.h>

#include <nuttx/arch.h>
#include <nuttx/board.h>
#include <nuttx/video/fb.h>

#include <arch/board/board.h>

#include "arm_internal.h"
#include "stm32_ltdc.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* LCD resolution - typical for 3.5" or 4.3" LCD on Wildfire boards */

#define LCD_WIDTH   480
#define LCD_HEIGHT  320

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_lcdinitialize
 *
 * Description:
 *   Initialize the LCD controller and LTDC interface.
 *
 ****************************************************************************/

int stm32_lcdinitialize(void)
{
  int ret;

  /* Initialize LTDC controller */

  ret = stm32_ltdcinitialize();
  if (ret < 0)
    {
      lcderr("ERROR: stm32_ltdcinitialize failed: %d\n", ret);
      return ret;
    }

  lcdinfo("LCD initialized: %dx%d\n", LCD_WIDTH, LCD_HEIGHT);
  return OK;
}

/****************************************************************************
 * Name: up_fbinitialize
 *
 * Description:
 *   Initialize the framebuffer support for the specified display.
 *
 ****************************************************************************/

int up_fbinitialize(int display)
{
  static bool initialized = false;
  int ret = OK;

  if (!initialized)
    {
      ret = stm32_lcdinitialize();
      if (ret >= OK)
        {
          initialized = true;
        }
    }

  return ret;
}

/****************************************************************************
 * Name: up_fbgetvplane
 *
 * Description:
 *   Return a pointer to the framebuffer object for the specified video
 *   plane of the specified display.
 *
 ****************************************************************************/

struct fb_vtable_s *up_fbgetvplane(int display, int vplane)
{
  return stm32_ltdcgetvplane(vplane);
}

/****************************************************************************
 * Name: up_fbuninitialize
 *
 * Description:
 *   Uninitialize the framebuffer support for the specified display.
 *
 ****************************************************************************/

void up_fbuninitialize(int display)
{
  /* Nothing to do */
}
