#include "stm32f1xx_hal.h"                  // Device header
#include "driver_W25Q64_Ins.h"
#include "driver_W25Q64.h"
#include "spi.h"

uint8_t ReadData[1];
uint8_t WriteData[1];

void SPI_Start(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);				//����SS����ʼʱ��
}
 
void SPI_Stop(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);				//����SS������ʱ��
}
										  
/**
  * ��    ����MPU6050��ȡID��
  * ��    ����MID ����ID��ʹ�������������ʽ����
  * ��    ����DID �豸ID��ʹ�������������ʽ����
  * �� �� ֵ����
  */
void W25Q64_ReadID(uint8_t* ID)
{
	uint8_t cmd[1] = {W25Q64_JEDEC_ID};
	SPI_Start();								//SPI��ʼ
	HAL_SPI_Transmit(&hspi1, cmd, 1, 1000);
	HAL_SPI_Receive(&hspi1, ID, 3, 1000);
	SPI_Stop();								//SPI��ֹ
}
 
 
/**
  * ��    ����W25Q64дʹ��
  * ��    ������
  * �� �� ֵ����
  */
void W25Q64_WriteEnable(void)
{
	uint8_t cmd[1] = {W25Q64_WRITE_ENABLE};
	SPI_Start();								//SPI��ʼ
	HAL_SPI_Transmit(&hspi1, cmd, 1, 1000);
	SPI_Stop();								//SPI��ֹ
}
 
/**
  * ��    ����W25Q64�ȴ�æ
  * ��    ������
  * �� �� ֵ����
  */
void W25Q64_WaitBusy(void)
{
	//uint32_t Timeout;
	uint8_t ret = 0;
	uint32_t Timeout = 10000;
	uint8_t cmd[1] = {W25Q64_READ_STATUS_REGISTER_1};
	SPI_Start();								//SPI��ʼ
	HAL_SPI_Transmit(&hspi1, cmd, 1, 1000);
	do
	{
		HAL_SPI_Receive(&hspi1, &ret, 1, 1000);
		Timeout--;
		if(Timeout == 0)break;
	}while((ret & 0x01) == 0x01);
	SPI_Stop();								//SPI��ֹ	
}
 
/**
  * ��    ����W25Q64ҳ���
  * ��    ����Address ҳ��̵���ʼ��ַ����Χ��0x000000~0x7FFFFF
  * ��    ����DataArray	����д�����ݵ�����
  * ��    ����Count Ҫд�����ݵ���������Χ��0~256
  * �� �� ֵ����
  * ע�����д��ĵ�ַ��Χ���ܿ�ҳ
  */
void W25Q64_PageProgram(uint32_t Address, uint8_t *DataArray, uint16_t Count)
{
	uint8_t cmd[4] = {W25Q64_PAGE_PROGRAM, (uint8_t)Address>>16, (uint8_t)Address>>8, (uint8_t)Address};
	W25Q64_WriteEnable();						//дʹ��
	
	SPI_Start();								//SPI��ʼ
	HAL_SPI_Transmit(&hspi1, cmd, 4, 1000);
	HAL_SPI_Transmit(&hspi1, DataArray, Count, 1000);

	SPI_Stop();
	W25Q64_WaitBusy();							//�ȴ�æ
}
 
/**
  * ��    ����W25Q64����������4KB��
  * ��    ����Address ָ�������ĵ�ַ����Χ��0x000000~0x7FFFFF
  * �� �� ֵ����
  */
void W25Q64_SectorErase(uint32_t Address)
{
	uint8_t cmd[4] = {W25Q64_SECTOR_ERASE_4KB, (uint8_t)Address>>16, (uint8_t)Address>>8, (uint8_t)Address};
	W25Q64_WriteEnable();						//дʹ��
	SPI_Start();								//SPI��ʼ

	HAL_SPI_Transmit(&hspi1, cmd, 4, 1000);
	SPI_Stop();								//SPI��ֹ
	W25Q64_WaitBusy();							//�ȴ�æ
}
 
/**
  * ��    ����W25Q64��ȡ����
  * ��    ����Address ��ȡ���ݵ���ʼ��ַ����Χ��0x000000~0x7FFFFF
  * ��    ����DataArray ���ڽ��ն�ȡ���ݵ����飬ͨ�������������
  * ��    ����Count Ҫ��ȡ���ݵ���������Χ��0~0x800000
  * �� �� ֵ����
  */
void W25Q64_ReadData(uint32_t Address, uint8_t *DataArray, uint32_t Count)
{
	uint8_t cmd[4] = {W25Q64_READ_DATA, (uint8_t)Address>>16, (uint8_t)Address>>8, (uint8_t)Address};

	SPI_Start();								//SPI��ʼ

	HAL_SPI_Transmit(&hspi1, cmd, 4, 1000);

	HAL_SPI_Receive(&hspi1, DataArray, Count, 1000);
	SPI_Stop();								//SPI��ֹ
}