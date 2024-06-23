#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "string.h"
#include <stdlib.h> 
#include <math.h>
#include "stdio.h"
#include "drivers_oled.h"
#include "driver_sound.h"

#define  RX_DBG_EN   0

#define  RX_SEQ_NUM  33

#if RX_DBG_EN
#define RX_DBG(format, ...) printf(format, ##__VA_ARGS__)
#else
#define RX_DBG(format, ...) ;
#endif

static uint8_t  tim_udt_cnt      = 0;
static uint8_t  cap_pol          = 0; 
static uint8_t  cap_pulse_cnt    = 0;
static uint8_t  sta_idle         = 1;
static uint8_t  cap_frame        = 0;

extern int lockState;

static uint16_t rx_frame[RX_SEQ_NUM*2] = {0}; 

struct {
    uint16_t  src_data[RX_SEQ_NUM*2];
    uint16_t  repet_cnt;
    union{
        uint32_t rev;
        struct
        {
            uint32_t key_val_n:8;
            uint32_t key_val  :8;
            uint32_t addr_n   :8;
            uint32_t addr     :8;
        }_rev;
    }data;
}rx;

uint8_t appro(int num1,int num2)
{
    return (abs(num1-num2) < 300);
}

void rx_rcv_init(void)
{
    cap_frame     = 0;                                       //未捕获到新数据
    sta_idle      = 0;                                       //非空闲状态
    tim_udt_cnt   = 0;                                       //定时器溢出清0
    cap_pulse_cnt = 0;                                       //捕获到的计数清0
    
    memset(rx_frame,0x00,sizeof(rx_frame));
}

extern uint8_t stayTime; 
/* 溢出中断回调函数 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{   
	if(TIM1 == htim->Instance)
	{
        if(sta_idle)                                          //空闲状态，不作任何处理
        {
            return;
        }
        
        tim_udt_cnt++;                                         //溢出一次
        if(tim_udt_cnt == 3)                                   //溢出3次
        {
            tim_udt_cnt = 0;                                   //溢出次数清零
            sta_idle    = 1;                                   //这是为空闲状态
            cap_frame   = 1;                                   //标记捕获到新的数据
        }
	}else if (htim->Instance == TIM4) {
		HAL_IncTick();
	}else if(htim->Instance == TIM2)
	{
		stayTime++;
	}
	
}


/* 电平捕获中断回调 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    static uint16_t tmp_cnt_l,tmp_cnt_h;
	if(TIM1 == htim->Instance)
	{
        switch(cap_pol)                                                                                         //根据极性标志位判断捕获是低电平还是高电平
        {   
            /* 捕获到下降沿 */
            case 0:
                tmp_cnt_l = HAL_TIM_ReadCapturedValue(&htim1,TIM_CHANNEL_1);                                    //记录当前时刻
                TIM_RESET_CAPTUREPOLARITY(&htim1, TIM_CHANNEL_1);                                               //复位极性配置
                TIM_SET_CAPTUREPOLARITY(&htim1, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);                //改变极性
                
                cap_pol = 1;                                                                                    //极性标志位改为上升沿
                
                if(sta_idle)                                                                                    //如果当前为空闲状态，空闲捕获到的时序，为第一个下降沿
                {
                    rx_rcv_init();
                    break;                                                                                      //返回
                }
                rx_frame[cap_pulse_cnt] = tim_udt_cnt * 10000 + tmp_cnt_l - tmp_cnt_h;                          //与上次捕获的计时作差，记录值
                tim_udt_cnt = 0;                                                                                //溢出次数清0
                RX_DBG("(%2d)%4d us:H\r\n",cap_pulse_cnt,rx_frame[cap_pulse_cnt]);                              //DBG：打印捕获到的电平及其时长
                cap_pulse_cnt++;                                                                                //计数++
                break;
            
            /* 捕获到上升沿 */
            case 1:
                tmp_cnt_h = HAL_TIM_ReadCapturedValue(&htim1,TIM_CHANNEL_1);
                TIM_RESET_CAPTUREPOLARITY(&htim1, TIM_CHANNEL_1);               
                TIM_SET_CAPTUREPOLARITY(&htim1, TIM_CHANNEL_1, TIM_ICPOLARITY_FALLING);
                
                cap_pol = 0;   
                if(sta_idle)
                {
                    rx_rcv_init();
                    break;
                }
                rx_frame[cap_pulse_cnt] = tim_udt_cnt * 10000 + tmp_cnt_h - tmp_cnt_l;
                tim_udt_cnt = 0;
                RX_DBG("(%2d)%4d us:L\r\n",cap_pulse_cnt,rx_frame[cap_pulse_cnt]);
                cap_pulse_cnt++;
                break;
            
            default:
                break;
        }
    }
}


/*************************************  API Layer **************************************************/
void hx1838_cap_start(void)
{
    HAL_TIM_Base_Start_IT(&htim1);
    HAL_TIM_IC_Start_IT(&htim1,TIM_CHANNEL_1);
}

uint8_t hx1838_data_decode(void)
{
    memcpy(rx.src_data,rx_frame,RX_SEQ_NUM*4);
    memset(rx_frame,0x00,RX_SEQ_NUM*4);   
    RX_DBG("========= rx.src[] =================\r\n");
    for(uint8_t i = 0;i<=(RX_SEQ_NUM*2);i++)
    {
        RX_DBG("[%d]%d\r\n",i,rx.src_data[i]);
    }
    RX_DBG("========= rx.rec =================\r\n");
    if(appro(rx.src_data[0],9000) && appro(rx.src_data[1],4500))                 //#1. 检测前导码
    {
        uint8_t tmp_idx = 0;
        rx.repet_cnt  = 0;                                                       //按键重复个数清0
        for(uint8_t i = 2;i<(RX_SEQ_NUM*2);i++)                                  //#2. 检测数据
        {
            if(!appro(rx.src_data[i],560))
            {
                RX_DBG("%d,err:%d != 560\r\n",i,rx.src_data[i]);
                return 0;
            }
            i++;
            if(appro(rx.src_data[i],1680))
            {
                rx.data.rev |= (0x80000000 >> tmp_idx);                          //第 tmp_idx 为置1
                tmp_idx++;
            }
            else if(appro(rx.src_data[i],560))
            {
                rx.data.rev &= ~(0x80000000 >> tmp_idx);                         //第 tmp_idx 位清0
                tmp_idx++;
            }
            else
            {
                RX_DBG("%d,err:%d != 560||1680\r\n",i,rx.src_data[i+1]);
                return 0;
            }
        }
    }
    else if(appro(rx.src_data[0],9000) && appro(rx.src_data[1],2250) && appro(rx.src_data[2],560))
    {
        rx.repet_cnt++;
        return 2;
    }
    else
    {
        RX_DBG("前导码检测错误\r\n");
		OLED_PrintString(4,0,"qd erreo");
        return 0;
    }
    return 1;
}

void hx1838_proc(uint8_t res)
{
	OLED_ClearLine(0,0);
    if(res == 0)
    {
       // printf("error \r\n");
		OLED_PrintString(0,0,"error");
        return;
    }

    if(res == 2)
    {
       // printf("repet(%d)\r\n",rx.repet_cnt);
		OLED_PrintString(0,0,"repet");
        return;
    }      

    switch(rx.data._rev.key_val)
    {
        case 162:
          //  printf("detected code [%s] \r\n","1");
			OLED_PrintString(0,0,"1");
            break;
        
        case 98:
          //  printf("detected code [%s] \r\n","2");
			OLED_PrintString(0,0,"2");
            break;
        
        case 226:
         //   printf("detected code [%s] \r\n","3");
			OLED_PrintString(0,0,"3");
            break;
        
        case 34:
           // printf("detected code [%s] \r\n","4");
			OLED_PrintString(0,0,"4");
            break;
        
        case 2:
          //  printf("detected code [%s] \r\n","5");
			OLED_PrintString(0,0,"5");
            break;
        
        case 194:
           // printf("detected code [%s] \r\n","6");
			OLED_PrintString(0,0,"6");
            break;
        
        case 224:
           // printf("detected code [%s] \r\n","7");
			OLED_PrintString(0,0,"7");
            break;
        
        case 168:
           // printf("detected code [%s] \r\n","8");
			OLED_PrintString(0,0,"8");
            break;
        
        case 144:
           // printf("detected code [%s] \r\n","9");
			OLED_PrintString(0,0,"9");
            break;
        
        case 152:
            //printf("detected code [%s] \r\n","0");
			OLED_PrintString(0,0,"0");
            break;
        
        case 104:
            //printf("detected code [%s] \r\n","*");
			OLED_PrintString(0,0,"*");
            break;
        
        case 176:
          //  printf("detected code [%s] \r\n","#");
			//关锁
			if(lockState == 0)
			{
				broadcast_sound(0, 1000);
				lockState = 1;
				OLED_PrintString(0,0,"#");
			}
            break;
                
        case 24:
           // printf("detected code [%s] \r\n","↑");
			OLED_PrintString(0,0,"shang");
            break;
                
        case 16:
            //printf("detected code [%s] \r\n","←");
			OLED_PrintString(0,0,"zuo");
            break;
        
        case 74:
            //printf("detected code [%s] \r\n","↓");
			OLED_PrintString(0,0,"xia");
            break;
        
        case 90:
            //printf("detected code [%s] \r\n","→");
			OLED_PrintString(0,0,"you");
            break;
        
        case 56:
            //printf("detected code [%s] \r\n","OK");
			//解锁
			if(lockState == 1)
			{
				broadcast_sound(4, 1000);
				lockState = 0;
				OLED_PrintString(0,0,"OK");
			}
            break;
        
        default:
            //printf("detected unknow code \r\n");
			OLED_PrintString(0,0,"default");
            break;
        
    }
}

void HX1838_demo(void)
{
//	
//    hx1838_cap_start();
//    while(1)
//    {
//        if(cap_frame)
//        {   
//            hx1838_proc(hx1838_data_decode());
//            cap_frame = 0;
//        }
//    };
    
        if(cap_frame)
        {   
            hx1838_proc(hx1838_data_decode());
            cap_frame = 0;
        }
}