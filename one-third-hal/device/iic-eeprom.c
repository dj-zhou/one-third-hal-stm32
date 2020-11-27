
#include "config.h"

#include "crc-soft.h"
#include "iic-eeprom.h"
#include "iic.h"
#include "uart-console.h"

// ============================================================================
#pragma pack(1)
typedef struct EepromNode_s {
    uint16_t key;
    uint32_t addr;
    uint16_t size;
} EepromNode_t;
#pragma pack()

// ============================================================================
// ============================================================================
#ifdef EEPROM_IS_USED

static EepromNode_t node_[_EEPROM_NODE_MAX_NUM];

// ----------------------------------------------------------------------------
static void IicCheck(void) {
    if (EEPROM_IIC.bit_rate == 0) {
        console.error("%s(): iic not initialized!\r\n", __func__);
    }
}

// ----------------------------------------------------------------------------
// this function is unsafe to use externally
static HAL_StatusTypeDef EepromIicRead(uint16_t block_addr,
                                       uint16_t offset_block_addr,
                                       uint16_t mem_add_size, uint8_t* p_data,
                                       uint16_t size, uint32_t timeout) {
    IicCheck();
    uint8_t result = EEPROM_IIC.read(block_addr, offset_block_addr,
                                     mem_add_size, p_data, size, timeout);
    return result;
}

// ----------------------------------------------------------------------------
// this function is unsafe to use externally
static HAL_StatusTypeDef EepromIicWrite(uint16_t block_addr,
                                        uint16_t offset_block_addr,
                                        uint16_t mem_add_size, uint8_t* p_data,
                                        uint16_t size, uint32_t timeout) {

    IicCheck();
    uint8_t result = EEPROM_IIC.write(block_addr, offset_block_addr,
                                      mem_add_size, p_data, size, timeout);
    return result;
}

// ============================================================================
static uint16_t GetBlockAddr(uint16_t addr) {
    uint16_t block_addr =
        EEPROM_FIRST_BLOCK_ADDR + ((addr / EEPROM_BLOCK_SIZE) << 1);
    return block_addr;
}

// ============================================================================
static uint16_t GetOffsetBlockAddr(uint16_t addr) {
    uint16_t offset_block_addr = addr % EEPROM_BLOCK_SIZE;
    return offset_block_addr;
}

// ============================================================================
static uint16_t GetOffsetPageAddr(uint16_t addr) {
    uint16_t offset_page_addr = addr % EEPROM_PAGE_SIZE;
    return offset_page_addr;
}

// ============================================================================
static void EepromConfig(void) {
    for (uint16_t i = 0; i < _EEPROM_NODE_MAX_NUM; i++) {
        node_[i].key  = 0;
        node_[i].addr = 0;
        node_[i].size = 0;
    }
}

// ============================================================================
static uint8_t EepromReadByte(uint16_t addr) {
    uint8_t  data;
    uint16_t block_addr        = GetBlockAddr(addr);
    uint16_t offset_block_addr = GetOffsetBlockAddr(addr);

    uint8_t result = EepromIicRead(block_addr, offset_block_addr,
                                   I2C_MEMADD_SIZE_8BIT, &data, 1, 10000);
    if (HAL_OK == result) {
        return data;
    }
    else {
        console.printf("\r\n%s(): EepromIicRead Error!\r\n", __func__);
        return 0;
    }
    stime.delay.ms(1);  // if FreeRTOS is used, use RTOS delay
}

// ============================================================================
static void EepromWriteByte(uint16_t addr, uint8_t data) {
    uint16_t block_addr        = GetBlockAddr(addr);
    uint16_t offset_block_addr = GetOffsetBlockAddr(addr);
    uint8_t  result            = EepromIicWrite(block_addr, offset_block_addr,
                                    I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
    if (HAL_OK == result) {
        stime.delay.ms(10);  // if FreeRTOS is used, use RTOS delay
        return;
    }
    console.printf("\r\n%s(): EepromIicWrite Error!\r\n", __func__);
}

// ============================================================================
static void EepromReadNbytes(uint16_t addr, uint8_t* buf_read, uint16_t size) {
    if (addr + size > EEPROM_TOTAL_BYTE_NUM) {
        console.printf("%s(): exceeds memory range!\r\n", __func__);
        return;
    }
    uint16_t block_addr;
    uint16_t offset_block_addr;
    uint16_t offset_page_addr;
    uint8_t* remain_bytes_addr = buf_read;
    while (size > 0) {
        block_addr        = GetBlockAddr(addr);
        offset_block_addr = GetOffsetBlockAddr(addr);
        offset_page_addr  = GetOffsetPageAddr(addr);
        if (offset_page_addr + size <= EEPROM_PAGE_SIZE) {
            EepromIicRead(block_addr, offset_block_addr, I2C_MEMADD_SIZE_8BIT,
                          remain_bytes_addr, size, 10000);
            size = 0;
        }
        else {
            EepromIicRead(block_addr, offset_block_addr, I2C_MEMADD_SIZE_8BIT,
                          remain_bytes_addr,
                          EEPROM_PAGE_SIZE - offset_page_addr, 10000);
            size -= (EEPROM_PAGE_SIZE - offset_page_addr);
            remain_bytes_addr += (EEPROM_PAGE_SIZE - offset_page_addr);
            addr += (EEPROM_PAGE_SIZE - offset_page_addr);
        }
        stime.delay.ms(1);  // if FreeRTOS is used, use RTOS delay
    }
}

// ============================================================================
static void EepromWriteNbytes(uint16_t addr, uint8_t* buf_write,
                              uint16_t size) {
    if (addr + size > EEPROM_TOTAL_BYTE_NUM) {
        console.printf("%s(): exceeds memory range!\r\n", __func__);
        return;
    }
    uint16_t block_addr;
    uint16_t offset_block_addr;
    uint16_t offset_page_addr;
    uint8_t* remain_bytes_addr = buf_write;

    while (size > 0) {
        block_addr        = GetBlockAddr(addr);
        offset_block_addr = GetOffsetBlockAddr(addr);
        offset_page_addr  = GetOffsetPageAddr(addr);
        if (offset_page_addr + size <= EEPROM_PAGE_SIZE) {
            EepromIicWrite(block_addr, offset_block_addr, I2C_MEMADD_SIZE_8BIT,
                           remain_bytes_addr, size, 10000);
            size = 0;
        }
        else {
            EepromIicWrite(block_addr, offset_block_addr, I2C_MEMADD_SIZE_8BIT,
                           remain_bytes_addr,
                           EEPROM_PAGE_SIZE - offset_page_addr, 10000);
            size -= (EEPROM_PAGE_SIZE - offset_page_addr);
            remain_bytes_addr += (EEPROM_PAGE_SIZE - offset_page_addr);
            addr += (EEPROM_PAGE_SIZE - offset_page_addr);
        }
        stime.delay.ms(10);  // if FreeRTOS is used, use RTOS delay
    }
}

// ============================================================================
static uint16_t EepromGetKeyfromNode(uint8_t* ram_addr, uint16_t size) {
    if (size < 2) {
        console.error("\r\n%s(): not a node!\r\n", __func__);
    }
    uint16_t key = (uint8_t)(ram_addr[1]) << 8 | (uint8_t)(ram_addr[0]);
    if (!IS_EEPROM_KEY(key)) {
        console.error("\r\n%s(): not a valid key!\r\n", __func__);
    }
    return key;
}

// ============================================================================
static uint8_t attached_node_num_ = 0;
static void    EepromAttachNode(uint8_t* ram_addr, uint16_t size) {
    uint16_t eeprom_key = EepromGetKeyfromNode(ram_addr, size);
    if (attached_node_num_ == 0) {
        node_[0].key  = eeprom_key;
        node_[0].addr = _EEPROM_NODE_START_ADDR;
        node_[0].size = size + 2;  // two bytes for crc16 checksum: todo
    }
    else {
        for (uint8_t i = 0; i < attached_node_num_; i++) {
            if (eeprom_key == node_[i].key) {
                console.error("\r\n%s(): node key already attached!\r\n",
                              __func__);
            }
            node_[attached_node_num_].addr = node_[i].addr + node_[i].size;
        }
        node_[attached_node_num_].key  = eeprom_key;
        node_[attached_node_num_].size = size + 2;
    }
    attached_node_num_++;
}

// ============================================================================
/// input arguments:
/// ram_addr: it should be the address of a memory block, the eeprom key should
/// be stored to it first.
//  size: the size of the memory block, NOT including the two crc16 bytes
static void EepromReadNode(uint8_t* ram_addr, uint16_t size) {
    uint16_t crc16_read;
    bool     read_done  = false;
    uint8_t  read_cout  = 0;
    uint16_t eeprom_key = EepromGetKeyfromNode(ram_addr, size);
    for (uint8_t i = 0; i < attached_node_num_; i++) {
        if (node_[i].key != eeprom_key) {
            continue;
        }
        if (node_[i].size != size + 2) {
            console.error("%s(): read a wrong node!\r\n", __func__);
        }
        while ((!read_done) && (read_cout < _EEPROM_NODE_MAX_READ_TIME)) {
            EepromReadNbytes(node_[i].addr, ram_addr, size);
            EepromReadNbytes(node_[i].addr + size, ( uint8_t* )&crc16_read, 2);
            if (crc16_read == crc_soft.calculate8bitCrc16(ram_addr, size, 1)) {
                read_done = true;
                return;
            }
            read_cout++;
            console.printf("%s(): crc read error!\r\n", __func__);
            stime.delay.ms(10);  // if FreeRTOS is used, use RTOS delay
        }
    }
}

// ============================================================================
static void EepromWriteNode(uint8_t* ram_addr, uint16_t size) {
    uint16_t eeprom_key = EepromGetKeyfromNode(ram_addr, size);
    uint16_t crc16_calc = crc_soft.calculate8bitCrc16(ram_addr, size, 1);
    for (uint8_t i = 0; i < attached_node_num_; i++) {
        if (node_[i].key != eeprom_key) {
            continue;
        }
        EepromWriteNbytes(node_[i].addr, ram_addr, size);
        EepromWriteNbytes(node_[i].addr + size, ( uint8_t* )&crc16_calc, 2);
        return;
    }
    console.printf("%s(): node not attached!\r\n", __func__);
}

// ============================================================================
static void EepromShowNodes(void) {
    console.printf("\r\n");
    CONSOLE_PRINTF_SEG;
    if (attached_node_num_ == 0) {
        console.printf("No EEPROM node attached!\r\n");
        return;
    }
    console.printf("EEPROM Nodes Attached (%d nodes):\r\n", attached_node_num_);
    console.printf(" node     key           virtual addr     size (w/crc)\r\n");
    for (uint8_t i = 0; i < attached_node_num_; i++) {
        console.printf(" %2u       0x%04X        0d%03u (0x%03X)   %3u\r\n",
                       i + 1, node_[i].key, node_[i].addr, node_[i].addr,
                       node_[i].size);
    }
    CONSOLE_PRINTF_SEG;
}

// ============================================================================
// clang-format off
EepromApi_t eeprom = {
    .config      = EepromConfig     ,
    .read.byte   = EepromReadByte   ,
    .read.bytes  = EepromReadNbytes ,
    .write.byte  = EepromWriteByte  ,
    .write.bytes = EepromWriteNbytes,
    .node.attach = EepromAttachNode ,
    .node.read   = EepromReadNode   ,
    .node.write  = EepromWriteNode  ,
    .node.show   = EepromShowNodes  ,
};
// clang-format on

#endif  // #ifdef EEPROM_IS_USED
