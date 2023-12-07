/*
 * Copyright (c) 2006-2023, SecondHandCoder
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author                Notes
 * 2023-11-22     SecondHandCoder       first version.
 */

#include "ch32f205_config.h"
#include "usb_main.h"
#include "usbd_core.h"
#include "startup.h"
#include "ch32f205_clk.h"
#include "ch32f205_backup.h"


typedef void (*funct_ptr)(void);

void Reset_Handler(void);

/* Minimal initial Flash-based vector table */
uint32_t *const VectorTable[] __USED __VECTOR_TABLE_ATTRIBUTE =
{
	(uint32_t *)CHIP_SRAM_END,
	(uint32_t *)Reset_Handler
};

/**
 * @brief Boot main entry.
 *
 * @return None.
 */
void Reset_Handler(void)
{
	/* TODO App write update flag or app vector is empty, run boot process */
	//if ((get_backup_data() == BACK_UP_DATA) || ((*(__IO uint32_t *)CHIP_APP_START) != CHIP_SRAM_END))
    //{
	//	__disable_irq();
	//	SystemInit();
	//	init_data_bss();
	//	usb_interface_init();
	//	__enable_irq();
	//	while (1)
	//	{
	//		if (boot_update_main() == BOOT_END)
	//		{
	//			set_backup_data(0);
	//			break;
	//		}	
	//	};
	//	__disable_irq();
	//	NVIC_SystemReset();
	//}
	__disable_irq();
	funct_ptr app_ptr = (funct_ptr) *(__IO uint32_t *)(CHIP_APP_START + 0x04);
	SCB->VTOR = CHIP_APP_START;
	__set_PSP(*(__IO uint32_t *)CHIP_APP_START);
    __set_CONTROL(0);
	__set_MSP(*(__IO uint32_t *)CHIP_APP_START);
	__enable_irq();
	app_ptr();
	while(1);
}
