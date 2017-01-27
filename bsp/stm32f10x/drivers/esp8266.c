#include <rtthread.h>
#include <string.h>
#include "stm32f10x.h"

#include "esp8266.h"
struct rt_semaphore rx_esp8266_sem;
struct rt_semaphore rx_esp8266_sem1;
rt_device_t uart;
void AT(unsigned char *cmd);
void uart_esp8266_rx(void* parameter)
{
	char ch,last_ch = 0;
	int i=0;
	rt_bool_t show = RT_FALSE;
	unsigned short x=0,y=0;
	rt_kprintf("\n\nuart_esp8266_rx thread\r\n");
	while(1)
	{
		if (rt_sem_take(&rx_esp8266_sem, 
			RT_WAITING_FOREVER) != RT_EOK) 
			continue;
		while (rt_device_read((rt_device_t)parameter, 
			0, &ch, 1) == 1)
		{
			if(show)
				rt_kprintf("%c",ch);
			if (last_ch == 'O' && ch == 'K' ||
				last_ch == 'd' && ch == 'y' )
			{
				rt_sem_release(&rx_esp8266_sem1);
				if (last_ch == 'd' && ch == 'y')
					show = RT_TRUE;
			}
			last_ch = ch;
		}		
	}
	return ;
}

rt_err_t uart_esp8266_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_sem_release(&rx_esp8266_sem);
	return RT_EOK;
}
void config_gpio()
{
	GPIO_InitTypeDef GPIO_InitStructure;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = 
		(GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_10|GPIO_Pin_11);
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	GPIO_SetBits(GPIOB,GPIO_Pin_10);
	GPIO_SetBits(GPIOB,GPIO_Pin_11);
	GPIO_SetBits(GPIOB,GPIO_Pin_0);
	GPIO_SetBits(GPIOB,GPIO_Pin_1);
	rt_thread_delay(30);
	GPIO_ResetBits(GPIOB,GPIO_Pin_0);
	rt_thread_delay(30);
	GPIO_SetBits(GPIOB,GPIO_Pin_0);
}
rt_device_t init_esp8266()
{
	uart = rt_device_find("uart2");
	if (uart == RT_NULL)
	{
		rt_kprintf("init_esp8266: can not find device: uart2\n");
		return RT_NULL;
	}

	if (rt_device_open(uart, RT_DEVICE_OFLAG_RDWR) == RT_EOK)
	{		
		rt_sem_init(&rx_esp8266_sem, "shrx", 0, 0);
		rt_sem_init(&rx_esp8266_sem1, "shrx1", 0, 0);
		rt_device_set_rx_indicate(uart, uart_esp8266_rx_ind);
	}
	config_gpio();
	rt_thread_t rx_thread = rt_thread_create("rx_thread",
	uart_esp8266_rx, (void *)(uart),2048, 20, 10);	
	if(rx_thread!=RT_NULL)
		rt_thread_startup(rx_thread);
	rt_sem_take(&rx_esp8266_sem1, RT_WAITING_FOREVER);
	//AT("AT+CWLAP");
	//rt_sem_take(&rx_esp8266_sem1, RT_WAITING_FOREVER);
	AT("AT+CWJAP=\"333\",\"xky888mf666tanghua\"");
	rt_sem_take(&rx_esp8266_sem1, RT_WAITING_FOREVER);
	AT("AT+CIFSR");
	rt_sem_take(&rx_esp8266_sem1, RT_WAITING_FOREVER);
	return uart;
}
//#ifdef RT_USING_FINSH
//#include <finsh.h>
void AT(unsigned char *cmd)
{
	unsigned char *out=(unsigned char *)
		rt_malloc(rt_strlen(cmd)+3);
	rt_memset(out,'0',rt_strlen(cmd)+3);
	strcpy(out,cmd);
	strcat(out,"\r\n");
    rt_device_write(uart, 0, out, rt_strlen(out));
	rt_free(out);
}
//FINSH_FUNCTION_EXPORT(AT, send at to esp8266. e.g: at(at))
//#endif
