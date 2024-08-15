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
uint8_t localBuffer[PAGE_SIZE_BYTE];

inline uint16_t Manager::pageAligned_calcAddress(uint16_t block, uint16_t page) const { return block << 6 | page; }
inline uint32_t calcAddress(uint16_t block, uint16_t page, uint16_t byte) { return (block << 6 | page) << 12 | byte; }

inline uint32_t min(uint32_t a, uint32_t b) { return a < b ? a : b; }
int lastAddr = 0;

inline uint32_t ADDR_STORE_START(uint16_t blockNUM)
{
    uint16_t byteAddr = blockNUM * 4;
    uint16_t pageAddr = PAGE_PER_BLOCK - 2;
    if (byteAddr >= PAGE_SIZE_BYTE)
    {
        byteAddr = 0;
        pageAddr += 1;
    }
    return (RESERVE_BLOCK_BLOCKADDR << 18) | (pageAddr << 12) | byteAddr;
}

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
    if (!isInited)
    {
        return State::OBJECT_NOT_INIT;
    }
    uint8_t regData = 0;
    if (ReadStatusReg(RegisterAddress::STATUS_REGISTER, &regData) != State::OK)
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

Manager::State Manager::SetBufferMode(bool state) const
{
    while (isBusy())
        ;
    uint8_t regData = 0;
    if (ReadStatusReg(RegisterAddress::CONFIGURATION_REGISTER, &regData) != State::OK)
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
    if (WriteStatusReg(RegisterAddress::CONFIGURATION_REGISTER, regData) != State::OK)
    {
        return State::QSPI_ERR;
    }
    return State::OK;
}

Manager::Manager(uint16_t subsec) : subsections(subsec), reservedBlock(RESERVE_BLOCK_BLOCKADDR), sudoMode(false), isInited(false)
{
    for (unsigned int i = 0; i < BLOCK_COUNT; i++)
    {
        nextAddr[i] = 0;
    }
}

Manager::State Manager::init()
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
    isInited = true;
    State state;
    uint8_t buffer[4] = {0};
    setSudoMode(true);
    for (int i = 0; i < BLOCK_COUNT - 1; i++)
    {
        state = ReadMemory(ADDR_STORE_START(i), buffer, 4);
        if (state != State::OK)
        {
            i--;
            continue;
        }
        if (buffer[0] == 0xFF)
        {
            nextAddr[i] = 0;
            continue;
        }
        nextAddr[i] = buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3];
    }
    setSudoMode(false);
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
    if (curByte + size >= PAGE_SIZE_BYTE)
    {
        allowedSize             = PAGE_SIZE_BYTE - curByte;
        uint16_t numPagesNeeded = std::ceil(((float)size - allowedSize) / ((float)PAGE_SIZE_BYTE));
        if (curPage + numPagesNeeded >= PAGE_PER_BLOCK)
        {
            return false;
        }
    }
    return true;
}

Manager::State Manager::WriteStatusReg(RegisterAddress reg_addr, uint8_t data) const
{
    if (!isInited)
    {
        return State::OBJECT_NOT_INIT;
    }
    while (isBusy())
        ;
    if (StatusReg_Tx(OPCode::WRITE_STATUS_REG, reg_addr, data) != HAL_OK)
    {
        return State::QSPI_ERR;
    }
    return State::OK;
}

Manager::State Manager::ReadStatusReg(RegisterAddress reg_addr, uint8_t *buffer) const
{
    if (!isInited)
    {
        return State::OBJECT_NOT_INIT;
    }
    if (StatusReg_Rx(OPCode::READ_STATUS_REG, reg_addr, buffer) != HAL_OK)
    {
        return State::QSPI_ERR;
    }
    return State::OK;
}

Manager::State Manager::WriteMemory(uint16_t curBlock, uint8_t *data, uint16_t size)
{
    if (!isInited)
    {
        return State::OBJECT_NOT_INIT;
    }
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
        sizeWriteNow = min(size, PAGE_SIZE_BYTE - nextByte);
    }

    saveAddr();
    return State::OK;
}

Manager::State Manager::reWrite_WithinBlock(uint32_t address, uint8_t *data, uint16_t size)
{
    if (!isInited)
    {
        return State::OBJECT_NOT_INIT;
    }
    if (!PassAddressCheck(address))
    {
        return State::PARAM_ERR;
    }
    uint16_t curBlock  = blockAddrFilter(address);
    uint16_t curPage   = pageAddrFilter(address);
    uint16_t startByte = byteAddrFilter(address);

    uint32_t pageByteAddr = calcAddress(0, curPage, startByte);
    if (nextAddr[curBlock] == pageByteAddr)
    {
        return WriteMemory(curBlock, data, size);
    }
    else if (nextAddr[curBlock] < pageByteAddr)
    {
        nextAddr[curBlock] = pageByteAddr;
        return WriteMemory(curBlock, data, size);
    }
    State state;
    state = EraseRange_WithinBlock(address, nextAddr[curBlock]);
    if (state != State::OK)
    {
        return state;
    }

    state = WriteMemory(curBlock, data, size);
    return state;
}

bool Manager::PassAddressCheck(uint32_t address) const
{
    uint16_t curBlock = blockAddrFilter(address);
    if (curBlock >= BLOCK_COUNT)  // checks if the block number being accessed is outside the range
    {
        return false;
    }
    if (curBlock == reservedBlock && !sudoMode)  // checks if the block being access is the reserved block
    {
        return false;
    }

    uint16_t curPage = pageAddrFilter(address);
    if (curPage >= PAGE_PER_BLOCK)  // checks if the page being accessed is out of range
    {
        return false;
    }

    uint16_t curByte = byteAddrFilter(address);  // checks if the byte being accessed is out of range
    if (curByte >= PAGE_SIZE_BYTE)
    {
        return false;
    }
    if (address & 0x800)  // checks if the byte falls under the ECC range
    {
        return false;
    }

    return true;
}

Manager::State Manager::ReadMemory(uint32_t address, uint8_t *buffer, uint16_t size) const
{
    if (!isInited)
    {
        return State::OBJECT_NOT_INIT;
    }
    if (!PassAddressCheck(address))
    {
        return State::PARAM_ERR;
    }
    uint16_t curBlock    = blockAddrFilter(address);
    uint16_t curPage     = pageAddrFilter(address);
    uint16_t startByte   = byteAddrFilter(address);
    uint16_t sizeReadNow = min(size, PAGE_SIZE_BYTE - startByte);

    while (size)
    {
        taskENTER_CRITICAL();
        SetBufferMode(true);
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
        sizeReadNow = min(size, PAGE_SIZE_BYTE - startByte);
    }

    return State::OK;
}

Manager::State Manager::EraseBlock(uint32_t blockNUM, bool canSaveAddr)
{
    if (!isInited)
    {
        return State::OBJECT_NOT_INIT;
    }
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
    if (canSaveAddr)
    {
        saveAddr();
    }
    return State::OK;
}

Manager::State Manager::EraseRange_WithinBlock(uint32_t startAddress, uint32_t endAddress)
{
    if (!isInited)
    {
        return State::OBJECT_NOT_INIT;
    }
    if (startAddress >= endAddress)
    {
        return State::PARAM_ERR;
    }
    if (!PassAddressCheck(startAddress) || !PassAddressCheck(endAddress))
    {
        return State::PARAM_ERR;
    }
    if (blockAddrFilter(startAddress) != blockAddrFilter(endAddress))
    {
        return State::PARAM_ERR;
    }

    State state;
    uint32_t curBlock = blockAddrFilter(startAddress);
    uint8_t pagesAffected[2];
    pagesAffected[0] = pageAddrFilter(startAddress);
    pagesAffected[1] = pageAddrFilter(endAddress);

    state = SetWritePin(true);
    if (state != State::OK)
    {
        return state;
    }
    setSudoMode(true);
    state = EraseBlock(RESERVE_BLOCK_BLOCKADDR, false);
    if (state != State::OK)
    {
        return state;
    }
    uint32_t oldSize = nextAddr[curBlock];

    for (uint8_t pageIndex = 0; pageIndex < PAGE_PER_BLOCK; pageIndex++)
    {
        uint16_t size  = 0;
        uint16_t start = 0;
        if (pageIndex == pagesAffected[0] && pageIndex == pagesAffected[1])
        {
            size  = byteAddrFilter(startAddress);
            start = 0;
            if (size != 0)
            {
                state = ReadMemory(calcAddress(curBlock, pageIndex, start), localBuffer, size);
                if (state != State::OK)
                {
                    return state;
                }
                state = WriteMemory((uint16_t)RESERVE_BLOCK_BLOCKADDR, localBuffer, size);
                if (state != State::OK)
                {
                    return state;
                }
            }

            size  = PAGE_SIZE_BYTE - byteAddrFilter(endAddress);
            start = byteAddrFilter(endAddress);

            if (size != 0)
            {
                state = ReadMemory(calcAddress(curBlock, pageIndex, start), localBuffer, size);
                if (state != State::OK)
                {
                    return state;
                }
                state = WriteMemory((uint16_t)RESERVE_BLOCK_BLOCKADDR, localBuffer, size);
                if (state != State::OK)
                {
                    return state;
                }
            }
            continue;
        }
        else if (pageIndex == pagesAffected[0])
        {
            size = byteAddrFilter(startAddress);
        }
        else if (pageIndex == pagesAffected[1])
        {
            size  = PAGE_SIZE_BYTE - byteAddrFilter(endAddress);
            start = byteAddrFilter(endAddress);
        }
        else
        {
            size = PAGE_SIZE_BYTE;
        }
        if (size != 0)
        {
            State state;
            state = ReadMemory(calcAddress(curBlock, pageIndex, start), localBuffer, size);
            if (state != State::OK)
            {
                return state;
            }
            state = WriteMemory((uint16_t)RESERVE_BLOCK_BLOCKADDR, localBuffer, size);
            if (state != State::OK)
            {
                return state;
            }
        }
    }
    EraseBlock(curBlock);

    for (uint8_t i = 0; i < PAGE_PER_BLOCK; i++)
    {
        state = ReadMemory(calcAddress(RESERVE_BLOCK_BLOCKADDR, i, 0), localBuffer, 2048);
        if (state != State::OK)
        {
            return state;
        }
        state = WriteMemory(curBlock, localBuffer, 2048);
        if (state != State::OK)
        {
            return state;
        }
    }
    state = EraseBlock(RESERVE_BLOCK_BLOCKADDR, false);
    setSudoMode(false);

    nextAddr[curBlock] = oldSize - (min(endAddress, oldSize) - startAddress);
    return state;
}

Manager::State Manager::EraseChip()
{
    if (!isInited)
    {
        return State::OBJECT_NOT_INIT;
    }
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
        State state = EraseBlock(i, false);
        switch (state)
        {
        default:
            i--;
        case State::OK:
            continue;
        }
        if (sudoMode)
        {
            setSudoMode(false);
        }
        taskEXIT_CRITICAL();
    }

    saveAddr();
    return State::OK;
}

Manager::State Manager::BB_LUT(uint8_t *buffer) const
{
    if (!isInited)
    {
        return State::OBJECT_NOT_INIT;
    }
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
    if (ReadStatusReg(RegisterAddress::STATUS_REGISTER, &ECC_Check) != State::OK)
    {
        return State::QSPI_ERR;
    }
    if (ECC_Check & 64)
    {
        return State::ECC_ERR;
    }
    return State::OK;
}

Manager::State Manager::BB_Entry(const uint16_t &badBlockAddr, const uint16_t &goodBlockAddr) const { return State::OK; }

Manager::State Manager::BB_management()
{
    if (!isInited)
    {
        return State::OBJECT_NOT_INIT;
    }
    uint8_t data[PAGE_SIZE_BYTE];
    uint8_t buffer[PAGE_SIZE_BYTE];
    for (int a = 0; a < PAGE_SIZE_BYTE; a++)
    {
        data[a] = a;
    }
    uint16_t badBlocks[20] = {0}, j = 0;
    uint16_t goodBlocks[1024] = {0}, k = 0;
    for (unsigned int i = 0; i < BLOCK_COUNT; i++)
    {
        Command_Tx_4DataLine(OPCode::QUAD_LOAD_PROGRAM_DATA, data, 0, PAGE_SIZE_BYTE);
        BufferCommand(pageAligned_calcAddress(i, 0), OPCode::PROGRAM_EXECUTE);
        uint32_t addr = 0;
        State result  = getLast_ECC_page_failure(addr);
        if (result == State::ECC_ERR && (addr >> 18) == (uint32_t)i)
        {
            badBlocks[j++] = i;
        }
        ReadMemory(calcAddress(i, 0, 0), buffer, PAGE_SIZE_BYTE);
        EraseBlock(i, false);
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
    if (nextByte >= PAGE_SIZE_BYTE)
    {
        nextByte = 0;
        pageNum += 1;
    }
    nextAddr[blockNum] = calcAddress(0, pageNum, nextByte);
}

void Manager::setSudoMode(bool mode)
{
    sudoMode = mode;
    if (mode)
    {
        taskENTER_CRITICAL();
    }
    else
    {
        taskEXIT_CRITICAL();
    }
}

void Manager::saveAddr()
{
    State state;
    EraseBlock(RESERVE_BLOCK_BLOCKADDR, false);
    for (uint16_t i = 0; i < BLOCK_COUNT; i++)
    {
        if (i == RESERVE_BLOCK_BLOCKADDR)
        {
            continue;
        }

        setSudoMode(true);
        uint32_t curAddr = nextAddr[i];
        uint8_t addr[4]  = {(curAddr & 0xFF000000) >> 24, (curAddr & 0xFF0000) >> 16, (curAddr & 0xFF00) >> 8, (curAddr & 0xFF)};
        if (WriteEnable() != State::OK)
        {
            setSudoMode(false);
            return;
        }

        while (isBusy())
            ;
        uint32_t storeAddr = ADDR_STORE_START(i);
        uint16_t byteAddr  = byteAddrFilter(storeAddr);
        if (Command_Tx_4DataLine(OPCode::QUAD_LOAD_PROGRAM_DATA, addr, byteAddr, 4) != HAL_OK)
        {
            setSudoMode(false);
            return;
        }
        uint16_t pageAddr = storeAddr >> 12;
        while (isBusy())
            ;
        if (BufferCommand(pageAddr, OPCode::PROGRAM_EXECUTE) != HAL_OK)
        {
            setSudoMode(false);
            return;
        }
        setSudoMode(false);
    }
}

}  // namespace W25N01
}  // namespace Drivers
}  // namespace Core