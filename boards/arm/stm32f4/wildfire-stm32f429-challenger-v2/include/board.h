/****************************************************************************
 * boards/arm/stm32f4/wildfire-stm32f429-challenger-v2/include/board.h
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

#ifndef __BOARDS_ARM_STM32F4_WILDFIRE_STM32F429_CHALLENGER_V2_INCLUDE_BOARD_H
#define __BOARDS_ARM_STM32F4_WILDFIRE_STM32F429_CHALLENGER_V2_INCLUDE_BOARD_H

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Clocking *****************************************************************/

/* The Wildfire STM32F429 Challenger V2 board provides:
 *
 *   - An external 8MHz crystal (HSE) on PH0/PH1
 *   - A 32.768KHz crystal (LSE) on PC14/PC15
 *
 * This is the default configuration:
 *
 * System Clock source: PLL (HSE)
 * SYSCLK(Hz):          168000000    Determined by PLL configuration
 * HCLK(Hz):            168000000    (STM32_RCC_CFGR_HPRE)
 * AHB Prescaler:       1            (STM32_RCC_CFGR_HPRE)
 * APB1 Prescaler:      4            (STM32_RCC_CFGR_PPRE1)
 * APB2 Prescaler:      2            (STM32_RCC_CFGR_PPRE2)
 * PLL_M:               8            (STM32_RCC_PLLCFG_PLLM)
 * PLL_N:               336          (STM32_RCC_PLLCFG_PLLN)
 * PLL_P:               2            (STM32_RCC_PLLCFG_PLLP)
 * PLL_Q:               7            (STM32_RCC_PLLCFG_PLLQ)
 * Flash Latency:       5
 */

/* HSE frequency - 25MHz crystal on Wildfire Challenger V2 */
#define STM32_HSE_FREQUENCY    25000000UL
#define STM32_LSE_FREQUENCY    32768UL

/* Main PLL Configuration for 25MHz HSE -> 168MHz SYSCLK
 * PLLM = 25 (VCO input = 25MHz / 25 = 1MHz)
 * PLLN = 336 (VCO output = 1MHz * 336 = 336MHz)
 * PLLP = 2 (SYSCLK = 336MHz / 2 = 168MHz)
 * PLLQ = 7 (USB clock = 336MHz / 7 = 48MHz)
 */
#define STM32_PLLCFG_PLLM      RCC_PLLCFG_PLLM(25)
#define STM32_PLLCFG_PLLN      RCC_PLLCFG_PLLN(336)
#define STM32_PLLCFG_PLLP      RCC_PLLCFG_PLLP_2
#define STM32_PLLCFG_PLLQ      RCC_PLLCFG_PLLQ(7)

/* System Clock Configuration */
#define STM32_RCC_CFGR_HPRE    RCC_CFGR_HPRE_SYSCLK
#define STM32_RCC_CFGR_PPRE1   RCC_CFGR_PPRE1_HCLKd4
#define STM32_RCC_CFGR_PPRE2   RCC_CFGR_PPRE2_HCLKd2

/* Flash Configuration */
#define STM32_FLASH_WAITSTATES 5

/* Clock frequencies */
#define STM32_SYSCLK_FREQUENCY 168000000UL
#define STM32_HCLK_FREQUENCY   168000000UL
#define STM32_PCLK1_FREQUENCY  (STM32_HCLK_FREQUENCY / 4)
#define STM32_PCLK2_FREQUENCY  (STM32_HCLK_FREQUENCY / 2)
#define STM32_APB1_FREQUENCY   STM32_PCLK1_FREQUENCY
#define STM32_APB2_FREQUENCY   STM32_PCLK2_FREQUENCY

/* LED definitions **********************************************************/

/* The Wildfire STM32F429 Challenger V2 board has 3 RGB LEDs:
 *   LED1 (Red):   PH10  (active low)
 *   LED2 (Green): PH11  (active low)
 *   LED3 (Blue):  PH12  (active low)
 */

#define GPIO_LED1    (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | \
                      GPIO_OUTPUT_SET | GPIO_PORTH | GPIO_PIN10)
#define GPIO_LED2    (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | \
                      GPIO_OUTPUT_SET | GPIO_PORTH | GPIO_PIN11)
#define GPIO_LED3    (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | \
                      GPIO_OUTPUT_SET | GPIO_PORTH | GPIO_PIN12)
#define GPIO_LED4    GPIO_LED3  /* No LED4, alias to LED3 */

/* LED bit format: LED1=bit0, LED2=bit1, LED3=bit2 */
#define BOARD_LED1   0
#define BOARD_LED2   1
#define BOARD_LED3   2
#define BOARD_LED4   3
#define BOARD_NLEDS  4

#define BOARD_LED_RED    BOARD_LED1
#define BOARD_LED_GREEN  BOARD_LED2
#define BOARD_LED_BLUE   BOARD_LED3

/* LED active low: setting the pin HIGH turns the LED off */

#define LED_STARTED       0  /* LED1 */
#define LED_HEAPALLOCATE  1  /* LED2 */
#define LED_IRQSENABLED   2  /* LED1 + LED2 */
#define LED_STACKCREATED  3  /* LED3 */
#define LED_INIRQ         4  /* LED1 + LED3 */
#define LED_SIGNAL        5  /* LED2 + LED3 */
#define LED_ASSERTION     6  /* LED1 + LED2 + LED3 */
#define LED_PANIC         7  /* N/C  + N/C  + N/C + LED4 */

/* LED macros for NuttX internal use */
#define LED_LED1 0
#define LED_LED2 1
#define LED_LED3 2
#define LED_LED4 3

/* Button definitions *******************************************************/

/* The Wildfire STM32F429 Challenger V2 board has:
 *   KEY1: PA0 (WKUP, active high)
 *   KEY2: PC13 (active high)
 */

#define GPIO_BTN_KEY1  (GPIO_INPUT | GPIO_FLOAT | GPIO_EXTI | GPIO_PORTA | GPIO_PIN0)
#define GPIO_BTN_KEY2  (GPIO_INPUT | GPIO_FLOAT | GPIO_EXTI | GPIO_PORTC | GPIO_PIN13)

#define BUTTON_KEY1    0
#define BUTTON_KEY2    1
#define NUM_BUTTONS    2

/* USART configuration ******************************************************/

/* USART1: PA9=TX, PA10=RX (connected to ST-Link VCP on some boards) */
#define GPIO_USART1_TX  (GPIO_ALT | GPIO_AF7 | GPIO_SPEED_50MHz | GPIO_OUTPUT_SET | GPIO_PORTA | GPIO_PIN9)
#define GPIO_USART1_RX  (GPIO_ALT | GPIO_AF7 | GPIO_SPEED_50MHz | GPIO_PULLUP | GPIO_PORTA | GPIO_PIN10)

/* USART2: PD5=TX, PD6=RX (connected to RS232/USB-UART) */
#define GPIO_USART2_TX  (GPIO_ALT | GPIO_AF7 | GPIO_SPEED_50MHz | GPIO_OUTPUT_SET | GPIO_PORTD | GPIO_PIN5)
#define GPIO_USART2_RX  (GPIO_ALT | GPIO_AF7 | GPIO_SPEED_50MHz | GPIO_PULLUP | GPIO_PORTD | GPIO_PIN6)

/* USART3: PD8=TX, PD9=RX (connected to RS232/USB-UART) */
#define GPIO_USART3_TX  (GPIO_ALT | GPIO_AF7 | GPIO_SPEED_50MHz | GPIO_OUTPUT_SET | GPIO_PORTD | GPIO_PIN8)
#define GPIO_USART3_RX  (GPIO_ALT | GPIO_AF7 | GPIO_SPEED_50MHz | GPIO_PULLUP | GPIO_PORTD | GPIO_PIN9)

/* USART6 (UART6): PC6=TX, PC7=RX (AF8, connected to RS232 via J69/J70) */
#define GPIO_USART6_TX  (GPIO_ALT | GPIO_AF8 | GPIO_SPEED_50MHz | GPIO_OUTPUT_SET | GPIO_PORTC | GPIO_PIN6)
#define GPIO_USART6_RX  (GPIO_ALT | GPIO_AF8 | GPIO_SPEED_50MHz | GPIO_PULLUP | GPIO_PORTC | GPIO_PIN7)

/* SD Card (SDIO) configuration *********************************************/

/* SDIO interface for SD card
 *
 * PC8:  SDIO_D0  (AF12)
 * PC9:  SDIO_D1  (AF12)
 * PC10: SDIO_D2  (AF12)
 * PC11: SDIO_D3  (AF12)
 * PC12: SDIO_CK  (AF12)
 * PD2:  SDIO_CMD (AF12)
 * PB8:  SDIO_D4  (AF12) - optional for wide bus
 * PB9:  SDIO_D5  (AF12) - optional for wide bus
 * PC6:  SDIO_D6  (AF12) - optional for wide bus
 * PC7:  SDIO_D7  (AF12) - optional for wide bus
 */

#define GPIO_SDIO_D0     (GPIO_SDIO_D0_0 | GPIO_SPEED_50MHz)
#define GPIO_SDIO_D1     (GPIO_SDIO_D1_0 | GPIO_SPEED_50MHz)
#define GPIO_SDIO_D2     (GPIO_SDIO_D2_0 | GPIO_SPEED_50MHz)
#define GPIO_SDIO_D3     (GPIO_SDIO_D3_0 | GPIO_SPEED_50MHz)
#define GPIO_SDIO_CK     (GPIO_SDIO_CK_0 | GPIO_SPEED_50MHz)
#define GPIO_SDIO_CMD    (GPIO_SDIO_CMD_0 | GPIO_SPEED_50MHz)

/* SDIO clock configuration (48MHz max SDIO clock, 393KHz init clock)
 * SDIOCLK = 48MHz (from PLL)
 * Init clock: SDIOCLK / (120 + 2) = 393.4KHz, matching the AP6181 example
 * Wi-Fi transfer clock: SDIOCLK / (10 + 2) = 4MHz.  The AP6181 signals
 * share the TF-card traces, so start conservatively until signal integrity
 * has been verified on hardware.
 */

#define SDIO_CLKCR_EDGE        SDIO_CLKCR_RISINGEDGE
#define SDIO_INIT_CLKDIV      (120 << SDIO_CLKCR_CLKDIV_SHIFT)
#define SDIO_MMCXFR_CLKDIV    (1 << SDIO_CLKCR_CLKDIV_SHIFT)
#define SDIO_SDXFR_CLKDIV     (10 << SDIO_CLKCR_CLKDIV_SHIFT)

/* STM32F4 SDIO DMA2 channel 4 can use stream 3 or stream 6.  Use the
 * conventional stream 3 mapping; no enabled board peripheral conflicts with
 * it in the Wi-Fi configuration.
 */

#define DMAMAP_SDIO           DMAMAP_SDIO_1

/* AP6181 Wi-Fi module ******************************************************/

/* The AP6181 (BCM43362) shares PC8-PC12/PD2 with the TF-card socket.
 * PB13 is shared by AP6181 WL_REG_ON and NAND R/B#.  NAND initialization
 * finishes before Wi-Fi is started; after that PB13 is kept as an output and
 * driven high.  PA0 receives WL_HOST_WAKE (active high).
 */

#define GPIO_AP6181_REG_ON \
  (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | GPIO_OUTPUT_CLEAR | \
   GPIO_PORTB | GPIO_PIN13)
#define GPIO_AP6181_HOST_WAKE \
  (GPIO_INPUT | GPIO_FLOAT | GPIO_EXTI | GPIO_PORTA | GPIO_PIN0)

#define BOARD_AP6181_SDIO_SLOT  0
#define BOARD_AP6181_MINOR      0

/* SPI Flash (SPI4) configuration *******************************************/

/* SPI4 interface for SPI Flash (W25Q/SST25)
 *
 * PE2:  SPI4_SCK  (AF5)
 * PE4:  SPI4_NSS  (AF5)
 * PE5:  SPI4_MISO (AF5)
 * PE6:  SPI4_MOSI (AF5)
 */

#define GPIO_SPI4_SCK    (GPIO_AF5 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTE | GPIO_PIN2)
#define GPIO_SPI4_NSS    (GPIO_AF5 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTE | GPIO_PIN4)
#define GPIO_SPI4_MISO   (GPIO_AF5 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTE | GPIO_PIN5)
#define GPIO_SPI4_MOSI   (GPIO_AF5 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTE | GPIO_PIN6)

/* LCD configuration (LTDC interface) ****************************************/

/* LCD resolution - 5" RGB888 capacitive LCD (800x480), per Wildfire BSP */

#define BOARD_LTDC_WIDTH   800
#define BOARD_LTDC_HEIGHT  480

/* PLL SAI configuration for LTDC
 *
 * Matches the Wildfire F429 BSP LCD_Init(): PLLSAIN=420, PLLSAIR=4, DIVR=Div4.
 * PLLSAI_VCO = HSE/PLLM * N = 25MHz/25 * 420 = 16.8MHz... wait, Wildfire uses
 * HSE=25MHz, PLLM=25, so VCO_input=1MHz, VCO_output=1MHz*420=420MHz.
 * LTDC pixel clock = 420MHz / PLLSAIR(4) / DIVR(4) = 26.25MHz.
 */

#define BOARD_LTDC_PLLSAIN              420
#define BOARD_LTDC_PLLSAIR              3
#define BOARD_LTDC_PLLSAIQ              7

#define STM32_RCC_PLLSAICFGR_PLLSAIN    RCC_PLLSAICFGR_PLLSAIN(BOARD_LTDC_PLLSAIN)
#define STM32_RCC_PLLSAICFGR_PLLSAIR    RCC_PLLSAICFGR_PLLSAIR(BOARD_LTDC_PLLSAIR)
#define STM32_RCC_PLLSAICFGR_PLLSAIQ    RCC_PLLSAICFGR_PLLSAIQ(BOARD_LTDC_PLLSAIQ)
#define STM32_RCC_DCKCFGR_PLLSAIDIVR    RCC_DCKCFGR_PLLSAIDIVR_DIV4

/* LTDC synchronization parameters for Wildfire 5" 800x480 RGB888 panel
 * (matches the Wildfire F429 BSP bsp_lcd.c lcd_param[INCH_5])
 */

#define BOARD_LTDC_HFP                  22
#define BOARD_LTDC_HBP                  46
#define BOARD_LTDC_VFP                  22
#define BOARD_LTDC_VBP                  23
#define BOARD_LTDC_HSYNC                1
#define BOARD_LTDC_VSYNC                1

/* LTDC polarity configuration
 * Per Wildfire BSP: HS/VS/DE active-low (pol=0), pixel clock inverted
 * (PCPOL=1, i.e. sample on falling edge / IPC mode).
 */

#define BOARD_LTDC_GCR_PCPOL            LTDC_GCR_PCPOL
#define BOARD_LTDC_GCR_DEPOL            0 /* !LTDC_GCR_DEPOL */
#define BOARD_LTDC_GCR_VSPOL            0 /* !LTDC_GCR_VSPOL */
#define BOARD_LTDC_GCR_HSPOL            0 /* !LTDC_GCR_HSPOL */

/* LTDC GPIO pin assignments for Wildfire STM32F429 Challenger V2
 * (per core board schematic V2.1, Page 6 - LCD/KEY/FLASH/E2PROM)
 *
 * LCD_R0:  PH2   (AF14)
 * LCD_R1:  PH3   (AF14)
 * LCD_R2:  PH8   (AF14)
 * LCD_R3:  PB0   (AF9)
 * LCD_R4:  PA11  (AF14)
 * LCD_R5:  PA12  (AF14)
 * LCD_R6:  PB1   (AF9)
 * LCD_R7:  PG6   (AF14)
 * LCD_G0:  PE5   (AF14)
 * LCD_G1:  PE6   (AF14)
 * LCD_G2:  PH13  (AF14)
 * LCD_G3:  PG10  (AF9)
 * LCD_G4:  PH15  (AF14)
 * LCD_G5:  PI0   (AF14)
 * LCD_G6:  PI1   (AF14)  [was PC7, moved to free USART6 RX]
 * LCD_G7:  PI2   (AF14)
 * LCD_B0:  PE4   (AF14)
 * LCD_B1:  PG12  (AF9)
 * LCD_B2:  PD6   (AF14)
 * LCD_B3:  PG11  (AF14)
 * LCD_B4:  PI4   (AF14)
 * LCD_B5:  PA3   (AF14)
 * LCD_B6:  PB8   (AF14)
 * LCD_B7:  PB9   (AF14)
 * LCD_CLK: PG7   (AF14)
 * LCD_DE:  PF10  (AF14)
 * LCD_HSYNC: PI10 (AF14)
 * LCD_VSYNC: PI9  (AF14)
 * LCD_BL:  PD7   (GPIO, active high)
 */

/* LTDC GPIO pin definitions */

#define GPIO_LTDC_R0   (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTH | GPIO_PIN2)
#define GPIO_LTDC_R1   (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTH | GPIO_PIN3)
#define GPIO_LTDC_R2   (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTH | GPIO_PIN8)
#define GPIO_LTDC_R3   (GPIO_ALT | GPIO_AF9  | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTB | GPIO_PIN0)
#define GPIO_LTDC_R4   (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTA | GPIO_PIN11)
#define GPIO_LTDC_R5   (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTA | GPIO_PIN12)
#define GPIO_LTDC_R6   (GPIO_ALT | GPIO_AF9  | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTB | GPIO_PIN1)
#define GPIO_LTDC_R7   (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTG | GPIO_PIN6)
#define GPIO_LTDC_G0   (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTE | GPIO_PIN5)
#define GPIO_LTDC_G1   (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTE | GPIO_PIN6)
#define GPIO_LTDC_G2   (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTH | GPIO_PIN13)
#define GPIO_LTDC_G3   (GPIO_ALT | GPIO_AF9  | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTG | GPIO_PIN10)
#define GPIO_LTDC_G4   (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTH | GPIO_PIN15)
#define GPIO_LTDC_G5   (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTI | GPIO_PIN0)
#define GPIO_LTDC_G6   (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTC | GPIO_PIN7)
#define GPIO_LTDC_G7   (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTI | GPIO_PIN2)
#define GPIO_LTDC_B0   (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTE | GPIO_PIN4)
#define GPIO_LTDC_B1   (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTG | GPIO_PIN12)
#define GPIO_LTDC_B2   (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTD | GPIO_PIN6)
#define GPIO_LTDC_B3   (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTG | GPIO_PIN11)
#define GPIO_LTDC_B4   (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTI | GPIO_PIN4)
#define GPIO_LTDC_B5   (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTA | GPIO_PIN3)
#define GPIO_LTDC_B6   (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTB | GPIO_PIN8)
#define GPIO_LTDC_B7   (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTB | GPIO_PIN9)
#define GPIO_LTDC_CLK  (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTG | GPIO_PIN7)
#define GPIO_LTDC_DE   (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTF | GPIO_PIN10)
#define GPIO_LTDC_HSYNC (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTI | GPIO_PIN10)
#define GPIO_LTDC_VSYNC (GPIO_ALT | GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTI | GPIO_PIN9)

/* LCD display enable (DISP) is hard-wired to 3V3 on this board (FPC pin 33),
 * so the panel is always enabled — no GPIO needed.
 */

/* LCD backlight (BL) - PD7, active high (FPC pin 35, user-confirmed) */

#define GPIO_LCD_BL    (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | \
                        GPIO_OUTPUT_CLEAR | GPIO_PORTD | GPIO_PIN7)

/* LCD reset pin (if available) */

#define GPIO_LCD_RESET (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | \
                        GPIO_OUTPUT_CLEAR | GPIO_PORTB | GPIO_PIN15)

/* FMC SDRAM configuration ***********************************************/

/* FMC SDRAM pins for Wildfire STM32F429 Challenger V2
 *
 * Data pins: PD0-PD15 (D0-D15)
 * Address pins: PF0-PF5, PF12-PF15, PG0-PG1, PG4-PG5 (A0-A11)
 * Control pins: PC3(SDCKE0), PC2(SDNE0), PH5(SDNWE), PF11(SDNRAS), PG15(SDNCAS), PG8(SDCLK)
 * Byte enable: PE0(NBL0), PE1(NBL1)
 */

/* FMC SDRAM control pins (Bank 2: PH7=SDCKE1, PH6=SDNE1) */

#define GPIO_FMC_SDCKE1  (GPIO_FMC_SDCKE1_2 | GPIO_SPEED_100MHz)
#define GPIO_FMC_SDNE1   (GPIO_FMC_SDNE1_2 | GPIO_SPEED_100MHz)
#define GPIO_FMC_SDNWE   (GPIO_FMC_SDNWE_1 | GPIO_SPEED_100MHz)

/* FMC SDRAM data pins (D0-D15) */

#define GPIO_FMC_D0      (GPIO_FMC_D0_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_D1      (GPIO_FMC_D1_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_D2      (GPIO_FMC_D2_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_D3      (GPIO_FMC_D3_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_D4      (GPIO_FMC_D4_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_D5      (GPIO_FMC_D5_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_D6      (GPIO_FMC_D6_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_D7      (GPIO_FMC_D7_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_D8      (GPIO_FMC_D8_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_D9      (GPIO_FMC_D9_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_D10     (GPIO_FMC_D10_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_D11     (GPIO_FMC_D11_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_D12     (GPIO_FMC_D12_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_D13     (GPIO_FMC_D13_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_D14     (GPIO_FMC_D14_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_D15     (GPIO_FMC_D15_0 | GPIO_SPEED_100MHz)

/* FMC SDRAM address pins (A0-A12) for W9825G6KH (32MB, 13 row + 9 col) */

#define GPIO_FMC_A0      (GPIO_FMC_A0_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_A1      (GPIO_FMC_A1_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_A2      (GPIO_FMC_A2_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_A3      (GPIO_FMC_A3_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_A4      (GPIO_FMC_A4_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_A5      (GPIO_FMC_A5_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_A6      (GPIO_FMC_A6_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_A7      (GPIO_FMC_A7_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_A8      (GPIO_FMC_A8_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_A9      (GPIO_FMC_A9_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_A10     (GPIO_FMC_A10_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_A11     (GPIO_FMC_A11_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_A12     (GPIO_FMC_A12_0 | GPIO_SPEED_100MHz)

/* FMC SDRAM byte enable pins */

#define GPIO_FMC_NBL0    (GPIO_FMC_NBL0_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_NBL1    (GPIO_FMC_NBL1_0 | GPIO_SPEED_100MHz)

/* FMC SDRAM timing pins */

#define GPIO_FMC_SDCLK   (GPIO_FMC_SDCLK_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_SDNCAS  (GPIO_FMC_SDNCAS_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_SDNRAS  (GPIO_FMC_SDNRAS_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_BA0     (GPIO_FMC_BA0_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_BA1     (GPIO_FMC_BA1_0 | GPIO_SPEED_100MHz)

/* FMC NAND control pins (for W29N01GV NAND Flash on Bank 3) */

#define GPIO_FMC_NOE     (GPIO_FMC_NOE_0 | GPIO_SPEED_100MHz)    /* PD4 - RE# */
#define GPIO_FMC_NWE     (GPIO_FMC_NWE_0 | GPIO_SPEED_100MHz)    /* PD5 - WE# */
#define GPIO_FMC_NWAIT   (GPIO_FMC_NWAIT_0 | GPIO_SPEED_100MHz)  /* PD6 - R/B# */
#define GPIO_FMC_NCE2    (GPIO_FMC_NCE2_0 | GPIO_SPEED_100MHz)   /* PD7 - CE# */
#define GPIO_FMC_NCE3    (GPIO_FMC_NCE3_0 | GPIO_SPEED_100MHz)   /* PG9 - CE# for Bank 3 */
#define GPIO_FMC_NADV    (GPIO_ALT|GPIO_AF12|GPIO_SPEED_100MHz|GPIO_PORTD|GPIO_PIN11)  /* PD11 - ALE */
#define GPIO_FMC_NCLE    (GPIO_ALT|GPIO_AF12|GPIO_SPEED_100MHz|GPIO_PORTD|GPIO_PIN12)  /* PD12 - CLE */

/* Goodix GT9xx capacitive touch panel (per Wildfire BSP bsp_i2c_touch.h)
 *
 *  IC       : Goodix GT911/GT9157/GT917S (5-point capacitive touch)
 *  Bus      : I2C2 (hardware I2C; BSP used software I2C over the same pins)
 *  I2C addr : 0x5D  (7-bit; BSP wrote 0xBA which is 0x5D<<1)
 *  SCL      : PH4   (AF4)
 *  SDA      : PH5   (AF4)
 *  RST      : PI8   (GPIO output, active-low reset)
 *  INT      : PD13  (GPIO input, rising-edge IRQ; BSP EXTI13)
 *  Coor reg : 0x814E (GTP_READ_COOR_ADDR)
 */

#define GPIO_GT9XX_SCL \
  (GPIO_ALT | GPIO_AF4 | GPIO_SPEED_50MHz | GPIO_OPENDRAIN | \
   GPIO_PORTH | GPIO_PIN4)
#define GPIO_GT9XX_SDA \
  (GPIO_ALT | GPIO_AF4 | GPIO_SPEED_50MHz | GPIO_OPENDRAIN | \
   GPIO_PORTH | GPIO_PIN5)
#define GPIO_GT9XX_RST \
  (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | \
   GPIO_OUTPUT_CLEAR | GPIO_PORTI | GPIO_PIN8)
#define GPIO_GT9XX_INT \
  (GPIO_INPUT | GPIO_FLOAT | GPIO_PORTD | GPIO_PIN13)
#define GPIO_GT9XX_INT_OUT \
  (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | \
   GPIO_OUTPUT_CLEAR | GPIO_PORTD | GPIO_PIN13)

/* stm32_i2c_m3m4_v1_f40xxx.c references GPIO_I2C2_SCL/SDA (no suffix) for
 * bus 2's pin pair. The F40xxx pinmap defines three I2C2 pairs; pair 3 is
 * PH4/PH5 AF4 — exactly the Wildfire BSP touch pins. Alias so the arch I2C
 * driver picks up the right pins.
 */

#define GPIO_I2C2_SCL   GPIO_I2C2_SCL_3
#define GPIO_I2C2_SDA   GPIO_I2C2_SDA_3

#define BOARD_GT9XX_I2C_PORT      2        /* I2C2 */
#define BOARD_GT9XX_I2C_ADDR      0x5d     /* 7-bit */

#endif /* __BOARDS_ARM_STM32F4_WILDFIRE_STM32F429_CHALLENGER_V2_INCLUDE_BOARD_H */
