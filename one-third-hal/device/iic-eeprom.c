
#include "config.h"

#include "crc-soft.h"
#include "iic-eeprom.h"
#include "iic.h"
#include "uart-console.h"

// ============================================================================
#pragma pack(1)
typedef struct EepromNode_s {
    uint16_t key;
    uint32_t eeprom_addr;
    uint16_t size;
} EepromNode_t;
#pragma pack()
// ============================================================================
// ============================================================================
#ifdef EEPROM_IS_USED

static EepromNode_t node_[_EEPROM_NODE_MAX_NUM];

// ----------------------------------------------------------------------------
// this function is unsafe to use externally
HAL_StatusTypeDef EEPROM_IIC_Read(uint16_t block_addr,
                                  uint16_t offset_block_addr,
                                  uint16_t mem_add_size, uint8_t* p_data,
                                  uint16_t size, uint32_t timeout) {
    if (EEPROM_IIC.bit_rate == 0) {
        console.error("%s(): iic not initialized!\r\n", __func__);
    }
    uint8_t result = EEPROM_IIC.read(block_addr, offset_block_addr,
                                     mem_add_size, p_data, size, timeout);
    return result;
}

// ----------------------------------------------------------------------------
// this function is unsafe to use externally
HAL_StatusTypeDef EEPROM_IIC_Write(uint16_t block_addr,
                                   uint16_t offset_block_addr,
                                   uint16_t mem_add_size, uint8_t* p_data,
                                   uint16_t size, uint32_t timeout) {
    if (EEPROM_IIC.bit_rate == 0) {
        console.error("%s(): iic not initialized!\r\n", __func__);
    }
    uint8_t result = EEPROM_IIC.write(block_addr, offset_block_addr,
                                      mem_add_size, p_data, size, timeout);
    return result;
}

// ============================================================================
static uint16_t Get_Block_Addr(uint16_t addr) {
    uint16_t block_addr =
        EEPROM_FIRST_BLOCK_ADDR + ((addr / EEPROM_BLOCK_SIZE) << 1);
    return block_addr;
}

// ============================================================================
static uint16_t Get_Offset_Block_Addr(uint16_t addr) {
    uint16_t offset_block_addr = addr % EEPROM_BLOCK_SIZE;
    return offset_block_addr;
}

// ============================================================================
static uint16_t Get_Offset_Page_Addr(uint16_t addr) {
    uint16_t offset_page_addr = addr % EEPROM_PAGE_SIZE;
    return offset_page_addr;
}

// ============================================================================
static void EEPROM_Config(void) {
    for (uint16_t i = 0; i < _EEPROM_NODE_MAX_NUM; i++) {
        node_[i].key         = 0;
        node_[i].eeprom_addr = 0;
        node_[i].size        = 0;
    }
}

// ============================================================================
static uint8_t EEPROM_Read_Byte(uint16_t addr) {
    uint8_t  result = HAL_ERROR;
    uint8_t  data;
    uint16_t block_addr        = Get_Block_Addr(addr);
    uint16_t offset_block_addr = Get_Offset_Block_Addr(addr);

    result = EEPROM_IIC_Read(block_addr, offset_block_addr,
                             I2C_MEMADD_SIZE_8BIT, &data, 1, 10000);
    if (HAL_OK == result) {
        return data;
    }
    else {
        console.printf("\r\n%s(): EEPROM_IIC_Read Error!\r\n", __func__);
        return 0;
    }
    stime.delay.ms(1);  // if FreeRTOS is used, use RTOS delay
}

// ============================================================================
static void EEPROM_Write_Byte(uint16_t addr, uint8_t data) {
    uint8_t  result            = HAL_ERROR;
    uint16_t block_addr        = Get_Block_Addr(addr);
    uint16_t offset_block_addr = Get_Offset_Block_Addr(addr);
    result                     = EEPROM_IIC_Write(block_addr, offset_block_addr,
                              I2C_MEMADD_SIZE_8BIT, &data, 1, 100);

    if (HAL_OK == result) {
        stime.delay.ms(10);  // if FreeRTOS is used, use RTOS delay
    }
    else {
        console.printf("\r\n%s(): EEPROM_IIC_Write Error!\r\n", __func__);
    }
}

// ============================================================================
static void EEPROM_Read_N_Bytes(uint16_t addr, uint8_t* buf_read,
                                uint16_t size) {
    if (addr + size > EEPROM_TOTAL_BYTE_NUM) {
        console.printf("%s(): exceed memory range!\r\n", __func__);
        return;
    }

    uint16_t block_addr;
    uint16_t offset_block_addr;
    uint16_t offset_page_addr;
    uint8_t* remain_bytes_addr = buf_read;

    while (size > 0) {
        block_addr        = Get_Block_Addr(addr);
        offset_block_addr = Get_Offset_Block_Addr(addr);
        offset_page_addr  = Get_Offset_Page_Addr(addr);

        if (offset_page_addr + size <= EEPROM_PAGE_SIZE) {
            EEPROM_IIC_Read(block_addr, offset_block_addr, I2C_MEMADD_SIZE_8BIT,
                            remain_bytes_addr, size, 10000);
            size = 0;
        }
        else {
            EEPROM_IIC_Read(block_addr, offset_block_addr, I2C_MEMADD_SIZE_8BIT,
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
static void EEPROM_Write_N_Bytes(uint16_t addr, uint8_t* buf_write,
                                 uint16_t size) {
    if (addr + size > EEPROM_TOTAL_BYTE_NUM) {
        console.printf("EEPROM_Write_N_Bytes(): exceed memory range!\r\n");
        return;
    }
    uint16_t block_addr;
    uint16_t offset_block_addr;
    uint16_t offset_page_addr;
    uint8_t* remain_bytes_addr = buf_write;

    // write the actual data first ------------------------
    while (size > 0) {
        block_addr        = Get_Block_Addr(addr);
        offset_block_addr = Get_Offset_Block_Addr(addr);
        offset_page_addr  = Get_Offset_Page_Addr(addr);

        if (offset_page_addr + size <= EEPROM_PAGE_SIZE) {
            EEPROM_IIC_Write(block_addr, offset_block_addr,
                             I2C_MEMADD_SIZE_8BIT, remain_bytes_addr, size,
                             10000);
            size = 0;
        }
        else {
            EEPROM_IIC_Write(block_addr, offset_block_addr,
                             I2C_MEMADD_SIZE_8BIT, remain_bytes_addr,
                             EEPROM_PAGE_SIZE - offset_page_addr, 10000);
            size -= (EEPROM_PAGE_SIZE - offset_page_addr);
            remain_bytes_addr += (EEPROM_PAGE_SIZE - offset_page_addr);
            addr += (EEPROM_PAGE_SIZE - offset_page_addr);
        }
        stime.delay.ms(10);  // if FreeRTOS is used, use RTOS delay
    }
}

// ============================================================================
static uint8_t registered_node_num_ = 0;
static void    EEPROM_Attach_Node(uint8_t* ram_addr, uint16_t size) {
    uint8_t  iter       = 0;
    uint16_t eeprom_key = (uint8_t)(ram_addr[1]) << 8 | (uint8_t)(ram_addr[0]);
    if (!IS_EEPROM_KEY(eeprom_key)) {
        console.error("\r\nEEPROM_Attach_Node(): A wrong key was set!\r\n");
    }
    if (registered_node_num_ == 0) {
        node_[0].key         = eeprom_key;
        node_[0].eeprom_addr = _EEPROM_NODE_START_ADDR;
        node_[0].size = size + 2;  // two bytes for crc16 checksum: todo
    }
    else {
        for (iter = 0; iter < registered_node_num_; iter++) {
            node_[registered_node_num_].eeprom_addr =
                node_[iter].eeprom_addr + node_[iter].size;
            if (eeprom_key == node_[iter].key) {
                node_[registered_node_num_].eeprom_addr = 0;
                console.error("\r\nEEPROM_Attach_Node(): Node key "
                              "already registered!\r\n");
            }
        }
        node_[registered_node_num_].key  = eeprom_key;
        node_[registered_node_num_].size = size + 2;
    }
    registered_node_num_++;
}

// ============================================================================
// input arguments:
// ram_addr: it should be the address of a memory block
// size: the size of the memory block, NOT including the two crc16 bytes
static void EEPROM_Read_Node(uint8_t* ram_addr, uint16_t size) {
    uint16_t crc16_read, crc16_calc;
    bool     read_done      = false;
    uint8_t  read_time_cout = 0;
    uint16_t eeprom_key = (uint8_t)(ram_addr[1]) << 8 | (uint8_t)(ram_addr[0]);
    for (uint8_t i = 0; i < registered_node_num_; i++) {
        if (node_[i].key == eeprom_key) {  // find the key
            if (node_[i].size - 2 == size) {
                while ((!read_done)
                       && (read_time_cout < _EEPROM_NODE_MAX_READ_TIME)) {
                    // read to the memory block
                    EEPROM_Read_N_Bytes(node_[i].eeprom_addr, ram_addr, size);
                    // read to the memory block
                    EEPROM_Read_N_Bytes(node_[i].eeprom_addr + size,
                                        ( uint8_t* )&crc16_read, 2);
                    crc16_calc = crc_soft.calculate8bitCrc16(ram_addr, size, 1);
                    if (crc16_read == crc16_calc) {
                        read_done = true;
                    }
                    else {
                        read_time_cout++;
                        console.printf("%s(): crc read error!\r\n", __func__);
                        // if FreeRTOS is used, use RTOS delay
                        stime.delay.ms(100);
                    }
                }
            }
            else {
                console.printf("%s(): wrong size of node to read!\r\n",
                               __func__);
                for (uint16_t i = 0; i < size; i++) {
                    ram_addr[i] = 0;
                }
            }
            return;
        }
    }
    console.printf("%s(): node not registered!\r\n", __func__);
}

// ============================================================================
static void EEPROM_Write_Node(uint8_t* ram_addr, uint16_t size) {
    uint16_t crc16_calc;
    uint16_t eeprom_key = (uint8_t)(ram_addr[1]) << 8 | (uint8_t)(ram_addr[0]);
    crc16_calc          = crc_soft.calculate8bitCrc16(ram_addr, size, 1);
    for (uint8_t i = 0; i < registered_node_num_; i++) {
        if (node_[i].key == eeprom_key) {  // find the key
            // write the memory block to eeprom
            EEPROM_Write_N_Bytes(node_[i].eeprom_addr, ram_addr, size);

            EEPROM_Write_N_Bytes(node_[i].eeprom_addr + size,
                                 ( uint8_t* )&crc16_calc, 2);
            return;
        }
    }
    console.printf("%s(): node not registered!\r\n", __func__);
}

// ============================================================================
static void EEPROM_Show_Nodes(void) {
    console.printf("\r\n");
    CONSOLE_PRINTF_SEG;
    console.printf("EEPROM Nodes Attached (%d nodes):\r\n",
                   registered_node_num_);
    if (registered_node_num_ == 0) {
        console.printf("No EEPROM node ached!\r\n");
    }
    else {
        console.printf(
            " node     key           eeprom addr     size (w/ crc)\r\n");
        for (uint8_t i = 0; i < registered_node_num_; i++) {
            console.printf(" %2u       0x%04X        0d%03u (0x%03X)   %3u\r\n",
                           i + 1, node_[i].key, node_[i].eeprom_addr,
                           node_[i].eeprom_addr, node_[i].size);
        }
    }
    CONSOLE_PRINTF_SEG;
}

// ============================================================================
// clang-format off
EepromApi_t eeprom = {
    .config      = EEPROM_Config       ,
    .readByte    = EEPROM_Read_Byte    ,
    .writeByte   = EEPROM_Write_Byte   ,
    .readNbytes  = EEPROM_Read_N_Bytes ,
    .writeNbytes = EEPROM_Write_N_Bytes,
    .attachNode  = EEPROM_Attach_Node  ,
    .readNode    = EEPROM_Read_Node    ,
    .writeNode   = EEPROM_Write_Node   ,
    .show        = EEPROM_Show_Nodes   ,
};
// clang-format on
#endif  // #ifdef EEPROM_IS_USED
