#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include "main.h"
#include "stdbool.h"
#include "quadspi.h"

//***************************************************
//* ͨ�ö���
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
//* STM32���
//***************************************************

#define QSPI_HANDLE        hqspi

//***************************************************
//* ��Ϊ����
//***************************************************

#define QSPI_WRITE_AUTO_ENABLE           true
#define QSPI_QUAD_READ_AUTO_ENABLE       true

//***************************************************
//* FLASH���ã���ο������ֲ�
//***************************************************

#define QSPI_FLASH_SIZE                  0x800000  // 8   MBytes
#define QSPI_BLOCK_SIZE                  0x10000   // 64  KBytes
#define QSPI_SUBSECTOR_SIZE              0x1000    // 4   KBytes
#define QSPI_PAGE_SIZE                   0x100     // 256 Bytes

#define DUMMY_CYCLES_READ                0         // ������ȡ����ʱ�ĵȴ�ѭ��
#define DUMMY_CYCLES_FAST_READ           8         // ���ٶ�ȡ����ʱ�ĵȴ�ѭ��
#define DUMMY_CYCLES_DUAL_READ           8         // ˫��ģʽ��ȡ����ʱ�ĵȴ�ѭ��
#define DUMMY_CYCLES_QUAD_READ           8         // ����ģʽ��ȡ����ʱ�ĵȴ�ѭ��
#define DUMMY_CYCLES_DUAL_IO_FAST_READ   0         // ˫������ģʽ��ȡ����ʱ�ĵȴ�ѭ��
#define DUMMY_CYCLES_QUAD_IO_FAST_READ   4         // ��������ģʽ��ȡ����ʱ�ĵȴ�ѭ��

#define CHIP_ERASE_MAX_TIME             250000     // оƬ����ʱ��ȴ�ʱ��
#define BLOCK_ERASE_MAX_TIME            2000       // �����ʱ��ȴ�ʱ��
#define SECTOR_ERASE_MAX_TIME           500        // ������ʱ��ȴ�ʱ��
#define CONTROL_COMMANS_MAX_TIME        1000       // ������ָ��(״̬�Ĵ����������Ĵ���)����ʱ����ȴ�ʱ��
#define AUTO_PULLING_WAIT_TIME          1000       // �����ȴ�оƬ״̬ʱ����ȴ�ʱ��

//***************************************************
//* ָ���
//***************************************************

#if (FLASH_MODEL == FLASH_GD25Q64)

// д�뱣�����ָ��
#define QSPI_CMD_WRITE_ENABLE                      0x06
#define QSPI_CMD_WRITE_DISABLE                     0x04
#define QSPI_CMD_VOLATILE_SR_WRITE_ENABLE          0x50

// ״̬�Ĵ�������
#define QSPI_CMD_READ_STATUS_REG1                  0x05
#define QSPI_CMD_READ_STATUS_REG2                  0x35
#define QSPI_CMD_READ_STATUS_REG3                  0x15

#define QSPI_CMD_WRITE_STATUS_REG1                 0x01
#define QSPI_CMD_WRITE_STATUS_REG2                 0x31
#define QSPI_CMD_WRITE_STATUS_REG3                 0x11

// ���ݶ�ȡָ��
#define QSPI_CMD_READ_DATA                         0x03
#define QSPI_CMD_FAST_READ                         0x0B
#define QSPI_CMD_DUAL_OUTPUT_FAST_READ             0x3B
#define QSPI_CMD_DUAL_IO_FAST_READ                 0xBB
#define QSPI_CMD_QUAD_OUPUT_FAST_READ              0x6B
#define QSPI_CMD_QUAD_IO_FAST_READ                 0xEB
#define QSPI_CMD_QUAD_IO_WORD_FAST_READ            0xE7

// ���ָ��
#define QSPI_CMD_PAGE_PROGRAM                      0x02
#define QSPI_CMD_QUAD_PAGE_PROGRAM                 0x32
#define QSPI_CMD_FAST_PAGE_PROGRAM                 0xF2

// ����ָ��
#define QSPI_CMD_SECTOR_ERASE                      0x20
#define QSPI_CMD_BLOCK32K_ERASE                    0x52
#define QSPI_CMD_BLOCK64K_ERASE                    0xD8
#define QSPI_CMD_CHIP_ERASE                        0x60

// ����ָ��
#define QSPI_CMD_ENABLE_RESET                      0x66
#define QSPI_CMD_RESET                             0x99
#define QSPI_CMD_SET_BURST_WITH_WRAP               0x77
#define QSPI_CMD_PROGRAM_ERASE_SUSPEND             0x75
#define QSPI_CMD_PROGRAM_ERASE_RESUME              0x7A
#define QSPI_CMD_RELEASE_FROM_DEEP_POWER_DOWN      0xAB
#define QSPI_CMD_DEEP_POWER_DOWN                   0xB9

// ʶ��ָ��
#define QSPI_CMD_READ_DEVICE_ID                    0xAB  // ͬʱҲ�ὫFlashоƬ��DeepSleep״̬�л���
#define QSPI_CMD_MANUFACTURER_DEVICE_ID            0x90
#define QSPI_CMD_MANUFACTURER_DEVICE_ID_DUAL_IO    0x92
#define QSPI_CMD_MANUFACTURER_DEVICE_ID_QUAD_IO    0x94
#define QSPI_CMD_READ_IDENTIFICATION               0x9F
#define QSPI_CMD_READ_UNIQUE_ID                    0x4B

// ����ָ��
#define QSPI_CMD_HIGH_PERFORMANCE_MODE             0xA3
#define QSPI_CMD_READ_DISCOVERABLE_PARAM           0x5A
#define QSPI_CMD_ERASE_SECURITY_REG                0x44
#define QSPI_CMD_PROGRAM_SECURITY_REG              0x42
#define QSPI_CMD_READ_SECURITY_REG                 0x48

#endif

//***************************************************
//* ״̬�Ĵ�����־λ(Flash Status Register/FSR)
//***************************************************

#define QSPI_FSR_WIP                    ((uint8_t)0x01)    // ֻ���Ĵ���λ����æλ  Ϊ1��æ
#define QSPI_FSR_WEL                    ((uint8_t)0x02)    // ֻ���Ĵ���λ��дʹ��  Ϊ1ʹ�� 
#define QSPI_FSR_BP0                    ((uint8_t)0x04)    // BP0~BP4Ϊ����ʧ�ԼĴ���λ�����ڱ�����صĴ洢�����ܱ��(PP)����������(SE)�Ϳ����(BE)�����Ӱ�졣���屣���ķ�Χ�ο�Datasheet��
#define QSPI_FSR_BP1                    ((uint8_t)0x08)    // BP0~BP4Ϊ����ʧ�ԼĴ���λ�����ڱ�����صĴ洢�����ܱ��(PP)����������(SE)�Ϳ����(BE)�����Ӱ�졣���屣���ķ�Χ�ο�Datasheet��
#define QSPI_FSR_BP2                    ((uint8_t)0x10)    // BP0~BP4Ϊ����ʧ�ԼĴ���λ�����ڱ�����صĴ洢�����ܱ��(PP)����������(SE)�Ϳ����(BE)�����Ӱ�졣���屣���ķ�Χ�ο�Datasheet��
#define QSPI_FSR_BP3                    ((uint8_t)0x20)    // BP0~BP4Ϊ����ʧ�ԼĴ���λ�����ڱ�����صĴ洢�����ܱ��(PP)����������(SE)�Ϳ����(BE)�����Ӱ�졣���屣���ķ�Χ�ο�Datasheet��
#define QSPI_FSR_BP4                    ((uint8_t)0x40)    // BP0~BP4Ϊ����ʧ�ԼĴ���λ�����ڱ�����صĴ洢�����ܱ��(PP)����������(SE)�Ϳ����(BE)�����Ӱ�졣���屣���ķ�Χ�ο�Datasheet��
#define QSPI_FSR_SRP0                   ((uint8_t)0x80)    // SRP1/SRP0Ϊ����ʧ�ԼĴ���λ�����ڿ���д�����ķ���:���������Ӳ����������Դ��������һ���Ա������������
#define QSPI_FSR_SRP1                   ((uint8_t)0x01)    // SRP1/SRP0Ϊ����ʧ�ԼĴ���λ�����ڿ���д�����ķ���:���������Ӳ����������Դ��������һ���Ա������������
#define QSPI_FSR_QE                     ((uint8_t)0x02)    // QEΪ����ʧ�ԼĴ���λ��Ϊ1��ʾ����ʹ�����ߣ�Quad��������
#define QSPI_FSR_LB1                    ((uint8_t)0x08)    // LB1~LB3Ϊ����ʧ�ԼĴ���λ��ֻ��д��һ�Σ�Ĭ��Ϊ0�����������Ƿ����д�밲ȫ�Ĵ�����һ��LB1~LB3��д��1����ȫ�Ĵ����������Ա�Ϊֻ���Ĵ�����
#define QSPI_FSR_LB2                    ((uint8_t)0x10)    // LB1~LB3Ϊ����ʧ�ԼĴ���λ��ֻ��д��һ�Σ�Ĭ��Ϊ0�����������Ƿ����д�밲ȫ�Ĵ�����һ��LB1~LB3��д��1����ȫ�Ĵ����������Ա�Ϊֻ���Ĵ�����
#define QSPI_FSR_LB3                    ((uint8_t)0x20)    // LB1~LB3Ϊ����ʧ�ԼĴ���λ��ֻ��д��һ�Σ�Ĭ��Ϊ0�����������Ƿ����д�밲ȫ�Ĵ�����һ��LB1~LB3��д��1����ȫ�Ĵ����������Ա�Ϊֻ���Ĵ�����
#define QSPI_FSR_CMP                    ((uint8_t)0x40)    // CMPΪ����ʧ�ԼĴ���λ��Ĭ��Ϊ0����BP0~BP4һ���ṩ����ı�����Χ��
#define QSPI_FSR_SUS1                   ((uint8_t)0x80)    // ֻ���Ĵ���λ��Ϊ1��ʾִ����EraseSuspend��������ͣ�����������ִ�в����ָ�ָ����Զ���Ϊ0��
#define QSPI_FSR_SUS2                   ((uint8_t)0x04)    // ֻ���Ĵ���λ��Ϊ1��ʾִ����ProgramSuspend�������ͣ�����������ִ�б�ָ̻�ָ����Զ���Ϊ0��
#define QSPI_FSR_HPF                    ((uint8_t)0x10)    // ֻ���Ĵ���λ��Ϊ1��ʾ���ڸ�����ģʽ��High Performance Mode����
#define QSPI_FSR_DRV0                   ((uint8_t)0x20)    // DRV0��DRV1λ���ʹ�ã���������Flash��IO������ǿ�ȣ�Ĭ��DRV1=0,DRV0=1��
#define QSPI_FSR_DRV1                   ((uint8_t)0x40)    // DRV0��DRV1λ���ʹ�ã���������Flash��IO������ǿ�ȣ�Ĭ��DRV1=0,DRV0=1��
/**
 * ��ʼ��QSPI-FLASH��
 * ��ִ���������
 *   ��λFlash
 *   ��Flash��Deep-Sleep״̬�л���
 */
uint8_t QSPI_InitFlash(void);

/**
 * ��ȡJEDEC ID��
 *
 * [31:24] : 0
 * [23:16] : MANUFACTURER ID
 * [15:8]  : ID15-ID8
 * [7:0]   : ID7-ID0
 */
uint32_t QSPI_ReadJEDEC(void);

/**
 * ��ȡ״̬�Ĵ���
 */
uint8_t QSPI_ReadStatusReg(uint8_t regNo, uint8_t* reg);

/**
 * д��״̬�Ĵ�����
 */
uint8_t QSPI_WriteStatusReg(uint8_t regNo, uint8_t regValue);

/**
 * ʹ������ģʽ��
 *
 * ����ģʽ��ȡ����ǰ����ʹ������ģʽ
 */
uint8_t QSPI_SetQuardEnable(bool enable);

/**
 * ��������FLASHоƬ
 */
uint8_t QSPI_EraseChip(void);

/**
 * ����ָ����64KB��
 */
uint8_t QSPI_EraseBlock64K(uint32_t BlockAddress);

/**
 * ����ָ��������
 */
uint8_t QSPI_EraseSector(uint32_t SectorAddress);

/**
 * ���߷�ʽ��ȡָ��λ��ָ�����ȵ�����
 */
uint8_t QSPI_Read(uint8_t* pData, uint32_t ReadAddr, uint32_t Size);

/**
 * ���߷�ʽ��ȡָ��λ��ָ�����ȵ�����
 */
uint8_t QSPI_QuadOutputFastRead(uint8_t* pData, uint32_t ReadAddr, uint32_t Size);

/**
 * ���߷�ʽ��ָ����ַд�����ݡ�
 */
uint8_t QSPI_Write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size);

/**
 * ������ģʽ�������ڴ�ӳ��ģʽ
 */
uint8_t QSPI_EnableMemoryMapped(void);

/**
 * ��˫��ģʽ�������ڴ�ӳ��ģʽ
 */
uint8_t QSPI_EnableDualLineMemoryMapped(void);

/**
 * �˳��ڴ�ӳ��ģʽ��
 */
uint8_t QSPI_ExitMemoryMapping(void);

/* DMA��ʽ + �жϷ�ʽ */
uint32_t QSPI_W25Qx_ReadID_DMA(void);
void QSPI_W25Qx_Read_Buffer_DMA(uint8_t *_pBuf,uint32_t _read_Addr,uint32_t _read_Size);
uint8_t QSPI_W25Qx_Write_Buffer_DMA(uint8_t *_pBuf,uint32_t _write_Addr,uint16_t _write_Size);
#endif /* __SPI_FLASH_H */

