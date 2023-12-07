#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* RT-Thread Kernel */
#define RT_NAME_MAX 8
#define RT_ALIGN_SIZE 8
#define RT_THREAD_PRIORITY_32
#define RT_THREAD_PRIORITY_MAX 32
#define RT_TICK_PER_SECOND 1000
#define RT_USING_IDLE_HOOK
#define IDLE_THREAD_STACK_SIZE 256

/* Inter-Thread communication */
#define RT_USING_SEMAPHORE
#define RT_USING_MAILBOX

/* Memory Management */
#define RT_USING_MEMPOOL
#define RT_USING_SMALL_MEM
#define RT_USING_SMALL_MEM_AS_HEAP
#define RT_USING_HEAP

/* Kernel Device Object */
#define RT_USING_HW_ATOMIC
#define RT_USING_CPU_FFS

/* RT-Thread Components */
#define RT_USING_COMPONENTS_INIT
#define RT_USING_USER_MAIN
#define RT_MAIN_THREAD_STACK_SIZE 512
#define RT_MAIN_THREAD_PRIORITY 10

/* LIBC */
#define RT_USING_LIBC

#endif
