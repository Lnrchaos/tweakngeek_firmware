/*
 * TweaknGeek Firmware Linker Script for STM32WB55RG
 * Memory layout for FlipperZero hardware
 */

/* Entry Point */
ENTRY(Reset_Handler)

/* Memory Regions */
MEMORY
{
    /* STM32WB55RG Flash memory layout */
    FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 1024K
    
    /* STM32WB55RG SRAM layout */
    SRAM1 (rwx)     : ORIGIN = 0x20000000, LENGTH = 192K
    SRAM2A (rwx)    : ORIGIN = 0x20030000, LENGTH = 32K
    SRAM2B (rwx)    : ORIGIN = 0x20038000, LENGTH = 32K
    
    /* Reserved for wireless stack */
    SRAM_SHARED (rwx) : ORIGIN = 0x20030000, LENGTH = 10K
}

/* Stack and heap sizes */
_Min_Heap_Size = 0x8000;   /* 32KB heap */
_Min_Stack_Size = 0x2000;  /* 8KB stack */

/* Sections */
SECTIONS
{
    /* Vector table and startup code */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } >FLASH

    /* Program code and constants */
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame)

        KEEP (*(.init))
        KEEP (*(.fini))

        . = ALIGN(4);
        _etext = .;
    } >FLASH

    /* Read-only data */
    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } >FLASH

    /* ARM exception handling */
    .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
    .ARM : {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } >FLASH

    /* Constructor/destructor tables */
    .preinit_array     :
    {
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array*))
        PROVIDE_HIDDEN (__preinit_array_end = .);
    } >FLASH
    
    .init_array :
    {
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array*))
        PROVIDE_HIDDEN (__init_array_end = .);
    } >FLASH
    
    .fini_array :
    {
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array*))
        PROVIDE_HIDDEN (__fini_array_end = .);
    } >FLASH

    /* Initialized data section */
    _sidata = LOADADDR(.data);
    .data : 
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } >SRAM1 AT> FLASH

    /* Uninitialized data section */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        __bss_start__ = _sbss;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
        __bss_end__ = _ebss;
    } >SRAM1

    /* Heap and stack */
    ._user_heap_stack :
    {
        . = ALIGN(8);
        PROVIDE ( end = . );
        PROVIDE ( _end = . );
        . = . + _Min_Heap_Size;
        . = . + _Min_Stack_Size;
        . = ALIGN(8);
    } >SRAM1

    /* TweaknGeek specific sections */
    
    /* Kernel code section */
    .kernel_text :
    {
        . = ALIGN(4);
        _kernel_start = .;
        *(.kernel_text)
        *(.kernel_text*)
        . = ALIGN(4);
        _kernel_end = .;
    } >FLASH

    /* HAL code section */
    .hal_text :
    {
        . = ALIGN(4);
        _hal_start = .;
        *(.hal_text)
        *(.hal_text*)
        . = ALIGN(4);
        _hal_end = .;
    } >FLASH

    /* Application runtime section */
    .runtime_text :
    {
        . = ALIGN(4);
        _runtime_start = .;
        *(.runtime_text)
        *(.runtime_text*)
        . = ALIGN(4);
        _runtime_end = .;
    } >FLASH

    /* System configuration section */
    .system_config :
    {
        . = ALIGN(4);
        _config_start = .;
        *(.system_config)
        *(.system_config*)
        . = ALIGN(4);
        _config_end = .;
    } >FLASH

    /* Application memory pool in SRAM2A */
    .app_memory :
    {
        . = ALIGN(4);
        _app_memory_start = .;
        . = . + LENGTH(SRAM2A) - 4K; /* Reserve 4K for system */
        _app_memory_end = .;
    } >SRAM2A

    /* System reserved memory in SRAM2B */
    .system_memory :
    {
        . = ALIGN(4);
        _system_memory_start = .;
        . = . + LENGTH(SRAM2B);
        _system_memory_end = .;
    } >SRAM2B

    /* Remove information from the standard libraries */
    /DISCARD/ :
    {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }

    .ARM.attributes 0 : { *(.ARM.attributes) }
}