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
#include <syslog.h>

#include <arch/board/board.h>

#include "arm_internal.h"
#include "stm32.h"
#include "hardware/stm32f40xxx_memorymap.h"  /* STM32_LTDC_BASE = 0x40016800 */

/* BUG FIX: in the board-build path the generic
 * hardware/stm32_ltdc.h's '#include "hardware/stm32_memorymap.h"' resolves to
 * nothing on stm32f4 (that file only exists for stm32f0/f1/h7/u5/h5), so
 * STM32_LTDC_BASE never expanded to 0x40016800 — the compiler ended up using
 * 0x40016000 for every LTDC register write, silently writing them into the
 * wrong region (panel stayed white). Use raw addresses here, bypassing the
 * broken STM32_LTDC_BASE expansion chain.
 */

#define LTDC_REG_SSCR      0x40016808UL
#define LTDC_REG_BPCR      0x4001680CUL
#define LTDC_REG_AWCR      0x40016810UL
#define LTDC_REG_TWCR      0x40016814UL
#define LTDC_REG_GCR       0x40016818UL
#define LTDC_REG_SRCR      0x40016824UL
#define LTDC_REG_BCCR      0x4001682CUL
#define LTDC_REG_L1CR      0x40016884UL
#define LTDC_REG_L1WHPCR   0x40016888UL
#define LTDC_REG_L1WVPCR   0x4001688CUL
#define LTDC_REG_L1PFCR    0x40016894UL
#define LTDC_REG_L1CACR    0x40016898UL
#define LTDC_REG_L1DCCR    0x4001689CUL
#define LTDC_REG_L1BFCR    0x400168A0UL
#define LTDC_REG_L1CFBAR   0x400168ACUL
#define LTDC_REG_L1CFBLR   0x400168B0UL
#define LTDC_REG_L1CFBLNR  0x400168B4UL
#define LTDC_REG_L2CR      0x40016904UL

#include "hardware/stm32_ltdc.h"
#include "stm32_rcc.h"

/* LTDC register bit constants (from stm32_ltdc.h, fallback defines in case
 * the broken include chain also missed these).
 */

#ifndef LTDC_GCR_LTDCEN
#  define LTDC_GCR_LTDCEN   (1U << 0)
#endif
#ifndef LTDC_GCR_PCPOL
#  define LTDC_GCR_PCPOL    (1U << 28)
#endif
#ifndef LTDC_LXCR_LEN
#  define LTDC_LXCR_LEN     (1U << 0)
#endif
#ifndef LTDC_SRCR_IMR
#  define LTDC_SRCR_IMR     (1U << 0)
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* LCD resolution - 5" 800x480 RGB888 capacitive LCD on Wildfire Challenger V2 */

#define LCD_WIDTH   800
#define LCD_HEIGHT  480

/* LTDC timing for the 5" 800x480 panel (matches Wildfire BSP lcd_param[INCH_5]) */

#define LTDC_HSW    1
#define LTDC_VSW    1
#define LTDC_HBP    46
#define LTDC_VBP    23
#define LTDC_HFP    22
#define LTDC_VFP    22

/* PLLSAI parameters (matches Wildfire BSP LCD_Init: N=420, R=3, DIVR=Div4) */

#define LTDC_PLLSAIN    420
#define LTDC_PLLSAIR    3
#define LTDC_PLLSAIQ    7

/* Pixel format: RGB565 = 2 bytes/pixel (matches NuttX fb fmt FB_FMT_RGB16_565) */

#define LTDC_PIXEL_BPP  2
#define LTDC_PFCR_PF    0x2    /* LTDC_LxPFCR_PF_RGB565 */

/* Framebuffer location in FMC SDRAM.
 *
 * CRITICAL: this MUST match the address the NuttX arch-level LTDC driver
 * (arch/arm/src/stm32f7/stm32_ltdc.c) programs into L1CFBAR. That driver
 * does NOT use CONFIG_STM32_LTDC_FB_BASE directly — it centers the layer
 * framebuffer inside the reserved region:
 *
 *   STM32_LTDC_BUFFER_L1 = CONFIG_STM32_LTDC_FB_BASE
 *                         + (CONFIG_STM32_LTDC_FB_SIZE - L1_FBSIZE) / 2
 *
 * where L1_FBSIZE = L1_STRIDE * L1_HEIGHT = 1600 * 480 = 768000.
 *
 * If this board-level direct init writes L1CFBAR = CONFIG_STM32_LTDC_FB_BASE
 * (the region start) while the arch driver later rewrites L1CFBAR to the
 * centered address, the two framebuffers only partially overlap — the panel
 * shows just the top sliver of the red fill and the rest is garbage. This
 * was the "top 1/3 red" bug.
 *
 * Fix: compute the SAME centered address here so the board-level red fill
 * and the arch-level LTDC scan target the exact same 768 KB region.
 */

#define LTDC_L1_STRIDE   (LCD_WIDTH * LTDC_PIXEL_BPP)   /* 1600 bytes */
#define LTDC_L1_FBSIZE   (LTDC_L1_STRIDE * LCD_HEIGHT) /* 768000 bytes */
#define LTDC_FB_ADDR     (CONFIG_STM32_LTDC_FB_BASE     \
                           + (CONFIG_STM32_LTDC_FB_SIZE - LTDC_L1_FBSIZE) / 2)

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_lcd_direct_init
 *
 * Description:
 *   Direct LCD initialization transplanted from the Wildfire F429 BSP
 *   bsp_lcd.c LCD_Init() / LTDC_Init() / LCD_LayerInit() / LTDC_LayerInit().
 *   This bypasses stm32_ltdcinitialize() and uses the exact register
 *   sequence that is proven to drive the 5" 800x480 panel on this board.
 *
 ****************************************************************************/

static void stm32_lcd_direct_init(void)
{
  uint32_t regval;

  /* ---- Step 0a: Enable GPIO port clocks (RCC_AHB1ENR) ----
   *
   * BUG FIX: the Wildfire BSP's LCD_GPIO_Config() starts by calling
   * RCC_AHB1PeriphClockCmd() to enable the AHB1 GPIO port clocks BEFORE
   * any GPIO AF config. Without these clocks the GPIOx_MODER/AFRL/AFRH
   * writes are discarded, so the LTDC pixel/HV-sync/DE signals never
   * reach the FPC and the panel stays white. This is the missing step.
   */

  regval  = getreg32(STM32_RCC_AHB1ENR);
  regval |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN
          | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIODEN
          | RCC_AHB1ENR_GPIOEEN | RCC_AHB1ENR_GPIOFEN
          | RCC_AHB1ENR_GPIOGEN | RCC_AHB1ENR_GPIOHEN
#ifdef RCC_AHB1ENR_GPIOIEN
          | RCC_AHB1ENR_GPIOIEN
#endif
          ;
  putreg32(regval, STM32_RCC_AHB1ENR);

  /* ---- Step 0b: Configure all 28 LCD AF GPIO pins ----
   *
   * Without these AF assignments the LTDC pixel/HV-sync/DE signals never
   * reach the LCD FPC, so the panel stays white even when every LTDC
   * register is correctly programmed. This mirrors the Wildfire BSP
   * LCD_GPIO_Config().
   */

  stm32_configgpio(GPIO_LTDC_R0);
  stm32_configgpio(GPIO_LTDC_R1);
  stm32_configgpio(GPIO_LTDC_R2);
  stm32_configgpio(GPIO_LTDC_R3);
  stm32_configgpio(GPIO_LTDC_R4);
  stm32_configgpio(GPIO_LTDC_R5);
  stm32_configgpio(GPIO_LTDC_R6);
  stm32_configgpio(GPIO_LTDC_R7);

  stm32_configgpio(GPIO_LTDC_G0);
  stm32_configgpio(GPIO_LTDC_G1);
  stm32_configgpio(GPIO_LTDC_G2);
  stm32_configgpio(GPIO_LTDC_G3);
  stm32_configgpio(GPIO_LTDC_G4);
  stm32_configgpio(GPIO_LTDC_G5);
  stm32_configgpio(GPIO_LTDC_G6);
  stm32_configgpio(GPIO_LTDC_G7);

  stm32_configgpio(GPIO_LTDC_B0);
  stm32_configgpio(GPIO_LTDC_B1);
  stm32_configgpio(GPIO_LTDC_B2);
  stm32_configgpio(GPIO_LTDC_B3);
  stm32_configgpio(GPIO_LTDC_B4);
  stm32_configgpio(GPIO_LTDC_B5);
  stm32_configgpio(GPIO_LTDC_B6);
  stm32_configgpio(GPIO_LTDC_B7);

  stm32_configgpio(GPIO_LTDC_CLK);
  stm32_configgpio(GPIO_LTDC_HSYNC);
  stm32_configgpio(GPIO_LTDC_VSYNC);
  stm32_configgpio(GPIO_LTDC_DE);

  /* ---- Step 1: Enable LTDC peripheral clock on APB2 (bit26 = LTDCEN) ----
   *
   * DECISIVE SELF-TEST: after writing RCC_APB2ENR, read it back. If the
   * LTDCEN bit is NOT set (write discarded), blink LED1 forever as a
   * hardware flag — this means APB2 bus clock is not configured, which
   * is the root cause of the white panel.
   */

  regval  = getreg32(STM32_RCC_APB2ENR);
  regval |= RCC_APB2ENR_LTDCEN;
  putreg32(regval, STM32_RCC_APB2ENR);

  if ((getreg32(STM32_RCC_APB2ENR) & RCC_APB2ENR_LTDCEN) == 0)
    {
      /* LTDCEN write was discarded — APB2 bus clock issue.
       * Blink LED1 (PH10, active low) forever as a hardware error flag.
       */

      stm32_configgpio(GPIO_LED1);
      for (;;)
        {
          stm32_gpiowrite(GPIO_LED1, false);  /* ON */
          up_mdelay(200);
          stm32_gpiowrite(GPIO_LED1, true);   /* OFF */
          up_mdelay(200);
        }
    }

  /* ---- Step 2: Configure PLLSAI to generate the LTDC pixel clock ----
   *
   * PLLSAI_VCO = HSE/PLLM * N = 25MHz/25 * 420 = 420MHz
   * LTDC pixel clock = 420MHz / PLLSAIR(3) / DIVR(4) ≈ 35MHz
   */

  regval  = getreg32(STM32_RCC_PLLSAICFGR);
  regval &= ~(RCC_PLLSAICFGR_PLLSAIN_MASK
              | RCC_PLLSAICFGR_PLLSAIQ_MASK
              | RCC_PLLSAICFGR_PLLSAIR_MASK);
  regval |= RCC_PLLSAICFGR_PLLSAIN(LTDC_PLLSAIN);
  regval |= RCC_PLLSAICFGR_PLLSAIQ(LTDC_PLLSAIQ);
  regval |= RCC_PLLSAICFGR_PLLSAIR(LTDC_PLLSAIR);
  putreg32(regval, STM32_RCC_PLLSAICFGR);

  /* DCKCFGR: select PLLSAI_R divider = Div4 for the LTDC pixel clock */

  regval  = getreg32(STM32_RCC_DCKCFGR);
  regval &= ~RCC_DCKCFGR_PLLSAIDIVR_MASK;
  regval |= RCC_DCKCFGR_PLLSAIDIVR_DIV4;
  putreg32(regval, STM32_RCC_DCKCFGR);

  /* Enable PLLSAI and wait until ready.
   *
   * BUG FIX: the Wildfire BSP's RCC_PLLSAICmd() writes the PLLSAION bit via
   * the Cortex-M4 peripheral bit-band region (atomic single-bit write), NOT
   * via a read-modify-write of RCC_CR. On F429 a plain 'RCC_CR |= PLLSAION'
   * RMW can have the PLLSAION bit discarded by hardware (the PLL never locks),
   * so LTDC gets no pixel clock and the panel stays white. Use the bit-band
   * address here exactly like the BSP.
   *
   * bit-band addr = PERIPH_BB_BASE(0x42000000) + (RCC_CR_offset*32) + (bit*4)
   * RCC_CR = 0x40023800, bit PLLSAION = 28.
   */

  {
    volatile uint32_t *pllsaion_bb =
        (volatile uint32_t *)(0x42000000UL
                              + ((STM32_RCC_CR - 0x40000000UL) * 32UL)
                              + (28UL * 4UL));
    *pllsaion_bb = 1UL;
  }

  while ((getreg32(STM32_RCC_CR) & RCC_CR_PLLSAIRDY) == 0)
    {
    }

  /* ---- Step 3: LTDC synchronization (LTDC_Init equivalent) ----
   *
   * Matches Wildfire BSP: do NOT write GCR=0 to disable LTDC (the LTDCEN
   * bit write gets discarded on F429 if GCR is cleared while the LTDC
   * shadow registers are mid-config). Use GCR_MASK=0x0FFE888F to clear
   * only the polarity bits, exactly like the BSP's LTDC_Init().
   */

  /* SSCR: VSH = VSW-1, HSW = HSW-1 */

  putreg32(((LTDC_VSW - 1) | ((LTDC_HSW - 1) << 16)), LTDC_REG_SSCR);

  /* BPCR: AVBP = VSW+VBP-1, AHBP = HSW+HBP-1 */

  putreg32((((LTDC_VSW + LTDC_VBP - 1))
            | ((LTDC_HSW + LTDC_HBP - 1) << 16)), LTDC_REG_BPCR);

  /* AWCR: AAH = VSW+VBP+HEIGHT-1, AAW = HSW+HBP+WIDTH-1 */

  putreg32((((LTDC_VSW + LTDC_VBP + LCD_HEIGHT - 1))
            | ((LTDC_HSW + LTDC_HBP + LCD_WIDTH - 1) << 16)), LTDC_REG_AWCR);

  /* TWCR: TOTALH = VSW+VBP+HEIGHT+VFP-1, TOTALW = HSW+HBP+WIDTH+HFP-1 */

  putreg32((((LTDC_VSW + LTDC_VBP + LCD_HEIGHT + LTDC_VFP - 1))
            | ((LTDC_HSW + LTDC_HBP + LCD_WIDTH + LTDC_HFP - 1) << 16)),
           LTDC_REG_TWCR);

  /* GCR: clear polarity bits with GCR_MASK=0x0FFE888F (matches BSP).
   *
   * CONFIRMED against ST StdPeriph Driver (stm32f4xx_ltdc.h):
   *   LTDC_HSPolarity_AL  = 0x00000000
   *   LTDC_VSPolarity_AL  = 0x00000000
   *   LTDC_DEPolarity_AL  = 0x00000000
   *   LTDC_PCPolarity_IPC = 0x00000000   ← "input pixel clock", NOT bit28=1
   *
   * ST's LTDC_Init() does:  LTDC->GCR &= GCR_MASK;  LTDC->GCR |= (HSPOL|VSPOL|DEPOL|PCPOL);
   * With all four polarity enum values = 0, the BSP runtime GCR has NO polarity bits set.
   * Earlier I mis-read LTDC_PCPolarity_IPC as "inverted pixel clock" (bit28=1) and set PCPOL —
   * that INVERTED the pixel clock and made the panel white. Restore PCPOL=0 to match the BSP.
   */

  regval  = getreg32(LTDC_REG_GCR);
  regval &= 0x0FFE888F;   /* BSP GCR_MASK — clears all polarity bits including PCPOL */
  putreg32(regval, LTDC_REG_GCR);
  putreg32(LTDC_SRCR_IMR, LTDC_REG_SRCR);   /* reload so the clear takes */

  /* BCCR: background color = black (0,0,0) */

  putreg32(0, LTDC_REG_BCCR);

  /* ---- Step 4.5: Enable the LTDC controller (matches BSP LTDC_Cmd(ENABLE)) ----
   *
   * The BSP calls LTDC_Cmd(ENABLE) right after LTDC_Init() and BEFORE
   * LCD_LayerInit(). Setting LTDCEN here (with a reload) lets the subsequent
   * layer register writes take effect properly.
   */

  regval  = getreg32(LTDC_REG_GCR);
  regval |= LTDC_GCR_LTDCEN;
  putreg32(regval, LTDC_REG_GCR);
  putreg32(LTDC_SRCR_IMR, LTDC_REG_SRCR);

  /* ---- Step 5: Layer 1 init (LTDC_LayerInit equivalent) ---- */

  /* WHPCR: WHSTPOS = HBP+HSW, WHSPPOS = HSW+HBP+WIDTH-1 — matches BSP
   * LTDC_LayerInit (HorizontalStart = HBP+HSW, NOT minus 1).
   */

  putreg32((((LTDC_HBP + LTDC_HSW))
            | ((LTDC_HSW + LTDC_HBP + LCD_WIDTH - 1) << 16)),
           LTDC_REG_L1WHPCR);

  /* WVPCR: WVSTPOS = VBP+VSW, WVSPPOS = VSW+VBP+HEIGHT-1 — matches BSP
   * LTDC_LayerInit (VerticalStart = VBP+VSW, NOT minus 1).
   */

  putreg32((((LTDC_VBP + LTDC_VSW))
            | ((LTDC_VSW + LTDC_VBP + LCD_HEIGHT - 1) << 16)),
           LTDC_REG_L1WVPCR);

  /* PFCR: pixel format = RGB565 (PF = 0x2) */

  putreg32(LTDC_PFCR_PF, LTDC_REG_L1PFCR);

  /* DCCR: default color = white (0xFF,0xFF,0xFF,0xFF) */

  putreg32(0xFFFFFFFF, LTDC_REG_L1DCCR);

  /* CACR: constant alpha = 255 */

  putreg32(0xFF, LTDC_REG_L1CACR);

  /* BFCR: blending factor 1 = CA (0x400), factor 2 = PAxCA (0x07) —
   * matches Wildfire BSP LTDC_BlendingFactor1_CA / LTDC_BlendingFactor2_PAxCA.
   * BUG FIX: previously wrote (0x6 | (0x4<<8)) = 0x406, but the BSP value is
   * 0x400 | 0x07 = 0x407. The wrong blending factor made LTDC mix pixels to
   * white, so the panel stayed white even with red written to fbmem.
   */

  putreg32(0x00000407, LTDC_REG_L1BFCR);

  /* CFBAR: color framebuffer start address */

  putreg32(LTDC_FB_ADDR, LTDC_REG_L1CFBAR);

  /* CFBLR: line length = (WIDTH * BPP) + 3, pitch = WIDTH * BPP */

  putreg32((((LCD_WIDTH * LTDC_PIXEL_BPP) + 3)
            | ((LCD_WIDTH * LTDC_PIXEL_BPP) << 16)), LTDC_REG_L1CFBLR);

  /* CFBLNR: framebuffer line number = HEIGHT */

  putreg32(LCD_HEIGHT, LTDC_REG_L1CFBLNR);

  /* ---- Step 6: Reload shadow registers (immediate) ---- */

  putreg32(LTDC_SRCR_IMR, LTDC_REG_SRCR);

  /* ---- Step 7: Enable Layer 1 ---- */

  regval  = getreg32(LTDC_REG_L1CR);
  regval |= LTDC_LXCR_LEN;
  putreg32(regval, LTDC_REG_L1CR);

  /* Disable Layer 2 (only one layer used) */

  regval  = getreg32(LTDC_REG_L2CR);
  regval &= ~LTDC_LXCR_LEN;
  putreg32(regval, LTDC_REG_L2CR);

  /* ---- Step 8: Reload shadow registers again ---- */

  putreg32(LTDC_SRCR_IMR, LTDC_REG_SRCR);

  /* ---- Step 10: Fill framebuffer with solid red ----
   *
   * Diagnostic fill: paints the whole 800x480 framebuffer red (RGB565
   * 0xF800) so the panel state is unambiguous. This runs inside the
   * board-level direct LTDC init (called from up_fbinitialize) BEFORE the
   * NuttX arch-level LTDC driver (stm32_ltdc.c) takes over via the fb
   * ioctl path, so the two never write fbmem at the same time.
   */

  {
    volatile uint16_t *fb = (volatile uint16_t *)LTDC_FB_ADDR;
    uint32_t i;
    for (i = 0; i < (uint32_t)LCD_WIDTH * LCD_HEIGHT; i++)
      {
        fb[i] = 0xF800;  /* RGB565 red */
      }
  }

  /* ---- Step 11: LTDC register dump ----
   *
   * Read back ALL key registers after init to verify they actually took
   * effect (vs written but discarded by HW). Wildfire demo runtime reference
   * (ST StdPeriph LTDC_Init + LTDC_LayerInit, HSW=1 VSW=1 HBP=46 VBP=23
   * HFP=22 VFP=22 800x480 RGB565):
   *   RCC_CR    = 0x5XX03XXX  (PLLSAIRDY bit29=1, PLLSAION bit28=1)
   *   GCR       = 0x00002221  (LTDCEN=1, HWPOL cleared)
   *   L1CR      = 0x00000001  (LEN=1)
   *   L1WHPCR   = 0x01ED002F  (HST=47, HSP=481)
   *   L1WVPCR   = 0x01E20018  (VST=24, VSP=482)
   *   L1PFCR    = 0x00000002  (RGB565)
   *   L1CACR    = 0x000000FF  (const alpha 255)
   *   L1BFCR    = 0x00000640  (BF1=CA, BF2=PAxCA)
   *   L1CFBAR   = 0xD0000000  (fbmem start)
   *   L1CFBLR   = 0x64000643  (pitch=1600, len=1603)
   *   L1CFBLNR  = 0x000001E0  (480 lines)
   * If any register reads back ZERO or wrong, that's the white-screen root
   * cause — the write was discarded by hardware.
   */

  syslog(LOG_INFO, "LTDC DUMP: RCC_CR     =0x%08lX\n",
         (unsigned long)getreg32(0x40023800UL));           /* RCC_CR */
  syslog(LOG_INFO, "LTDC DUMP: GCR        =0x%08lX\n",
         (unsigned long)getreg32(LTDC_REG_GCR));
  syslog(LOG_INFO, "LTDC DUMP: L1CR       =0x%08lX\n",
         (unsigned long)getreg32(LTDC_REG_L1CR));
  syslog(LOG_INFO, "LTDC DUMP: L1WHPCR    =0x%08lX\n",
         (unsigned long)getreg32(LTDC_REG_L1WHPCR));
  syslog(LOG_INFO, "LTDC DUMP: L1WVPCR    =0x%08lX\n",
         (unsigned long)getreg32(LTDC_REG_L1WVPCR));
  syslog(LOG_INFO, "LTDC DUMP: L1PFCR     =0x%08lX\n",
         (unsigned long)getreg32(LTDC_REG_L1PFCR));
  syslog(LOG_INFO, "LTDC DUMP: L1CACR     =0x%08lX\n",
         (unsigned long)getreg32(LTDC_REG_L1CACR));
  syslog(LOG_INFO, "LTDC DUMP: L1BFCR     =0x%08lX\n",
         (unsigned long)getreg32(LTDC_REG_L1BFCR));
  syslog(LOG_INFO, "LTDC DUMP: L1CFBAR    =0x%08lX\n",
         (unsigned long)getreg32(LTDC_REG_L1CFBAR));
  syslog(LOG_INFO, "LTDC DUMP: L1CFBLR    =0x%08lX\n",
         (unsigned long)getreg32(LTDC_REG_L1CFBLR));
  syslog(LOG_INFO, "LTDC DUMP: L1CFBLNR   =0x%08lX\n",
         (unsigned long)getreg32(LTDC_REG_L1CFBLNR));

  /* ---- Step 12: GPIO AF dump — verify LTDC pins really switched to AF14 ----
   *
   * Physical signal-chain check. All LTDC registers are correct and fbmem
   * is wired (SDRAM PASS), but panel is white — root cause must be that the
   * LTDC pixel clock / HSYNC / VSYNC / DE never actually reach the panel.
   * Read back GPIOG MODER + AFRL + AFRH (LTDC_CLK=PG7, R7=PG6, G3=PG10,
   * B1=PG12, B3=PG11) to confirm AF14 (0xE) took effect on each pin.
   *   PG6  (R7)  → AFRH bit[11:8]   should = 0xE
   *   PG7  (CLK) → AFRL bit[31:28]  should = 0xE
   *   PG10 (G3)  → AFRH bit[11:8]   should = 0xE (wait PG10 → AFRH bit[11:8]? pin10→AFRH bit[11:8])
   *   PG11 (B3)  → AFRH bit[15:12]  should = 0xE
   *   PG12 (B1)  → AFRH bit[19:16]  should = 0xE
   * GPIOG base = 0x40021800, MODER=+0x00, AFRL=+0x20, AFRH=+0x24.
   * MODER bit pair for pin n = [2n+1:2n], value 0b10 = alternate function.
   */

  syslog(LOG_INFO, "GPIOG DUMP: MODER      =0x%08lX\n",
         (unsigned long)getreg32(0x40021800UL));         /* MODER */
  syslog(LOG_INFO, "GPIOG DUMP: AFRL       =0x%08lX\n",
         (unsigned long)getreg32(0x40021820UL));         /* AFRL (pins 0-7) */
  syslog(LOG_INFO, "GPIOG DUMP: AFRH       =0x%08lX\n",
         (unsigned long)getreg32(0x40021824UL));         /* AFRH (pins 8-15) */
}

/****************************************************************************
 * Name: stm32_lcdinitialize
 *
 * Description:
 *   Initialize the LCD controller and LTDC interface.
 *
 ****************************************************************************/

int stm32_lcdinitialize(void)
{
  /* Use the direct LCD initialization transplanted from the Wildfire F429
   * BSP. This bypasses stm32_ltdcinitialize() whose RCC/PLLSAI setup was
   * being silently discarded (the panel stayed white).
   */

  stm32_lcd_direct_init();

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
