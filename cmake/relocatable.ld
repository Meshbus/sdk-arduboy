/* SPDX-License-Identifier: Apache-2.0 */
/* Resolve COMDAT first, then merge allocation classes for Zephyr LLEXT.
 * In particular the loader accepts exactly one SHT_NOBITS allocation.
 * This is a relocatable layout: no host address, memory region or entry point.
 */
SECTIONS
{
    .text : { *(.text .text.* .gnu.linkonce.t.*) }
    .rodata : { *(.rodata .rodata.* .gnu.linkonce.r.*) }
    .data : { *(.data .data.* .gnu.linkonce.d.*) }
    .bss (NOLOAD) : { *(.bss .bss.* .gnu.linkonce.b.*) *(COMMON) }
    .preinit_array : { KEEP(*(.preinit_array .preinit_array.*)) }
    .init_array : { KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*))) KEEP(*(.init_array)) }
    .fini_array : { KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*))) KEEP(*(.fini_array)) }
    .exported_sym : { KEEP(*(.exported_sym)) }
}
