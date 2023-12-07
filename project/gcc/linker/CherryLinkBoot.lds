/*###ICF### Section handled by SecondHandCoder ****/

ENTRY(Reset_Handler)

stack_size                      = 0x00000400;
heap_size                       = 0x00000200;
boot_size                       = 0x00002000;
m_vector_ram_size               = 0x00000400;

MEMORY
{
    m_interrupts   (RX)  : ORIGIN = 0x08000000, LENGTH = 0x00000400
    m_text         (RX)  : ORIGIN = 0x08000400, LENGTH = 0x0001EC00
    m_data         (RW)  : ORIGIN = 0x20000000, LENGTH = 0x00008000
}

SECTIONS
{
    .interrupts :
    {
        . = ALIGN(4);
        KEEP(*(.vectors))        /* Startup code */ 
        . = ALIGN(4);
    } > m_interrupts
    
    .text :
    {
        . = ALIGN(4);
        *(.text)                 /* .text sections (code) */
        *(.text*)                /* .text* sections (code) */
        *(.rodata)               /* .rodata sections (constants, strings, etc.) */
        *(.rodata*)              /* .rodata* sections (constants, strings, etc.) */
        *(.init)                 /* section used in crti.o files */
        *(.fini)                 /* section used in crti.o files */
        *(.eh_frame)             /* section used in crtbegin.o files */
        . = ALIGN(4);
    } > m_text

    /* Section used by the libgcc.a library for fvp4 */
    .ARM :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } > m_text

    _sidata = .;

    .interrupts_ram :
    {
        . = ALIGN(4);
        . += m_vector_ram_size;
        . = ALIGN(4); 
    } > m_data

    .data : AT(_sidata)
    {
        . = ALIGN(4);
        _sdata = .; 
        *(.data)                 /* .data sections */
        *(.data*)                /* .data* sections */
        . = ALIGN(4);
        _edata = .;
    } > m_data

    _eidata = _sidata + (_edata - _sdata);

    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > m_data

    .heap :
    {
        . = ALIGN(8);
        PROVIDE(end = .);
        PROVIDE(_end = .);
        PROVIDE(__end = .);
        . += heap_size;
        . = ALIGN(8);
        __HeapLimit = .;
    } > m_data

    __StackTop   = ORIGIN(m_data) + LENGTH(m_data);
    __StackLimit = __StackTop - stack_size;
    PROVIDE(__stack = __StackTop);

    .stack __StackLimit :
    {
        . = ALIGN(8);
        . += stack_size;
    } > m_data

    .ARM.attributes 0 : { *(.ARM.attributes) }
    ASSERT(__StackLimit >= __HeapLimit, "region m_data overflowed with stack and heap")
}
