#ifndef __CH32F205_DAP_H__
#define __CH32F205_DAP_H__

#include "ch32f20x.h"
#include "ch32f205_dap_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// LED
#define PERIPHERAL_LED_CON_IDX                              GPIOA
#define PERIPHERAL_LED_CON_PIN                              GPIO_BSHR_BS8
#define PERIPHERAL_LED_CON_BIT                              (8)
#define PERIPHERAL_LED_CON_MASK                             GPIO_CFG_MASK_PIN_8
#define PERIPHERAL_LED_CON_OPP_CFG                          GPIO_CFG_OPP_PIN_8
#define PERIPHERAL_LED_CON_RCC_EN()                         (RCC->APB2PCENR |= RCC_IOPAEN)

#if (PERIPHERAL_LED_CON_BIT < 8)    
#define DAP_LED_CON_TO_OPP()                                (PERIPHERAL_LED_CON_IDX->CFGLR = ((PERIPHERAL_LED_CON_IDX->CFGLR & PERIPHERAL_LED_CON_MASK) | PERIPHERAL_LED_CON_OPP_CFG))
#else
#define DAP_LED_CON_TO_OPP()                                (PERIPHERAL_LED_CON_IDX->CFGHR = ((PERIPHERAL_LED_CON_IDX->CFGHR & PERIPHERAL_LED_CON_MASK) | PERIPHERAL_LED_CON_OPP_CFG))
#endif

#define DAP_LED_CON_SET_ON()                                (PERIPHERAL_LED_CON_IDX->BCR = PERIPHERAL_LED_CON_PIN)  
#define DAP_LED_CON_SET_OFF()                               (PERIPHERAL_LED_CON_IDX->BSHR = PERIPHERAL_LED_CON_PIN)


#define PERIPHERAL_LED_STA_IDX                              GPIOA
#define PERIPHERAL_LED_STA_PIN                              GPIO_BSHR_BS10
#define PERIPHERAL_LED_STA_BIT                              (10)
#define PERIPHERAL_LED_STA_MASK                             GPIO_CFG_MASK_PIN_10
#define PERIPHERAL_LED_STA_OPP_CFG                          GPIO_CFG_OPP_PIN_10
#define PERIPHERAL_LED_STA_RCC_EN()                         (RCC->APB2PCENR |= RCC_IOPAEN)

#if (PERIPHERAL_LED_STA_BIT < 8)    
#define DAP_LED_STA_TO_OPP()                                (PERIPHERAL_LED_STA_IDX->CFGLR = ((PERIPHERAL_LED_STA_IDX->CFGLR & PERIPHERAL_LED_STA_MASK) | PERIPHERAL_LED_STA_OPP_CFG))
#else
#define DAP_LED_STA_TO_OPP()                                (PERIPHERAL_LED_STA_IDX->CFGHR = ((PERIPHERAL_LED_STA_IDX->CFGHR & PERIPHERAL_LED_STA_MASK) | PERIPHERAL_LED_STA_OPP_CFG))
#endif

#define DAP_LED_STA_SET_ON()                                (PERIPHERAL_LED_STA_IDX->BCR = PERIPHERAL_LED_STA_PIN)
#define DAP_LED_STA_SET_OFF()                               (PERIPHERAL_LED_STA_IDX->BSHR = PERIPHERAL_LED_STA_PIN)


// CTRL
// CTRL0 - TCK
#define PERIPHERAL_CTRL0_IDX                                GPIOB
#define PERIPHERAL_CTRL0_PIN                                GPIO_BSHR_BS10
#define PERIPHERAL_CTRL0_BIT                                (10)
#define PERIPHERAL_CTRL0_MASK                               GPIO_CFG_MASK_PIN_10
#define PERIPHERAL_CTRL0_OPP_CFG                            GPIO_CFG_OPP_PIN_10
#define PERIPHERAL_CTRL0_RCC_EN()                           (RCC->APB2PCENR |= RCC_IOPBEN)

#if (PERIPHERAL_CTRL0_BIT < 8)    
#define CTRL0_TO_OPP()                                      (PERIPHERAL_CTRL0_IDX->CFGLR = ((PERIPHERAL_CTRL0_IDX->CFGLR & PERIPHERAL_CTRL0_MASK) | PERIPHERAL_CTRL0_OPP_CFG))
#else
#define CTRL0_TO_OPP()                                      (PERIPHERAL_CTRL0_IDX->CFGHR = ((PERIPHERAL_CTRL0_IDX->CFGHR & PERIPHERAL_CTRL0_MASK) | PERIPHERAL_CTRL0_OPP_CFG))
#endif

#define DAP_TCK_TO_OUT()                                    (PERIPHERAL_CTRL0_IDX->BSHR = PERIPHERAL_CTRL0_PIN)
#define DAP_TCK_TO_IN()                                     (PERIPHERAL_CTRL0_IDX->BCR = PERIPHERAL_CTRL0_PIN)     

// CTRL1 - TMS-MO
#define PERIPHERAL_CTRL1_IDX                                GPIOC
#define PERIPHERAL_CTRL1_PIN                                GPIO_BSHR_BS7
#define PERIPHERAL_CTRL1_BIT                                (7)
#define PERIPHERAL_CTRL1_MASK                               GPIO_CFG_MASK_PIN_7
#define PERIPHERAL_CTRL1_OPP_CFG                            GPIO_CFG_OPP_PIN_7
#define PERIPHERAL_CTRL1_RCC_EN()                           (RCC->APB2PCENR |= RCC_IOPCEN)

#if (PERIPHERAL_CTRL1_BIT < 8)    
#define CTRL1_TO_OPP()                                      (PERIPHERAL_CTRL1_IDX->CFGLR = ((PERIPHERAL_CTRL1_IDX->CFGLR & PERIPHERAL_CTRL1_MASK) | PERIPHERAL_CTRL1_OPP_CFG))
#else
#define CTRL1_TO_OPP()                                      (PERIPHERAL_CTRL1_IDX->CFGHR = ((PERIPHERAL_CTRL1_IDX->CFGHR & PERIPHERAL_CTRL1_MASK) | PERIPHERAL_CTRL1_OPP_CFG))
#endif

#define DAP_SWD_TMS_TO_OUT()                                (PERIPHERAL_CTRL1_IDX->BSHR = PERIPHERAL_CTRL1_PIN)
#define DAP_SWD_TMS_TO_IN()                                 (PERIPHERAL_CTRL1_IDX->BCR = PERIPHERAL_CTRL1_PIN)
#define DAP_JTAG_TMS_TO_OUT()                               DAP_SWD_TMS_TO_OUT()          
#define DAP_JTAG_TMS_TO_IN()                                DAP_SWD_TMS_TO_IN()

// CTRL2 - TDI
#define PERIPHERAL_CTRL2_IDX                                GPIOC
#define PERIPHERAL_CTRL2_PIN                                GPIO_BSHR_BS6
#define PERIPHERAL_CTRL2_BIT                                (6)
#define PERIPHERAL_CTRL2_MASK                               GPIO_CFG_MASK_PIN_6
#define PERIPHERAL_CTRL2_OPP_CFG                            GPIO_CFG_OPP_PIN_6
#define PERIPHERAL_CTRL2_RCC_EN()                           (RCC->APB2PCENR |= RCC_IOPCEN)

#if (PERIPHERAL_CTRL2_BIT < 8)    
#define CTRL2_TO_OPP()                                      (PERIPHERAL_CTRL2_IDX->CFGLR = ((PERIPHERAL_CTRL2_IDX->CFGLR & PERIPHERAL_CTRL2_MASK) | PERIPHERAL_CTRL2_OPP_CFG))
#else
#define CTRL2_TO_OPP()                                      (PERIPHERAL_CTRL2_IDX->CFGHR = ((PERIPHERAL_CTRL2_IDX->CFGHR & PERIPHERAL_CTRL2_MASK) | PERIPHERAL_CTRL2_OPP_CFG))
#endif

#define DAP_JTAG_TDI_TO_OUT()                               (PERIPHERAL_CTRL2_IDX->BSHR = PERIPHERAL_CTRL2_PIN)          
#define DAP_JTAG_TDI_TO_IN()                                (PERIPHERAL_CTRL2_IDX->BCR = PERIPHERAL_CTRL2_PIN)

// CTRL3 - TX
#define PERIPHERAL_CTRL3_IDX                                GPIOB
#define PERIPHERAL_CTRL3_PIN                                GPIO_BSHR_BS0
#define PERIPHERAL_CTRL3_BIT                                (0)
#define PERIPHERAL_CTRL3_MASK                               GPIO_CFG_MASK_PIN_0
#define PERIPHERAL_CTRL3_OPP_CFG                            GPIO_CFG_OPP_PIN_0
#define PERIPHERAL_CTRL3_RCC_EN()                           (RCC->APB2PCENR |= RCC_IOPBEN)
 
#if (PERIPHERAL_CTRL3_BIT < 8)    
#define CTRL3_TO_OPP()                                      (PERIPHERAL_CTRL3_IDX->CFGLR = ((PERIPHERAL_CTRL3_IDX->CFGLR & PERIPHERAL_CTRL3_MASK) | PERIPHERAL_CTRL3_OPP_CFG))
#else
#define CTRL3_TO_OPP()                                      (PERIPHERAL_CTRL3_IDX->CFGHR = ((PERIPHERAL_CTRL3_IDX->CFGHR & PERIPHERAL_CTRL3_MASK) | PERIPHERAL_CTRL3_OPP_CFG))
#endif

#define DAP_USB_CDC_TO_OUT()                                (PERIPHERAL_CTRL3_IDX->BSHR = PERIPHERAL_CTRL3_PIN)
#define DAP_USB_CDC_TO_IN()                                 (PERIPHERAL_CTRL3_IDX->BCR = PERIPHERAL_CTRL3_PIN)


// SWD
#define PERIPHERAL_GPIO_TCK_SWD_IDX                         GPIOA
#define PERIPHERAL_GPIO_TCK_SWD_PIN                         GPIO_BSHR_BS5
#define PERIPHERAL_GPIO_TCK_SWD_BIT                         (5)
#define PERIPHERAL_GPIO_TCK_SWD_MASK                        GPIO_CFG_MASK_PIN_5
#define PERIPHERAL_GPIO_TCK_SWD_OPP_CFG                     GPIO_CFG_OPP_PIN_5
#define PERIPHERAL_GPIO_TCK_SWD_APP_CFG                     GPIO_CFG_APP_PIN_5
#define PERIPHERAL_GPIO_TCK_SWD_AIN_CFG                     GPIO_CFG_AIN_PIN_5
#define PERIPHERAL_GPIO_TCK_SWD_RCC_EN()                    (RCC->APB2PCENR |= RCC_IOPAEN)

#define PERIPHERAL_GPIO_TMS_MO_IDX                          GPIOA
#define PERIPHERAL_GPIO_TMS_MO_PIN                          GPIO_BSHR_BS7
#define PERIPHERAL_GPIO_TMS_MO_BIT                          (7)
#define PERIPHERAL_GPIO_TMS_MO_MASK                         GPIO_CFG_MASK_PIN_7
#define PERIPHERAL_GPIO_TMS_MO_OPP_CFG                      GPIO_CFG_OPP_PIN_7
#define PERIPHERAL_GPIO_TMS_MO_APP_CFG                      GPIO_CFG_APP_PIN_7
#define PERIPHERAL_GPIO_TMS_MO_AIN_CFG                      GPIO_CFG_AIN_PIN_7
#define PERIPHERAL_GPIO_TMS_MO_RCC_EN()                     (RCC->APB2PCENR |= RCC_IOPAEN)

#define PERIPHERAL_GPIO_TMS_MI_IDX                          GPIOA
#define PERIPHERAL_GPIO_TMS_MI_PIN                          GPIO_BSHR_BS6
#define PERIPHERAL_GPIO_TMS_MI_BIT                          (6)
#define PERIPHERAL_GPIO_TMS_MI_MASK                         GPIO_CFG_MASK_PIN_6
#define PERIPHERAL_GPIO_TMS_MI_FIN_CFG                      GPIO_CFG_FIN_PIN_6
#define PERIPHERAL_GPIO_TMS_MI_AIN_CFG                      GPIO_CFG_AIN_PIN_6
#define PERIPHERAL_GPIO_TMS_MI_RCC_EN()                     (RCC->APB2PCENR |= RCC_IOPAEN)

#if (PERIPHERAL_GPIO_TCK_SWD_BIT < 8)
#define DAP_SWD_TCK_TO_OPP()                                (PERIPHERAL_GPIO_TCK_SWD_IDX->CFGLR = ((PERIPHERAL_GPIO_TCK_SWD_IDX->CFGLR & PERIPHERAL_GPIO_TCK_SWD_MASK) | PERIPHERAL_GPIO_TCK_SWD_OPP_CFG))
#define DAP_SWD_TCK_TO_APP()                                (PERIPHERAL_GPIO_TCK_SWD_IDX->CFGLR = ((PERIPHERAL_GPIO_TCK_SWD_IDX->CFGLR & PERIPHERAL_GPIO_TCK_SWD_MASK) | PERIPHERAL_GPIO_TCK_SWD_APP_CFG))
#define DAP_SWD_TCK_TO_AIN()                                (PERIPHERAL_GPIO_TCK_SWD_IDX->CFGLR = ((PERIPHERAL_GPIO_TCK_SWD_IDX->CFGLR & PERIPHERAL_GPIO_TCK_SWD_MASK) | PERIPHERAL_GPIO_TCK_SWD_AIN_CFG))
#else
#define DAP_SWD_TCK_TO_OPP()                                (PERIPHERAL_GPIO_TCK_SWD_IDX->CFGHR = ((PERIPHERAL_GPIO_TCK_SWD_IDX->CFGHR & PERIPHERAL_GPIO_TCK_SWD_MASK) | PERIPHERAL_GPIO_TCK_SWD_OPP_CFG))
#define DAP_SWD_TCK_TO_APP()                                (PERIPHERAL_GPIO_TCK_SWD_IDX->CFGHR = ((PERIPHERAL_GPIO_TCK_SWD_IDX->CFGHR & PERIPHERAL_GPIO_TCK_SWD_MASK) | PERIPHERAL_GPIO_TCK_SWD_APP_CFG))
#define DAP_SWD_TCK_TO_AIN()                                (PERIPHERAL_GPIO_TCK_SWD_IDX->CFGHR = ((PERIPHERAL_GPIO_TCK_SWD_IDX->CFGHR & PERIPHERAL_GPIO_TCK_SWD_MASK) | PERIPHERAL_GPIO_TCK_SWD_AIN_CFG))
#endif
#define DAP_SWD_TCK_TO_HIGH()                               (PERIPHERAL_GPIO_TCK_SWD_IDX->BSHR = PERIPHERAL_GPIO_TCK_SWD_PIN)
#define DAP_SWD_TCK_TO_LOW()                                (PERIPHERAL_GPIO_TCK_SWD_IDX->BCR = PERIPHERAL_GPIO_TCK_SWD_PIN)
#define DAP_SWD_TCK_GET()                                   ((PERIPHERAL_GPIO_TCK_SWD_IDX->OUTDR & PERIPHERAL_GPIO_TCK_SWD_PIN) >> PERIPHERAL_GPIO_TCK_SWD_BIT)

#if (PERIPHERAL_GPIO_TMS_MO_BIT < 8)
#define DAP_SWD_TMS_MO_TO_OPP()                             (PERIPHERAL_GPIO_TMS_MO_IDX->CFGLR = ((PERIPHERAL_GPIO_TMS_MO_IDX->CFGLR & PERIPHERAL_GPIO_TMS_MO_MASK) | PERIPHERAL_GPIO_TMS_MO_OPP_CFG))
#define DAP_SWD_TMS_MO_TO_APP()                             (PERIPHERAL_GPIO_TMS_MO_IDX->CFGLR = ((PERIPHERAL_GPIO_TMS_MO_IDX->CFGLR & PERIPHERAL_GPIO_TMS_MO_MASK) | PERIPHERAL_GPIO_TMS_MO_APP_CFG))
#define DAP_SWD_TMS_MO_TO_AIN()                             (PERIPHERAL_GPIO_TMS_MO_IDX->CFGLR = ((PERIPHERAL_GPIO_TMS_MO_IDX->CFGLR & PERIPHERAL_GPIO_TMS_MO_MASK) | PERIPHERAL_GPIO_TMS_MO_AIN_CFG))
#define DAP_SWD_TMS_MI_TO_FIN()                             (PERIPHERAL_GPIO_TMS_MI_IDX->CFGLR = ((PERIPHERAL_GPIO_TMS_MI_IDX->CFGLR & PERIPHERAL_GPIO_TMS_MI_MASK) | PERIPHERAL_GPIO_TMS_MI_FIN_CFG))
#define DAP_SWD_TMS_MI_TO_AIN()                             (PERIPHERAL_GPIO_TMS_MI_IDX->CFGLR = ((PERIPHERAL_GPIO_TMS_MI_IDX->CFGLR & PERIPHERAL_GPIO_TMS_MI_MASK) | PERIPHERAL_GPIO_TMS_MI_AIN_CFG))
#else
#define DAP_SWD_TMS_MO_TO_OPP()                             (PERIPHERAL_GPIO_TMS_MO_IDX->CFGHR = ((PERIPHERAL_GPIO_TMS_MO_IDX->CFGHR & PERIPHERAL_GPIO_TMS_MO_MASK) | PERIPHERAL_GPIO_TMS_MO_OPP_CFG))
#define DAP_SWD_TMS_MO_TO_APP()                             (PERIPHERAL_GPIO_TMS_MO_IDX->CFGHR = ((PERIPHERAL_GPIO_TMS_MO_IDX->CFGHR & PERIPHERAL_GPIO_TMS_MO_MASK) | PERIPHERAL_GPIO_TMS_MO_APP_CFG))
#define DAP_SWD_TMS_MO_TO_AIN()                             (PERIPHERAL_GPIO_TMS_MO_IDX->CFGHR = ((PERIPHERAL_GPIO_TMS_MO_IDX->CFGHR & PERIPHERAL_GPIO_TMS_MO_MASK) | PERIPHERAL_GPIO_TMS_MO_AIN_CFG))
#define DAP_SWD_TMS_MI_TO_FIN()                             (PERIPHERAL_GPIO_TMS_MI_IDX->CFGHR = ((PERIPHERAL_GPIO_TMS_MI_IDX->CFGHR & PERIPHERAL_GPIO_TMS_MI_MASK) | PERIPHERAL_GPIO_TMS_MI_FIN_CFG))
#define DAP_SWD_TMS_MI_TO_AIN()                             (PERIPHERAL_GPIO_TMS_MI_IDX->CFGHR = ((PERIPHERAL_GPIO_TMS_MI_IDX->CFGHR & PERIPHERAL_GPIO_TMS_MI_MASK) | PERIPHERAL_GPIO_TMS_MI_AIN_CFG))

#endif
#define DAP_SWD_TMS_TO_HIGH()                               (PERIPHERAL_GPIO_TMS_MO_IDX->BSHR = PERIPHERAL_GPIO_TMS_MO_PIN)
#define DAP_SWD_TMS_TO_LOW()                                (PERIPHERAL_GPIO_TMS_MO_IDX->BCR = PERIPHERAL_GPIO_TMS_MO_PIN)
#define DAP_SWD_TMS_GET()                                   ((PERIPHERAL_GPIO_TMS_MO_IDX->OUTDR & PERIPHERAL_GPIO_TMS_MO_PIN) >> PERIPHERAL_GPIO_TMS_MO_BIT)
#define DAP_SWD_TMS_READ()                                  ((PERIPHERAL_GPIO_TMS_MI_IDX->INDR & PERIPHERAL_GPIO_TMS_MI_PIN) >> PERIPHERAL_GPIO_TMS_MI_BIT)
#if (PERIPHERAL_GPIO_TMS_MI_BIT < 8)
#define DAP_SWD_TMS_READ_0x80()                             ((PERIPHERAL_GPIO_TMS_MI_IDX->INDR & PERIPHERAL_GPIO_TMS_MI_PIN) << (7 - PERIPHERAL_GPIO_TMS_MI_BIT))
#else
#define DAP_SWD_TMS_READ_0x80()                             ((PERIPHERAL_GPIO_TMS_MI_IDX->INDR & PERIPHERAL_GPIO_TMS_MI_PIN) >> (PERIPHERAL_GPIO_TMS_MI_BIT - 7))
#endif

// SWD SPI
#define SWD_SPI_BASE                                        SPI1
#define SWD_SPI_RCC_EN()                                    (RCC->APB2PCENR |= RCC_SPI1EN)
#define SWD_SPI_RCC_DIS()                                   (RCC->APB2PCENR &= ~RCC_SPI1EN)

#define SWD_WRITE_DATA(data)                                (SWD_SPI_BASE->DATAR = data)
#define SWD_READ_DATA()                                     (SWD_SPI_BASE->DATAR)
#define SWD_WAIT_BUSY()                                     (SWD_SPI_BASE->STATR & SPI_STATR_BSY)


// JTAG
#define PERIPHERAL_GPIO_TCK_JTAG_IDX                        GPIOB
#define PERIPHERAL_GPIO_TCK_JTAG_PIN                        GPIO_BSHR_BS13
#define PERIPHERAL_GPIO_TCK_JTAG_BIT                        (13)
#define PERIPHERAL_GPIO_TCK_JTAG_MASK                       GPIO_CFG_MASK_PIN_13
#define PERIPHERAL_GPIO_TCK_JTAG_OPP_CFG                    GPIO_CFG_OPP_PIN_13
#define PERIPHERAL_GPIO_TCK_JTAG_APP_CFG                    GPIO_CFG_APP_PIN_13
#define PERIPHERAL_GPIO_TCK_JTAG_AIN_CFG                    GPIO_CFG_AIN_PIN_13
#define PERIPHERAL_GPIO_TCK_JTAG_RCC_EN()                   (RCC->APB2PCENR |= RCC_IOPBEN)

#define PERIPHERAL_GPIO_TDI_IDX                             GPIOB
#define PERIPHERAL_GPIO_TDI_PIN                             GPIO_BSHR_BS15
#define PERIPHERAL_GPIO_TDI_BIT                             (15)
#define PERIPHERAL_GPIO_TDI_MASK                            GPIO_CFG_MASK_PIN_15
#define PERIPHERAL_GPIO_TDI_OPP_CFG                         GPIO_CFG_OPP_PIN_15
#define PERIPHERAL_GPIO_TDI_APP_CFG                         GPIO_CFG_APP_PIN_15
#define PERIPHERAL_GPIO_TDI_AIN_CFG                         GPIO_CFG_AIN_PIN_15
#define PERIPHERAL_GPIO_TDI_RCC_EN()                        (RCC->APB2PCENR |= RCC_IOPBEN)

#define PERIPHERAL_GPIO_TDO_IDX                             GPIOB
#define PERIPHERAL_GPIO_TDO_PIN                             GPIO_BSHR_BS14
#define PERIPHERAL_GPIO_TDO_BIT                             (14)
#define PERIPHERAL_GPIO_TDO_MASK                            GPIO_CFG_MASK_PIN_14
#define PERIPHERAL_GPIO_TDO_FIN_CFG                         GPIO_CFG_FIN_PIN_14
#define PERIPHERAL_GPIO_TDO_AIN_CFG                         GPIO_CFG_AIN_PIN_14
#define PERIPHERAL_GPIO_TDO_RCC_EN()                        (RCC->APB2PCENR |= RCC_IOPBEN)

#define PERIPHERAL_GPIO_TRST_IDX                            GPIOC
#define PERIPHERAL_GPIO_TRST_PIN                            GPIO_BSHR_BS9
#define PERIPHERAL_GPIO_TRST_BIT                            (9)
#define PERIPHERAL_GPIO_TRST_MASK                           GPIO_CFG_MASK_PIN_9
#define PERIPHERAL_GPIO_TRST_OPP_CFG                        GPIO_CFG_OPP_PIN_9
#define PERIPHERAL_GPIO_TRST_AIN_CFG                        GPIO_CFG_AIN_PIN_9
#define PERIPHERAL_GPIO_TRST_RCC_EN()                       (RCC->APB2PCENR |= RCC_IOPCEN)

#define PERIPHERAL_GPIO_RST_IDX                             GPIOC
#define PERIPHERAL_GPIO_RST_PIN                             GPIO_BSHR_BS8
#define PERIPHERAL_GPIO_RST_BIT                             (8)
#define PERIPHERAL_GPIO_RST_MASK                            GPIO_CFG_MASK_PIN_8
#define PERIPHERAL_GPIO_RST_OPP_CFG                         GPIO_CFG_OPP_PIN_8
#define PERIPHERAL_GPIO_RST_AIN_CFG                         GPIO_CFG_AIN_PIN_8
#define PERIPHERAL_GPIO_RST_RCC_EN()                        (RCC->APB2PCENR |= RCC_IOPCEN)

#if (PERIPHERAL_GPIO_TCK_JTAG_BIT < 8)
#define DAP_JTAG_TCK_TO_OPP()                               (PERIPHERAL_GPIO_TCK_JTAG_IDX->CFGLR = ((PERIPHERAL_GPIO_TCK_JTAG_IDX->CFGLR & PERIPHERAL_GPIO_TCK_JTAG_MASK) | PERIPHERAL_GPIO_TCK_JTAG_OPP_CFG))
#define DAP_JTAG_TCK_TO_APP()                               (PERIPHERAL_GPIO_TCK_JTAG_IDX->CFGLR = ((PERIPHERAL_GPIO_TCK_JTAG_IDX->CFGLR & PERIPHERAL_GPIO_TCK_JTAG_MASK) | PERIPHERAL_GPIO_TCK_JTAG_APP_CFG))
#define DAP_JTAG_TCK_TO_AIN()                               (PERIPHERAL_GPIO_TCK_JTAG_IDX->CFGLR = ((PERIPHERAL_GPIO_TCK_JTAG_IDX->CFGLR & PERIPHERAL_GPIO_TCK_JTAG_MASK) | PERIPHERAL_GPIO_TCK_JTAG_AIN_CFG))
#else
#define DAP_JTAG_TCK_TO_OPP()                               (PERIPHERAL_GPIO_TCK_JTAG_IDX->CFGHR = ((PERIPHERAL_GPIO_TCK_JTAG_IDX->CFGHR & PERIPHERAL_GPIO_TCK_JTAG_MASK) | PERIPHERAL_GPIO_TCK_JTAG_OPP_CFG))
#define DAP_JTAG_TCK_TO_APP()                               (PERIPHERAL_GPIO_TCK_JTAG_IDX->CFGHR = ((PERIPHERAL_GPIO_TCK_JTAG_IDX->CFGHR & PERIPHERAL_GPIO_TCK_JTAG_MASK) | PERIPHERAL_GPIO_TCK_JTAG_APP_CFG))
#define DAP_JTAG_TCK_TO_AIN()                               (PERIPHERAL_GPIO_TCK_JTAG_IDX->CFGHR = ((PERIPHERAL_GPIO_TCK_JTAG_IDX->CFGHR & PERIPHERAL_GPIO_TCK_JTAG_MASK) | PERIPHERAL_GPIO_TCK_JTAG_AIN_CFG))
#endif
#define DAP_JTAG_TCK_TO_HIGH()                              (PERIPHERAL_GPIO_TCK_JTAG_IDX->BSHR = PERIPHERAL_GPIO_TCK_JTAG_PIN)
#define DAP_JTAG_TCK_TO_LOW()                               (PERIPHERAL_GPIO_TCK_JTAG_IDX->BCR = PERIPHERAL_GPIO_TCK_JTAG_PIN)

#define DAP_JTAG_TMS_TO_OPP()                               DAP_SWD_TMS_MO_TO_OPP()
#define DAP_JTAG_TMS_TO_AIN()                               DAP_SWD_TMS_MO_TO_AIN()
#define DAP_JTAG_TMS_TO_HIGH()                              DAP_SWD_TMS_TO_HIGH()
#define DAP_JTAG_TMS_TO_LOW()                               DAP_SWD_TMS_TO_LOW()

#if (PERIPHERAL_GPIO_TDI_BIT < 8)
#define DAP_JTAG_TDI_TO_OPP()                               (PERIPHERAL_GPIO_TDI_IDX->CFGLR = ((PERIPHERAL_GPIO_TDI_IDX->CFGLR & PERIPHERAL_GPIO_TDI_MASK) | PERIPHERAL_GPIO_TDI_OPP_CFG))
#define DAP_JTAG_TDI_TO_APP()                               (PERIPHERAL_GPIO_TDI_IDX->CFGLR = ((PERIPHERAL_GPIO_TDI_IDX->CFGLR & PERIPHERAL_GPIO_TDI_MASK) | PERIPHERAL_GPIO_TDI_APP_CFG))
#define DAP_JTAG_TDI_TO_AIN()                               (PERIPHERAL_GPIO_TDI_IDX->CFGLR = ((PERIPHERAL_GPIO_TDI_IDX->CFGLR & PERIPHERAL_GPIO_TDI_MASK) | PERIPHERAL_GPIO_TDI_AIN_CFG))
#else
#define DAP_JTAG_TDI_TO_OPP()                               (PERIPHERAL_GPIO_TDI_IDX->CFGHR = ((PERIPHERAL_GPIO_TDI_IDX->CFGHR & PERIPHERAL_GPIO_TDI_MASK) | PERIPHERAL_GPIO_TDI_OPP_CFG))
#define DAP_JTAG_TDI_TO_APP()                               (PERIPHERAL_GPIO_TDI_IDX->CFGHR = ((PERIPHERAL_GPIO_TDI_IDX->CFGHR & PERIPHERAL_GPIO_TDI_MASK) | PERIPHERAL_GPIO_TDI_APP_CFG))
#define DAP_JTAG_TDI_TO_AIN()                               (PERIPHERAL_GPIO_TDI_IDX->CFGHR = ((PERIPHERAL_GPIO_TDI_IDX->CFGHR & PERIPHERAL_GPIO_TDI_MASK) | PERIPHERAL_GPIO_TDI_AIN_CFG))
#endif
#define DAP_JTAG_TDI_TO_HIGH()                              (PERIPHERAL_GPIO_TDI_IDX->BSHR = PERIPHERAL_GPIO_TDI_PIN)
#define DAP_JTAG_TDI_TO_LOW()                               (PERIPHERAL_GPIO_TDI_IDX->BCR = PERIPHERAL_GPIO_TDI_PIN)
#define DAP_JTAG_TDI_GET()                                  ((PERIPHERAL_GPIO_TDI_IDX->OUTDR & PERIPHERAL_GPIO_TDI_PIN) >> PERIPHERAL_GPIO_TDI_BIT)

#if (PERIPHERAL_GPIO_TDO_BIT < 8)
#define DAP_JTAG_TDO_TO_FIN()                               (PERIPHERAL_GPIO_TDO_IDX->CFGLR = ((PERIPHERAL_GPIO_TDO_IDX->CFGLR & PERIPHERAL_GPIO_TDO_MASK) | PERIPHERAL_GPIO_TDO_FIN_CFG))
#define DAP_JTAG_TDO_TO_AIN()                               (PERIPHERAL_GPIO_TDO_IDX->CFGLR = ((PERIPHERAL_GPIO_TDO_IDX->CFGLR & PERIPHERAL_GPIO_TDO_MASK) | PERIPHERAL_GPIO_TDO_AIN_CFG))
#else
#define DAP_JTAG_TDO_TO_FIN()                               (PERIPHERAL_GPIO_TDO_IDX->CFGHR = ((PERIPHERAL_GPIO_TDO_IDX->CFGHR & PERIPHERAL_GPIO_TDO_MASK) | PERIPHERAL_GPIO_TDO_FIN_CFG))
#define DAP_JTAG_TDO_TO_AIN()                               (PERIPHERAL_GPIO_TDO_IDX->CFGHR = ((PERIPHERAL_GPIO_TDO_IDX->CFGHR & PERIPHERAL_GPIO_TDO_MASK) | PERIPHERAL_GPIO_TDO_AIN_CFG))
#endif
#define DAP_JTAG_TDO_READ()                                 ((PERIPHERAL_GPIO_TDO_IDX->INDR & PERIPHERAL_GPIO_TDO_PIN) >> PERIPHERAL_GPIO_TDO_BIT)
#if (PERIPHERAL_GPIO_TDO_BIT < 8)
#define DAP_JTAG_TDO_READ_0x80()                            ((PERIPHERAL_GPIO_TDO_IDX->INDR & PERIPHERAL_GPIO_TDO_PIN) << (7 - PERIPHERAL_GPIO_TDO_BIT))
#else
#define DAP_JTAG_TDO_READ_0x80()                            ((PERIPHERAL_GPIO_TDO_IDX->INDR & PERIPHERAL_GPIO_TDO_PIN) >> (PERIPHERAL_GPIO_TDO_BIT - 7))
#endif

#if (PERIPHERAL_GPIO_TRST_BIT < 8)
#define DAP_TRST_TO_OPP()                                   (PERIPHERAL_GPIO_TRST_IDX->CFGLR = ((PERIPHERAL_GPIO_TRST_IDX->CFGLR & PERIPHERAL_GPIO_TRST_MASK) | PERIPHERAL_GPIO_TRST_OPP_CFG))
#define DAP_TRST_TO_AIN()                                   (PERIPHERAL_GPIO_TRST_IDX->CFGLR = ((PERIPHERAL_GPIO_TRST_IDX->CFGLR & PERIPHERAL_GPIO_TRST_MASK) | PERIPHERAL_GPIO_TRST_AIN_CFG))
#else
#define DAP_TRST_TO_OPP()                                   (PERIPHERAL_GPIO_TRST_IDX->CFGHR = ((PERIPHERAL_GPIO_TRST_IDX->CFGHR & PERIPHERAL_GPIO_TRST_MASK) | PERIPHERAL_GPIO_TRST_OPP_CFG))
#define DAP_TRST_TO_AIN()                                   (PERIPHERAL_GPIO_TRST_IDX->CFGHR = ((PERIPHERAL_GPIO_TRST_IDX->CFGHR & PERIPHERAL_GPIO_TRST_MASK) | PERIPHERAL_GPIO_TRST_AIN_CFG))
#endif
#define DAP_TRST_TO_HIGH()                                  (PERIPHERAL_GPIO_TRST_IDX->BSHR = PERIPHERAL_GPIO_TRST_PIN)
#define DAP_TRST_TO_LOW()                                   (PERIPHERAL_GPIO_TRST_IDX->BCR = PERIPHERAL_GPIO_TRST_PIN)
#define DAP_TRST_GET()                                      ((PERIPHERAL_GPIO_TRST_IDX->OUTDR & PERIPHERAL_GPIO_TRST_PIN) >> PERIPHERAL_GPIO_TRST_BIT)

#if (PERIPHERAL_GPIO_RST_BIT < 8)
#define DAP_RST_TO_OPP()                                    (PERIPHERAL_GPIO_RST_IDX->CFGLR = ((PERIPHERAL_GPIO_RST_IDX->CFGLR & PERIPHERAL_GPIO_RST_MASK) | PERIPHERAL_GPIO_RST_OPP_CFG))
#define DAP_RST_TO_AIN()                                    (PERIPHERAL_GPIO_RST_IDX->CFGLR = ((PERIPHERAL_GPIO_RST_IDX->CFGLR & PERIPHERAL_GPIO_RST_MASK) | PERIPHERAL_GPIO_RST_AIN_CFG))
#else
#define DAP_RST_TO_OPP()                                    (PERIPHERAL_GPIO_RST_IDX->CFGHR = ((PERIPHERAL_GPIO_RST_IDX->CFGHR & PERIPHERAL_GPIO_RST_MASK) | PERIPHERAL_GPIO_RST_OPP_CFG))
#define DAP_RST_TO_AIN()                                    (PERIPHERAL_GPIO_RST_IDX->CFGHR = ((PERIPHERAL_GPIO_RST_IDX->CFGHR & PERIPHERAL_GPIO_RST_MASK) | PERIPHERAL_GPIO_RST_AIN_CFG))
#endif
#define DAP_RST_TO_HIGH()                                   (PERIPHERAL_GPIO_RST_IDX->BCR = PERIPHERAL_GPIO_RST_PIN)
#define DAP_RST_TO_LOW()                                    (PERIPHERAL_GPIO_RST_IDX->BSHR = PERIPHERAL_GPIO_RST_PIN)
#define DAP_RST_GET()                                       (((PERIPHERAL_GPIO_RST_IDX->OUTDR & PERIPHERAL_GPIO_RST_PIN) >> PERIPHERAL_GPIO_RST_BIT) ^ 1)

// JTAG SPI
#define JTAG_SPI_BASE                                       SPI2
#define JTAG_SPI_RCC_EN()                                   (RCC->APB1PCENR |= RCC_SPI2EN)
#define JTAG_SPI_RCC_DIS()                                  (RCC->APB1PCENR &= ~RCC_SPI2EN)

#define JTAG_WRITE_DATA(data)                               (JTAG_SPI_BASE->DATAR = data)
#define JTAG_READ_DATA()                                    (JTAG_SPI_BASE->DATAR)
#define JTAG_WAIT_BUSY()                                    (JTAG_SPI_BASE->STATR & SPI_STATR_BSY)


// USART
#define PERIPHERAL_GPIO_USART_RX_IDX                        GPIOA
#define PERIPHERAL_GPIO_USART_RX_BIT                        (3)
#define PERIPHERAL_GPIO_USART_RX_MASK                       GPIO_CFG_MASK_PIN_3
#define PERIPHERAL_GPIO_USART_RX_FIN_CFG                    GPIO_CFG_FIN_PIN_3
#define PERIPHERAL_GPIO_USART_RX_RCC_EN()                   (RCC->APB2PCENR |= RCC_IOPAEN)

#if (PERIPHERAL_GPIO_USART_RX_BIT < 8)
#define USART_RX_TO_FIN()                                   (PERIPHERAL_GPIO_USART_RX_IDX->CFGLR = ((PERIPHERAL_GPIO_USART_RX_IDX->CFGLR & PERIPHERAL_GPIO_USART_RX_MASK) | PERIPHERAL_GPIO_USART_RX_FIN_CFG))
#else
#define USART_RX_TO_FIN()                                   (PERIPHERAL_GPIO_USART_RX_IDX->CFGHR = ((PERIPHERAL_GPIO_USART_RX_IDX->CFGHR & PERIPHERAL_GPIO_USART_RX_MASK) | PERIPHERAL_GPIO_USART_RX_FIN_CFG))
#endif

#define PERIPHERAL_GPIO_USART_TX_IDX                        GPIOA
#define PERIPHERAL_GPIO_USART_TX_BIT                        (2)
#define PERIPHERAL_GPIO_USART_TX_MASK                       GPIO_CFG_MASK_PIN_2
#define PERIPHERAL_GPIO_USART_TX_APP_CFG                    GPIO_CFG_APP_PIN_2
#define PERIPHERAL_GPIO_USART_TX_RCC_EN()                   (RCC->APB2PCENR |= RCC_IOPAEN)

#if (PERIPHERAL_GPIO_USART_TX_BIT < 8)
#define USART_TX_TO_APP()                                   (PERIPHERAL_GPIO_USART_TX_IDX->CFGLR = ((PERIPHERAL_GPIO_USART_TX_IDX->CFGLR & PERIPHERAL_GPIO_USART_TX_MASK) | PERIPHERAL_GPIO_USART_TX_APP_CFG))
#else
#define USART_TX_TO_APP()                                   (PERIPHERAL_GPIO_USART_TX_IDX->CFGHR = ((PERIPHERAL_GPIO_USART_TX_IDX->CFGHR & PERIPHERAL_GPIO_USART_TX_MASK) | PERIPHERAL_GPIO_USART_TX_APP_CFG))
#endif

#define USART_BASE                                          USART2
#define USART_IRQ_VECTOR                                    USART2_IRQn
#define USART_IRQ_HANDLE                                    USART2_IRQHandler
#define USART_GET_IDLE_STATUS()                             (USART_BASE->STATR & USART_STATR_IDLE)
#define USART_CLR_IDLE_STATUS()                             ((void)USART_BASE->DATAR)
#define USART_RCC_EN()                                      (RCC->APB1PCENR |= RCC_USART2EN)

#define USART_DMA_RX                                        DMA1
#define USART_DMA_RX_CHANNEL                                DMA1_Channel6
#define USART_DMA_RX_VECTOR                                 DMA1_Channel6_IRQn
#define USART_DMA_RX_HANDLE                                 DMA1_Channel6_IRQHandler
#define USART_DMA_RX_GET_HALF_STATUS()                      (USART_DMA_RX->INTFR & DMA_HTIF6)
#define USART_DMA_RX_GET_FULL_STATUS()                      (USART_DMA_RX->INTFR & DMA_TCIF6)
#define USART_DMA_RX_CLR_HALF_STATUS()                      (USART_DMA_RX->INTFCR = DMA_CHTIF6)
#define USART_DMA_RX_CLR_FULL_STATUS()                      (USART_DMA_RX->INTFCR = DMA_CTCIF6)
#define USART_DMA_RX_GET_NUM()                              (USART_DMA_RX_CHANNEL->CNTR)
#define USART_DMA_RCC_EN()                                  (RCC->AHBPCENR |= RCC_DMA1EN) 

#define USART_DMA_TX                                        DMA1
#define USART_DMA_TX_CHANNEL                                DMA1_Channel7
#define USART_DMA_TX_VECTOR                                 DMA1_Channel7_IRQn
#define USART_DMA_TX_HANDLE                                 DMA1_Channel7_IRQHandler
#define USART_DMA_TX_EN()                                   (USART_DMA_TX_CHANNEL->CFGR |= DMA_CFGR1_EN)
#define USART_DMA_TX_DIS()                                  (USART_DMA_TX_CHANNEL->CFGR &= ~DMA_CFGR1_EN)
#define USART_DMA_TX_GET_STATUS()                           (USART_DMA_TX->INTFR & DMA_TCIF7)
#define USART_DMA_TX_CLR_STATUS()                           (USART_DMA_TX->INTFCR = DMA_CTCIF7)
#define USART_DMA_TX_BUFFER(addr)                           (USART_DMA_TX_CHANNEL->MADDR = addr)
#define USART_DMA_TX_NUM(len)                               (USART_DMA_TX_CHANNEL->CNTR = len)


#if (DAP_SWD != 0)
extern void *dap_swd_trans_interface_init(uint16_t clk);
extern void dap_swd_gpio_init(void);
extern void dap_swd_gpio_deinit(void);
extern void dap_swd_io_reconfig(void);
extern void dap_swd_trans_init(void);
extern void dap_swd_trans_deinit(void);
#endif
#if (DAP_JTAG != 0)
extern void *dap_jtag_trans_interface_init(uint16_t clk);
extern void dap_jtag_gpio_init(void);
extern void dap_jtag_gpio_deinit(void);
extern void dap_jtag_io_reconfig(void);
extern void dap_jtag_trans_init(void);
extern void dap_jtag_trans_deinit(void);
#endif
#if (DAP_UART != 0)
extern void usart_gpio_init(void);
extern void usart_trans_init(void);
extern void usart_param_config(uint32_t baudrate, uint8_t databits, uint8_t stopbits, uint8_t parity);
extern void dma_param_config(uint8_t *rx_addr, uint32_t rx_len);
#endif
extern void dap_gpio_init(void);

#ifdef __cplusplus
}
#endif

#endif
