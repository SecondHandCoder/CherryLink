/*
 * Copyright (c) 2006-2023, SecondHandCoder
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author                Notes
 * 2023-11-12     SecondHandCoder       first version.
 */

#include "ch32f205_dap.h"
#include "stddef.h"
#include "ch32f205_clk.h"
#include "ch32f205_time.h"


/* transmission parameter settings */
typedef struct
{
    uint32_t spi_clk;                               /* spi clock frequency unit khz */
    uint16_t spi_clk_prescaler;                     /* spi clock prescaler */
    void (*time_delay)(void);                       /* delay function of IO simulation clock line */
}clk_config_t;


/**
 * @brief Parameter settings corresponding to frequency in the clock configuration table.
 *
 * @param table             A pointer to clock configuration table.
 * @param table_size        Size of clock configuration table.
 * @param clk               Clock frequency to be set unit khz.
 *
 * @return Index of the frequency to be set in the frequency configuration table.
 */
static uint8_t search_clk_table(const clk_config_t *table, uint8_t table_size, uint16_t clk)
{
    for (uint8_t i = 0; i < table_size; i++)
    {
        if (clk >= table[i].spi_clk)
            return i;
    }
    
    return (table_size - 1);
}

#if (DAP_SWD != 0)

/* ARM Compiler V6 */
#if defined(__clang__)
void delay_swd_4500khz(void)    __attribute__((optnone))
{
    __NOP();
    __NOP();
    __NOP();
}

void delay_swd_2250khz(void)    __attribute__((optnone))
{
    uint8_t dummy = 1;
    while (--dummy);
    __NOP();
    __NOP();
}

void delay_swd_1125khz(void)    __attribute__((optnone))
{
    uint8_t dummy = 4;
    while (--dummy);
}

/* ARM Compiler V5 */
#elif defined(__CC_ARM)
#pragma push
#pragma O0
void delay_swd_4500khz(void)
{
    __NOP();
    __NOP();
}
#pragma pop

#pragma push
#pragma O0
void delay_swd_2250khz(void)
{
    uint8_t dummy = 1;
    while (--dummy);
    __NOP();
    __NOP();
}
#pragma pop

#pragma push
#pragma O0
void delay_swd_1125khz(void)
{
    uint8_t dummy = 4;
    while (--dummy);
}
#pragma pop

/* GCC */         
#elif defined(__GNUC__)
#pragma GCC push_options
#pragma GCC optimize ("O0")
void delay_swd_4500khz(void)
{
    __NOP();
    __NOP();
}
#pragma GCC pop_options

#pragma GCC push_options
#pragma GCC optimize ("O0")
void delay_swd_2250khz(void)
{
    uint8_t dummy = 1;
    while (--dummy);
    __NOP();
    __NOP();
}
#pragma GCC pop_options

#pragma GCC push_options
#pragma GCC optimize ("O0")
void delay_swd_1125khz(void)
{
    uint8_t dummy = 4;
    while (--dummy);
}
#pragma GCC pop_options

/* IAR */
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma optimize=none
void delay_swd_4500khz(void)
{
    __NOP();
    __NOP();
}

#pragma optimize=none
void delay_swd_2250khz(void)
{
    uint8_t dummy = 1;
    while (--dummy);
    __NOP();
    __NOP();
}

#pragma optimize=none
void delay_swd_1125khz(void)
{
    uint8_t dummy = 4;
    while (--dummy);
}
#else
    #error "Unknown compiler for delay" 
#endif


/* SWD clock configuration table */
const static clk_config_t swd_clk_table[] =
{
    {18000,     SPI_CTLR1_BR_PRE8,      NULL              },    /* 144M / 8   = 18M     */
    { 9000,     SPI_CTLR1_BR_PRE16,     NULL              },    /* 144M / 16  = 9M      */
    { 4500,     SPI_CTLR1_BR_PRE32,     delay_swd_4500khz },    /* 144M / 32  = 4.5M    */
    { 2250,     SPI_CTLR1_BR_PRE64,     delay_swd_2250khz },    /* 144M / 64  = 2.25M   */
    { 1125,     SPI_CTLR1_BR_PRE128,    delay_swd_1125khz },    /* 144M / 128 = 1.125M  */
};

/**
 * @brief SWD interface transmission parameter set.
 *
 * @param clk           Clock frequency to be set unit khz.
 *
 * @return Point of delay function of IO simulation clock line.
 */
void *dap_swd_trans_interface_init(uint16_t clk)                                                                                                                                                                                                               \
{                                                                                                                                   
    uint8_t temp = search_clk_table((const clk_config_t *)&swd_clk_table[0], sizeof(swd_clk_table)/sizeof(clk_config_t), clk);
                                                                     
    SWD_SPI_BASE->CTLR1 &= ~(SPI_CTLR1_SPE | SPI_CTLR1_BR | SPI_CTLR1_BIDIMODE | SPI_CTLR1_CRCEN | SPI_CTLR1_DFF);                  
    SWD_SPI_BASE->CTLR1 |= swd_clk_table[temp].spi_clk_prescaler;                                                                                
    SWD_SPI_BASE->CTLR1 |= SPI_CTLR1_MSTR | SPI_CTLR1_SSM | SPI_CTLR1_SSI | SPI_CTLR1_LSBFIRST | SPI_CTLR1_CPOL | SPI_CTLR1_CPHA;   
    SWD_SPI_BASE->CTLR1 |= SPI_CTLR1_SPE;

    return (void *)swd_clk_table[temp].time_delay;
}

/**
 * @brief SWD interface GPIO rcc init.
 *
 * @return None.
 */
void dap_swd_gpio_init(void)
{
    PERIPHERAL_GPIO_TCK_SWD_RCC_EN();
    PERIPHERAL_GPIO_TMS_MO_RCC_EN();
    PERIPHERAL_GPIO_TMS_MI_RCC_EN();
    PERIPHERAL_GPIO_RST_RCC_EN();
}

/**
 * @brief SWD interface GPIO deinit, set GPIO to analog input.
 *
 * @return None.
 */
void dap_swd_gpio_deinit(void)
{
    DAP_SWD_TCK_TO_AIN();
    DAP_TCK_TO_IN();

    DAP_SWD_TMS_MO_TO_AIN();
    DAP_SWD_TMS_TO_IN();

    DAP_SWD_TMS_MI_TO_AIN();
    DAP_RST_TO_AIN();
}

/**
 * @brief SWD interface GPIO config, tck   - push-pull output high, 
 *                                   tms-o - analog input,
 *                                   tms-i - floating input,
 *                                   reset - push-pull output high.
 *
 * @return None.
 */
void dap_swd_io_reconfig(void)
{
    DAP_SWD_TCK_TO_OPP();
    DAP_SWD_TCK_TO_HIGH();
    DAP_TCK_TO_OUT();

    DAP_SWD_TMS_MO_TO_AIN();
    DAP_SWD_TMS_TO_IN();

    DAP_SWD_TMS_MI_TO_FIN();

    DAP_RST_TO_OPP();
    DAP_RST_TO_HIGH();
}

/**
 * @brief SWD interface SPI rcc init.
 *
 * @return None.
 */
void dap_swd_trans_init(void)
{
    SWD_SPI_RCC_EN();   
}

/**
 * @brief SWD interface SPI deinit.
 *
 * @return None.
 */
void dap_swd_trans_deinit(void)
{                                                                 
    SWD_SPI_BASE->CTLR1 = 0;
    SWD_SPI_BASE->CTLR2 = 0;
    SWD_SPI_RCC_DIS();
}
#endif

#if (DAP_JTAG != 0)

/* ARM Compiler V6 */
#if defined(__clang__)
void delay_jtag_4500khz(void)    __attribute__((optnone))
{

}

void delay_jtag_2250khz(void)    __attribute__((optnone))
{
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
}

void delay_jtag_1125khz(void)    __attribute__((optnone))
{
    uint8_t dummy = 3;
    while (--dummy);
}

/* ARM Compiler V5 */
#elif defined(__CC_ARM)
#pragma push
#pragma O0
void delay_jtag_4500khz(void)
{

}
#pragma pop

#pragma push
#pragma O0
void delay_jtag_2250khz(void)
{
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
}
#pragma pop

#pragma push
#pragma O0
void delay_jtag_1125khz(void)
{
    uint8_t dummy = 3;
    while (--dummy);
}
#pragma pop

/* GCC */         
#elif defined(__GNUC__)
#pragma GCC push_options
#pragma GCC optimize ("O0")
void delay_jtag_4500khz(void)
{

}
#pragma GCC pop_options

#pragma GCC push_options
#pragma GCC optimize ("O0")
void delay_jtag_2250khz(void)
{
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
}
#pragma GCC pop_options

#pragma GCC push_options
#pragma GCC optimize ("O0")
void delay_jtag_1125khz(void)
{
    uint8_t dummy = 3;
    while (--dummy);
}
#pragma GCC pop_options

/* IAR */
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma optimize=none
void delay_jtag_4500khz(void)
{
  
}

#pragma optimize=none
void delay_jtag_2250khz(void)
{
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
}

#pragma optimize=none
void delay_jtag_1125khz(void)
{
    uint8_t dummy = 3;
    while (--dummy);
}

#else
    #error "Unknown compiler for delay" 
#endif


/* JTAG clock configuration table */
const static clk_config_t jtag_clk_table[] =
{
    { 9000,      SPI_CTLR1_BR_PRE8,     NULL               },    /* 72M / 8   = 9M      */
    { 4500,      SPI_CTLR1_BR_PRE16,    delay_jtag_4500khz },    /* 72M / 16  = 4.5M    */
    { 2250,      SPI_CTLR1_BR_PRE32,    delay_jtag_2250khz },    /* 72M / 32  = 2.25M   */
    { 1125,      SPI_CTLR1_BR_PRE64,    delay_jtag_1125khz },    /* 72M / 64  = 1.125M  */
};

/**
 * @brief JTAG interface transmission parameter set.
 *
 * @param clk           Clock frequency to be set unit khz.
 *
 * @return Point of delay function of IO simulation clock line.
 */
void *dap_jtag_trans_interface_init(uint16_t clk)                                                                                                                                                                                                               \
{                                                                                                                                   
    uint8_t temp = search_clk_table((const clk_config_t *)&jtag_clk_table[0], sizeof(jtag_clk_table)/sizeof(clk_config_t), clk);
                                                                     
    JTAG_SPI_BASE->CTLR1 &= ~(SPI_CTLR1_SPE | SPI_CTLR1_BR | SPI_CTLR1_BIDIMODE | SPI_CTLR1_CRCEN | SPI_CTLR1_DFF);                  
    JTAG_SPI_BASE->CTLR1 |= jtag_clk_table[temp].spi_clk_prescaler;                                                                                
    JTAG_SPI_BASE->CTLR1 |= SPI_CTLR1_MSTR | SPI_CTLR1_SSM | SPI_CTLR1_SSI | SPI_CTLR1_LSBFIRST | SPI_CTLR1_CPOL | SPI_CTLR1_CPHA;   
    JTAG_SPI_BASE->CTLR1 |= SPI_CTLR1_SPE;

    return (void *)jtag_clk_table[temp].time_delay;
}

/**
 * @brief JTAG interface GPIO rcc init.
 *
 * @return None.
 */
void dap_jtag_gpio_init(void)
{
    PERIPHERAL_GPIO_TCK_JTAG_RCC_EN();
    PERIPHERAL_GPIO_TDI_RCC_EN();
    PERIPHERAL_GPIO_TDO_RCC_EN();
    PERIPHERAL_GPIO_TRST_RCC_EN();
    PERIPHERAL_GPIO_RST_RCC_EN();
}

/**
 * @brief JTAG interface GPIO deinit, set GPIO to analog input.
 *
 * @return None.
 */
void dap_jtag_gpio_deinit(void)
{
    DAP_JTAG_TCK_TO_AIN();
    DAP_TCK_TO_IN();

    DAP_JTAG_TDI_TO_AIN();
    DAP_JTAG_TDI_TO_IN();

    DAP_JTAG_TDO_TO_AIN();
    DAP_JTAG_TMS_TO_AIN();
    DAP_JTAG_TMS_TO_IN();

    DAP_TRST_TO_AIN();
    DAP_RST_TO_AIN();
}

/**
 * @brief JTAG interface GPIO config, tck   - push-pull output high, 
 *                                    tdi   - push-pull output high,
 *                                    tdo   - floating input,
 *                                    tms   - push-pull output high,
 *                                    trst  - push-pull output high,
 *                                    reset - push-pull output high.
 *
 * @return None.
 */
void dap_jtag_io_reconfig(void)
{
    DAP_JTAG_TCK_TO_OPP();
    DAP_JTAG_TCK_TO_HIGH();
    DAP_TCK_TO_OUT();

    DAP_JTAG_TDI_TO_OPP();
    DAP_JTAG_TDI_TO_HIGH();
    DAP_JTAG_TDI_TO_OUT();

    DAP_JTAG_TDO_TO_FIN();

    DAP_JTAG_TMS_TO_OPP();
    DAP_JTAG_TMS_TO_HIGH();
    DAP_JTAG_TMS_TO_OUT();

    DAP_TRST_TO_OPP();
    DAP_TRST_TO_HIGH();
    DAP_RST_TO_OPP();
    DAP_RST_TO_HIGH();
}

/**
 * @brief JTAG interface SPI rcc init.
 *
 * @return None.
 */
void dap_jtag_trans_init(void)
{
    JTAG_SPI_RCC_EN();   
}

/**
 * @brief JTAG interface SPI deinit.
 *
 * @return None.
 */
void dap_jtag_trans_deinit(void)
{                                                                 
    JTAG_SPI_BASE->CTLR1 = 0;
    JTAG_SPI_BASE->CTLR2 = 0;
    JTAG_SPI_RCC_DIS();
}
#endif

#if (DAP_UART != 0)
/**
 * @brief USART GPIO config, rcc init,
 *                           rx - floating input,
 *                           tx - multiplex push-pull output
 *
 * @return None.
 */
void usart_gpio_init(void)
{
    PERIPHERAL_GPIO_USART_RX_RCC_EN();
    PERIPHERAL_GPIO_USART_TX_RCC_EN();

    USART_RX_TO_FIN();
    USART_TX_TO_APP();
    DAP_USB_CDC_TO_OUT();
}

/**
 * @brief USART and DMA rcc init
 *
 * @return None.
 */
void usart_trans_init(void)
{
    USART_RCC_EN();
    USART_DMA_RCC_EN();
}

/**
 * @brief USART transmission parameters config.
 *
 * @param baudrate      Baudrate.
 * @param databits      Len of data bits, 0 : 8, 1 : 9, other : 8.
 * @param stopbits      Len of stop bits, 0 : 1bit, 1 : 1.5bit, 2 : 2bit, other : 1bit.
 * @param parity        Parity, 0 : NONE, 1 : ODD, 2 : EVEN, other : NONE.
 *
 * @return None.
 */
void usart_param_config(uint32_t baudrate, uint8_t databits, uint8_t stopbits, uint8_t parity)
{
    uint32_t tmpreg = 0;
    uint32_t integerdivider, fractionaldivider;
    
    // Data terminal rate, in bits per second
    // baudrate = fclk / (16 * (div_m + (div_f / 16)))
    if (USART_BASE == USART1)    
        integerdivider = (25 * Pclk2Clock) / (4 * baudrate);
    else
        integerdivider = (25 * Pclk1Clock) / (4 * baudrate);
    
    tmpreg = (integerdivider / 100) << 4;
    fractionaldivider = integerdivider - (100 * (tmpreg >> 4));
    tmpreg |= ((((fractionaldivider * 16) + 50) / 100) & 0x0F);
    USART_BASE->BRR = (uint16_t)tmpreg;

    // Data bits (5, 6, 7, 8 or 16)
    // Parity: 0 - None; 1 - Odd; 2 - Even; 3 - Mark; 4 - Space
    USART_BASE->CTLR1 &= ~(USART_CTLR1_M | USART_CTLR1_PS | USART_CTLR1_PCE);
    if (parity == 1)
    {
        USART_BASE->CTLR1 |= 0x1600;  // 9bit ODD
    }    
    else if (parity == 2)
    {
        USART_BASE->CTLR1 |= 0x1400;  // 9bit EVEN
    }    
    else
    {
        if (databits == 8)
            USART_BASE->CTLR1 |= 0x0000;
        else if (databits == 9)
            USART_BASE->CTLR1 |= 0x1000;    
    }        

    // Stop bits: 0 - 1 Stop bit; 1 - 1.5 Stop bits; 2 - 2 Stop bits
    USART_BASE->CTLR2 &= ~USART_CTLR2_STOP;
    if (stopbits == 0)
        USART_BASE->CTLR2 |= 0x0000;
    else if (stopbits == 1)
        USART_BASE->CTLR2 |= 0x3000;
    else if (stopbits == 2)
        USART_BASE->CTLR2 |= 0x2000;

    // rx tx enable, idle isr enable    
    USART_BASE->CTLR1 |= USART_CTLR1_RE | USART_CTLR1_TE | USART_CTLR1_IDLEIE;
           
    // rx tx dma enable
    USART_BASE->CTLR3 |= USART_CTLR3_DMAR | USART_CTLR3_DMAT;
    
    // usart enable
    USART_BASE->CTLR1 |= USART_CTLR1_UE;

    // usart isr config
    NVIC_SetPriority(USART_IRQ_VECTOR, 5);
    NVIC_EnableIRQ(USART_IRQ_VECTOR);
}

/**
 * @brief DMA transmission parameters config.
 *
 * @param rx_addr      A point of DMA receiving data.
 * @param rx_len       Len of DMA receiving data.
 *
 * @return None.
 */
void dma_param_config(uint8_t *rx_addr, uint32_t rx_len)
{
    // usart rx
    USART_DMA_RX_CHANNEL->CFGR = DMA_CFGR1_MINC | DMA_CFGR1_CIRC | DMA_CFGR1_HTIE | DMA_CFGR1_TCIE | DMA_CFGR1_PL;
    USART_DMA_RX_CHANNEL->CNTR = rx_len;
    USART_DMA_RX_CHANNEL->PADDR = (uint32_t)(&USART_BASE->DATAR);
    USART_DMA_RX_CHANNEL->MADDR = (uint32_t)rx_addr;
    // dma rx config
    NVIC_SetPriority(USART_DMA_RX_VECTOR, 6);
    NVIC_EnableIRQ(USART_DMA_RX_VECTOR);
    USART_DMA_RX_CHANNEL->CFGR |= DMA_CFGR1_EN;

    // usart tx
    USART_DMA_TX_CHANNEL->CFGR = DMA_CFGR1_MINC | DMA_CFGR1_DIR | DMA_CFGR1_TCIE | DMA_CFGR1_PL;
    USART_DMA_TX_CHANNEL->CNTR = 0;
    USART_DMA_TX_CHANNEL->PADDR = (uint32_t)(&USART_BASE->DATAR);
    USART_DMA_TX_CHANNEL->MADDR = (uint32_t)NULL;
    // dma tx config
    NVIC_SetPriority(USART_DMA_TX_VECTOR, 7);
    NVIC_EnableIRQ(USART_DMA_TX_VECTOR);
}
#endif

/**
 * @brief DAP GPIO init, LED config, data transmission direction control pin config.
 *
 * @return None.
 */
void dap_gpio_init(void)
{
    // CON LED
    PERIPHERAL_LED_CON_RCC_EN();
    DAP_LED_CON_TO_OPP();
    DAP_LED_CON_SET_OFF();

    // STA_LED
    PERIPHERAL_LED_STA_RCC_EN();
    DAP_LED_STA_TO_OPP();
    DAP_LED_STA_SET_OFF();

    // CTRL0
    PERIPHERAL_CTRL0_RCC_EN();
    CTRL0_TO_OPP();
    DAP_TCK_TO_IN();

    // CTRL1
    PERIPHERAL_CTRL1_RCC_EN();
    CTRL1_TO_OPP();
    DAP_SWD_TMS_TO_IN();

    // CTRL2
    PERIPHERAL_CTRL2_RCC_EN();
    CTRL2_TO_OPP();
    DAP_JTAG_TDI_TO_IN();

    // CTRL3
    PERIPHERAL_CTRL3_RCC_EN();
    CTRL3_TO_OPP();
    DAP_USB_CDC_TO_IN();
}
