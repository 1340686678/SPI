#include "QSPI_Flash.h"

static uint8_t QSPI_AutoPollingMemReady(uint32_t timeout)
{
	QSPI_CommandTypeDef     s_command;
	QSPI_AutoPollingTypeDef s_config;
	/* �����Զ���ѯģʽ�ȴ��洢��׼������ */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = QSPI_CMD_READ_STATUS_REG1;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_1_LINE;
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	//��ȡMaskλ�ж��Ƿ�ΪMatch
	s_config.Match           = 0x00;		
	s_config.Mask            = QSPI_FSR_WIP;
	s_config.MatchMode       = QSPI_MATCH_MODE_AND;	//���ж�
	s_config.StatusBytesSize = 1;					//״̬�ֽڴ�С
	s_config.Interval        = 0x10;				//�Զ���ѯ ���ζ�ȡ ���ʱ��������
	s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;	//ƥ���ֹͣ��ѯ

	//��ѯ�ж�״̬�Ĵ���1��BUSYλ
	if (HAL_QSPI_AutoPolling(&QSPI_HANDLE, &s_command, &s_config, timeout) != HAL_OK)
	{
		return QSPI_ERROR;
	}
	return QSPI_OK;
}

static uint8_t QSPI_AutoPollingWriteEnabled()
{
	QSPI_CommandTypeDef     s_command;
	QSPI_AutoPollingTypeDef s_config;

	/* ����д���� */

	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = QSPI_CMD_WRITE_ENABLE;	//д��дʹ��
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_NONE;
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(&QSPI_HANDLE, &s_command, CONTROL_COMMANS_MAX_TIME) != HAL_OK)
	{
		return QSPI_ERROR;
	}

	/* �����Զ���ѯģʽ�ȴ�д���� */
	//��ѯ��ȡ״̬�Ĵ���bit1(WEL)дʹ��
	s_config.Match           = QSPI_FSR_WEL;
	s_config.Mask            = QSPI_FSR_WEL;
	s_config.MatchMode       = QSPI_MATCH_MODE_AND;
	s_config.StatusBytesSize = 1;
	s_config.Interval        = 0x10;
	s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

	s_command.Instruction    = QSPI_CMD_READ_STATUS_REG1;
	s_command.DataMode       = QSPI_DATA_1_LINE;
	s_command.NbData         = 1;

	if (HAL_QSPI_AutoPolling(&QSPI_HANDLE, &s_command, &s_config, AUTO_PULLING_WAIT_TIME) != HAL_OK)
	{
		return QSPI_ERROR;
	}
	return QSPI_OK;
}

/**
 * ʹ������ģʽ��
 *
 * ����ģʽ��ȡ����ǰ����ʹ������ģʽ
 */
uint8_t QSPI_SetQuardEnable(bool enable){
	// ��ȡ״̬�Ĵ���
	uint8_t regValue;
	if (QSPI_ReadStatusReg(2, &regValue) != QSPI_OK) {
		return QSPI_ERROR;
	}

	enable = enable ? QSPI_FSR_QE : 0;
	if((regValue & QSPI_FSR_QE) == (enable ? QSPI_FSR_QE : 0) ){
		return QSPI_OK;
	}

	if(enable){
		regValue |= QSPI_FSR_QE;
	}else{
		regValue &= (~QSPI_FSR_QE);
	}

	// д��״̬�Ĵ���
	if (QSPI_WriteStatusReg(2, regValue) != QSPI_OK) {
		return QSPI_ERROR;
	}

	// �����Զ���ѯģʽ�ȴ�QE����
	QSPI_CommandTypeDef s_command;
	QSPI_AutoPollingTypeDef s_config;
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = QSPI_CMD_READ_STATUS_REG2;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_1_LINE;
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	s_config.Match              = enable ? QSPI_FSR_QE : 0;
	s_config.Mask               = QSPI_FSR_QE;
	s_config.MatchMode          = QSPI_MATCH_MODE_AND;
	s_config.StatusBytesSize    = 1;
	s_config.Interval           = 0x10;
	s_config.AutomaticStop      = QSPI_AUTOMATIC_STOP_ENABLE;
	if (HAL_QSPI_AutoPolling(&QSPI_HANDLE, &s_command, &s_config, AUTO_PULLING_WAIT_TIME) != HAL_OK) {
		return QSPI_ERROR;
	}
	
	return QSPI_OK;
}

uint8_t QSPI_InitFlash(void){
	QSPI_CommandTypeDef s_command;
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_NONE;
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	// ʹ�ܸ�λ
	s_command.Instruction = QSPI_CMD_ENABLE_RESET;
	if (HAL_QSPI_Command(&QSPI_HANDLE, &s_command, CONTROL_COMMANS_MAX_TIME) != HAL_OK){
		return QSPI_ERROR;
	}

	// ��λFLASH
	s_command.Instruction = QSPI_CMD_RESET;
	if (HAL_QSPI_Command(&QSPI_HANDLE, &s_command, CONTROL_COMMANS_MAX_TIME) != HAL_OK) {
		return QSPI_ERROR;
	}

	// �ȴ���λ�����
	if (QSPI_AutoPollingMemReady(AUTO_PULLING_WAIT_TIME) != QSPI_OK)
	{
		return QSPI_ERROR;
	}

	return QSPI_OK;
}

uint32_t QSPI_ReadJEDEC(void){
	QSPI_CommandTypeDef s_command;

	s_command.Instruction       = QSPI_CMD_READ_IDENTIFICATION;

	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_1_LINE;
	s_command.DummyCycles       = 0;
	s_command.NbData            = 3;	//����3λ
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;	//������DDR
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(&QSPI_HANDLE, &s_command, CONTROL_COMMANS_MAX_TIME) != HAL_OK) {
		return 0;
	}

	uint8_t value[3] = {0};
	if (HAL_QSPI_Receive(&QSPI_HANDLE, value, CONTROL_COMMANS_MAX_TIME) != HAL_OK) {
		return 0;
	}

	return value[0] << 16 | value[1] << 8 | value[2];
}

uint8_t QSPI_ReadStatusReg(uint8_t regNo, uint8_t *value){

	QSPI_CommandTypeDef s_command;

	if (regNo == 1) {
		s_command.Instruction = QSPI_CMD_READ_STATUS_REG1;
	} else if (regNo == 2) {
		s_command.Instruction = QSPI_CMD_READ_STATUS_REG2;
	} else if (regNo == 3) {
		s_command.Instruction = QSPI_CMD_READ_STATUS_REG3;
	} else {
		return QSPI_ERROR;
	}

	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_1_LINE;
	s_command.DummyCycles       = 0;
	s_command.NbData            = 1;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(&QSPI_HANDLE, &s_command, CONTROL_COMMANS_MAX_TIME) != HAL_OK) {
		return QSPI_ERROR;
	}

	if (HAL_QSPI_Receive(&QSPI_HANDLE, value, CONTROL_COMMANS_MAX_TIME) != HAL_OK) {
		return QSPI_ERROR;
	}
	return QSPI_OK;
}

/**
  * @brief  ��ȡReadStatusReg
  * @param 	��
  * @retval ReadStatusReg
  */
uint8_t QSPI_WriteStatusReg(uint8_t regNo, uint8_t regValue) {
	QSPI_CommandTypeDef s_command;

	if (regNo == 1) {
		s_command.Instruction = QSPI_CMD_WRITE_STATUS_REG1;
	} else if (regNo == 2) {
		s_command.Instruction = QSPI_CMD_WRITE_STATUS_REG2;
	} else if (regNo == 3) {
		s_command.Instruction = QSPI_CMD_WRITE_STATUS_REG3;
	} else {
		return QSPI_ERROR;
	}

	/* ʹ��д���� */
	if (QSPI_AutoPollingWriteEnabled() != QSPI_OK) {
		return QSPI_ERROR;
	}

	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_1_LINE;
	s_command.DummyCycles       = 0;
	s_command.NbData            = 1;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(&QSPI_HANDLE, &s_command, CONTROL_COMMANS_MAX_TIME) != HAL_OK)
	{
		return QSPI_ERROR;
	}
	if (HAL_QSPI_Transmit(&QSPI_HANDLE, &regValue, CONTROL_COMMANS_MAX_TIME) != HAL_OK)
	{
		return QSPI_ERROR;
	}
	/* �Զ���ѯģʽ�ȴ��洢������ */
	if (QSPI_AutoPollingMemReady(AUTO_PULLING_WAIT_TIME) != QSPI_OK)
	{
		return QSPI_ERROR;
	}

	return QSPI_OK;
}

uint8_t QSPI_EraseChip(void)
{
	QSPI_CommandTypeDef s_command;
	/* ��ʼ���������� */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = QSPI_CMD_CHIP_ERASE;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_NONE;
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* ����д���� */
	if (QSPI_AutoPollingWriteEnabled() != QSPI_OK)
	{
		return QSPI_ERROR;
	}

	/* �������� */
	if (HAL_QSPI_Command(&QSPI_HANDLE, &s_command, CONTROL_COMMANS_MAX_TIME) != HAL_OK)
	{
		return QSPI_ERROR;
	}

	/* �����Զ���ѯģʽ�ȴ��������� */
	if (QSPI_AutoPollingMemReady(CHIP_ERASE_MAX_TIME) != QSPI_OK)
	{
		return QSPI_ERROR;
	}

	return QSPI_OK;
}

uint8_t QSPI_EraseBlock64K(uint32_t BlockAddress) {
	// ��ʼ����������
	QSPI_CommandTypeDef s_command;
	s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction = QSPI_CMD_BLOCK64K_ERASE;
	s_command.AddressMode = QSPI_ADDRESS_1_LINE;
	s_command.AddressSize = QSPI_ADDRESS_24_BITS;
	s_command.Address = BlockAddress;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode = QSPI_DATA_NONE;
	s_command.DummyCycles = 0;
	s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	// ����д����
	if (QSPI_AutoPollingWriteEnabled() != QSPI_OK) {
		return QSPI_ERROR;
	}

	// ���Ͳ�������
	if (HAL_QSPI_Command(&QSPI_HANDLE, &s_command, CONTROL_COMMANS_MAX_TIME) != HAL_OK) {
		return QSPI_ERROR;
	}

	// �����Զ���ѯģʽ�ȴ���������
	if (QSPI_AutoPollingMemReady(BLOCK_ERASE_MAX_TIME) != QSPI_OK) {
		return QSPI_ERROR;
	}

	return QSPI_OK;
}

uint8_t QSPI_EraseSector(uint32_t SectorAddress) {
	// ��ʼ����������
	QSPI_CommandTypeDef s_command;
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = QSPI_CMD_SECTOR_ERASE;
	s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
	s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
	s_command.Address           = SectorAddress;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_NONE;
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
	// ����д����
	if (QSPI_AutoPollingWriteEnabled() != QSPI_OK) {
		return QSPI_ERROR;
	}

	// ���Ͳ�������
	if (HAL_QSPI_Command(&QSPI_HANDLE, &s_command, CONTROL_COMMANS_MAX_TIME) != HAL_OK) {
		return QSPI_ERROR;
	}

	// �����Զ���ѯģʽ�ȴ���������
	if (QSPI_AutoPollingMemReady(SECTOR_ERASE_MAX_TIME) != QSPI_OK) {
		return QSPI_ERROR;
	}

	return QSPI_OK;
}

uint8_t QSPI_Write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size) {
	QSPI_CommandTypeDef s_command;
	uint32_t end_addr, currentSize, currentAddr;

	//��С�� ����Ҫд��
	if (Size == 0) {
		return QSPI_OK;
	}

	/* ����д���ַ��ҳ��ĩβ֮��Ĵ�С */
	currentAddr = 0;

	//�ж�writerAdd���ĸ�ҳ
	while (currentAddr <= WriteAddr) {
		currentAddr += QSPI_PAGE_SIZE;
	}
	//writer��ַ�����ҳ����д������ֽ�����
	currentSize = currentAddr - WriteAddr;

	/* ������ݵĴ�С�Ƿ�С��ҳ���е�ʣ��λ�� */
	if (currentSize > Size) {
		currentSize = Size;
	}

	/* ��ʼ����ַ���� */
	currentAddr = WriteAddr;		//д��ַ
	end_addr = WriteAddr + Size;	//ֹͣ��ַ

	/* ��ʼ���������� */
	s_command.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction        = QSPI_CMD_PAGE_PROGRAM;	//ҳд����
	s_command.AddressMode        = QSPI_ADDRESS_1_LINE;		//����д���ַ
	s_command.AddressSize        = QSPI_ADDRESS_24_BITS;	//24λ��ַ
	s_command.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode           = QSPI_DATA_1_LINE;		//����д������
	s_command.DummyCycles        = 0;
	s_command.DdrMode            = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

	/* ��ҳִ��д�� */
	do {
		s_command.Address = currentAddr;
		if (currentSize == 0) {
			return QSPI_OK;
		}

		s_command.NbData = currentSize;

		/* ����д���� */
		if (QSPI_AutoPollingWriteEnabled() != QSPI_OK) {
			return QSPI_ERROR;
		}

		/* �������� */
		if (HAL_QSPI_Command(&QSPI_HANDLE, &s_command, CONTROL_COMMANS_MAX_TIME) != HAL_OK) {
			return QSPI_ERROR;
		}

		/* �������� */
		if (HAL_QSPI_Transmit(&QSPI_HANDLE, pData, CONTROL_COMMANS_MAX_TIME) != HAL_OK) {
			return QSPI_ERROR;
		}

		/* �����Զ���ѯģʽ�ȴ�������� */
		if (QSPI_AutoPollingMemReady(AUTO_PULLING_WAIT_TIME) != QSPI_OK) {
			return QSPI_ERROR;
		}

		/* ������һҳ��̵ĵ�ַ�ʹ�С���� */
		currentAddr += currentSize;
		pData += currentSize;
		currentSize = ((currentAddr + QSPI_PAGE_SIZE) > end_addr) ? (end_addr - currentAddr) : QSPI_PAGE_SIZE;
	} while (currentAddr < end_addr);
	return QSPI_OK;
}

uint8_t QSPI_Read(uint8_t* pData, uint32_t ReadAddr, uint32_t Size) {
	QSPI_CommandTypeDef s_command;

	if (Size == 0) {
		printf("BSP_QSPI_Read Size = 0");
		return QSPI_OK;
	}

	/* ��ʼ�������� */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = QSPI_CMD_READ_DATA;
	s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
	s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
	s_command.Address           = ReadAddr;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_1_LINE;
	s_command.DummyCycles       = 0;
	s_command.NbData            = Size;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* �������� */
	if (HAL_QSPI_Command(&QSPI_HANDLE, &s_command, CONTROL_COMMANS_MAX_TIME) != HAL_OK) {
		return QSPI_ERROR;
	}

	/* �������� */
	if (HAL_QSPI_Receive(&QSPI_HANDLE, pData, CONTROL_COMMANS_MAX_TIME) != HAL_OK) {
		return QSPI_ERROR;
	}
	return QSPI_OK;
}

uint8_t QSPI_QuadOutputFastRead(uint8_t* pData, uint32_t ReadAddr, uint32_t Size) {
	QSPI_CommandTypeDef s_command;

	if (Size == 0) {
		return QSPI_OK;
	}

	// ������߿��ٶ�ȡ�Ƿ����
	uint8_t regValue;
	if (QSPI_ReadStatusReg(2, &regValue) != QSPI_OK) {
		return QSPI_ERROR;
	}

	if((regValue & QSPI_FSR_QE) != QSPI_FSR_QE){
		return QSPI_NOT_SUPPORTED;
	}

	/* ��ʼ�������� */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = QSPI_CMD_QUAD_OUPUT_FAST_READ;
	s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
	s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
	s_command.Address           = ReadAddr;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_4_LINES;
	s_command.DummyCycles       = DUMMY_CYCLES_QUAD_READ;
	s_command.NbData            = Size;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* �������� */
	if (HAL_QSPI_Command(&QSPI_HANDLE, &s_command, CONTROL_COMMANS_MAX_TIME) != HAL_OK) {
		return QSPI_ERROR;
	}

	/* �������� */
	if (HAL_QSPI_Receive(&QSPI_HANDLE, pData, CONTROL_COMMANS_MAX_TIME) != HAL_OK) {
		return QSPI_ERROR;
	}
	return QSPI_OK;
}

uint8_t QSPI_EnableMemoryMapped(void){
	QSPI_CommandTypeDef sCommand;

	// Instruction every command
	sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction        = QSPI_CMD_QUAD_OUPUT_FAST_READ;
	sCommand.AddressMode        = QSPI_ADDRESS_1_LINE;
	sCommand.Address            = 0;
	sCommand.AddressSize        = QSPI_ADDRESS_24_BITS;
	sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode           = QSPI_DATA_4_LINES;
	sCommand.DummyCycles        = DUMMY_CYCLES_QUAD_READ;
	sCommand.NbData             = 0;
	sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

	// Instruction only first cmd(Better Performance)
	sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction        = QSPI_CMD_QUAD_IO_FAST_READ;
	sCommand.AddressMode        = QSPI_ADDRESS_4_LINES;
	sCommand.Address            = 0;
	sCommand.AddressSize        = QSPI_ADDRESS_24_BITS;
	sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_4_LINES;
	sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
	sCommand.AlternateBytes     = 0x20;
	sCommand.DataMode           = QSPI_DATA_4_LINES;
	sCommand.DummyCycles        = DUMMY_CYCLES_QUAD_IO_FAST_READ;
	sCommand.NbData             = 0;
	sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode           = QSPI_SIOO_INST_ONLY_FIRST_CMD;

	QSPI_MemoryMappedTypeDef sMemMappedCfg;
	sMemMappedCfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;

	if (HAL_QSPI_MemoryMapped(&hqspi, &sCommand, &sMemMappedCfg) != HAL_OK) {
		return QSPI_ERROR;
	}

	return QSPI_OK;
}

/**
 * ��˫��ģʽ�������ڴ�ӳ��ģʽ
 */
uint8_t QSPI_EnableDualLineMemoryMapped(void){
	QSPI_CommandTypeDef sCommand;

	// Instruction every command
	sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction        = QSPI_CMD_DUAL_OUTPUT_FAST_READ;
	sCommand.AddressMode        = QSPI_ADDRESS_1_LINE;
	sCommand.Address            = 0;
	sCommand.AddressSize        = QSPI_ADDRESS_24_BITS;
	sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode           = QSPI_DATA_2_LINES;
	sCommand.DummyCycles        = DUMMY_CYCLES_DUAL_READ;
	sCommand.NbData             = 0;
	sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

	QSPI_MemoryMappedTypeDef sMemMappedCfg;
	sMemMappedCfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;

	if (HAL_QSPI_MemoryMapped(&hqspi, &sCommand, &sMemMappedCfg) != HAL_OK) {
		return QSPI_ERROR;
	}

	return QSPI_OK;
}

uint8_t QSPI_ExitMemoryMapping(void) {
	if (HAL_QSPI_Abort(&hqspi) != HAL_OK) {
		return QSPI_ERROR;
	}

	return QSPI_OK;
}
