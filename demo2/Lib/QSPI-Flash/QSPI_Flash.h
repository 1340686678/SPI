#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include "main.h"
#include "stdbool.h"
#include "quadspi.h"

//***************************************************
//* 通用定义
//***************************************************

#define FLASH_GD25Q64      1
#define FLASH_MODEL        FLASH_GD25Q64

/* QSPI Status code */
#define QSPI_OK            ((uint8_t)0x00)
#define QSPI_ERROR         ((uint8_t)0x01)
#define QSPI_BUSY          ((uint8_t)0x02)
#define QSPI_NOT_SUPPORTED ((uint8_t)0x04)
#define QSPI_SUSPENDED     ((uint8_t)0x08)

//***************************************************
//* STM32相关
//***************************************************

#define QSPI_HANDLE        hqspi

//***************************************************
//* 行为定义
//***************************************************

#define QSPI_WRITE_AUTO_ENABLE           true
#define QSPI_QUAD_READ_AUTO_ENABLE       true

//***************************************************
//* FLASH配置，请参考数据手册
//***************************************************

#define QSPI_FLASH_SIZE                  0x800000  // 8   MBytes
#define QSPI_BLOCK_SIZE                  0x10000   // 64  KBytes
#define QSPI_SUBSECTOR_SIZE              0x1000    // 4   KBytes
#define QSPI_PAGE_SIZE                   0x100     // 256 Bytes

#define DUMMY_CYCLES_READ                0         // 正常读取数据时的等待循环
#define DUMMY_CYCLES_FAST_READ           8         // 快速读取数据时的等待循环
#define DUMMY_CYCLES_DUAL_READ           8         // 双线模式读取数据时的等待循环
#define DUMMY_CYCLES_QUAD_READ           8         // 四线模式读取数据时的等待循环
#define DUMMY_CYCLES_DUAL_IO_FAST_READ   0         // 双线连续模式读取数据时的等待循环
#define DUMMY_CYCLES_QUAD_IO_FAST_READ   4         // 四线连续模式读取数据时的等待循环

#define CHIP_ERASE_MAX_TIME             250000     // 芯片擦除时最长等待时间
#define BLOCK_ERASE_MAX_TIME            2000       // 块擦除时最长等待时间
#define SECTOR_ERASE_MAX_TIME           500        // 区擦除时最长等待时间
#define CONTROL_COMMANS_MAX_TIME        1000       // 控制类指令(状态寄存器和其他寄存器)操作时的最长等待时间
#define AUTO_PULLING_WAIT_TIME          1000       // 其他等待芯片状态时的最长等待时间

//***************************************************
//* 指令表
//***************************************************

#if (FLASH_MODEL == FLASH_GD25Q64)

// 写入保护相关指令
#define QSPI_CMD_WRITE_ENABLE                      0x06
#define QSPI_CMD_WRITE_DISABLE                     0x04
#define QSPI_CMD_VOLATILE_SR_WRITE_ENABLE          0x50

// 状态寄存器操作
#define QSPI_CMD_READ_STATUS_REG1                  0x05
#define QSPI_CMD_READ_STATUS_REG2                  0x35
#define QSPI_CMD_READ_STATUS_REG3                  0x15

#define QSPI_CMD_WRITE_STATUS_REG1                 0x01
#define QSPI_CMD_WRITE_STATUS_REG2                 0x31
#define QSPI_CMD_WRITE_STATUS_REG3                 0x11

// 数据读取指令
#define QSPI_CMD_READ_DATA                         0x03
#define QSPI_CMD_FAST_READ                         0x0B
#define QSPI_CMD_DUAL_OUTPUT_FAST_READ             0x3B
#define QSPI_CMD_DUAL_IO_FAST_READ                 0xBB
#define QSPI_CMD_QUAD_OUPUT_FAST_READ              0x6B
#define QSPI_CMD_QUAD_IO_FAST_READ                 0xEB
#define QSPI_CMD_QUAD_IO_WORD_FAST_READ            0xE7

// 编程指令
#define QSPI_CMD_PAGE_PROGRAM                      0x02
#define QSPI_CMD_QUAD_PAGE_PROGRAM                 0x32
#define QSPI_CMD_FAST_PAGE_PROGRAM                 0xF2

// 擦除指令
#define QSPI_CMD_SECTOR_ERASE                      0x20
#define QSPI_CMD_BLOCK32K_ERASE                    0x52
#define QSPI_CMD_BLOCK64K_ERASE                    0xD8
#define QSPI_CMD_CHIP_ERASE                        0x60

// 控制指令
#define QSPI_CMD_ENABLE_RESET                      0x66
#define QSPI_CMD_RESET                             0x99
#define QSPI_CMD_SET_BURST_WITH_WRAP               0x77
#define QSPI_CMD_PROGRAM_ERASE_SUSPEND             0x75
#define QSPI_CMD_PROGRAM_ERASE_RESUME              0x7A
#define QSPI_CMD_RELEASE_FROM_DEEP_POWER_DOWN      0xAB
#define QSPI_CMD_DEEP_POWER_DOWN                   0xB9

// 识别指令
#define QSPI_CMD_READ_DEVICE_ID                    0xAB  // 同时也会将Flash芯片从DeepSleep状态中唤醒
#define QSPI_CMD_MANUFACTURER_DEVICE_ID            0x90
#define QSPI_CMD_MANUFACTURER_DEVICE_ID_DUAL_IO    0x92
#define QSPI_CMD_MANUFACTURER_DEVICE_ID_QUAD_IO    0x94
#define QSPI_CMD_READ_IDENTIFICATION               0x9F
#define QSPI_CMD_READ_UNIQUE_ID                    0x4B

// 其他指令
#define QSPI_CMD_HIGH_PERFORMANCE_MODE             0xA3
#define QSPI_CMD_READ_DISCOVERABLE_PARAM           0x5A
#define QSPI_CMD_ERASE_SECURITY_REG                0x44
#define QSPI_CMD_PROGRAM_SECURITY_REG              0x42
#define QSPI_CMD_READ_SECURITY_REG                 0x48

#endif

//***************************************************
//* 状态寄存器标志位(Flash Status Register/FSR)
//***************************************************

#define QSPI_FSR_WIP                    ((uint8_t)0x01)    // 只读寄存器位，繁忙位  为1繁忙
#define QSPI_FSR_WEL                    ((uint8_t)0x02)    // 只读寄存器位，写使能  为1使能 
#define QSPI_FSR_BP0                    ((uint8_t)0x04)    // BP0~BP4为非易失性寄存器位。用于保护相关的存储区域不受编程(PP)，扇区擦除(SE)和块擦除(BE)命令的影响。具体保护的范围参考Datasheet。
#define QSPI_FSR_BP1                    ((uint8_t)0x08)    // BP0~BP4为非易失性寄存器位。用于保护相关的存储区域不受编程(PP)，扇区擦除(SE)和块擦除(BE)命令的影响。具体保护的范围参考Datasheet。
#define QSPI_FSR_BP2                    ((uint8_t)0x10)    // BP0~BP4为非易失性寄存器位。用于保护相关的存储区域不受编程(PP)，扇区擦除(SE)和块擦除(BE)命令的影响。具体保护的范围参考Datasheet。
#define QSPI_FSR_BP3                    ((uint8_t)0x20)    // BP0~BP4为非易失性寄存器位。用于保护相关的存储区域不受编程(PP)，扇区擦除(SE)和块擦除(BE)命令的影响。具体保护的范围参考Datasheet。
#define QSPI_FSR_BP4                    ((uint8_t)0x40)    // BP0~BP4为非易失性寄存器位。用于保护相关的存储区域不受编程(PP)，扇区擦除(SE)和块擦除(BE)命令的影响。具体保护的范围参考Datasheet。
#define QSPI_FSR_SRP0                   ((uint8_t)0x80)    // SRP1/SRP0为非易失性寄存器位。用于控制写保护的方法:软件保护、硬件保护、电源锁定还是一次性编程锁定保护。
#define QSPI_FSR_SRP1                   ((uint8_t)0x01)    // SRP1/SRP0为非易失性寄存器位。用于控制写保护的方法:软件保护、硬件保护、电源锁定还是一次性编程锁定保护。
#define QSPI_FSR_QE                     ((uint8_t)0x02)    // QE为非易失性寄存器位。为1表示可以使用四线（Quad）操作。
#define QSPI_FSR_LB1                    ((uint8_t)0x08)    // LB1~LB3为非易失性寄存器位且只能写入一次，默认为0。用来控制是否可以写入安全寄存器。一旦LB1~LB3被写入1，则安全寄存器将永久性变为只读寄存器。
#define QSPI_FSR_LB2                    ((uint8_t)0x10)    // LB1~LB3为非易失性寄存器位且只能写入一次，默认为0。用来控制是否可以写入安全寄存器。一旦LB1~LB3被写入1，则安全寄存器将永久性变为只读寄存器。
#define QSPI_FSR_LB3                    ((uint8_t)0x20)    // LB1~LB3为非易失性寄存器位且只能写入一次，默认为0。用来控制是否可以写入安全寄存器。一旦LB1~LB3被写入1，则安全寄存器将永久性变为只读寄存器。
#define QSPI_FSR_CMP                    ((uint8_t)0x40)    // CMP为非易失性寄存器位，默认为0。与BP0~BP4一起提供更大的保护范围。
#define QSPI_FSR_SUS1                   ((uint8_t)0x80)    // 只读寄存器位，为1表示执行了EraseSuspend（擦除暂停）命令。它会在执行擦除恢复指令后自动变为0。
#define QSPI_FSR_SUS2                   ((uint8_t)0x04)    // 只读寄存器位，为1表示执行了ProgramSuspend（编程暂停）命令。它会在执行编程恢复指令后自动变为0。
#define QSPI_FSR_HPF                    ((uint8_t)0x10)    // 只读寄存器位，为1表示处于高性能模式（High Performance Mode）。
#define QSPI_FSR_DRV0                   ((uint8_t)0x20)    // DRV0和DRV1位配合使用，用来设置Flash的IO口驱动强度，默认DRV1=0,DRV0=1。
#define QSPI_FSR_DRV1                   ((uint8_t)0x40)    // DRV0和DRV1位配合使用，用来设置Flash的IO口驱动强度，默认DRV1=0,DRV0=1。
/**
 * 初始化QSPI-FLASH。
 * 将执行以下命令：
 *   复位Flash
 *   将Flash从Deep-Sleep状态中唤醒
 */
uint8_t QSPI_InitFlash(void);

/**
 * 读取JEDEC ID。
 *
 * [31:24] : 0
 * [23:16] : MANUFACTURER ID
 * [15:8]  : ID15-ID8
 * [7:0]   : ID7-ID0
 */
uint32_t QSPI_ReadJEDEC(void);

/**
 * 读取状态寄存器
 */
uint8_t QSPI_ReadStatusReg(uint8_t regNo, uint8_t* reg);

/**
 * 写入状态寄存器。
 */
uint8_t QSPI_WriteStatusReg(uint8_t regNo, uint8_t regValue);

/**
 * 使能四线模式。
 *
 * 四线模式读取数据前必须使能四线模式
 */
uint8_t QSPI_SetQuardEnable(bool enable);

/**
 * 擦除整个FLASH芯片
 */
uint8_t QSPI_EraseChip(void);

/**
 * 擦除指定的64KB块
 */
uint8_t QSPI_EraseBlock64K(uint32_t BlockAddress);

/**
 * 擦除指定的区域
 */
uint8_t QSPI_EraseSector(uint32_t SectorAddress);

/**
 * 单线方式读取指定位置指定长度的数据
 */
uint8_t QSPI_Read(uint8_t* pData, uint32_t ReadAddr, uint32_t Size);

/**
 * 四线方式读取指定位置指定长度的数据
 */
uint8_t QSPI_QuadOutputFastRead(uint8_t* pData, uint32_t ReadAddr, uint32_t Size);

/**
 * 单线方式在指定地址写入数据。
 */
uint8_t QSPI_Write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size);

/**
 * 在四线模式下启动内存映射模式
 */
uint8_t QSPI_EnableMemoryMapped(void);

/**
 * 在双线模式下启动内存映射模式
 */
uint8_t QSPI_EnableDualLineMemoryMapped(void);

/**
 * 退出内存映射模式。
 */
uint8_t QSPI_ExitMemoryMapping(void);

/* DMA方式 + 中断方式 */
uint32_t QSPI_W25Qx_ReadID_DMA(void);
void QSPI_W25Qx_Read_Buffer_DMA(uint8_t *_pBuf,uint32_t _read_Addr,uint32_t _read_Size);
uint8_t QSPI_W25Qx_Write_Buffer_DMA(uint8_t *_pBuf,uint32_t _write_Addr,uint16_t _write_Size);
#endif /* __SPI_FLASH_H */

