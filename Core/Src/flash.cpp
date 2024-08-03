#include "flash.hpp"

#include <cstring>

#include "FreeRTOS.h"
#include "math.h"
#include "task.h"

namespace Core
{
namespace Drivers

{
namespace W25N01
{
inline uint16_t Manager::pageAligned_calcAddress(uint16_t block, uint16_t page) const { return block << 6 | page; }
inline uint32_t calcAddress(uint16_t block, uint16_t page, uint16_t byte) { return (block << 6 | page) << 12 | byte; }

inline uint32_t min(uint32_t a, uint32_t b) { return a < b ? a : b; }
int lastAddr = 0;

bool isBusy()
{
    uint8_t status;
    StatusReg_Rx(OPCode::READ_STATUS_REG, RegisterAddress::STATUS_REGISTER, &status);
    return (status & 0x01) || hqspi1.State != HAL_QSPI_STATE_READY || hqspi1.hdma->State != HAL_DMA_STATE_READY || hqspi1.TxXferCount != 0 ||
           hqspi1.RxXferCount != 0;
}

uint32_t Manager::get_JEDECID() const
{
    uint8_t buffer[3] = {0};
    Command_Rx_1DataLine(OPCode::JEDEC_ID, buffer, 3, 8);
    return (buffer[0] << 16) | (buffer[1] << 8) | buffer[2];
}

Manager::State Manager::SetWritePin(bool state) const
{
    uint8_t regData = 0;
    if (ReadStatusReg(&regData, RegisterAddress::STATUS_REGISTER) != State::OK)
    {
        return State::QSPI_ERR;
    }
    if ((state && (regData & 0x2)) || (!state && !(regData & 0x2))) /*skip if the buffer is already set to the desired mode*/
    {
        return State::OK;
    }
    if (state)
    {
        return WriteEnable();
    }
    return WriteDisable();
}

Manager::State Manager::WriteEnable() const
{
    while (isBusy())
        ;

    if (PureCommand(OPCode::WRITE_ENABLE) != HAL_OK)
    {
        return State::QSPI_ERR;
    }
    return State::OK;
}

Manager::State Manager::WriteDisable() const
{
    while (isBusy())
        ;
    if (PureCommand(OPCode::WRITE_DISABLE) != HAL_OK)
    {
        return State::QSPI_ERR;
    }
    return State::OK;
}

Manager::State Manager::SetBuffer(bool state) const
{
    while (isBusy())
        ;
    uint8_t regData = 0;
    if (ReadStatusReg(&regData, RegisterAddress::CONFIGURATION_REGISTER) != State::OK)
    {
        return State::QSPI_ERR;
    }
    if ((state && (regData & 0x8)) || (!state && !(regData & 0x8))) /*skip if the buffer is already set to the desired mode*/
    {
        return State::OK;
    }
    if (state)
    {
        regData |= 0x8;
    }
    else
    {
        regData &= 0xF7;
    }
    if (WriteStatusReg(regData, RegisterAddress::CONFIGURATION_REGISTER) != State::OK)
    {
        return State::QSPI_ERR;
    }
    return State::OK;
}

Manager::Manager(uint16_t subsec) : subsections(subsec), reservedBlock(RESERVE_BLOCK_BLOCK_ADDR), sudoMode(false)
{
    for (unsigned int i = 0; i < BLOCK_COUNT; i++)
    {
        nextAddr[i] = 0;
    }
}

Manager::State Manager::init() const
{
    if (PureCommand(OPCode::DEVICE_RESET) != HAL_OK)
    {
        return State::QSPI_ERR;
    }
    if (StatusReg_Tx(OPCode::WRITE_STATUS_REG, RegisterAddress::PROTECT_REGISTER, (uint8_t)0x00) != HAL_OK)
    {
        return State::QSPI_ERR;
    }

    if (StatusReg_Tx(OPCode::WRITE_STATUS_REG, RegisterAddress::CONFIGURATION_REGISTER, (uint8_t)0x18) != HAL_OK)
    {
        return State::QSPI_ERR;
    }

    if (get_JEDECID() != JEDECID_EXEPECTED)
    {
        return State::QSPI_ERR;
    }
    return State::OK;
}

bool Manager::PassLegalCheck(uint16_t block, uint16_t size, uint16_t &allowedSize) const
{
    allowedSize = size;
    if (block >= BLOCK_COUNT || (block == reservedBlock && !sudoMode))
    {
        return false;
    }
    uint16_t curPage = pageAddrFilter(nextAddr[block]);
    uint16_t curByte = byteAddrFilter(nextAddr[block]);
    if (curByte + size >= MEM_PAGE_SIZE)
    {
        allowedSize             = MEM_PAGE_SIZE_BYTE - curByte;
        uint16_t numPagesNeeded = std::ceil(((float)size - allowedSize) / ((float)MEM_PAGE_SIZE_BYTE));
        if (curPage + numPagesNeeded >= PAGE_PER_BLOCK)
        {
            return false;
        }
    }
    return true;
}

Manager::State Manager::WriteStatusReg(uint8_t data, RegisterAddress reg_addr) const
{
    while (isBusy())
        ;
    if (StatusReg_Tx(OPCode::WRITE_STATUS_REG, reg_addr, data) != HAL_OK)
    {
        return State::QSPI_ERR;
    }
    return State::OK;
}

Manager::State Manager::ReadStatusReg(uint8_t *buffer, RegisterAddress reg_addr) const
{
    if (StatusReg_Rx(OPCode::READ_STATUS_REG, reg_addr, buffer) != HAL_OK)
    {
        return State::QSPI_ERR;
    }
    return State::OK;
}

Manager::State Manager::WriteMemory(uint16_t curBlock, uint8_t *data, uint16_t size)
{
    uint32_t curAddr   = nextAddr[curBlock];
    uint16_t nextBlock = blockAddrFilter(curAddr);

    if (nextBlock)
    {
        return State::PARAM_ERR;
    }

    uint16_t curPage      = pageAddrFilter(curAddr);
    uint16_t nextByte     = byteAddrFilter(curAddr);
    uint16_t sizeWriteNow = size;

    if (!PassLegalCheck(curBlock, size, sizeWriteNow))
    {
        return State::PARAM_ERR;
    }

    while (size)
    {
        taskENTER_CRITICAL();
        if (WriteEnable() != State::OK)
        {
            taskEXIT_CRITICAL();
            return State::QSPI_ERR;
        }
        while (isBusy())
            ;
        if (Command_Tx_4DataLine(OPCode::QUAD_LOAD_PROGRAM_DATA, data, nextByte, sizeWriteNow) != HAL_OK)
        {
            taskEXIT_CRITICAL();
            return State::QSPI_ERR;
        }

        while (isBusy())
            ;
        if (BufferCommand(pageAligned_calcAddress(curBlock, curPage), OPCode::PROGRAM_EXECUTE) != HAL_OK)
        {
            taskEXIT_CRITICAL();
            return State::QSPI_ERR;
        }
        incrementAddr(curBlock, sizeWriteNow);
        taskEXIT_CRITICAL();

        size -= sizeWriteNow;
        data += sizeWriteNow;

        curPage      = pageAddrFilter(nextAddr[curBlock]);
        nextByte     = byteAddrFilter(nextAddr[curBlock]);
        sizeWriteNow = min(size, MEM_PAGE_SIZE_BYTE - nextByte);
    }

    return State::OK;
}

// Manager::State Manager::WriteMemory(uint16_t block, uint16_t page, uint16_t startByte, uint8_t *data, uint16_t size)
// {
//     if (!PassAddressCheck(block, page, startByte))
//     {
//         return State::PARAM_ERR;
//     }
//     uint32_t pageByteAddr = calcAddress(0, page, startByte);
//     if (nextAddr[block] == pageByteAddr)
//     {
//         return WriteMemory(block, data, size);
//     }
//     else if (nextAddr[block] < pageByteAddr)
//     {
//         nextAddr[block] = pageByteAddr;
//         return WriteMemory(block, data, size);
//     }

//     uint16_t bytesWritten = size;
//     if (startByte + size >= MEM_PAGE_SIZE_BYTE)
//     {
//         bytesWritten                 = MEM_PAGE_SIZE_BYTE - startByte;
//         uint16_t numberOfPagesNeeded = std::ceil(((float)size - bytesWritten) / ((float)MEM_PAGE_SIZE_BYTE));
//         if (page + numberOfPagesNeeded >= PAGE_PER_BLOCK)
//         {
//             return State::PARAM_ERR;
//         }
//     }
//     uint8_t localBuffer[2048];
// }

bool Manager::PassAddressCheck(uint32_t address) const
{
    uint16_t curBlock = blockAddrFilter(address);
    if (curBlock >= BLOCK_COUNT)
    {
        return false;
    }
    if (curBlock == reservedBlock && !sudoMode)
    {
        return false;
    }

    uint16_t curPage = pageAddrFilter(address);
    if (curPage >= PAGE_PER_BLOCK)
    {
        return false;
    }

    uint16_t curByte = byteAddrFilter(address);
    if (curByte >= MEM_PAGE_SIZE_BYTE)
    {
        return false;
    }

    return true;
}

Manager::State Manager::ReadMemory(uint32_t address, uint8_t *buffer, uint16_t size) const
{
    if (!PassAddressCheck(address))
    {
        return State::PARAM_ERR;
    }
    uint16_t curBlock    = blockAddrFilter(address);
    uint16_t curPage     = blockAddrFilter(address);
    uint16_t startByte   = byteAddrFilter(address);
    uint16_t sizeReadNow = min(size, MEM_PAGE_SIZE_BYTE - startByte);

    while (size)
    {
        taskENTER_CRITICAL();
        SetBuffer(true);
        while (isBusy())
            ;
        if (BufferCommand(pageAligned_calcAddress(curBlock, curPage), OPCode::PAGE_DATA_READ) != HAL_OK)
        {
            taskEXIT_CRITICAL();
            return State::QSPI_ERR;
        }
        while (isBusy())
            ;

        if (Command_Rx_2DataLine(OPCode::FAST_READ_DUAL_OUTPUT, buffer, startByte, sizeReadNow) != HAL_OK)
        {
            taskEXIT_CRITICAL();
            return State::QSPI_ERR;
        }
        taskEXIT_CRITICAL();
        size -= sizeReadNow;
        buffer += sizeReadNow;

        curPage += 1;
        startByte   = 0;
        sizeReadNow = min(size, MEM_PAGE_SIZE_BYTE - startByte);
    }

    return State::OK;
}

Manager::State Manager::EraseBlock(uint32_t blockNUM)
{
    if (blockNUM >= BLOCK_COUNT || (blockNUM == reservedBlock && !sudoMode))
    {
        return State::PARAM_ERR;
    }

    while (isBusy())
        ;

    if (WriteEnable() != State::OK)
    {
        return State::QSPI_ERR;
    }

    while (isBusy())
        ;

    if (BufferCommand(pageAligned_calcAddress(blockNUM, 0), OPCode::BLOCK_ERASE) != HAL_OK)
    {
        return State::QSPI_ERR;
    }

    nextAddr[blockNUM] = 0;
    return State::OK;
}

// Manager::State Manager::EraseRange() {}

Manager::State Manager::EraseChip()
{
    if (WriteEnable() != State::OK)
    {
        return State::QSPI_ERR;
    }

    for (unsigned int i = 0; i < BLOCK_COUNT; i++)
    {
        taskENTER_CRITICAL();
        if (i == reservedBlock)
        {
            setSudoMode(true);
        }
        State state = EraseBlock(i);
        switch (state)
        {
        default:
            i--;
        case State::OK:
            continue;
        }
        if (sudoMode)
        {
            sudoMode = false;
        }
        taskEXIT_CRITICAL();
    }

    return State::OK;
}

Manager::State Manager::BB_LUT(uint8_t *buffer) const
{
    while (isBusy())
        ;

    if (Command_Rx_1DataLine(OPCode::READ_BBM_LUT, buffer, 20, 8) != HAL_OK)
    {
        return State::QSPI_ERR;
    }

    return State::OK;
}

Manager::State Manager::getLast_ECC_page_failure(uint32_t &buffer) const
{
    while (isBusy())
        ;

    uint8_t info[2] = {0};
    if (Command_Rx_1DataLine(OPCode::LAST_ECC_FAILURE_ADDR, info, 2, 8) != HAL_OK)
    {
        return State::QSPI_ERR;
    }
    buffer            = info[0] << 8 | info[1];
    uint8_t ECC_Check = 0;
    if (ReadStatusReg(&ECC_Check, RegisterAddress::STATUS_REGISTER) != State::OK)
    {
        return State::QSPI_ERR;
    }
    if (ECC_Check & 64)
    {
        return State::ECC_ERR;
    }
    return State::OK;
}

Manager::State Manager::BB_management()
{
    uint8_t data[MEM_PAGE_SIZE_BYTE];
    uint8_t buffer[MEM_PAGE_SIZE_BYTE];
    for (int a = 0; a < MEM_PAGE_SIZE_BYTE; a++)
    {
        data[a] = a;
    }
    uint16_t badBlocks[20], j    = 0;
    uint16_t goodBlocks[1024], k = 0;
    for (unsigned int i = 0; i < BLOCK_COUNT; i++)
    {
        EraseBlock(i);
        WriteMemory(i, data, MEM_PAGE_SIZE_BYTE);
        uint32_t addr = 0;
        State result  = getLast_ECC_page_failure(addr);
        if (result == State::ECC_ERR && (addr >> 18) == (uint32_t)i)
        {
            badBlocks[j++] = i;
        }
        ReadMemory(calcAddress(i, 0, 0), buffer, MEM_PAGE_SIZE_BYTE);
    }
    for (unsigned int i = 0; i < j; i++)
    {
        if (BB_Entry(badBlocks[i], goodBlocks[--k]) != State::OK)
        {
            return State::QSPI_ERR;
        }
    }
    return State::OK;
}

void Manager::incrementAddr(uint16_t blockNum, uint16_t size)
{
    uint16_t pageNum  = pageAddrFilter(nextAddr[blockNum]);
    uint16_t nextByte = byteAddrFilter(nextAddr[blockNum]);
    nextByte += size;
    if (nextByte >= MEM_PAGE_SIZE_BYTE)
    {
        nextByte = 0;
        pageNum += 1;
    }
    nextAddr[blockNum] = calcAddress(0, pageNum, nextByte);
}

void Manager::setSudoMode(bool mode) { sudoMode = mode; }

}  // namespace W25N01
}  // namespace Drivers
}  // namespace Core