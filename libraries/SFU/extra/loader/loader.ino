/*
  Portenta C33 - 2nd stage bootloader

  The sketch checks for available update files on QSPI system partion
  or SD card and try to update the Sketch

  This example code is in the public domain.
*/
#include <BlockDevice.h>
#include <MBRBlockDevice.h>
#include <SDCardBlockDevice.h>
#include <CodeFlashBlockDevice.h>
#include <FATFileSystem.h>
#include "ota.h"

/* Key code for writing PRCR register. */
#define BSP_PRV_PRCR_KEY                              (0xA500U)
#define BSP_PRV_PRCR_PRC1_UNLOCK                      ((BSP_PRV_PRCR_KEY) | 0x2U)
#define BSP_PRV_PRCR_LOCK                             ((BSP_PRV_PRCR_KEY) | 0x0U)

#define BSP_PRV_BITS_PER_WORD                         (32)

#define SKETCH_FLASH_OFFSET                           (64 * 1024)

#define FULL_UPDATE_FILE_PATH                         "/ota/UPDATE.BIN"
#define FULL_UPDATE_FILE_PATH_COMPRESSED              FULL_UPDATE_FILE_PATH ".LZSS"

#define VERSION                                       1

SDCardBlockDevice sd(PIN_SDHI_CLK, PIN_SDHI_CMD, PIN_SDHI_D0, PIN_SDHI_D1, PIN_SDHI_D2, PIN_SDHI_D3, PIN_SDHI_CD, PIN_SDHI_WP);
BlockDevice* qspi = BlockDevice::get_default_instance();
CodeFlashBlockDevice& cf = CodeFlashBlockDevice::getInstance();
MBRBlockDevice sys_bd(qspi, 2);
FATFileSystem sys_fs("ota");

void setup() {
  Serial.begin(9600);
  while(!Serial) {}
  Serial.print("SFU version ");Serial.println(VERSION);

  qspi->init();
  int err =  sys_fs.mount(&sys_bd);
  if (!err) {
    debug_if(OTA_DBG, "Try QSPI OTA");
    try_ota();
  }

  sd.init();
  err = sys_fs.mount(&sd);
  if (!err) {
    debug_if(OTA_DBG, "Try SD OTA");
    try_ota();
  }
  
  int app_valid = (((*(uint32_t *) SKETCH_FLASH_OFFSET + POST_APPLICATION_ADDR) & 0xFF000000) == 0x20000000);
  if (app_valid) {
    debug_if(OTA_DBG, "Booting application @ 0x%x", SKETCH_FLASH_OFFSET + POST_APPLICATION_ADDR);
    boot5(SKETCH_FLASH_OFFSET + POST_APPLICATION_ADDR);
  } else {
    debug_if(OTA_DBG, "Application not found");
  }
}

void loop() {

}

void boot5(uint32_t address) {

  R_SYSTEM->PRCR = (uint16_t) BSP_PRV_PRCR_PRC1_UNLOCK;
  R_BSP_MODULE_STOP(FSP_IP_USBFS, 0);
  R_BSP_MODULE_STOP(FSP_IP_USBHS, 0);
  R_SYSTEM->PRCR = (uint16_t) BSP_PRV_PRCR_LOCK;

  /* Disable MSP monitoring. */
#if BSP_FEATURE_TZ_HAS_TRUSTZONE
    __set_MSPLIM(0);
#else
    R_MPU_SPMON->SP[0].CTL = 0;
#endif

  __disable_irq(); // Note: remember to enable IRQ in application
  __DSB();
  __ISB();

  // Disable SysTick
  SysTick->CTRL = 0;

  SCB->VTOR  = address;

  // Cleanup NVIC
  for (size_t i = 0U; i < BSP_ICU_VECTOR_MAX_ENTRIES / BSP_PRV_BITS_PER_WORD; i++)
  {
    NVIC->ICER[i] = 0xFF;
  }

  uint32_t mainStackPointer = *(volatile uint32_t *)(address);
  __set_MSP(mainStackPointer);
  uint32_t programResetHandlerAddress = *(volatile uint32_t *) (address + 4);
  void (* programResetHandler)(void) = (void (*)(void)) programResetHandlerAddress;
  programResetHandler();
}

int try_ota(void) {
  FILE *update_file;
  FILE *target_file;

  update_file = fopen(FULL_UPDATE_FILE_PATH_COMPRESSED, "rb");
  if (update_file) {
    debug_if(OTA_DBG, "Compressed update file found");
    target_file = fopen(FULL_UPDATE_FILE_PATH, "wb");
    if (!target_file) {
      debug_if(OTA_DBG, "Error creating file for decompression");
      return -1;
    }
    int err = verify_decompress(update_file, target_file);
    fclose(update_file);
    fclose(target_file);
    remove(FULL_UPDATE_FILE_PATH_COMPRESSED);
    remove(FULL_UPDATE_FILE_PATH);
  } else {
    debug_if(OTA_DBG, "Compressed update file not found");
  }

  update_file = fopen(FULL_UPDATE_FILE_PATH, "rb");
  if (update_file) {
    debug_if(OTA_DBG, "Update file found");
    int err = verify_flash(update_file);
    fclose(update_file);
    remove(FULL_UPDATE_FILE_PATH);
  } else {
    debug_if(OTA_DBG, "Update file not found");
  }
}

int verify_decompress(FILE* update_file, FILE* target_file) {
  int err = verify_header(update_file);
  if (err) {
    debug_if(OTA_DBG, "Error during header verification: %d", err);
    return err;
  }
  err = decompress(update_file, target_file, nullptr);
  return err;
}

int verify_flash(FILE* file) {
  int err = verify_sketch(file);
  if (err != 0) {
    debug_if(OTA_DBG, "Error during sketch verification: %d", err);
    return err;
  }
  err = flash(file, &cf, SKETCH_FLASH_OFFSET + POST_APPLICATION_ADDR);
  return err;
}
