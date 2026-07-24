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
  /* SDRAM is initialized in stm32_boardinitialize() before the heap is set
   * up.  Do NOT write test patterns here - 0xD0100000 is inside the heap2
   * region (0xD0000000..0xD2000000) and would corrupt heap metadata.
   */
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

#ifdef CONFIG_WILDFIRE_CHALLENGER_V2_NAND_FLASH
  /* Initialize NAND Flash */

  ret = stm32_nandflash_initialize();
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: stm32_nandflash_initialize failed: %d\n", ret);
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
