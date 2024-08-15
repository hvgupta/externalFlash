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

namespace Core
{
namespace Drivers
{
namespace W25N01
{
/**
 * @brief  Calculate the address to save the last address of the non-reserved blocks
 * @param  blockNUM: the non-reserved block number whos last address is to be saved
 */
inline uint32_t ADDR_STORE_START(uint16_t blockNUM);

static constexpr uint8_t MANUFACTURER_ID = 0xEF;
static constexpr uint16_t DEVICE_ID      = 0xAA21;

/**
 * @brief  The opcodes for the W25N01 which are used in this driver, there are more opcodes available in the datasheet
 * @note   The opcodes are used to send commands to the external flash commands in quadspi.h
 */
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

/**
 * @brief  The addresses of the registers in the W25N01
 * @note   The addresses for the setting registers in the W25N01
 */
enum RegisterAddress : uint8_t
{
    PROTECT_REGISTER       = 0xA0,
    CONFIGURATION_REGISTER = 0xB0,
    STATUS_REGISTER        = 0xC0
};

/**
 * @brief The class that manages the W25N01 external memory, all the API commands are called from this function
 * @param subsections: the number of subsections that the memory is divided into (not implemented)
 * @param nextAddr: The address at which data can be written for each block
 * @param reservedBlock: The block number that is reserved for replacement commands
 * @param sudoMode: This mode is only for the replacement commands and is managed by the class
 * @param isInited: This is to check if the `init` function has been called
 */
class Manager
{
   public:
    /**
     * @brief The state of the chip, this is returned by all the functions to indicate the state of the chip
     */
    enum class State
    {
        OK              = 0,  ///< Chip OK - Execution fine
        PARAM_ERR       = 1,  ///< Function parameters error
        ECC_ERR         = 2,  ///< ECC error
        QSPI_ERR        = 3,  ///< SPI Bus err
        OBJECT_NOT_INIT = 4
    };

    /**
     * @brief The constructor for the W25N01 class
     * @param subsections: the number of subsections that the memory is divided into (not implemented)
     */
    Manager(uint16_t subsections = 1);
    /**
     * @brief This function is responsible to write the setting into the setting register
     * @param reg_addr: The address of the setting register to be written.  `RegisterAddress` can be used here
     * @param data: The data to be written to the register
     */
    State WriteStatusReg(RegisterAddress reg_addr, uint8_t data) const;

    /**
     * @brief This function is responsible to read the setting from the setting register
     * @param reg_addr: The address of the setting register to be read.  `RegisterAddress` can be used here
     * @param buffer: The buffer to store the data read from the setting register
     */
    State ReadStatusReg(RegisterAddress reg_addr, uint8_t *buffer) const;

    /**
     * @brief This function is responsible for writing data to the memory within the block `blockNumber`
     * @note Only the block number needs to be provided, and the data will be written to the next available address in the block
     * @attention the size of the `data` should match `size`
     * @param blockNumber: The block number to which the data is to be written
     * @param data: The buffer with the data to be written
     * @param size: The size of the data to be written
     */
    State WriteMemory(uint16_t blockNumber, uint8_t *data, uint16_t size);
    /**
     * @brief This function is responsible for writing data to the memory within the block `blockNumber`, this is more flexible and allows the user to
     * write to any address
     * @note re-writing over already existing data is very expensive, and should be avoided
     * @note if address is skipped, then it will be treated as written region and has to be re-written to save data there
     * @attention the size of the `data` should match `size`
     * @param address: The address to which the data is to be written, `calcAddress` can be used to calculate the address
     * @param data: The buffer with the data to be written
     * @param size: The size of the data to be written
     */
    State reWrite_WithinBlock(uint32_t address, uint8_t *data, uint16_t size);

    /**
     * @brief This function is responsible for reading data from the memory
     * @attention the size of the `buffer` should match `size`
     * @param address: The address from which the data is to be read, `calcAddress` can be used to calculate the address
     * @param buffer: The buffer to store the data read from the memory
     * @param size: The size of the data to be read
     */
    State ReadMemory(uint32_t address, uint8_t *buffer, uint16_t size) const;

    /**
     * @brief This function is responsible for erasing the block `blockNUM`
     * @param blockNUM: The block number to be erased, if not provided
     * @param canSaveAddr: If the address of the block is to be saved, this is to avoid recursion
     */
    State EraseBlock(uint32_t blockNUM = RESERVE_BLOCK_BLOCKADDR, bool canSaveAddr = true);
    /**
     * @brief This function is responsible for erasing the range of memory from `start_addr` to `end_addr`, can only erase contigious space withing a
     * block
     * @param start_addr: The start address of the range to be erased
     * @param end_addr: The end address of the range to be erased
     */
    State EraseRange_WithinBlock(uint32_t start_addr, uint32_t end_addr);
    /**
     * @brief This function is responsible for erasing the entire chip
     */
    State EraseChip();

    /**
     * @brief This function is responsible for reading the JEDEC ID of the memory
     * @param buffer: The buffer to store the Look Up Table data
     * @attention the size of the `buffer` should be 8
     */
    State BB_LUT(uint8_t *buffer) const;

    /**
     * @brief This function is responsible for reading the last ECC failure address
     * @param buffer: The buffer to store the last ECC failure address
     */
    State getLast_ECC_page_failure(uint32_t &buffer) const;

    /**
     * @brief This function checks if the address passed to it is legal
     * @param address: The address to be checked
     */
    bool PassAddressCheck(uint32_t address) const;

    /**
     * @brief This function is responsible for setting the write pin setting
     * @param state: The state of the write pin, `True` to allow, `False` to disallow
     */
    State SetWritePin(bool state) const;

    /**
     * @brief This function is responsible for setting the basic settings and initializing the `nextAddr` array
     */
    State init();

    /**
     * @brief This function is responsible for getting the JEDEC ID of the memory
     */
    uint32_t get_JEDECID() const;

    /**
     * @brief This function is responsible for finding out the bad blocks (still needs to be tested on a new chip)
     */
    State BB_management(); /*#TO DO*/

   private:
    const int subsections;           // divides up the 1024 blocks, Right now does not do anything
    uint32_t nextAddr[BLOCK_COUNT];  // gives the next byte
    const uint16_t reservedBlock;
    bool sudoMode;
    bool isInited;

    /**
     * @brief This function is called when the write command exceeds a single page
     * @param block: The block number to within which the data is to be written
     * @param size: The size of the data to be written
     * @param allowedSize: the functions writes the allowed size in this variable
     */
    bool PassLegalCheck(uint16_t block, uint16_t size, uint16_t &allowedSize) const;

    /**
     * @brief This function is responsible to go into sudo mode, and only in this mode can edit the reserve block
     */
    void setSudoMode(bool mode);

    /**
     * @brief sets the write enable latch
     */
    State WriteEnable() const;
    /**
     * @brief resets the write enable latch
     */
    State WriteDisable() const;
    /**
     * @brief repsonsible for filling the bad block look up table
     * @param badBlockAddr: The address of the bad block
     * @param goodBlockAddr: The address of the good block
     */
    State BB_Entry(const uint16_t &badBlockAddr, const uint16_t &goodBlockAddr) const;

    /**
     * @brief This function is responsible for setting the buffer mode
     */
    State SetBufferMode(bool state) const;

    /**
     * @brief This function is responsible for incrementing the address for write functions
     * @param blockNum: The block number to which the data is written
     * @param size: The size of the data written
     */
    void incrementAddr(uint16_t blockNum, uint16_t size);

    /**
     * @brief This function is responsible for calculating the address of a page
     * @param block: The block number
     * @param page: The page number
     */
    inline uint16_t pageAligned_calcAddress(uint16_t block, uint16_t page) const;

    /**
     * @brief This function is responsible for saving the last address of each block in the reversed block
     */
    void saveAddr();
};

inline uint32_t calcAddress(uint16_t block, uint16_t page, uint16_t byte);

bool isBusy();

};  // namespace W25N01
};  // namespace Drivers
};  // namespace Core

#endif