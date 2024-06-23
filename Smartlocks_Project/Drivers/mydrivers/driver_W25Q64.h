#ifndef __DRIVER_W25Q64__H
#define __DRIVER_W25Q64__H



void W25Q64_ReadID(uint8_t* ID);
void W25Q64_PageProgram(uint32_t Address, uint8_t *DataArray, uint16_t Count);
void W25Q64_SectorErase(uint32_t Address);
void W25Q64_ReadData(uint32_t Address, uint8_t *DataArray, uint32_t Count);

	
#endif
