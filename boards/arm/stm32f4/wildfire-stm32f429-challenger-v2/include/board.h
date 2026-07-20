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
                      GPIO_OUTPUT_CLEAR | GPIO_PORTH | GPIO_PIN10)
#define GPIO_LED2    (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | \
                      GPIO_OUTPUT_CLEAR | GPIO_PORTH | GPIO_PIN11)
#define GPIO_LED3    (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | \
                      GPIO_OUTPUT_CLEAR | GPIO_PORTH | GPIO_PIN12)
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

#define GPIO_SDIO_D0     (GPIO_AF12 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTC | GPIO_PIN8)
#define GPIO_SDIO_D1     (GPIO_AF12 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTC | GPIO_PIN9)
#define GPIO_SDIO_D2     (GPIO_AF12 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTC | GPIO_PIN10)
#define GPIO_SDIO_D3     (GPIO_AF12 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTC | GPIO_PIN11)
#define GPIO_SDIO_CK     (GPIO_AF12 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTC | GPIO_PIN12)
#define GPIO_SDIO_CMD    (GPIO_AF12 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTD | GPIO_PIN2)

/* SDIO clock configuration (48MHz max SDIO clock, 400KHz init clock)
 * SDIOCLK = 48MHz (from PLL)
 * Init clock: SDIOCLK / (118 + 2) = 400KHz
 * Transfer clock: SDIOCLK / (1 + 2) = 16MHz (for SD card)
 */

#define SDIO_INIT_CLKDIV      (118 << SDIO_CLKCR_CLKDIV_SHIFT)
#define SDIO_MMCXFR_CLKDIV    (1 << SDIO_CLKCR_CLKDIV_SHIFT)
#define SDIO_SDXFR_CLKDIV     (1 << SDIO_CLKCR_CLKDIV_SHIFT)

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

/* LCD resolution */

#define BOARD_LTDC_WIDTH   480
#define BOARD_LTDC_HEIGHT  320

/* PLL SAI configuration for LTDC
 *
 * PLLSAI_VCO = STM32_HSE_FREQUENCY / PLLM = 25MHz / 25 = 1MHz
 * PLLSAI_VCO * PLLSAIN = 1MHz * 192 = 192MHz
 * LTDC clock = PLLSAI_VCO * PLLSAIN / PLLSAIR / PLLSAIDIVR
 *            = 192MHz / 4 / 8 = 6MHz
 */

#define BOARD_LTDC_PLLSAIN              192
#define BOARD_LTDC_PLLSAIR              4
#define BOARD_LTDC_PLLSAIQ              7

#define STM32_RCC_PLLSAICFGR_PLLSAIN    RCC_PLLSAICFGR_PLLSAIN(BOARD_LTDC_PLLSAIN)
#define STM32_RCC_PLLSAICFGR_PLLSAIR    RCC_PLLSAICFGR_PLLSAIR(BOARD_LTDC_PLLSAIR)
#define STM32_RCC_PLLSAICFGR_PLLSAIQ    RCC_PLLSAICFGR_PLLSAIQ(BOARD_LTDC_PLLSAIQ)
#define STM32_RCC_DCKCFGR_PLLSAIDIVR    RCC_DCKCFGR_PLLSAIDIVR_DIV8

/* LTDC synchronization parameters (typical for 480x320 LCD) */

#define BOARD_LTDC_HFP                  10
#define BOARD_LTDC_HBP                  20
#define BOARD_LTDC_VFP                  4
#define BOARD_LTDC_VBP                  2
#define BOARD_LTDC_HSYNC                10
#define BOARD_LTDC_VSYNC                2

/* LTDC polarity configuration */

#define BOARD_LTDC_GCR_PCPOL            0 /* !LTDC_GCR_PCPOL */
#define BOARD_LTDC_GCR_DEPOL            0 /* !LTDC_GCR_DEPOL */
#define BOARD_LTDC_GCR_VSPOL            0 /* !LTDC_GCR_VSPOL */
#define BOARD_LTDC_GCR_HSPOL            0 /* !LTDC_GCR_HSPOL */

/* LTDC GPIO pin assignments for Wildfire STM32F429 Challenger V2
 *
 * LCD_R0:  PH2   (AF14)
 * LCD_R1:  PH3   (AF14)
 * LCD_R2:  PH8   (AF14)
 * LCD_R3:  PH9   (AF14)
 * LCD_R4:  PH10  (AF14)
 * LCD_R5:  PH11  (AF14)
 * LCD_R6:  PH12  (AF14)
 * LCD_R7:  PE15  (AF14)
 * LCD_G0:  PE5   (AF14)
 * LCD_G1:  PE6   (AF14)
 * LCD_G2:  PH13  (AF14)
 * LCD_G3:  PH14  (AF14)
 * LCD_G4:  PH15  (AF14)
 * LCD_G5:  PI0   (AF14)
 * LCD_G6:  PI1   (AF14)
 * LCD_G7:  PI2   (AF14)
 * LCD_B0:  PE4   (AF14)
 * LCD_B1:  PG12  (AF9)
 * LCD_B2:  PD6   (AF14)
 * LCD_B3:  PG11  (AF14)
 * LCD_B4:  PE12  (AF14)
 * LCD_B5:  PI4   (AF14)
 * LCD_B6:  PI5   (AF14)
 * LCD_B7:  PI6   (AF14)
 * LCD_CLK: PE14  (AF14)
 * LCD_DE:  PF10  (AF14)
 * LCD_HSYNC: PI10 (AF14)
 * LCD_VSYNC: PI9  (AF14)
 * LCD_BL:  PA1   (GPIO, active high)
 */

/* LTDC GPIO pin definitions (AF14 for most, AF9 for PG12) */

#define GPIO_LTDC_R0   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTH | GPIO_PIN2)
#define GPIO_LTDC_R1   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTH | GPIO_PIN3)
#define GPIO_LTDC_R2   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTH | GPIO_PIN8)
#define GPIO_LTDC_R3   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTH | GPIO_PIN9)
#define GPIO_LTDC_R4   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTH | GPIO_PIN10)
#define GPIO_LTDC_R5   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTH | GPIO_PIN11)
#define GPIO_LTDC_R6   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTH | GPIO_PIN12)
#define GPIO_LTDC_R7   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTE | GPIO_PIN15)
#define GPIO_LTDC_G0   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTE | GPIO_PIN5)
#define GPIO_LTDC_G1   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTE | GPIO_PIN6)
#define GPIO_LTDC_G2   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTH | GPIO_PIN13)
#define GPIO_LTDC_G3   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTH | GPIO_PIN14)
#define GPIO_LTDC_G4   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTH | GPIO_PIN15)
#define GPIO_LTDC_G5   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTI | GPIO_PIN0)
#define GPIO_LTDC_G6   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTI | GPIO_PIN1)
#define GPIO_LTDC_G7   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTI | GPIO_PIN2)
#define GPIO_LTDC_B0   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTE | GPIO_PIN4)
#define GPIO_LTDC_B1   (GPIO_AF9  | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTG | GPIO_PIN12)
#define GPIO_LTDC_B2   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTD | GPIO_PIN6)
#define GPIO_LTDC_B3   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTG | GPIO_PIN11)
#define GPIO_LTDC_B4   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTE | GPIO_PIN12)
#define GPIO_LTDC_B5   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTI | GPIO_PIN4)
#define GPIO_LTDC_B6   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTI | GPIO_PIN5)
#define GPIO_LTDC_B7   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTI | GPIO_PIN6)
#define GPIO_LTDC_CLK  (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTE | GPIO_PIN14)
#define GPIO_LTDC_DE   (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTF | GPIO_PIN10)
#define GPIO_LTDC_HSYNC (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTI | GPIO_PIN10)
#define GPIO_LTDC_VSYNC (GPIO_AF14 | GPIO_SPEED_50MHz | GPIO_PUSHPULL | GPIO_PORTI | GPIO_PIN9)

/* LCD backlight control (PA1, active high) */

#define GPIO_LCD_BL    (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | \
                        GPIO_OUTPUT_SET | GPIO_PORTA | GPIO_PIN1)

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

/* FMC SDRAM control pins */

#define GPIO_FMC_SDCKE1  (GPIO_FMC_SDCKE1_1 | GPIO_SPEED_100MHz)
#define GPIO_FMC_SDNE1   (GPIO_FMC_SDNE1_1 | GPIO_SPEED_100MHz)
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

/* FMC SDRAM address pins (A0-A11) */

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

/* FMC SDRAM byte enable pins */

#define GPIO_FMC_NBL0    (GPIO_FMC_NBL0_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_NBL1    (GPIO_FMC_NBL1_0 | GPIO_SPEED_100MHz)

/* FMC SDRAM timing pins */

#define GPIO_FMC_SDCLK   (GPIO_FMC_SDCLK_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_SDNCAS  (GPIO_FMC_SDNCAS_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_SDNRAS  (GPIO_FMC_SDNRAS_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_BA0     (GPIO_FMC_BA0_0 | GPIO_SPEED_100MHz)
#define GPIO_FMC_BA1     (GPIO_FMC_BA1_0 | GPIO_SPEED_100MHz)

#endif /* __BOARDS_ARM_STM32F4_WILDFIRE_STM32F429_CHALLENGER_V2_INCLUDE_BOARD_H */
