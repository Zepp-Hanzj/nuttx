/****************************************************************************
 * boards/arm/stm32f4/wildfire-stm32f429-challenger-v2/src/
 * stm32_ap6181_firmware.c
 *
 * SPDX-License-Identifier: Apache-2.0
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>

/* NuttX already carries a tested BCM43362 firmware image for the Photon.
 * The firmware is chip-specific, while the NVRAM calibration is module and
 * board-specific.  Reuse only that firmware and replace the Photon NVRAM
 * symbols with the AP6181 calibration below.
 */

#define bcm43362_nvram_image     bcm43362_photon_nvram_image
#define bcm43362_nvram_image_len bcm43362_photon_nvram_image_len
#include "../../../stm32f2/photon/src/stm32_wlan_firmware.c"
#undef bcm43362_nvram_image
#undef bcm43362_nvram_image_len

/* AMPAK AP6181 NVRAM V1.1 (26MHz crystal, P304-P307 PA calibration). */

const char
locate_data(".wlan_nvram_image")
aligned_data(CONFIG_IEEE80211_BROADCOM_DMABUF_ALIGNMENT)
bcm43362_nvram_image[] =
  "manfid=0x2d0"                                            "\x00"
  "prodid=0x492"                                            "\x00"
  "vendid=0x14e4"                                           "\x00"
  "devid=0x4343"                                            "\x00"
  "boardtype=0x0598"                                        "\x00"
  "boardrev=0x1307"                                         "\x00"
  "boardnum=777"                                            "\x00"
  "xtalfreq=26000"                                          "\x00"
  "boardflags=0xa00"                                        "\x00"
  "sromrev=3"                                               "\x00"
  "wl0id=0x431b"                                            "\x00"
  "macaddr=02:42:43:36:62:01"                               "\x00"
  "aa2g=1"                                                  "\x00"
  "ag0=2"                                                   "\x00"
  "maxp2ga0=74"                                             "\x00"
  "cck2gpo=0x2222"                                          "\x00"
  "ofdm2gpo=0x66666666"                                     "\x00"
  "mcs2gpo0=0x7777"                                         "\x00"
  "mcs2gpo1=0x7777"                                         "\x00"
  "pa0maxpwr=56"                                            "\x00"
  "pa0b0=5447"                                              "\x00"
  "pa0b1=-607"                                              "\x00"
  "pa0b2=-160"                                              "\x00"
  "pa0itssit=62"                                            "\x00"
  "pa1itssit=62"                                            "\x00"
  "cckPwrOffset=5"                                          "\x00"
  "ccode=0"                                                 "\x00"
  "rssismf2g=0xa"                                           "\x00"
  "rssismc2g=0x3"                                           "\x00"
  "rssisav2g=0x7"                                           "\x00"
  "triso2g=0"                                               "\x00"
  "noise_cal_enable_2g=0"                                   "\x00"
  "noise_cal_po_2g=0"                                       "\x00"
  "swctrlmap_2g=0x04040404,0x02020202,0x02020202,0x010101,0x1ff" "\x00"
  "temp_add=29767"                                           "\x00"
  "temp_mult=425"                                            "\x00"
  "\x00\x00";

const unsigned int bcm43362_nvram_image_len =
  sizeof(bcm43362_nvram_image);
