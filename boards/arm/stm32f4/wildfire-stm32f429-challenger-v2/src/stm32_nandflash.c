/****************************************************************************
 * boards/arm/stm32f4/wildfire-stm32f429-challenger-v2/src/stm32_nandflash.c
 *
 * W29N01GVSIAA NAND Flash driver using the FMC hardware NAND controller.
 *
 * The FMC hardware controller drives the CE#/CLE/ALE/RE#/WE#/D0-D7 signals
 * and automatically arbitrates the shared D0-D7 data bus with SDRAM, so
 * NAND and SDRAM can be used concurrently (unlike a GPIO bit-bang driver).
 *
 * R/B# is wired to PB13 (NOT the FMC NWAIT/PD6 pin), so the FMC wait
 * feature (PWAITEN) is disabled and R/B# is polled in software.
 *
 ****************************************************************************/

#include <nuttx/config.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/arch.h>
#include <nuttx/mtd/nand.h>
#include <nuttx/mtd/nand_raw.h>
#include <nuttx/mtd/nand_model.h>
#include <nuttx/mtd/nand_scheme.h>

#include <arch/board/board.h>
#include "arm_internal.h"
#include "stm32.h"
#include "stm32_gpio.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define W29N_PAGE_SIZE      2048
#define W29N_SPARE_SIZE     64
#define W29N_PAGES_PER_BLK  64
#define W29N_BLOCK_SIZE     (W29N_PAGE_SIZE * W29N_PAGES_PER_BLK)
#define W29N_TOTAL_BLOCKS   1024
#define W29N_DEVICE_SIZE    128

/* FMC NAND Bank 3 memory-mapped addresses.
 * Per STM32F429 reference manual (RM0090) Table 13 (FMC memory map):
 *   NAND Bank 2 base = 0x70000000 (NCE2 = PC2)
 *   NAND Bank 3 base = 0x80000000 (NCE3 = PG9)  <-- this board uses PG9
 *   PC Card Bank 4  = 0x90000000 (NOT NAND -- accessing this faults)
 * A16 = CLE, A17 = ALE.
 */

#define NAND_BANK_BASE    0x80000000u
#define NAND_CMD_ADDR     (NAND_BANK_BASE | (1u << 16))   /* A16 -> CLE */
#define NAND_ADDR_ADDR    (NAND_BANK_BASE | (1u << 17))   /* A17 -> ALE */
#define NAND_DATA_ADDR    NAND_BANK_BASE

/* R/B# is on PB13 (NOT FMC NWAIT/PD6).  Polled in software. */

#define NAND_RB_PIN       (GPIO_PORTB | GPIO_PIN13)

/* FMC NAND timing (HCLK = 180 MHz, 1 cycle ~= 5.56 ns).
 * W29N01GV: tWP/tRP = 25ns, tDH = 15ns, tWH = 10ns, tCLR/tAR = 10ns.
 *
 * IMPORTANT: Per NuttX stm32_fmc.h, PBKEN is bit 2 (NOT bit 1).
 * Bit 1 is PWAITEN.  Setting bit 1 instead of bit 2 was the root cause
 * of the precise bus fault at 0x80000000 — the bank was never enabled.
 *
 * PCR: PBKEN, 8-bit NAND, no wait, TCLR=2, TAR=2
 * PMEM: MEMSET=2, MEMWAIT=5, MEMHOLD=3, MEMHIZ=2
 */

#define NAND_PCR3_VAL     (FMC_PCR_PBKEN | FMC_PCR_PTYP | FMC_PCR_PWID8 | \
                           FMC_PCR_TCLR(2) | FMC_PCR_TAR(2))

/* PMEM3: common memory (data) timing.  HCLK = 180 MHz, 1 cycle ~= 5.56 ns.
 * W29N01GV: tRP/tWP = 25ns, tDH = 15ns, tWH = 10ns.
 *   MEMSET  = 2  (setup,   ~11ns)
 *   MEMWAIT = 5  (data access, ~28ns)
 *   MEMHOLD = 3  (hold,   ~17ns)
 *   MEMHIZ  = 2  (HiZ,    ~11ns)
 */

#define NAND_PMEM_VAL     (FMC_PMEM_MEMSET(2) | FMC_PMEM_MEMWAIT(5) | \
                           FMC_PMEM_MEMHOLD(3) | FMC_PMEM_MEMHIZ(2))

/* PATT3: attribute memory (cmd/addr) timing.  CLE/ALE cycles are short --
 * the NAND chip only needs the CLE/ALE signal asserted long enough to
 * latch the byte (~tCS = 10ns).  Use a shorter wait than PMEM3 so that
 * the FMC does not stretch the CLE/ALE pulse and cause the subsequent
 * data access to miss the bus handshake window.
 */

#define NAND_PATT_VAL     (FMC_PATT_ATTSET(2) | FMC_PATT_ATTWAIT(4) | \
                           FMC_PATT_ATTHOLD(2) | FMC_PATT_ATTHIZ(2))

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct w29n_dev_s
{
  struct nand_raw_s raw;
  FAR struct mtd_dev_s *mtd;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct w29n_dev_s g_w29n_dev;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/* Memory-mapped access helpers.  The FMC hardware drives the bus signals. */

static inline void nand_cmd(uint8_t c)
{
  *(volatile uint8_t *)NAND_CMD_ADDR = c;
}

static inline void nand_addr(uint8_t a)
{
  *(volatile uint8_t *)NAND_ADDR_ADDR = a;
}

static inline uint8_t nand_dataread(void)
{
  return *(volatile uint8_t *)NAND_DATA_ADDR;
}

static inline void nand_datawrite(uint8_t d)
{
  *(volatile uint8_t *)NAND_DATA_ADDR = d;
}

/* Wait for R/B# (PB13) to go high = ready. */

static int nand_wait_ready(void)
{
  int timeout = 2000000;
  while (timeout-- > 0)
    {
      if (stm32_gpioread(NAND_RB_PIN))
        {
          return OK;
        }
    }

  return -ETIMEDOUT;
}

/* Configure the FMC NAND Bank 3 controller. */

void nand_fmc_init(void)
{
  /* Configure NAND control pins as FMC alternate function (AF12).
   * D0-D7 are shared with SDRAM and are already configured by
   * stm32_sdram_initialize(); the FMC hardware arbitrates the bus.
   */

  stm32_configgpio(GPIO_FMC_NCE3_0 | GPIO_SPEED_100MHz);  /* PG9  CE#  */
  stm32_configgpio(GPIO_FMC_A16_0  | GPIO_SPEED_100MHz);  /* PD11 CLE  */
  stm32_configgpio(GPIO_FMC_A17_0  | GPIO_SPEED_100MHz);  /* PD12 ALE  */
  stm32_configgpio(GPIO_FMC_NOE_0  | GPIO_SPEED_100MHz);  /* PD4  RE#  */
  stm32_configgpio(GPIO_FMC_NWE_0  | GPIO_SPEED_100MHz);  /* PD5  WE#  */

  /* R/B# on PB13 as GPIO input with pull-up */

  stm32_configgpio(GPIO_INPUT | GPIO_PULLUP | NAND_RB_PIN);

  /* Enable FMC clock (also used by SDRAM) */

  stm32_fmc_enable();

  /* Disable Bank1 Region4 (NOR/PSRAM).  In the STM32F429 FMC map the
   * 0x90000000-0x9FFFFFFF range is shared between Bank1 Region4 and
   * NAND Bank3; make sure Region4 doesn't claim it.
   */
  putreg32(0, STM32_FMC_BCR4);

  /* Disable NAND bank 3 before configuring */

  putreg32(0, STM32_FMC_PCR3);

  /* Common memory (data) and attribute memory (cmd/addr) timing.
   * PATT3 must use shorter wait cycles than PMEM3 -- the CLE/ALE pulse
   * only needs to latch the byte, not drive the data bus.
   */

  putreg32(NAND_PMEM_VAL, STM32_FMC_PMEM3);
  putreg32(NAND_PATT_VAL, STM32_FMC_PATT3);

  /* Enable NAND bank 3: 8-bit, NAND type, no NWAIT wait feature */

  putreg32(NAND_PCR3_VAL, STM32_FMC_PCR3);

  /* Ensure the FMC configuration is committed before any NAND access */

  __asm__ volatile ("dsb" ::: "memory");
  up_udelay(100);

  syslog(LOG_INFO, "NAND: PCR3=0x%08lx SR3=0x%08lx PMEM3=0x%08lx PATT3=0x%08lx\n",
         (unsigned long)getreg32(STM32_FMC_PCR3),
         (unsigned long)getreg32(STM32_FMC_SR3),
         (unsigned long)getreg32(STM32_FMC_PMEM3),
         (unsigned long)getreg32(STM32_FMC_PATT3));
  syslog(LOG_INFO, "NAND: BCR4=0x%08lx PCR4=0x%08lx SDCR1=0x%08lx\n",
         (unsigned long)getreg32(STM32_FMC_BCR4),
         (unsigned long)getreg32(STM32_FMC_PCR4),
         (unsigned long)getreg32(STM32_FMC_SDCR1));
}

static int nand_readid(uint32_t *id)
{
  uint8_t m, d, id3, id4;

  syslog(LOG_INFO, "NAND: readid start\n");

  /* Test: read common memory space (0x90000000) before any command */

  volatile uint8_t test = nand_dataread();
  syslog(LOG_INFO, "NAND: dataread test=0x%02x\n", test);

  /* Reset first */

  nand_cmd(0xff);
  syslog(LOG_INFO, "NAND: cmd 0xff done\n");
  nand_wait_ready();

  /* Read ID command */

  nand_cmd(0x90);
  nand_addr(0x00);

  /* tWHR delay before the first data read */

  up_udelay(1);

  m   = nand_dataread();
  d   = nand_dataread();
  id3 = nand_dataread();
  id4 = nand_dataread();

  *id = (uint32_t)m | ((uint32_t)d << 8);

  syslog(LOG_INFO, "NAND: ID=%02x %02x %02x %02x\n", m, d, id3, id4);

  return (d == 0xf1) ? OK : -ENODEV;
}

static int w29n_eraseblock(FAR struct nand_raw_s *raw, off_t block)
{
  uint32_t row = (uint32_t)block * W29N_PAGES_PER_BLK;

  /* W29N01GV (1Gbit) erase: 2 row address cycles (16-bit row) */

  nand_cmd(0x60);
  nand_addr((uint8_t)(row & 0xff));
  nand_addr((uint8_t)((row >> 8) & 0xff));
  nand_cmd(0xd0);

  /* tWB: R/B# takes ~100ns to go low after the 0xd0 erase confirm.
   * Without this delay nand_wait_ready() returns immediately (R/B# still
   * high) and erase never actually happens.  Erase takes 2-10ms.
   */

  up_udelay(1);

  return nand_wait_ready();
}

static int w29n_rawread(FAR struct nand_raw_s *raw, off_t block,
                        unsigned int page, FAR void *data, FAR void *spare)
{
  uint32_t row = (uint32_t)block * W29N_PAGES_PER_BLK + page;
  FAR uint8_t *d = (FAR uint8_t *)data;
  FAR uint8_t *s = (FAR uint8_t *)spare;
  int i;

  /* For large-page NAND, when only spare is requested (data=NULL), set the
   * column address to the spare area start (offset 2048 = 0x0800).
   * When both data and spare are requested, start at column 0 and read
   * sequentially (2048 bytes data + 64 bytes spare).
   */

  /* W29N01GV (1Gbit, 65536 pages) needs 4 address cycles: 2 column + 2 row.
   * Row = block * 64 + page, fits in 16 bits (max 65535).
   */

  nand_cmd(0x00);
  if (d == NULL && s != NULL)
    {
      nand_addr(0x00);                                /* column 2048 low byte */
      nand_addr(0x08);                                /* column 2048 high byte */
    }
  else
    {
      nand_addr(0x00);                                /* column byte 0 */
      nand_addr(0x00);                                /* column byte 1 */
    }
  nand_addr((uint8_t)(row & 0xff));
  nand_addr((uint8_t)((row >> 8) & 0xff));
  nand_cmd(0x30);

  /* tWHR: the FMC NAND controller needs ~60ns after the confirm command
   * (0x30) before it will route data reads to the data register instead of
   * echoing the command byte.  nand_wait_ready() alone is NOT enough --
   * during the tWHR window the controller leaks the 0x30 byte onto the
   * data bus, producing the "30 30 30 ..." symptom.
   */

  up_udelay(1);

  if (nand_wait_ready() < 0)
    {
      return -ETIMEDOUT;
    }

  /* The FMC arbitrates D0-D7 with SDRAM, so the caller's buffer may live
   * in SDRAM; no need for an intermediate SRAM buffer.
   */

  if (d)
    {
      for (i = 0; i < W29N_PAGE_SIZE; i++)
        {
          d[i] = nand_dataread();
        }
    }

  if (s)
    {
      for (i = 0; i < W29N_SPARE_SIZE; i++)
        {
          s[i] = nand_dataread();
        }
    }

  return OK;
}

static int w29n_rawwrite(FAR struct nand_raw_s *raw, off_t block,
                         unsigned int page, FAR const void *data,
                         FAR const void *spare)
{
  uint32_t row = (uint32_t)block * W29N_PAGES_PER_BLK + page;
  FAR const uint8_t *d = (FAR const uint8_t *)data;
  FAR const uint8_t *s = (FAR const uint8_t *)spare;
  int i;

  nand_cmd(0x80);
  if (d == NULL && s != NULL)
    {
      nand_addr(0x00);                                /* column 2048 low byte */
      nand_addr(0x08);                                /* column 2048 high byte */
    }
  else
    {
      nand_addr(0x00);                                /* column byte 0 */
      nand_addr(0x00);                                /* column byte 1 */
    }
  nand_addr((uint8_t)(row & 0xff));
  nand_addr((uint8_t)((row >> 8) & 0xff));

  if (d)
    {
      for (i = 0; i < W29N_PAGE_SIZE; i++)
        {
          nand_datawrite(d[i]);
        }
    }

  if (s)
    {
      for (i = 0; i < W29N_SPARE_SIZE; i++)
        {
          nand_datawrite(s[i]);
        }
    }

  nand_cmd(0x10);

  /* tWB: after the 0x10 program confirm command, R/B# takes ~100ns to go
   * low (busy).  If we poll immediately, nand_wait_ready() sees R/B# still
   * high and returns OK WITHOUT waiting for the ~200us-700us program cycle
   * to finish -- so data is never actually written.  Delay 1us so R/B# has
   * gone low before we start polling for ready.
   */

  up_udelay(1);

  return nand_wait_ready();
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int stm32_nandflash_initialize(void)
{
  FAR struct w29n_dev_s *priv = &g_w29n_dev;
  FAR struct nand_raw_s *raw = &priv->raw;
  uint32_t chipid;

  /* nand_fmc_init() was called early in stm32_boardinitialize()
   * (before SDRAM) so FMC Bank3 is configured before the SDRAM
   * controller is enabled.  D0-D7 are now AF12 (set up by SDRAM
   * init), so NAND data access works.
   */

  if (nand_readid(&chipid) < 0)
    {
      syslog(LOG_WARNING, "NAND: Not detected\n");
      return OK;
    }

  memset(raw, 0, sizeof(*raw));
  raw->model.devid     = 0xf1;
  raw->model.options   = NANDMODEL_DATAWIDTH8;
  raw->model.pagesize  = W29N_PAGE_SIZE;
  raw->model.sparesize = W29N_SPARE_SIZE;
  raw->model.devsize   = W29N_DEVICE_SIZE;
  raw->model.blocksize = W29N_BLOCK_SIZE >> 10;
  raw->model.scheme    = &g_nand_sparescheme2048;
  raw->ecctype    = NANDECC_NONE;
  raw->eraseblock = w29n_eraseblock;
  raw->rawread    = w29n_rawread;
  raw->rawwrite   = w29n_rawwrite;

  priv->mtd = nand_raw_initialize(raw);
  if (!priv->mtd)
    {
      return -ENODEV;
    }

  syslog(LOG_INFO, "NAND: W29N01GV %uMB OK (FMC)\n", W29N_DEVICE_SIZE);

#ifdef CONFIG_WILDFIRE_CHALLENGER_V2_NAND_PART
  {
    FAR struct mtd_dev_s *p;
    struct mtd_geometry_s geo;
    off_t total_blocks;
    off_t part0_blocks = 512;  /* 512 pages = 1MB (8 erase blocks) */
    int sret;

    priv->mtd->ioctl(priv->mtd, MTDIOC_GEOMETRY,
                     (unsigned long)((uintptr_t)&geo));
    total_blocks = geo.neraseblocks * (geo.erasesize / geo.blocksize);

    /* Partition 0: config area */

    p = mtd_partition(priv->mtd, 0, part0_blocks);
    if (p)
      {
        syslog(LOG_INFO, "NAND: Partition 0 created (%lu pages)\n",
               (unsigned long)part0_blocks);
        sret = smart_initialize(0, p, NULL);
        if (sret < 0)
          {
            syslog(LOG_ERR, "NAND: smart_initialize(0) failed: %d\n", sret);
          }
        else
          {
            syslog(LOG_INFO, "NAND: /dev/smart0 ready\n");
          }
      }
    else
      {
        syslog(LOG_ERR, "NAND: Failed to create partition 0\n");
      }

    /* Partition 1: remaining data area.
     * FMC hardware is fast enough (~5s for the full 127MB scan) and
     * arbitrates the bus with SDRAM, so both partitions can be brought
     * up synchronously here.
     */

    p = mtd_partition(priv->mtd, part0_blocks, total_blocks - part0_blocks);
    if (p)
      {
        syslog(LOG_INFO, "NAND: Partition 1 created (%lu pages)\n",
               (unsigned long)(total_blocks - part0_blocks));
        sret = smart_initialize(1, p, NULL);
        if (sret < 0)
          {
            syslog(LOG_ERR, "NAND: smart_initialize(1) failed: %d\n", sret);
          }
        else
          {
            syslog(LOG_INFO, "NAND: /dev/smart1 ready\n");
          }
      }
    else
      {
        syslog(LOG_ERR, "NAND: Failed to create partition 1\n");
      }
  }
#endif

  return OK;
}
