#include "QSPI_Flash.h"

static uint8_t QSPI_AutoPollingMemReady(uint32_t timeout)
{
	QSPI_CommandTypeDef     s_command;
	QSPI_AutoPollingTypeDef s_config;
	/* 配置自动轮询模式等待存储器准备就绪 */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = QSPI_CMD_READ_STATUS_REG1;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_1_LINE;
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	//读取Mask位判断是否为Match
	s_config.Match           = 0x00;		
	s_config.Mask            = QSPI_FSR_WIP;
	s_config.MatchMode       = QSPI_MATCH_MODE_AND;	//与判断
	s_config.StatusBytesSize = 1;					//状态字节大小
	s_config.Interval        = 0x10;				//自动轮询 两次读取 间隔时钟周期数
	s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;	//匹配后停止轮询

	//轮询判断状态寄存器1的BUSY位
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

	/* 启用写操作 */

	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = QSPI_CMD_WRITE_ENABLE;	//写入写使能
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

	/* 配置自动轮询模式等待写启用 */
	//轮询读取状态寄存器bit1(WEL)写使能
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
 * 使能四线模式。
 *
 * 四线模式读取数据前必须使能四线模式
 */
uint8_t QSPI_SetQuardEnable(bool enable){
	// 读取状态寄存器
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

	// 写入状态寄存器
	if (QSPI_WriteStatusReg(2, regValue) != QSPI_OK) {
		return QSPI_ERROR;
	}

	// 配置自动轮询模式等待QE就绪
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

	// 使能复位
	s_command.Instruction = QSPI_CMD_ENABLE_RESET;
	if (HAL_QSPI_Command(&QSPI_HANDLE, &s_command, CONTROL_COMMANS_MAX_TIME) != HAL_OK){
		return QSPI_ERROR;
	}

	// 复位FLASH
	s_command.Instruction = QSPI_CMD_RESET;
	if (HAL_QSPI_Command(&QSPI_HANDLE, &s_command, CONTROL_COMMANS_MAX_TIME) != HAL_OK) {
		return QSPI_ERROR;
	}

	// 等待复位置完成
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
	s_command.NbData            = 3;	//传输3位
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;	//不开启DDR
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
  * @brief  读取ReadStatusReg
  * @param 	无
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

	/* 使能写操作 */
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
	/* 自动轮询模式等待存储器就绪 */
	if (QSPI_AutoPollingMemReady(AUTO_PULLING_WAIT_TIME) != QSPI_OK)
	{
		return QSPI_ERROR;
	}

	return QSPI_OK;
}

uint8_t QSPI_EraseChip(void)
{
	QSPI_CommandTypeDef s_command;
	/* 初始化擦除命令 */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = QSPI_CMD_CHIP_ERASE;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_NONE;
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* 启用写操作 */
	if (QSPI_AutoPollingWriteEnabled() != QSPI_OK)
	{
		return QSPI_ERROR;
	}

	/* 发送命令 */
	if (HAL_QSPI_Command(&QSPI_HANDLE, &s_command, CONTROL_COMMANS_MAX_TIME) != HAL_OK)
	{
		return QSPI_ERROR;
	}

	/* 配置自动轮询模式等待擦除结束 */
	if (QSPI_AutoPollingMemReady(CHIP_ERASE_MAX_TIME) != QSPI_OK)
	{
		return QSPI_ERROR;
	}

	return QSPI_OK;
}

uint8_t QSPI_EraseBlock64K(uint32_t BlockAddress) {
	// 初始化擦除命令
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

	// 启用写操作
	if (QSPI_AutoPollingWriteEnabled() != QSPI_OK) {
		return QSPI_ERROR;
	}

	// 发送擦除命令
	if (HAL_QSPI_Command(&QSPI_HANDLE, &s_command, CONTROL_COMMANS_MAX_TIME) != HAL_OK) {
		return QSPI_ERROR;
	}

	// 配置自动轮询模式等待擦除结束
	if (QSPI_AutoPollingMemReady(BLOCK_ERASE_MAX_TIME) != QSPI_OK) {
		return QSPI_ERROR;
	}

	return QSPI_OK;
}

uint8_t QSPI_EraseSector(uint32_t SectorAddress) {
	// 初始化擦除命令
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
	// 启用写操作
	if (QSPI_AutoPollingWriteEnabled() != QSPI_OK) {
		return QSPI_ERROR;
	}

	// 发送擦除命令
	if (HAL_QSPI_Command(&QSPI_HANDLE, &s_command, CONTROL_COMMANS_MAX_TIME) != HAL_OK) {
		return QSPI_ERROR;
	}

	// 配置自动轮询模式等待擦除结束
	if (QSPI_AutoPollingMemReady(SECTOR_ERASE_MAX_TIME) != QSPI_OK) {
		return QSPI_ERROR;
	}

	return QSPI_OK;
}

uint8_t QSPI_Write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size) {
	QSPI_CommandTypeDef s_command;
	uint32_t end_addr, currentSize, currentAddr;

	//大小零 不需要写入
	if (Size == 0) {
		return QSPI_OK;
	}

	/* 计算写入地址和页面末尾之间的大小 */
	currentAddr = 0;

	//判断writerAdd在哪个页
	while (currentAddr <= WriteAddr) {
		currentAddr += QSPI_PAGE_SIZE;
	}
	//writer地址在这个页中能写入多少字节数据
	currentSize = currentAddr - WriteAddr;

	/* 检查数据的大小是否小于页面中的剩余位置 */
	if (currentSize > Size) {
		currentSize = Size;
	}

	/* 初始化地址变量 */
	currentAddr = WriteAddr;		//写地址
	end_addr = WriteAddr + Size;	//停止地址

	/* 初始化程序命令 */
	s_command.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction        = QSPI_CMD_PAGE_PROGRAM;	//页写命令
	s_command.AddressMode        = QSPI_ADDRESS_1_LINE;		//单线写入地址
	s_command.AddressSize        = QSPI_ADDRESS_24_BITS;	//24位地址
	s_command.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode           = QSPI_DATA_1_LINE;		//单线写入数据
	s_command.DummyCycles        = 0;
	s_command.DdrMode            = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

	/* 逐页执行写入 */
	do {
		s_command.Address = currentAddr;
		if (currentSize == 0) {
			return QSPI_OK;
		}

		s_command.NbData = currentSize;

		/* 启用写操作 */
		if (QSPI_AutoPollingWriteEnabled() != QSPI_OK) {
			return QSPI_ERROR;
		}

		/* 配置命令 */
		if (HAL_QSPI_Command(&QSPI_HANDLE, &s_command, CONTROL_COMMANS_MAX_TIME) != HAL_OK) {
			return QSPI_ERROR;
		}

		/* 传输数据 */
		if (HAL_QSPI_Transmit(&QSPI_HANDLE, pData, CONTROL_COMMANS_MAX_TIME) != HAL_OK) {
			return QSPI_ERROR;
		}

		/* 配置自动轮询模式等待程序结束 */
		if (QSPI_AutoPollingMemReady(AUTO_PULLING_WAIT_TIME) != QSPI_OK) {
			return QSPI_ERROR;
		}

		/* 更新下一页编程的地址和大小变量 */
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

	/* 初始化读命令 */
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

	/* 配置命令 */
	if (HAL_QSPI_Command(&QSPI_HANDLE, &s_command, CONTROL_COMMANS_MAX_TIME) != HAL_OK) {
		return QSPI_ERROR;
	}

	/* 接收数据 */
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

	// 检查四线快速读取是否可用
	uint8_t regValue;
	if (QSPI_ReadStatusReg(2, &regValue) != QSPI_OK) {
		return QSPI_ERROR;
	}

	if((regValue & QSPI_FSR_QE) != QSPI_FSR_QE){
		return QSPI_NOT_SUPPORTED;
	}

	/* 初始化读命令 */
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

	/* 配置命令 */
	if (HAL_QSPI_Command(&QSPI_HANDLE, &s_command, CONTROL_COMMANS_MAX_TIME) != HAL_OK) {
		return QSPI_ERROR;
	}

	/* 接收数据 */
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
 * 在双线模式下启动内存映射模式
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
