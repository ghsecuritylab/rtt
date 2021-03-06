/*
 * File      : drv_uart.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2013, RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://openlab.rt-thread.com/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 *
 */


#include "drv_uart.h"

static struct rt_serial_device _k20_serial3;  //abstracted serial for RTT
static struct rt_serial_device _k20_serial4;  //abstracted serial for RTT

struct k20_serial_device
{
    /* UART base address */
    UART_Type *baseAddress;

    /* UART IRQ Number */
    int irq_num;

    /* device config */
    struct serial_configure config;
};

//hardware abstract device
/*static struct k20_serial_device _k20_node_3 =
{
    (UART_Type *)UART3_BASE,
    UART3_RX_TX_IRQn,
};*/
static struct k20_serial_device _k20_node_4 =
{
    (UART_Type *)UART4_BASE,
    UART4_RX_TX_IRQn,
};

static rt_err_t _configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    unsigned int reg_C1 = 0,reg_C3 = 0,reg_C4 = 0,reg_BDH = 0,reg_BDL = 0,reg_S2 = 0,reg_BRFA=0;
    unsigned int cal_SBR = 0;
    UART_Type *uart_reg;

    /* ref : drivers\system_MK60F12.c Line 64 ,BusClock = 60MHz
     * calculate baud_rate
     */
    uart_reg = ((struct k20_serial_device *)serial->parent.user_data)->baseAddress;

    /*
     * set bit order
     */
    if (cfg->bit_order == BIT_ORDER_LSB)
        reg_S2 &= ~(UART_S2_MSBF_MASK<<UART_S2_MSBF_SHIFT);
    else if (cfg->bit_order == BIT_ORDER_MSB)
        reg_S2 |= UART_S2_MSBF_MASK<<UART_S2_MSBF_SHIFT;

    /*
     * set data_bits
     */
    if (cfg->data_bits == DATA_BITS_8)
        reg_C1 &= ~(UART_C1_M_MASK<<UART_C1_M_SHIFT);
    else if (cfg->data_bits == DATA_BITS_9)
        reg_C1 |= UART_C1_M_MASK<<UART_C1_M_SHIFT;

    /*
     * set parity
     */
    if (cfg->parity == PARITY_NONE)
    {
        reg_C1 &= ~(UART_C1_PE_MASK);
    }
    else
    {
        /* first ,set parity enable bit */
        reg_C1 |= (UART_C1_PE_MASK);

        /* second ,determine parity odd or even*/
        if (cfg->parity == PARITY_ODD)
            reg_C1 |= UART_C1_PT_MASK;
        if (cfg->parity == PARITY_EVEN)
            reg_C1 &= ~(UART_C1_PT_MASK);
    }

    /*
     * set NZR mode
     * not tested
     */
    if (cfg->invert != NRZ_NORMAL)
    {
        /* not in normal mode ,set inverted polarity */
        reg_C3 |= UART_C3_TXINV_MASK;
    }

    switch ((unsigned int)uart_reg)
    {
    /*
     * if you're using other board
     * set clock and pin map for UARTx
     */
    case UART3_BASE:
            /* calc SBR */
        cal_SBR = /*SystemCoreClock*/(SystemCoreClock/(((SIM->CLKDIV1&SIM_CLKDIV1_OUTDIV2_MASK)>>SIM_CLKDIV1_OUTDIV2_SHIFT)+1)) / (16 * cfg->baud_rate);

        /* check to see if sbr is out of range of register bits */
        if ((cal_SBR > 0x1FFF) || (cal_SBR < 1))
        {
            /* unsupported baud rate for given source clock input*/
            return -RT_ERROR;
        }

        /* calc baud_rate */
        reg_BDH = (cal_SBR & 0x1FFF) >> 8 & 0x00FF;
        reg_BDL = cal_SBR & 0x00FF;

        /* fractional divider */
        reg_BRFA = (((SystemCoreClock/(((SIM->CLKDIV1&SIM_CLKDIV1_OUTDIV2_MASK)>>SIM_CLKDIV1_OUTDIV2_SHIFT)+1)) * 32) / (cfg->baud_rate * 16)) - (cal_SBR * 32);

        reg_C4 = (unsigned char)(reg_BRFA & 0x001F);

        /*SIM->SOPT5 &= ~ SIM_SOPT5_UART3RXSRC(0);
        		SIM->SOPT5 |= SIM_SOPT5_UART0RXSRC(0);
        		SIM->SOPT5 &= ~ SIM_SOPT5_UART0TXSRC(0);
        		SIM->SOPT5 |= SIM_SOPT5_UART0TXSRC(0);
	  */
        // set UART3 clock
        // Enable UART gate clocking
        // Enable PORTE gate clocking
        SIM->SCGC4 |= SIM_SCGC4_UART3_MASK;
        SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;

        // set UART3 pin
        PORTE->PCR[4] &= ~(3UL <<  8);
        PORTE->PCR[4] |= (3UL <<  8);      // Pin mux configured as ALT3

        PORTE->PCR[5] &= ~(3UL <<  8);
        PORTE->PCR[5] |= (3UL <<  8);      // Pin mux configured as ALT3
        break;
    case UART4_BASE:
	    /* calc SBR */
	cal_SBR = /*SystemCoreClock*/(SystemCoreClock/(((SIM->CLKDIV1&SIM_CLKDIV1_OUTDIV2_MASK)>>SIM_CLKDIV1_OUTDIV2_SHIFT)+1)) / (16 * cfg->baud_rate);
  
	/* check to see if sbr is out of range of register bits */
	if ((cal_SBR > 0x1FFF) || (cal_SBR < 1))
	{
	    /* unsupported baud rate for given source clock input*/
	    return -RT_ERROR;
	}
  
	/* calc baud_rate */
	reg_BDH = (cal_SBR & 0x1FFF) >> 8 & 0x00FF;
	reg_BDL = cal_SBR & 0x00FF;
  
	/* fractional divider */
	reg_BRFA = (((SystemCoreClock/(((SIM->CLKDIV1&SIM_CLKDIV1_OUTDIV2_MASK)>>SIM_CLKDIV1_OUTDIV2_SHIFT)+1)) * 32) / (cfg->baud_rate * 16)) - (cal_SBR * 32);
  
	reg_C4 = (unsigned char)(reg_BRFA & 0x001F);
  
	/*SIM->SOPT5 &= ~ SIM_SOPT5_UART0RXSRC(0);
	SIM->SOPT5 |= SIM_SOPT5_UART0RXSRC(0);
	SIM->SOPT5 &= ~ SIM_SOPT5_UART0TXSRC(0);
	SIM->SOPT5 |= SIM_SOPT5_UART0TXSRC(0);*/
  
	// set UART4 clock
	// Enable UART gate clocking
	// Enable PORTE gate clocking
	SIM->SCGC1 |= SIM_SCGC1_UART4_MASK;
	SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
  
	// set UART4 pin
	PORTC->PCR[14] &= ~(3UL <<  8);
	PORTC->PCR[14] |= (3UL <<  8);      // Pin mux configured as ALT3
  
	PORTC->PCR[15] &= ~(3UL <<  8);
	PORTC->PCR[15] |= (3UL <<  8);      // Pin mux configured as ALT3
	break;

    default:
        return -RT_ERROR;
    }

    uart_reg->BDH = reg_BDH;
    uart_reg->BDL = reg_BDL;
    uart_reg->C1 = reg_C1;
    uart_reg->C4 = reg_C4;
    uart_reg->S2 = reg_S2;

    uart_reg->S2  =  0;
    uart_reg->C3  =  0;

    uart_reg->RWFIFO = UART_RWFIFO_RXWATER(1);
    uart_reg->TWFIFO = UART_TWFIFO_TXWATER(0);

    uart_reg->C2  =  UART_C2_RE_MASK |    //Receiver enable
                     UART_C2_TE_MASK;     //Transmitter enable

    return RT_EOK;
}

static rt_err_t _control(struct rt_serial_device *serial, int cmd, void *arg)
{
    UART_Type *uart_reg;
    int uart_irq_num = 0;

    uart_reg = ((struct k20_serial_device *)serial->parent.user_data)->baseAddress;
    uart_irq_num = ((struct k20_serial_device *)serial->parent.user_data)->irq_num;
	

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        /* disable rx irq */
        uart_reg->C2 &= ~UART_C2_RIE_MASK;
        //disable NVIC
        NVIC->ICER[uart_irq_num / 32] = 1 << (uart_irq_num % 32);
        break;
    case RT_DEVICE_CTRL_SET_INT:
        /* enable rx irq */
        uart_reg->C2 |= UART_C2_RIE_MASK;
        //enable NVIC,we are sure uart's NVIC vector is in NVICICPR1
        NVIC->ICPR[uart_irq_num / 32] = 1 << (uart_irq_num % 32);
        NVIC->ISER[uart_irq_num / 32] = 1 << (uart_irq_num % 32);
        break;
    case RT_DEVICE_CTRL_SUSPEND:
        /* suspend device */
        uart_reg->C2  &=  ~(UART_C2_RE_MASK |    //Receiver enable
                            UART_C2_TE_MASK);     //Transmitter enable
        break;
    case RT_DEVICE_CTRL_RESUME:
        /* resume device */
        uart_reg->C2  =  UART_C2_RE_MASK |    //Receiver enable
                         UART_C2_TE_MASK;     //Transmitter enable
        break;
    }

    return RT_EOK;
}

static int _putc(struct rt_serial_device *serial, char c)
{
    UART_Type *uart_reg;
    uart_reg = ((struct k20_serial_device *)serial->parent.user_data)->baseAddress;

    while (!(uart_reg->S1 & UART_S1_TDRE_MASK));
    uart_reg->D = (c & 0xFF);
    return 1;
}

static int _getc(struct rt_serial_device *serial)
{
    UART_Type *uart_reg;
    uart_reg = ((struct k20_serial_device *)serial->parent.user_data)->baseAddress;

    if (uart_reg->S1 & UART_S1_RDRF_MASK)
        return (uart_reg->D);
    else
        return -1;
}

static const struct rt_uart_ops _k20_ops =
{
    _configure,
    _control,
    _putc,
    _getc,
};


void UART3_RX_TX_IRQHandler(void)
{
    rt_interrupt_enter();
    rt_hw_serial_isr((struct rt_serial_device*)&_k20_serial3, RT_SERIAL_EVENT_RX_IND);
    rt_interrupt_leave();
}

void UART4_RX_TX_IRQHandler(void)
{
    rt_interrupt_enter();
    rt_hw_serial_isr((struct rt_serial_device*)&_k20_serial4, RT_SERIAL_EVENT_RX_IND);
    rt_interrupt_leave();
}

void rt_hw_uart_init(void)
{
    struct serial_configure config;

    /* fake configuration */
    config.baud_rate = BAUD_RATE_115200;
    config.bit_order = BIT_ORDER_LSB;
    config.data_bits = DATA_BITS_8;
    config.parity    = PARITY_NONE;
    config.stop_bits = STOP_BITS_1;
    config.invert    = NRZ_NORMAL;
	config.bufsz	 = RT_SERIAL_RB_BUFSZ;

    _k20_serial3.ops    = &_k20_ops;
    _k20_serial3.config = config;
    _k20_serial4.ops    = &_k20_ops;
    _k20_serial4.config = config;
	//uart3 conflict with sdhc pin*/
    /*rt_hw_serial_register(&_k20_serial3, "uart3",
                          RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_STREAM,
                          (void*)&_k20_node_3);*/
    
    rt_hw_serial_register(&_k20_serial4, "uart4",
                          RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_STREAM,
                          (void*)&_k20_node_4);
}

void rt_hw_console_output(const char *str)
{
    while(*str != '\0')
    {
        if (*str == '\n')
            _putc(&_k20_serial4,'\r');
        _putc(&_k20_serial4,*str);
        str++;
    }
}
