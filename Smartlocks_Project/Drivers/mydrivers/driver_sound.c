#include "usart.h"
#include "sound_commend.h"


void broadcast_sound(uint8_t index, uint16_t timeout)
{
	HAL_UART_Transmit(&huart1, commend[index], 6, timeout);
}

void set_volume(int volume, uint16_t timeout)
{
	volume_commend[3] = volume;
	volume_commend[4] = volume + 0xBE;
	HAL_UART_Transmit(&huart1, volume_commend, 5, timeout);
}