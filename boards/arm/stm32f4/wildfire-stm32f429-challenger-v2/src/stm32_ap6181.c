/****************************************************************************
 * boards/arm/stm32f4/wildfire-stm32f429-challenger-v2/src/stm32_ap6181.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <syslog.h>

#include <nuttx/arch.h>
#include <nuttx/sdio.h>
#include <nuttx/wireless/ieee80211/bcmf_board.h>
#include <nuttx/wireless/ieee80211/bcmf_sdio.h>

#include <arch/board/board.h>

#include "stm32_gpio.h"
#include "stm32_sdio.h"
#include "hardware/stm32f40xxx_memorymap.h"

static FAR struct sdio_dev_s *g_ap6181_sdio;

void bcmf_board_initialize(int minor)
{
  if (minor == BOARD_AP6181_MINOR)
    {
      stm32_configgpio(GPIO_AP6181_REG_ON);
      stm32_gpiowrite(GPIO_AP6181_REG_ON, false);
      stm32_configgpio(GPIO_AP6181_HOST_WAKE);
    }
}

void bcmf_board_power(int minor, bool power)
{
  if (minor == BOARD_AP6181_MINOR)
    {
      stm32_gpiowrite(GPIO_AP6181_REG_ON, power);
    }
}

void bcmf_board_reset(int minor, bool reset)
{
  /* AP6181 exposes only WL_REG_ON.  Pulling it low resets the BCM43362 and
   * disables its internal regulators; releasing it starts the device.
   */

  if (minor == BOARD_AP6181_MINOR)
    {
      stm32_gpiowrite(GPIO_AP6181_REG_ON, !reset);
    }
}

void bcmf_board_setup_oob_irq(int minor, CODE int (*func)(FAR void *),
                              FAR void *arg)
{
  if (minor != BOARD_AP6181_MINOR)
    {
      return;
    }

  /* This callback is the BCM43362 SDIO function interrupt, not the
   * WL_HOST_WAKE signal.  Route it through SDIO DAT1 (PC9), as done by the
   * upstream Photon BCM43362 board support.  WL_HOST_WAKE on PA0 is only a
   * host power-management wake signal and does not report every control or
   * data response.
   */

  if (g_ap6181_sdio != NULL)
    {
      sdio_set_sdio_card_isr(g_ap6181_sdio, func, arg);
    }
}

bool bcmf_board_etheraddr(FAR struct ether_addr *ethaddr)
{
  FAR const uint32_t *uid = (FAR const uint32_t *)STM32_SYSMEM_UID;
  uint32_t hash = uid[0] ^ uid[1] ^ uid[2];

  /* Derive a stable locally-administered unicast address from the STM32's
   * 96-bit unique ID.  This replaces the placeholder MAC in the NVRAM image.
   */

  ethaddr->ether_addr_octet[0] = 0x02;
  ethaddr->ether_addr_octet[1] = 0x42;
  ethaddr->ether_addr_octet[2] = (uid[0] >> 8) & 0xff;
  ethaddr->ether_addr_octet[3] = hash & 0xff;
  ethaddr->ether_addr_octet[4] = (hash >> 8) & 0xff;
  ethaddr->ether_addr_octet[5] = (hash >> 16) & 0xff;
  return true;
}

int stm32_ap6181_initialize(void)
{
  FAR struct sdio_dev_s *sdio;
  int ret;

  syslog(LOG_INFO, "AP6181: initialize BCM43362 on SDIO%d\n",
         BOARD_AP6181_SDIO_SLOT);

  sdio = sdio_initialize(BOARD_AP6181_SDIO_SLOT);
  if (sdio == NULL)
    {
      syslog(LOG_ERR, "AP6181: sdio_initialize failed\n");
      return -ENODEV;
    }

  g_ap6181_sdio = sdio;

  ret = bcmf_sdio_initialize(BOARD_AP6181_MINOR, sdio);
  if (ret < 0)
    {
      syslog(LOG_ERR, "AP6181: bcmf_sdio_initialize failed: %d\n", ret);
      return ret;
    }

  syslog(LOG_INFO, "AP6181: registered as wlan%d\n", BOARD_AP6181_MINOR);
  return OK;
}
