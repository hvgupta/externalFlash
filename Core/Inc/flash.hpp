#pragma once
#include "AppConfig.h"
#include "quadspi.h"
#include "stdint-gcc.h"

#if USE_FLASH

// #define MANUFACTURER_ID 0xEF
// #define DEVICE_ID 0xAA21

/// Mem size in KB
#define FLASH_SIZE_BYTE 131072U  // 128MB : 128*2^(10)KB

/// page size in KB
#define PAGE_SIZE_KBYTE 2U  // 2 Kbyte : 1 page

/// Mem block size in KB
#define BLOCK_SIZE_KBYTE (64 * PAGE_SIZE_KBYTE)  // 128 KB: 64 pages

/// Blocks count
#define BLOCK_COUNT (FLASH_SIZE_BYTE / BLOCK_SIZE_KBYTE)  // 1023 blocks + 1 resevered
#define RESERVE_BLOCK_BLOCKADDR 1023

/// Sector count
#define PAGE_COUNT (FLASH_SIZE_BYTE / PAGE_SIZE_KBYTE)  // 65536 pages

// the number of pages per block
#define PAGE_PER_BLOCK (PAGE_COUNT / BLOCK_COUNT)  // 64 pages per block

#define PAGE_SIZE_BYTE 2048
#define ECC_SIZE_BYTE 64

#define JEDECID_EXEPECTED 0xEFAA21

#define blockAddrFilter(A) (A & 0xFFC0000) >> 18
#define pageAddrFilter(A) (A & 0x3F000) >> 12
#define byteAddrFilter(A) (A & 0x7FF)

// #define ADDR_STORE_START(blockNUM) (RESERVE_BLOCK_BLOCKADDR << 18 | (PAGE_COUNT - 2) << 12 | blockNUM * 4)

namespace Core
{
namespace Drivers
{
namespace W25N01
{

inline uint32_t ADDR_STORE_START(uint16_t blockNUM);

static constexpr uint8_t MANUFACTURER_ID = 0xEF;
static constexpr uint16_t DEVICE_ID      = 0xAA21;

constexpr uint8_t maxDataSize = 40;

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
        OK        = 0,  ///< Chip OK - Execution fine
        PARAM_ERR = 1,  ///< Function parameters error
        ECC_ERR   = 2,  ///< ECC error
        QSPI_ERR  = 3   ///< SPI Bus err
    };

    Manager(uint16_t subsections = 1);

    State WriteStatusReg(uint8_t data, RegisterAddress reg_addr) const;    // can be literal
    State ReadStatusReg(uint8_t *buffer, RegisterAddress reg_addr) const;  // has to be a refernce

    State reWrite_WithinBlock(uint32_t address, uint8_t *data, uint16_t size); /*TO DO: requires replacement*/
    State WriteMemory(uint16_t blockNumber, uint8_t *data, uint16_t size);     // can be a string/array

    State ReadMemory(uint32_t address, uint8_t *buffer, uint16_t size) const;  // has to be an array

    State EraseRange_WithinBlock(uint32_t start_addr, uint32_t end_addr); /*TO DO: requires replacement*/
    State EraseBlock(uint32_t blockNUM = RESERVE_BLOCK_BLOCKADDR, bool canSaveAddr = true);
    State EraseChip();

    State BB_LUT(uint8_t *buffer) const;

    State getLast_ECC_page_failure(uint32_t &buffer) const;

    bool PassAddressCheck(uint32_t address) const;

    State SetWritePin(bool state) const;

    State init();

    bool PassLegalCheck(uint16_t block, uint16_t size, uint16_t &allowedSize) const;

    uint32_t get_JEDECID() const;

    State BB_management(); /*#TO DO*/

   private:
    State status;  // the current state of the chip

    const int subsections;           // divides up the 1024 blocks, Right now does not do anything
    uint32_t nextAddr[BLOCK_COUNT];  // gives the next byte
    const uint16_t reservedBlock;
    bool sudoMode;

    void setSudoMode(bool mode);

    State WriteEnable() const;
    State WriteDisable() const;

    State BB_Entry(const uint16_t &badBlockAddr, const uint16_t &goodBlockAddr) const;

    State SetBuffer(bool state) const;

    void incrementAddr(uint16_t blockNum, uint16_t size);
    inline uint16_t pageAligned_calcAddress(uint16_t block, uint16_t page) const;

    void saveAddr();
};

inline uint32_t calcAddress(uint16_t block, uint16_t page, uint16_t byte);
// inline uint16_t calcAddress(uint16_t block, uint16_t page);

bool isBusy();

};  // namespace W25N01
};  // namespace Drivers
};  // namespace Core

#endif