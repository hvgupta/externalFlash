#pragma once
#include "AppConfig.h"
#include "quadspi.h"
#include "stdint-gcc.h"

#if USE_FLASH

// #define MANUFACTURER_ID 0xEF
// #define DEVICE_ID 0xAA21

/// Mem size in KB
#define MEM_FLASH_SIZE 131072U  // 128MB : 128*2^(10)KB

/// page size in KB
#define MEM_PAGE_SIZE 2U  // 2 Kbyte : 1 page

/// Mem block size in KB
#define MEM_BLOCK_SIZE (64 * MEM_PAGE_SIZE)  // 128 KB: 64 pages

/// Blocks count
#define BLOCK_COUNT (MEM_FLASH_SIZE / MEM_BLOCK_SIZE)  // 1024 blocks

/// Sector count
#define PAGE_COUNT (MEM_FLASH_SIZE / MEM_PAGE_SIZE)  // 65536 pages

// the number of pages per block
#define PAGE_PER_BLOCK (PAGE_COUNT / BLOCK_COUNT)  // 64 pages per block

#define JEDECID_EXEPECTED 0xEFAA21

namespace Core
{
namespace Drivers
{
namespace W25N01
{

static constexpr uint8_t MANUFACTURER_ID = 0xEF;
static constexpr uint16_t DEVICE_ID      = 0xAA21;

enum OPCode : uint8_t
{
    DEVICE_RESET = 0XFF,
    JEDEC_ID     = 0x9F,

    READ_STATUS_REG  = 0x0F,
    WRITE_STATUS_REG = 0x01,

    WRITE_ENABLE  = 0x06,
    WRITE_DISABLE = 0x04,

    BAD_BLOCK_MANAGEMENT  = 0xA1,
    READ_BBM_LUT          = 0xA5,
    LAST_ECC_FAILURE_ADDR = 0xA9,

    BLOCK_ERASE = 0xD8,

    PROGRAM_EXECUTE               = 0x10,
    QUAD_LOAD_PROGRAM_DATA        = 0x32,
    RANDOM_QUAD_LOAD_PROGRAM_DATA = 0x34,

    PAGE_DATA_READ               = 0x13,
    FAST_READ_DUAL_OUTPUT        = 0x3B,
    FAST_READ_DUAL_OUTPUT_4_BYTE = 0x3C,

    FAST_READ_DUAL_IO        = 0xBB,
    FAST_READ_DUAL_IO_4_BYTE = 0xBC
};

enum RegisterAddress : uint8_t
{
    PROTECT_REGISTER       = 0xA0,
    CONFIGURATION_REGISTER = 0xB0,
    STATUS_REGISTER        = 0xC0
};

class Manager
{
   public:
    enum class State
    {
        OK          = 0,  ///< Chip OK - Execution fine
        BUSY        = 1,  ///< Chip busy
        PARAM_ERR   = 2,  ///< Function parameters error
        CHIP_ERR    = 3,  ///< Chip error
        SPI_ERR     = 4,  ///< SPI Bus err
        CHIP_IGNORE = 5,  ///< Chip ignore state
    };

    Manager(uint16_t subsections = 1);

    State WriteStatusReg(uint8_t data, RegisterAddress reg_addr);
    State ReadStatusReg(uint8_t *buffer, RegisterAddress reg_addr);

    State WriteMemory(uint16_t block, uint16_t page, uint16_t startByte, uint8_t *data, uint32_t size); /*TO DO: requires replacement*/
    State WriteMemory(uint16_t blockNumber, uint8_t *data, uint32_t size);

    State ReadMemory(uint16_t block, uint16_t page, uint16_t startByte, uint8_t *buffer, uint32_t size);

    State EraseRange(uint32_t start_addr, uint32_t end_addr); /*TO DO: requires replacement*/
    State EraseBlock(uint32_t blockNUM);
    State EraseChip();

    State BB_LUT(uint8_t buffer[]);

    uint16_t getLast_ECC_page_failure();

    bool CheckAddress(uint16_t block, uint16_t page, uint16_t startByte);

   private:
    State status;  // the current state of the chip

    const int subsections;           // divides up the 1024 blocks, Right now does not do anything
    uint32_t nextAddr[BLOCK_COUNT];  // gives the next byte

    State WriteEnable();
    State WriteDisable();

    uint32_t get_JEDECID();

    State BB_management(); /*#TO DO*/

    State SetBuffer(bool state);
};

uint32_t calcAddress(uint16_t block, uint16_t page, uint16_t byte);

bool isBusy();

};  // namespace W25N01
};  // namespace Drivers
};  // namespace Core

#endif