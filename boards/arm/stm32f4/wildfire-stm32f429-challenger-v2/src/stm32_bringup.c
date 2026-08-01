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
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>

#include <nuttx/board.h>
#include <nuttx/fs/smart.h>
#include <nuttx/fs/ioctl.h>

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

#ifdef CONFIG_WILDFIRE_CHALLENGER_V2_NAND_FLASH
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

#ifdef CONFIG_WILDFIRE_CHALLENGER_V2_NAND_FLASH
int stm32_nandflash_initialize(void);
#endif

#ifdef CONFIG_WILDFIRE_CHALLENGER_V2_TOUCHSCREEN
int stm32_touchscreen_initialize(void);
#endif

/* LED test - directly toggle GPIO for debugging */
#include "stm32_gpio.h"
#include <arch/board/board.h>

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_mount_smart
 *
 * Description:
 *   Mount a SMARTFS block device at the given mount point.  If the volume
 *   has not been formatted yet (mount returns ENODEV / EFTYPE / ENOENT),
 *   format it with mksmartfs and retry the mount.  This makes first-boot
 *   fully automatic - no need to run 'mksmartfs' from the NSH prompt.
 *
 * Returned Value:
 *   OK on success (mounted), a negated errno on unrecoverable failure.
 *
 ****************************************************************************/

#ifdef CONFIG_FS_SMARTFS
static int stm32_mount_smart(const char *devpath, const char *mntpt)
{
  struct smart_read_write_s rw;
  struct smart_format_s fmt;
  int fd;
  int ret;

  /* Create the mount point directory tree */

  mkdir(mntpt, 0777);

  /* First attempt: mount an already-formatted volume */

  ret = mount(devpath, mntpt, "smartfs", 0, NULL);
  if (ret == OK)
    {
      syslog(LOG_INFO, "%s: mounted at %s\n", devpath, mntpt);
      return OK;
    }

  /* Mount failed - assume the volume is unformatted and low-level
   * format it ourselves (equivalent to 'mksmartfs <devpath>').
   */

  syslog(LOG_INFO, "%s: not formatted, initializing...\n", devpath);

  fd = open(devpath, O_RDWR);
  if (fd < 0)
    {
      syslog(LOG_ERR, "%s: open failed (%d)\n", devpath, errno);
      return -errno;
    }

  /* sectorsize << 16 == 0 lets the driver pick CONFIG_MTD_SMART_SECTOR_SIZE */

  ret = ioctl(fd, BIOC_LLFORMAT, 0);
  if (ret < 0)
    {
      syslog(LOG_ERR, "%s: BIOC_LLFORMAT failed (%d)\n", devpath, errno);
      close(fd);
      return -errno;
    }

  /* Allocate the root directory sector and mark it as a directory */

  ret = ioctl(fd, BIOC_ALLOCSECT, SMARTFS_ROOT_DIR_SECTOR);
  if (ret != SMARTFS_ROOT_DIR_SECTOR)
    {
      syslog(LOG_ERR, "%s: BIOC_ALLOCSECT failed (%d)\n", devpath, ret);
      close(fd);
      return -EIO;
    }

  rw.logsector = SMARTFS_ROOT_DIR_SECTOR;
  rw.offset    = 0;
  rw.count     = 1;
  rw.buffer    = (const uint8_t *)"\x01";  /* SMARTFS_SECTOR_TYPE_DIR */

  ret = ioctl(fd, BIOC_WRITESECT, (unsigned long)&rw);
  if (ret < 0)
    {
      syslog(LOG_ERR, "%s: BIOC_WRITESECT failed (%d)\n", devpath, errno);
      close(fd);
      return -errno;
    }

  /* Sanity check: confirm the format took */

  ret = ioctl(fd, BIOC_GETFORMAT, (unsigned long)&fmt);
  close(fd);
  if (ret < 0 || !(fmt.flags & SMART_FMT_ISFORMATTED))
    {
      syslog(LOG_ERR, "%s: format check failed\n", devpath);
      return -EIO;
    }

  /* Second attempt: mount the freshly formatted volume */

  ret = mount(devpath, mntpt, "smartfs", 0, NULL);
  if (ret < 0)
    {
      syslog(LOG_ERR, "%s: mount after format failed (%d)\n",
             devpath, errno);
      return -errno;
    }

  syslog(LOG_INFO, "%s: formatted and mounted at %s\n", devpath, mntpt);
  return OK;
}
#endif /* CONFIG_FS_SMARTFS */

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
  /* SDRAM data-integrity self-test — runs BEFORE LTDC init so that if the
   * panel is white we can distinguish "fbmem SDRAM path broken" from "LTDC
   * signal chain broken". fbmem region (CONFIG_STM32_LTDC_FB_BASE,
   * CONFIG_STM32_LTDC_FB_SIZE) was excluded from heap2 (heap2 starts at
   * 0xD0200000), so writing it here does NOT corrupt heap metadata.
   *
   * Writes a known 32-bit LFSR pattern to every fbmem word, reads back, and
   * logs the first mismatch + pass/fail count via syslog so the result shows
   * on the USART6 console boot log.
   */

  {
    volatile uint32_t *fb = (volatile uint32_t *)CONFIG_STM32_LTDC_FB_BASE;
    uint32_t fb_words = CONFIG_STM32_LTDC_FB_SIZE / sizeof(uint32_t);
    uint32_t i, expect, got, mism = 0, first_bad_off = 0xFFFFFFFF;

    /* Write phase: LFSR pattern (tap bits 31,21) so each word is unique but
     * reconstructable, not all-0/all-1 (which a dead bus would fake-pass). */

    expect = 0x55AA55AAu;
    for (i = 0; i < fb_words; i++)
      {
        fb[i] = expect;
        expect = (expect << 1) | (expect >> 31);
        expect ^= (expect >> 10);
      }

    /* Read-back phase */

    expect = 0x55AA55AAu;
    for (i = 0; i < fb_words; i++)
      {
        got = fb[i];
        if (got != expect)
          {
            if (mism == 0)
              {
                first_bad_off = i;
              }
            mism++;
            if (mism >= 8)
              {
                break;  /* log first few only */
              }
          }
        expect = (expect << 1) | (expect >> 31);
        expect ^= (expect >> 10);
      }

    syslog(LOG_INFO, "SDRAM TEST: %lu words, %lu mismatch, first_bad_off=0x%lX\n",
           (unsigned long)fb_words, (unsigned long)mism,
           (unsigned long)first_bad_off);
    syslog(LOG_INFO, "SDRAM TEST: %s\n", (mism == 0) ? "PASS — SDRAM OK" :
           "FAIL — SDRAM path broken (white screen root cause)");
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

  /* BUG FIX: match the Wildfire demo (gui_lcd_port.c GUI_DisplayInit)
   * order exactly — LTDC init FIRST, then fill framebuffer, THEN turn on
   * backlight. Previously backlight was turned on BEFORE LTDC init, so the
   * panel entered its default white-power-on state while LTDC had no valid
   * signal yet, and that white state persisted over the LTDC output → white
   * panel. Backlight must come LAST so the panel's first lit frame is the
   * LTDC framebuffer content, not its internal default.
   */

  stm32_configgpio(GPIO_LCD_BL);   /* configure the BL pin now, but keep it OFF */

  ret = stm32_lcdinitialize();
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: stm32_lcdinitialize failed: %d\n", ret);
    }
  else
    {
      /* Register the framebuffer character device as /dev/fb0
       * so the LVGL NuttX port can open it via info.fb_path.
       */

      ret = fb_register(0, 0);
      if (ret < 0)
        {
          syslog(LOG_ERR, "ERROR: fb_register failed: %d\n", ret);
        }
    }

  /* Turn on the LCD backlight AFTER LTDC init + framebuffer fill, matching
   * the demo's LCD_BkLight(TRUE) at the end of GUI_DisplayInit.
   */

  stm32_gpiowrite(GPIO_LCD_BL, true);
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

#ifdef CONFIG_WILDFIRE_CHALLENGER_V2_NAND_FLASH
  /* Initialize NAND Flash */

  ret = stm32_nandflash_initialize();
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: stm32_nandflash_initialize failed: %d\n", ret);
    }
#endif

#ifdef CONFIG_WILDFIRE_CHALLENGER_V2_TOUCHSCREEN
  /* Initialize the Goodix GT9xx capacitive touch panel on I2C2.
   * Registers /dev/input0 via the arch-level gt9xx driver.
   */

  ret = stm32_touchscreen_initialize();
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: stm32_touchscreen_initialize failed: %d\n",
             ret);
    }
#endif

#ifdef CONFIG_FS_SMARTFS
  /* Auto-mount the SMARTFS volumes created by the NAND driver.
   * This is board-level startup logic, kept here (user/board layer)
   * rather than buried inside the NAND driver.
   *
   *   /dev/smart0 -> /mnt/config
   *   /dev/smart1 -> /mnt/data
   *
   * If a volume has not been formatted yet, it is formatted automatically
   * on first boot - no need to run 'mksmartfs' from the NSH prompt.
   */

  mkdir("/mnt", 0777);

  stm32_mount_smart("/dev/smart0", "/mnt/config");
  stm32_mount_smart("/dev/smart1", "/mnt/data");
#endif

  UNUSED(ret);
  return OK;
}
