#include "flash.hpp"

#include <cstring>

#include "FreeRTOS.h"
#include "task.h"

namespace Core
{
namespace Drivers
{
namespace W25N01
{
uint32_t calcAddress(uint16_t block, uint16_t page, uint16_t byte) { return ((block << 6 | page) << 12) | byte; }

inline uint32_t min(uint32_t a, uint32_t b) { return a < b ? a : b; }

bool isBusy()
{
    uint8_t status;
    StatusReg_Rx(OPCode::READ_STATUS_REG, RegisterAddress::STATUS_REGISTER, &status);
    return (status & 0x01) || hqspi1.State != HAL_QSPI_STATE_READY;
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

Manager::Manager(uint16_t subsections) : subsections(subsections)
{
    for (int i = 0; i < BLOCK_COUNT; i++)
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

    if (StatusReg_Tx(OPCode::WRITE_STATUS_REG, RegisterAddress::CONFIGURATION_REGISTER, (uint8_t)0x10) != HAL_OK)
    {
        return State::QSPI_ERR;
    }

    if (get_JEDECID() != JEDECID_EXEPECTED)
    {
        return State::CHIP_ERR;
    }
    return State::OK;
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

Manager::State Manager::WriteMemory(uint16_t blockNumber, uint8_t *data, uint16_t size)
{
    if (blockNumber >= BLOCK_COUNT || blockNumber < 0)
    {
        return State::PARAM_ERR;
    }

    if (nextAddr[blockNumber] + size > 2048)
    {
        return State::PARAM_ERR;
    }

    if (WriteEnable() != State::OK)
    {
        return State::QSPI_ERR;
    }
    while (isBusy())
        ;

    uint32_t totalAddr = calcAddress(blockNumber, nextAddr[blockNumber] & 0x3F000, nextAddr[blockNumber] & 0xFFF);

    if (Command_Tx_4DataLine(OPCode::RANDOM_QUAD_LOAD_PROGRAM_DATA, data, totalAddr & 0xFFF, size) != HAL_OK)
    {
        return State::QSPI_ERR;
    }
    while (isBusy())
        ;
    if (BufferCommand(totalAddr >> 12, OPCode::PROGRAM_EXECUTE) != HAL_OK)
    {
        return State::QSPI_ERR;
    }

    nextAddr[blockNumber] += size;

    return State::OK;
}

bool Manager::CheckAddress(uint16_t block, uint16_t page, uint16_t startByte) const
{
    if (block >= BLOCK_COUNT || block < 0)
    {
        return false;
    }

    if (page >= MEM_BLOCK_SIZE || page < 0)
    {
        return false;
    }

    if (startByte >= 2048 || startByte < 0)
    {
        return false;
    }

    return true;
}

Manager::State Manager::ReadMemory(uint16_t block, uint16_t page, uint16_t startByte, uint8_t *buffer, uint16_t size) const
{
    if (!CheckAddress(block, page, startByte))
    {
        return State::PARAM_ERR;
    }

    if (startByte + size > 2048)
    {
        return State::PARAM_ERR;
    }

    uint32_t address = calcAddress(block, page, startByte);

    SetBuffer(true);
    while (isBusy())
        ;
    if (BufferCommand(address >> 12, OPCode::PAGE_DATA_READ) != HAL_OK)
    {
        return State::QSPI_ERR;
    }
    while (isBusy())
        ;

    if (Command_Rx_2DataLine(OPCode::FAST_READ_DUAL_OUTPUT, buffer, address & 0xFFF, size) != HAL_OK)
    {
        return State::QSPI_ERR;
    }

    return State::OK;
}

Manager::State Manager::EraseBlock(uint32_t blockNUM)
{
    if (blockNUM >= BLOCK_COUNT || blockNUM < 0)
    {
        return State::PARAM_ERR;
    }

    if (WriteEnable() != State::OK)
    {
        return State::QSPI_ERR;
    }

    if (BufferCommand(calcAddress(blockNUM, 0, 0) >> 12, OPCode::BLOCK_ERASE) != HAL_OK)
    {
        return State::QSPI_ERR;
    }

    nextAddr[blockNUM] = 0;
    return State::OK;
}

Manager::State Manager::EraseChip()
{
    if (WriteEnable() != State::OK)
    {
        return State::QSPI_ERR;
    }

    taskENTER_CRITICAL();
    for (int i = 0; i < BLOCK_COUNT; i++)
    {
        State state = EraseBlock(i);
        switch (state)
        {
        default:
            i--;
        case State::OK:
            continue;
        }
    }
    taskEXIT_CRITICAL();
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

}  // namespace W25N01
}  // namespace Drivers
}  // namespace Core