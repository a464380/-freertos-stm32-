#include "stm32f1xx_hal.h"                  // Device header
#include "driver_W25Q64_Ins.h"
#include "driver_W25Q64.h"
#include "spi.h"

uint8_t ReadData[1];
uint8_t WriteData[1];

void SPI_Start(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);				//拉低SS，开始时序
}
 
void SPI_Stop(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);				//拉高SS，结束时序
}
										  
/**
  * 函    数：MPU6050读取ID号
  * 参    数：MID 工厂ID，使用输出参数的形式返回
  * 参    数：DID 设备ID，使用输出参数的形式返回
  * 返 回 值：无
  */
void W25Q64_ReadID(uint8_t* ID)
{
	uint8_t cmd[1] = {W25Q64_JEDEC_ID};
	SPI_Start();								//SPI起始
	HAL_SPI_Transmit(&hspi1, cmd, 1, 1000);
	HAL_SPI_Receive(&hspi1, ID, 3, 1000);
	SPI_Stop();								//SPI终止
}
 
 
/**
  * 函    数：W25Q64写使能
  * 参    数：无
  * 返 回 值：无
  */
void W25Q64_WriteEnable(void)
{
	uint8_t cmd[1] = {W25Q64_WRITE_ENABLE};
	SPI_Start();								//SPI起始
	HAL_SPI_Transmit(&hspi1, cmd, 1, 1000);
	SPI_Stop();								//SPI终止
}
 
/**
  * 函    数：W25Q64等待忙
  * 参    数：无
  * 返 回 值：无
  */
void W25Q64_WaitBusy(void)
{
	//uint32_t Timeout;
	uint8_t ret = 0;
	uint32_t Timeout = 10000;
	uint8_t cmd[1] = {W25Q64_READ_STATUS_REGISTER_1};
	SPI_Start();								//SPI起始
	HAL_SPI_Transmit(&hspi1, cmd, 1, 1000);
	do
	{
		HAL_SPI_Receive(&hspi1, &ret, 1, 1000);
		Timeout--;
		if(Timeout == 0)break;
	}while((ret & 0x01) == 0x01);
	SPI_Stop();								//SPI终止	
}
 
/**
  * 函    数：W25Q64页编程
  * 参    数：Address 页编程的起始地址，范围：0x000000~0x7FFFFF
  * 参    数：DataArray	用于写入数据的数组
  * 参    数：Count 要写入数据的数量，范围：0~256
  * 返 回 值：无
  * 注意事项：写入的地址范围不能跨页
  */
void W25Q64_PageProgram(uint32_t Address, uint8_t *DataArray, uint16_t Count)
{
	uint8_t cmd[4] = {W25Q64_PAGE_PROGRAM, (uint8_t)Address>>16, (uint8_t)Address>>8, (uint8_t)Address};
	W25Q64_WriteEnable();						//写使能
	
	SPI_Start();								//SPI起始
	HAL_SPI_Transmit(&hspi1, cmd, 4, 1000);
	HAL_SPI_Transmit(&hspi1, DataArray, Count, 1000);

	SPI_Stop();
	W25Q64_WaitBusy();							//等待忙
}
 
/**
  * 函    数：W25Q64扇区擦除（4KB）
  * 参    数：Address 指定扇区的地址，范围：0x000000~0x7FFFFF
  * 返 回 值：无
  */
void W25Q64_SectorErase(uint32_t Address)
{
	uint8_t cmd[4] = {W25Q64_SECTOR_ERASE_4KB, (uint8_t)Address>>16, (uint8_t)Address>>8, (uint8_t)Address};
	W25Q64_WriteEnable();						//写使能
	SPI_Start();								//SPI起始

	HAL_SPI_Transmit(&hspi1, cmd, 4, 1000);
	SPI_Stop();								//SPI终止
	W25Q64_WaitBusy();							//等待忙
}
 
/**
  * 函    数：W25Q64读取数据
  * 参    数：Address 读取数据的起始地址，范围：0x000000~0x7FFFFF
  * 参    数：DataArray 用于接收读取数据的数组，通过输出参数返回
  * 参    数：Count 要读取数据的数量，范围：0~0x800000
  * 返 回 值：无
  */
void W25Q64_ReadData(uint32_t Address, uint8_t *DataArray, uint32_t Count)
{
	uint8_t cmd[4] = {W25Q64_READ_DATA, (uint8_t)Address>>16, (uint8_t)Address>>8, (uint8_t)Address};

	SPI_Start();								//SPI起始

	HAL_SPI_Transmit(&hspi1, cmd, 4, 1000);

	HAL_SPI_Receive(&hspi1, DataArray, Count, 1000);
	SPI_Stop();								//SPI终止
}