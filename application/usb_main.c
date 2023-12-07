/*
 * Copyright (c) 2006-2023, SecondHandCoder
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author                Notes
 * 2023-11-11     SecondHandCoder       first version.
 */

#include "usb_main.h"
#include "dap_main.h"
#include "completion.h"
#include "ringbuffer.h"
#include "rthw.h"
#include "rtthread.h"
#include "usb_descriptor.h"
#include "ch32f205_dap.h"


/* USB dap transfer info */
typedef struct
{
    uint8_t buffer[DAP_PACKET_SIZE];            /* transfer buffer */
    uint16_t buffer_size;                       /* transfer buffer size */
} usb_transfer_t;

/* USB dap request info */
typedef struct 
{
    rt_sem_t usb_req_sem;                       /* semaphore for USB has a request */
    rt_mp_t usb_req_mempool;                    /* mempool for USB transfer data(at least 4-byte alignment) */
    rt_mailbox_t usb_req_mailbox;               /* mailbox for signal and address for transmitting USB pending data */
    usb_transfer_t *cur_req_buffer;             /* current USB request data buffer address */
}usb_dap_req_info_t;

/* USB dap response info */
typedef struct 
{
    rt_mp_t usb_res_mempool;                    /* mempool for USB transfer data(at least 4-byte alignment) */
    rt_mailbox_t usb_res_mailbox;               /* mailbox for signal and address for transmitting USB to sending data */     
    usb_transfer_t *cur_res_buffer;             /* current USB response data buffer address */
    struct rt_completion send_completion;       /* USB send completion synchronization flag */
}usb_dap_res_info_t;

/* USB CDC usb to serial info */
typedef struct
{
    uint32_t usb_rev_len;                       /* the length of data received by USB */
    rt_sem_t usb_revs_sem;                      /* semaphore for USB has received data */
    rt_sem_t usb_reve_sem;                      /* semaphore for USB data has been loaded into ringbuffer */
    struct rt_ringbuffer *rb_usb2usart;         /* ringbuffer for caching data from USB to serial */
    struct rt_completion send_completion;       /* USB to serial data forwarding completion synchronization flag */
    struct rt_completion full_completion;       /* USB to serial data buffer full status synchronization flag */ 
} usb_cdc_info_t;

/* USB CDC serial to usb info */
typedef struct
{
    uint32_t remaining_cnt;                     /* Last DMA data receiving location */
    rt_sem_t usart_rev_sem;                     /* semaphore for serial has received data */
    rt_sem_t usart_empty_sem;                   /* semaphore for serial to USB data cache is empty */
    struct rt_ringbuffer *rb_usart2usb;         /* ringbuffer for caching data from serial to USB */
    struct rt_completion send_completion;       /* serial to USB data forwarding completion synchronization flag */
} usart_cdc_info_t;


static usb_dap_req_info_t usb_dap_reqinfo;
static usb_dap_res_info_t usb_dap_resinfo;

static usb_cdc_info_t usb_cdc_info;
static usart_cdc_info_t usart_cdc_info;

struct usbd_interface dap_intf;
struct usbd_interface intf1;
struct usbd_interface intf2;

/* ch32 USB receive and send buffers require 4-byte alignment
 * If there is no alignment requirement, the buffer can be omitted 
 * and the data can be directly manipulated in the ringbuffer */
static USB_MEM_ALIGNX uint8_t usb_cdc_rev_buff[DAP_PACKET_SIZE];
static USB_MEM_ALIGNX uint8_t usb_cdc_send_buff[DAP_PACKET_SIZE];

/* default serial config 115200 8-n-1 */
#if (DAP_UART != 0)
static struct cdc_line_coding g_cdc_lincoding = {115200, 0, 0, 8};
#endif

/* USB CDC convert data ringbuffer size */
#define USB2USART_RINGBUFFER_SIZE   (4 * 1024)
#define USART2USB_RINGBUFFER_SIZE   (4 * 1024)

static void dap_out_callback(uint8_t ep, uint32_t nbytes);
static void dap_in_callback(uint8_t ep, uint32_t nbytes);
static void usbd_cdc_acm_bulk_out(uint8_t ep, uint32_t nbytes);
static void usbd_cdc_acm_bulk_in(uint8_t ep, uint32_t nbytes);

static struct usbd_endpoint dap_out_ep =
{
    .ep_addr = DAP_OUT_EP,
    .ep_cb = dap_out_callback
};

static struct usbd_endpoint dap_in_ep =
{
    .ep_addr = DAP_IN_EP,
    .ep_cb = dap_in_callback
};

static struct usbd_endpoint cdc_out_ep =
{
    .ep_addr = CDC_OUT_EP,
    .ep_cb = usbd_cdc_acm_bulk_out
};

static struct usbd_endpoint cdc_in_ep =
{
    .ep_addr = CDC_IN_EP,
    .ep_cb = usbd_cdc_acm_bulk_in
};


/**
 * @brief Get a linear block of data from the ring buffer.
 *
 * @param rb            A pointer to the ring buffer object.
 * @param ptr           A pointer to the data buffer.
 *
 * @return Return the linear data size we get from the ring buffer.
 */
static rt_ssize_t rt_get_linear_buffer(struct rt_ringbuffer *rb, rt_uint8_t **ptr)
{
    rt_size_t size;

    RT_ASSERT(rb != RT_NULL);

    *ptr = RT_NULL;

    /* whether has enough data  */
    size = rt_ringbuffer_data_len(rb);

    /* no data */
    if (size == 0)
        return 0;

    *ptr = &rb->buffer_ptr[rb->read_index];

    if(rb->buffer_size - rb->read_index > size)
    {
        return size;
    }

    return rb->buffer_size - rb->read_index;
}

/**
 * @brief Update only read pointers.
 *
 * @param rb            A pointer to the ring buffer object.
 * @param read_size     The size of the read pointer increasing.
 *
 * @return Return the size of the read pointer increasing.
 */
static rt_ssize_t rt_update_read_index(struct rt_ringbuffer *rb, rt_uint16_t read_size)
{
    rt_size_t size;

    RT_ASSERT(rb != RT_NULL);

    /* whether has enough data  */
    size = rt_ringbuffer_data_len(rb);

    /* no data */
    if (size == 0)
        return 0;

    /* less data */
    if(size < read_size)
        read_size = size;

    if(rb->buffer_size - rb->read_index > read_size)
    {
        rb->read_index += read_size;
        return read_size;
    }

    read_size = rb->buffer_size - rb->read_index;

    /* we are going into the other side of the mirror */
    rb->read_mirror = ~rb->read_mirror;
    rb->read_index = 0;

    return read_size;
}

/**
 * @brief Update only write pointers.
 *
 * @param rb            A pointer to the ring buffer object.
 * @param write_size    The size of the write pointer increasing.
 *
 * @return Return the size of the write pointer increasing.
 */
static rt_ssize_t rt_update_write_index(struct rt_ringbuffer *rb, rt_uint16_t write_size)
{
    rt_uint16_t size;
    RT_ASSERT(rb != RT_NULL);

    /* whether has enough space */
    size = rt_ringbuffer_space_len(rb);

    /* no space, drop some data */
    if (size < write_size)
    {
        write_size = size;
    }

    if (rb->buffer_size - rb->write_index > write_size)
    {
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        rb->write_index += write_size;
        return write_size;
    }

    /* we are going into the other side of the mirror */
    rb->write_mirror = ~rb->write_mirror;
    rb->write_index = write_size - (rb->buffer_size - rb->write_index);

    return write_size;
}

/**
 * @brief Usart send data using DMA.
 *
 * @param data          A pointer to the send data.
 * @param len           The size of the send data.
 *
 * @return None.
 */
static void usart_send_bydma(uint8_t *data, uint16_t len)
{
    USART_DMA_TX_DIS();
    USART_DMA_TX_BUFFER((uint32_t)&data[0]);
    USART_DMA_TX_NUM(len);
    USART_DMA_TX_CLR_STATUS();
    USART_DMA_TX_EN();
}

/**
 * @brief Usart interrupt handle, capture idle interrupt.
 * 
 * @return None.
 */
void USART_IRQ_HANDLE(void)
{  
    if (USART_GET_IDLE_STATUS())
    {
        USART_CLR_IDLE_STATUS();
        rt_sem_release(usart_cdc_info.usart_rev_sem);
    }
}

/**
 * @brief DMA receive interrupt handle, capture half transfer interrupt and full transfer interrupt.
 * 
 * @return None.
 */
void USART_DMA_RX_HANDLE(void)
{
    if (USART_DMA_RX_GET_HALF_STATUS())
	{
		USART_DMA_RX_CLR_HALF_STATUS();
        rt_sem_release(usart_cdc_info.usart_rev_sem);
	}
	if (USART_DMA_RX_GET_FULL_STATUS())
	{
		USART_DMA_RX_CLR_FULL_STATUS();
        rt_sem_release(usart_cdc_info.usart_rev_sem);
	}
}

/**
 * @brief DMA transmission completion interrupt.
 * 
 * @return None.
 */
void USART_DMA_TX_HANDLE(void)
{
    if (USART_DMA_TX_GET_STATUS())
	{	
		USART_DMA_TX_CLR_STATUS();
        USART_DMA_TX_DIS();
        rt_completion_done(&usb_cdc_info.send_completion);
	}
}

/**
 * @brief USB DAP has data requests.
 *
 * @param ep            USB endpoint.
 * @param nbytes        The size of the data request.
 *
 * @return None.
 */
static void dap_out_callback(uint8_t ep, uint32_t nbytes)
{
    usb_dap_reqinfo.cur_req_buffer->buffer_size = nbytes;
    rt_sem_release(usb_dap_reqinfo.usb_req_sem);
}

/**
 * @brief USB DAP has data response.
 *
 * @param ep            USB endpoint.
 * @param nbytes        The size of the data response.
 *
 * @return None.
 */
static void dap_in_callback(uint8_t ep, uint32_t nbytes)
{
    if (((nbytes % DAP_PACKET_SIZE) == 0) && (nbytes))
    {
        /* send zlp */
        usbd_ep_start_write(DAP_IN_EP, NULL, 0);
    }
    else
    {
        rt_completion_done(&usb_dap_resinfo.send_completion); 
    }
}

/**
 * @brief USB CDC has data received.
 *
 * @param ep            USB endpoint.
 * @param nbytes        The size of the data received.
 *
 * @return None.
 */
static void usbd_cdc_acm_bulk_out(uint8_t ep, uint32_t nbytes)
{
    usb_cdc_info.usb_rev_len = nbytes;
    rt_sem_release(usb_cdc_info.usb_revs_sem);
}

/**
 * @brief USB CDC has data sent.
 *
 * @param ep            USB endpoint.
 * @param nbytes        The size of the data sent.
 *
 * @return None.
 */
static void usbd_cdc_acm_bulk_in(uint8_t ep, uint32_t nbytes)
{
    if (((nbytes % DAP_PACKET_SIZE) == 0) && (nbytes))
    {
        /* send zlp */
        usbd_ep_start_write(CDC_IN_EP, NULL, 0);
    }
    else
    {
        rt_completion_done(&usart_cdc_info.send_completion); 
    }      
}


/**
 * @brief USB DAP request thread.
 *
 * @param arg           thread arg.
 * 
 * @return None.
 */
static void usb_dap_req_thread(void *arg)
{
    while (1)
    {
        if (rt_sem_take(usb_dap_reqinfo.usb_req_sem, RT_WAITING_FOREVER) == RT_EOK)
        {
            if (usb_dap_reqinfo.cur_req_buffer->buffer[0] == ID_DAP_TransferAbort)
            {
                dap_do_abort();
            }
            else
            {   
                if (rt_mb_send_wait(usb_dap_reqinfo.usb_req_mailbox, (rt_ubase_t)usb_dap_reqinfo.cur_req_buffer, RT_WAITING_FOREVER) == RT_EOK)
                {
                    usb_dap_reqinfo.cur_req_buffer = (usb_transfer_t *)rt_mp_alloc(usb_dap_reqinfo.usb_req_mempool, RT_WAITING_FOREVER);
                }
            }
            
            if (usb_dap_reqinfo.cur_req_buffer != RT_NULL)
            {
                usbd_ep_start_read(DAP_OUT_EP, usb_dap_reqinfo.cur_req_buffer->buffer, DAP_PACKET_SIZE);
		    }
        }
    }
}

/**
 * @brief USB DAP response thread.
 *
 * @param arg           thread arg.
 * 
 * @return None.
 */
static void usb_dap_res_thread(void *arg)
{
    usb_transfer_t *usb_transfer;
    
    while (1)
    {
        if (rt_mb_recv(usb_dap_resinfo.usb_res_mailbox, (rt_ubase_t *)&usb_transfer, RT_WAITING_FOREVER) == RT_EOK)
        {
            rt_completion_init(&usb_dap_resinfo.send_completion);
            do
            {
                usbd_ep_start_write(DAP_IN_EP, usb_transfer->buffer, usb_transfer->buffer_size);
            } while (rt_completion_wait(&usb_dap_resinfo.send_completion, RT_WAITING_FOREVER) != RT_EOK);
            rt_mp_free(usb_transfer);   
        }
    }
}

/**
 * @brief USB DAP data process thread.
 *
 * @param arg           thread arg.
 * 
 * @return None.
 */
static void dap_process_thread(void *arg)
{
    usb_transfer_t *usb_transfer;
    
    while (1)
    {
        if (rt_mb_recv(usb_dap_reqinfo.usb_req_mailbox, (rt_ubase_t *)&usb_transfer, RT_WAITING_FOREVER) == RT_EOK)
        {
            usb_dap_resinfo.cur_res_buffer = (usb_transfer_t *)rt_mp_alloc(usb_dap_resinfo.usb_res_mempool, RT_WAITING_FOREVER);
            usb_dap_resinfo.cur_res_buffer->buffer_size = dap_request_handler(usb_transfer->buffer,
                                                                      usb_dap_resinfo.cur_res_buffer->buffer, DAP_PACKET_SIZE);                    
            rt_mp_free(usb_transfer);
            rt_mb_send_wait(usb_dap_resinfo.usb_res_mailbox, (rt_ubase_t)usb_dap_resinfo.cur_res_buffer, RT_WAITING_FOREVER);
		}
    }
}

/**
 * @brief USB CDC USB data receive thread.
 *
 * @param arg           thread arg.
 * 
 * @return None.
 */
static void usb_cdc_rev_thread(void *arg)
{
    while (1)
    {
        if (rt_sem_take(usb_cdc_info.usb_revs_sem, RT_WAITING_FOREVER) == RT_EOK)
        {
            if (rt_ringbuffer_space_len(usb_cdc_info.rb_usb2usart) < DAP_PACKET_SIZE)
            {
                rt_completion_init(&usb_cdc_info.full_completion);
                while (rt_completion_wait(&usb_cdc_info.full_completion, RT_WAITING_FOREVER) != RT_EOK);
            }
            rt_ringbuffer_put(usb_cdc_info.rb_usb2usart, usb_cdc_rev_buff, usb_cdc_info.usb_rev_len);    
            usbd_ep_start_read(CDC_OUT_EP, usb_cdc_rev_buff, DAP_PACKET_SIZE);
            rt_sem_release(usb_cdc_info.usb_reve_sem);            
        }
    }
}

/**
 * @brief USB CDC USB to usart data process thread.
 *
 * @param arg           thread arg.
 * 
 * @return None.
 */
static void usb_cdc_usb2usart_thread(void *arg)
{
    rt_ssize_t len = 0;
    rt_uint8_t *put_ptr;
    
    while (1)
    {
        if ((len = rt_get_linear_buffer(usb_cdc_info.rb_usb2usart, &put_ptr)) == 0)
        {
            rt_sem_take(usb_cdc_info.usb_reve_sem, RT_WAITING_FOREVER);
        }
        else
        {
            rt_completion_init(&usb_cdc_info.send_completion);
            usart_send_bydma(put_ptr, len);
            if (rt_completion_wait(&usb_cdc_info.send_completion, RT_WAITING_FOREVER) == RT_EOK)
            {
                rt_update_read_index(usb_cdc_info.rb_usb2usart, len);
                if (rt_ringbuffer_space_len(usb_cdc_info.rb_usb2usart) >= DAP_PACKET_SIZE)
                {
                    rt_completion_done(&usb_cdc_info.full_completion);
                }
            }       
        }
    }
}

/**
 * @brief USB CDC usart data receive thread.
 *
 * @param arg           thread arg.
 * 
 * @return None.
 */
static void usart_cdc_rev_thread(void *arg)
{  
    while (1)
    {
        if (rt_sem_take(usart_cdc_info.usart_rev_sem, RT_WAITING_FOREVER) == RT_EOK)
        {
            uint32_t recv_len = 0;
            uint32_t counter = USART_DMA_RX_GET_NUM();

            if (counter <= usart_cdc_info.remaining_cnt)
                recv_len = usart_cdc_info.remaining_cnt - counter;
            else
                recv_len = USART2USB_RINGBUFFER_SIZE + usart_cdc_info.remaining_cnt - counter;

            if (recv_len)
            {
                usart_cdc_info.remaining_cnt = counter;
                rt_base_t level = rt_hw_interrupt_disable();
                rt_update_write_index(usart_cdc_info.rb_usart2usb, recv_len);
                rt_hw_interrupt_enable(level);
                rt_sem_release(usart_cdc_info.usart_empty_sem);
            }
        }
    }
}

/**
 * @brief USB CDC usart to USB data process thread.
 *
 * @param arg           thread arg.
 * 
 * @return None.
 */
static void usb_cdc_usart2usb_thread(void *arg)
{
    while (1)
    {
        if (rt_ringbuffer_data_len(usart_cdc_info.rb_usart2usb) == 0)
        {
            rt_sem_take(usart_cdc_info.usart_empty_sem, RT_WAITING_FOREVER);
        }
        else
        {
            uint32_t len = rt_ringbuffer_get(usart_cdc_info.rb_usart2usb, usb_cdc_send_buff, DAP_PACKET_SIZE);
            rt_completion_init(&usart_cdc_info.send_completion);
            do
            {
                usbd_ep_start_write(CDC_IN_EP, usb_cdc_send_buff, len);
            } while (rt_completion_wait(&usart_cdc_info.send_completion, RT_WAITING_FOREVER) != RT_EOK);   
        }
    }
}

/**
 * @brief Usart gpio register and parameter init, dma init.
 *
 * @return None.
 */
void usart_dma_init(void)
{
#if (DAP_UART != 0)    
    usart_gpio_init();
    usart_trans_init();
    usart_param_config(g_cdc_lincoding.dwDTERate, g_cdc_lincoding.bDataBits, g_cdc_lincoding.bCharFormat, g_cdc_lincoding.bParityType);
    dma_param_config(&usart_cdc_info.rb_usart2usb->buffer_ptr[0], USART2USB_RINGBUFFER_SIZE);
#endif    
}

/**
 * @brief USB descriptor interface endpoints init.
 *
 * @return None.
 */
void usb_interface_init(void)
{
    usbd_desc_register(cmsisdap_descriptor);
    usbd_bos_desc_register(&bos_desc);
    usbd_msosv2_desc_register(&msosv2_desc);

    /*!< winusb */
    usbd_add_interface(&dap_intf);
    usbd_add_endpoint(&dap_out_ep);
    usbd_add_endpoint(&dap_in_ep);

    /*!< cdc acm */
#if (DAP_UART != 0)        
    usbd_add_interface(usbd_cdc_acm_init_intf(&intf1));
    usbd_add_interface(usbd_cdc_acm_init_intf(&intf2));
    usbd_add_endpoint(&cdc_out_ep);
    usbd_add_endpoint(&cdc_in_ep);
#endif       
    usbd_initialize();
}

/**
 * @brief Thread create, ipc create.
 *
 * @return None.
 */
void thread_ipc_int(void)
{
    rt_thread_t tid;
    
    // usb req
    usb_dap_reqinfo.usb_req_sem = rt_sem_create("usb_reqsem", 0, RT_IPC_FLAG_FIFO);
    RT_ASSERT(usb_dap_reqinfo.usb_req_sem != RT_NULL)
    usb_dap_reqinfo.usb_req_mailbox = rt_mb_create("usb_reqmb", DAP_PACKET_COUNT, RT_IPC_FLAG_FIFO);                  
    RT_ASSERT(usb_dap_reqinfo.usb_req_mailbox != RT_NULL)
    usb_dap_reqinfo.usb_req_mempool = rt_mp_create("usb_reqmp", DAP_PACKET_COUNT, sizeof(usb_transfer_t));
    RT_ASSERT(usb_dap_reqinfo.usb_req_mempool != RT_NULL)

    tid = rt_thread_create("usb_dap_req",
                        usb_dap_req_thread, RT_NULL,
                        512, 3, 10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    // usb res
    rt_completion_init(&usb_dap_resinfo.send_completion);
    usb_dap_resinfo.usb_res_mailbox = rt_mb_create("usb_resmb", DAP_PACKET_COUNT, RT_IPC_FLAG_FIFO);                  
    RT_ASSERT(usb_dap_resinfo.usb_res_mailbox != RT_NULL)
    usb_dap_resinfo.usb_res_mempool = rt_mp_create("usb_resmp", DAP_PACKET_COUNT, sizeof(usb_transfer_t));
    RT_ASSERT(usb_dap_resinfo.usb_res_mempool != RT_NULL)

    tid = rt_thread_create("usb_dap_res",
                        usb_dap_res_thread, RT_NULL,
                        512, 3, 10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    // usb dap
    tid = rt_thread_create("dap_process",
                        dap_process_thread, RT_NULL,
                        1024, 5, 10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

#if (DAP_UART != 0)
    // usb cdc
    usb_cdc_info.usb_rev_len = 0;
    rt_completion_init(&usb_cdc_info.send_completion);
    rt_completion_init(&usb_cdc_info.full_completion);
    usb_cdc_info.usb_revs_sem = rt_sem_create("usb_cdc_revs", 0, RT_IPC_FLAG_FIFO);
    RT_ASSERT(usb_cdc_info.usb_revs_sem != RT_NULL)
    usb_cdc_info.usb_reve_sem = rt_sem_create("usb_cdc_reve", 0, RT_IPC_FLAG_FIFO);
    RT_ASSERT(usb_cdc_info.usb_reve_sem != RT_NULL)
    usb_cdc_info.rb_usb2usart = rt_ringbuffer_create(USB2USART_RINGBUFFER_SIZE);
    RT_ASSERT(usb_cdc_info.rb_usb2usart != RT_NULL)

    tid = rt_thread_create("usb_cdc_rev",
                        usb_cdc_rev_thread, RT_NULL,
                        512, 5, 10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);
    
    tid = rt_thread_create("usb_cdc_usb",
                        usb_cdc_usb2usart_thread, RT_NULL,
                        512, 3, 10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    // usart cdc
    usart_cdc_info.remaining_cnt = 0;
    rt_completion_init(&usart_cdc_info.send_completion);
    usart_cdc_info.usart_rev_sem = rt_sem_create("usart_cdc_rev", 0, RT_IPC_FLAG_FIFO);
    RT_ASSERT(usart_cdc_info.usart_rev_sem != RT_NULL)
    usart_cdc_info.usart_empty_sem = rt_sem_create("usart_cdc_empty", 0, RT_IPC_FLAG_FIFO);
    RT_ASSERT(usart_cdc_info.usart_empty_sem != RT_NULL)
    usart_cdc_info.rb_usart2usb = rt_ringbuffer_create(USART2USB_RINGBUFFER_SIZE);
    RT_ASSERT(usart_cdc_info.rb_usart2usb != RT_NULL)

    tid = rt_thread_create("usart_cdc_rev",
                        usart_cdc_rev_thread, RT_NULL,
                        512, 3, 10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);
    
    tid = rt_thread_create("usart_cdc_usart",
                        usb_cdc_usart2usb_thread, RT_NULL,
                        512, 5, 10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);
#endif               
}

/**
 * @brief USB start to transfer.
 * 
 * @return None.
 */
void usb_transfer_start(void)
{
    usb_dap_reqinfo.cur_req_buffer = (usb_transfer_t *)rt_mp_alloc(usb_dap_reqinfo.usb_req_mempool, 0);
    usbd_ep_start_read(DAP_OUT_EP, usb_dap_reqinfo.cur_req_buffer->buffer, DAP_PACKET_SIZE);
#if (DAP_UART != 0)    
    usbd_ep_start_read(CDC_OUT_EP, usb_cdc_rev_buff, DAP_PACKET_SIZE);
#endif
}

/**
 * @brief USB CDC set usart parameter, usbd_cdc.c called.
 * 
 * @param intf           USB setup packet wIndex.
 * @param line_coding    USB CDC usart parameter.
 *
 * @return None.
 */
#if (DAP_UART != 0)
void usbd_cdc_acm_set_line_coding(uint8_t intf, struct cdc_line_coding *line_coding)
{
    if (rt_memcmp(line_coding, (uint8_t *)&g_cdc_lincoding, sizeof(struct cdc_line_coding)) != 0)
    {
        rt_memcpy((uint8_t *)&g_cdc_lincoding, line_coding, sizeof(struct cdc_line_coding));

        usb_cdc_info.usb_rev_len = 0;
        rt_completion_init(&usb_cdc_info.send_completion);
        rt_completion_init(&usb_cdc_info.full_completion);
        rt_sem_control(usb_cdc_info.usb_revs_sem, RT_IPC_CMD_RESET, 0);
        rt_sem_control(usb_cdc_info.usb_reve_sem, RT_IPC_CMD_RESET, 0);
        rt_ringbuffer_reset(usb_cdc_info.rb_usb2usart);

        usart_cdc_info.remaining_cnt = 0;
        rt_completion_init(&usart_cdc_info.send_completion);
        rt_sem_control(usart_cdc_info.usart_rev_sem, RT_IPC_CMD_RESET, 0);
        rt_sem_control(usart_cdc_info.usart_empty_sem, RT_IPC_CMD_RESET, 0);
        rt_ringbuffer_reset(usart_cdc_info.rb_usart2usb);

        usart_param_config(g_cdc_lincoding.dwDTERate, g_cdc_lincoding.bDataBits, g_cdc_lincoding.bCharFormat, g_cdc_lincoding.bParityType);
        dma_param_config(&usart_cdc_info.rb_usart2usb->buffer_ptr[0], USART2USB_RINGBUFFER_SIZE);
    }   
}
#endif

/**
 * @brief USB CDC get usart parameter, usbd_cdc.c called.
 * 
 * @param intf           USB setup packet wIndex.
 * @param line_coding    USB CDC usart parameter.
 *
 * @return None.
 */
#if (DAP_UART != 0)
void usbd_cdc_acm_get_line_coding(uint8_t intf, struct cdc_line_coding *line_coding)
{
    rt_memcpy(line_coding, (uint8_t *)&g_cdc_lincoding, sizeof(struct cdc_line_coding));
}
#endif
