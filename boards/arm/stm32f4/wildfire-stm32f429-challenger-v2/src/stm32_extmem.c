/****************************************************************************
 * boards/arm/stm32f4/wildfire-stm32f429-challenger-v2/src/stm32_extmem.c
 *
 * FMC SDRAM initialization for Wildfire STM32F429 Challenger V2.
 * Based on Wildfire official example code (bsp_sdram.c).
 *
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <syslog.h>
#include <nuttx/debug.h>

#include <arch/board/board.h>

#include "chip.h"
#include "arm_internal.h"
#include "stm32.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* W9825G6KH: 256Mbit = 32MB, 13 row x 9 col x 4 banks x 16-bit
 * FMC Bank 2 at 0xD0000000
 * SDCLK = HCLK/2 = 84MHz
 */

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const uint32_t g_sdram_config[] =
{
  /* 16 data lines */

  GPIO_FMC_D0, GPIO_FMC_D1, GPIO_FMC_D2, GPIO_FMC_D3,
  GPIO_FMC_D4, GPIO_FMC_D5, GPIO_FMC_D6, GPIO_FMC_D7,
  GPIO_FMC_D8, GPIO_FMC_D9, GPIO_FMC_D10, GPIO_FMC_D11,
  GPIO_FMC_D12, GPIO_FMC_D13, GPIO_FMC_D14, GPIO_FMC_D15,

  /* 13 address lines (A0-A12) */

  GPIO_FMC_A0, GPIO_FMC_A1, GPIO_FMC_A2, GPIO_FMC_A3,
  GPIO_FMC_A4, GPIO_FMC_A5, GPIO_FMC_A6, GPIO_FMC_A7,
  GPIO_FMC_A8, GPIO_FMC_A9, GPIO_FMC_A10, GPIO_FMC_A11,
  GPIO_FMC_A12,

  /* control lines */

  GPIO_FMC_SDCKE1, GPIO_FMC_SDNE1, GPIO_FMC_SDNWE,
  GPIO_FMC_NBL0, GPIO_FMC_NBL1,
  GPIO_FMC_SDNRAS, GPIO_FMC_SDNCAS,
  GPIO_FMC_SDCLK,
  GPIO_FMC_BA0, GPIO_FMC_BA1,
};

#define NUM_SDRAM_GPIOS (sizeof(g_sdram_config) / sizeof(uint32_t))

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void stm32_sdram_initialize(void)
{
  uint32_t val;
  int i;
  volatile int count;

  /* Enable GPIOs as FMC / memory pins */

  for (i = 0; i < NUM_SDRAM_GPIOS; i++)
    {
      stm32_configgpio(g_sdram_config[i]);
    }

  /* Enable AHB clocking to the FMC */

  stm32_fmc_enable();

  /* Configure SDRAM control register (both banks)
   * Matches Wildfire example: 13 row, 9 col, 16-bit, CAS=2, SDCLK=HCLK/2
   */

  val = FMC_SDCR_RPIPE_0 |
    FMC_SDCR_SDCLK_2X |
    FMC_SDCR_CAS_LATENCY_2 |
    FMC_SDCR_NBANKS_4 |
    FMC_SDCR_WIDTH_16 |
    FMC_SDCR_ROWS_13 |
    FMC_SDCR_COLS_9;
  stm32_fmc_sdram_set_control(1, val);
  stm32_fmc_sdram_set_control(2, val);

  /* Configure SDRAM timing
   * Based on Wildfire example (adjusted for 84MHz = 11.9ns per cycle):
   *   TMRD=2, TXSR=7, TRAS=4, TRC=7, TWR=2, TRP=2, TRCD=2
   */

  val = FMC_SDTR_TMRD(2) |
    FMC_SDTR_TXSR(7) |
    FMC_SDTR_TRAS(4) |
    FMC_SDTR_TRC(7) |
    FMC_SDTR_TWR(2) |
    FMC_SDTR_TRP(2) |
    FMC_SDTR_TRCD(2);
  stm32_fmc_sdram_set_timing(1, val);
  stm32_fmc_sdram_set_timing(2, val);

  /* SDRAM initialization sequence (per W9825G6KH datasheet) */

  /* Step 1: Clock enable */
  stm32_fmc_sdram_command(FMC_SDCMR_CMD_CLK_ENABLE | FMC_SDCMR_BANK_2);

  /* Step 2: Wait >100us */
  for (count = 0; count < 200000; count++);

  /* Step 3: Precharge all */
  stm32_fmc_sdram_command(FMC_SDCMR_CMD_PALL | FMC_SDCMR_BANK_2);

  /* Step 4: Auto-refresh (2 cycles) */
  stm32_fmc_sdram_command(FMC_SDCMR_CMD_AUTO_REFRESH |
                           FMC_SDCMR_BANK_2 |
                           FMC_SDCMR_NRFS(2));

  /* Step 5: Load mode register
   * Burst length=4, sequential, CAS=2, single write
   */
  val = FMC_SDCMR_CMD_LOAD_MODE | FMC_SDCMR_BANK_2 |
         FMC_SDCMR_MDR_BURST_LENGTH_4 |
         FMC_SDCMR_MDR_BURST_TYPE_SEQUENTIAL |
         FMC_SDCMR_MDR_CAS_LATENCY_2 |
         FMC_SDCMR_MDR_WBL_SINGLE;
  stm32_fmc_sdram_command(val);

  /* Step 6: Set refresh rate
   * 64ms / 8192 rows = 7.8125us per refresh
   * Counter = (7.8125us * 84MHz) - 20 = 656
   */
  stm32_fmc_sdram_set_refresh_rate(656);
}
