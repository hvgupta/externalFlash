#include "flash.hpp"

namespace Core
{
namespace Drivers
{
namespace W25N01
{
uint32_t calcAddress(uint16_t block, uint16_t page, uint16_t byte) { return ((block << 6 | page) << 12) | byte; }

bool isBusy()
{
    uint8_t status;
    StatusReg_Rx(OPCode::READ_STATUS_REG, RegisterAddress::STATUS_REGISTER, &status);
    return status & 0x01;
}

uint32_t Manager::get_JEDECID()
{
    uint8_t buffer[3] = {0};
    Command_Rx_1DataLine(OPCode::JEDEC_ID, buffer, 3, 0);
    return (buffer[0] << 16) | (buffer[1] << 8) | buffer[2];
}

Manager::State Manager::WriteEnable()
{
    if (PureCommand(OPCode::WRITE_ENABLE) != HAL_OK)
    {
        return State::CHIP_ERR;
    }
    return State::OK;
}

Manager::State Manager::WriteDisable()
{
    if (PureCommand(OPCode::WRITE_DISABLE) != HAL_OK)
    {
        return State::CHIP_ERR;
    }
    return State::OK;
}

Manager::State Manager::SetBuffer(bool state)
{
    uint8_t regData = 0;
    if (ReadStatusReg(&regData, RegisterAddress::CONFIGURATION_REGISTER) != State::OK)
    {
        return State::CHIP_ERR;
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
        return State::CHIP_ERR;
    }
    return State::OK;
}

Manager::Manager(uint16_t subsections) : subsections(subsections)
{
    status = State::OK;
    PureCommand(OPCode::DEVICE_RESET);
    for (int i = 0; i < BLOCK_COUNT; i++)
    {
        nextAddr[i] = 0;
    }

    if (StatusReg_Tx(OPCode::WRITE_STATUS_REG, RegisterAddress::PROTECT_REGISTER, (uint8_t)0x00) != HAL_OK)
    {
        status = State::CHIP_ERR;
        return;
    }

    if (StatusReg_Tx(OPCode::WRITE_STATUS_REG, RegisterAddress::CONFIGURATION_REGISTER, (uint8_t)0x10) != HAL_OK)
    {
        status = State::CHIP_ERR;
        return;
    }

    if (get_JEDECID() != JEDECID_EXEPECTED)
    {
        status = State::CHIP_ERR;
    }
}

Manager::State Manager::WriteStatusReg(uint8_t data, RegisterAddress reg_addr)
{
    if (isBusy())
    {
        return State::BUSY;
    }
    if (StatusReg_Tx(OPCode::WRITE_STATUS_REG, reg_addr, data) != HAL_OK)
    {
        return State::CHIP_ERR;
    }
    return State::OK;
}

Manager::State Manager::ReadStatusReg(uint8_t *buffer, RegisterAddress reg_addr)
{
    if (StatusReg_Rx(OPCode::READ_STATUS_REG, reg_addr, buffer) != HAL_OK)
    {
        return State::CHIP_ERR;
    }
    return State::OK;
}

Manager::State Manager::WriteMemory(uint16_t blockNumber, uint8_t *data, uint32_t size)
{
    if (isBusy())
    {
        return State::BUSY;
    }

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
        return State::CHIP_ERR;
    }
    uint32_t totalAddr = calcAddress(blockNumber, nextAddr[blockNumber] & 0x3F000, nextAddr[blockNumber] & 0xFFF);
    if (Command_Tx_4DataLine(OPCode::RANDOM_QUAD_LOAD_PROGRAM_DATA, data, totalAddr & 0xFFF, size) != HAL_OK)
    {
        return State::CHIP_ERR;
    }
    if (BufferCommand(totalAddr >> 12, OPCode::PROGRAM_EXECUTE) != HAL_OK)
    {
        return State::CHIP_ERR;
    }

    nextAddr[blockNumber] += size;
    return State::OK;
}

bool Manager::CheckAddress(uint16_t block, uint16_t page, uint16_t startByte)
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

Manager::State Manager::ReadMemory(uint16_t block, uint16_t page, uint16_t startByte, uint8_t *buffer, uint32_t size)
{
    if (!CheckAddress(block, page, startByte))
    {
        return State::PARAM_ERR;
    }
    if (isBusy())
    {
        return State::BUSY;
    }
    uint32_t address = calcAddress(block, page, startByte);
    SetBuffer(true);
    if (BufferCommand(address >> 12, OPCode::PAGE_DATA_READ) != HAL_OK)
    {
        return State::CHIP_ERR;
    }
    while (isBusy())
        ;

    if (Command_Rx_2DataLine(OPCode::FAST_READ_DUAL_OUTPUT, buffer, address & 0xFFF, size) != HAL_OK)
    {
        return State::CHIP_ERR;
    }
    return State::OK;
}

Manager::State Manager::EraseBlock(uint32_t blockNUM)
{
    if (isBusy())
    {
        return State::BUSY;
    }

    if (blockNUM >= BLOCK_COUNT || blockNUM < 0)
    {
        return State::PARAM_ERR;
    }

    if (WriteEnable() != State::OK)
    {
        return State::CHIP_ERR;
    }

    if (BufferCommand(calcAddress(blockNUM, 0, 0), OPCode::BLOCK_ERASE) != HAL_OK)
    {
        return State::CHIP_ERR;
    }

    return State::OK;
}

Manager::State Manager::EraseChip()
{
    if (isBusy())
    {
        return State::BUSY;
    }

    if (WriteEnable() != State::OK)
    {
        return State::CHIP_ERR;
    }

    for (int i = 0; i < BLOCK_COUNT; i++)
    {
        State state = EraseBlock(i);
        switch (state)
        {
        case State::BUSY:
            i--;
        case State::OK:
            continue;
        case State::CHIP_ERR: /*Data until this block error has been erased*/
            return State::CHIP_ERR;
        }
    }

    return State::OK;
}

}  // namespace W25N01
}  // namespace Drivers
}  // namespace Core