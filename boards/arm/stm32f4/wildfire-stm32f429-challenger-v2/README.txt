Wildfire STM32F429 Challenger V2 Board
=======================================

This board support package is for the Wildfire STM32F429 Challenger V2
development board from Wildfire (野火).

Board Features:
- MCU: STM32F429IGT6 (176-pin LQFP)
- Flash: 2MB internal, 16MB external NOR Flash (optional)
- SRAM: 256KB internal, 8MB external SDRAM (optional)
- Crystal: 8MHz HSE, 32.768KHz LSE
- USB: USB OTG FS (PA11/PA12), USB OTG HS (PB14/PB15)
- Ethernet: 10/100M Ethernet (optional PHY)
- LCD: TFT LCD interface (LTDC)
- Debug: SWD (PA13/PA14)

Supported Configurations:
- nsh: NuttShell on USART1 (PA9/PA10) at 115200 baud

Hardware Connections:
- USART1 TX: PA9
- USART1 RX: PA10
- USART2 TX: PD5
- USART2 RX: PD6
- USART3 TX: PD8
- USART3 RX: PD9
- LED1: PB0 (active low)
- LED2: PB1 (active low)
- LED3: PB5 (active low)
- LED4: PB6 (active low)
- KEY1: PA0 (WKUP, active high)
- KEY2: PC13 (active high)

Building and Running:
  make distclean
  ./tools/configure.sh -l wildfire-stm32f429-challenger-v2:nsh
  make

Flashing:
  Use ST-Link or J-Link to flash the nuttx.bin file to the board.

Notes:
- This board support is based on the stm32f429i-disco board.
- The board has been tested with NuttX 12.x.
- For more information, see the hardware documentation in the
  野火STM32开发板/挑战者/ directory.
