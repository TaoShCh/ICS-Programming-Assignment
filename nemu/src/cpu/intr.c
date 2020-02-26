#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
	rtl_push((rtlreg_t *)&cpu.eflags_ini);
	cpu.eflags.IF = 0; 
	rtl_push((rtlreg_t *)&cpu.cs);
	rtl_push((rtlreg_t *)&ret_addr);
	uint32_t idt_base = cpu.idtr.base;
	uint32_t offset = vaddr_read(idt_base + NO * 8, 2);
	offset += (vaddr_read(idt_base + NO * 8 + 6, 2) << 16);
	decoding.jmp_eip = offset;
	decoding.is_jmp = true;
}

void dev_raise_intr() {
	cpu.INTR = true;
}
