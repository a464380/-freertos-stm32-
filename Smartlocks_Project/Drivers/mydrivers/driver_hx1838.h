
#ifndef __DRIVER_HX1838_H
#define __DRIVER_HX1838_H

void HX1838_demo(void);
void hx1838_cap_start(void);
uint8_t hx1838_data_decode(void);
void hx1838_proc(uint8_t res);

#endif /* __DRIVER_OLED_H */

