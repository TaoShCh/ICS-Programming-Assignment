#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  if (id_dest->width == 1) {
	rtl_sext(&t0, &id_dest->val, 1);
	rtl_push(&t0);
  } else {
	rtl_push(&id_dest -> val);
  }
  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&t0);
  operand_write(id_dest, &t0);
  print_asm_template1(pop);
}

make_EHelper(pusha) {	
  if (decoding.is_operand_size_16) {
	Assert(0, "pusha operand size is 16");
	//t0 = cpu.gpr[4]._16;
	//rtl_push((rtlreg_t *)&cpu.gpr[0]._16);
	//rtl_push((rtlreg_t *)&cpu.gpr[1]._16);
	//rtl_push((rtlreg_t *)&cpu.gpr[2]._16);
	//rtl_push((rtlreg_t *)&cpu.gpr[3]._16);
	//rtl_push(&t0);
	//rtl_push((rtlreg_t *)&cpu.gpr[5]._16);
	//rtl_push((rtlreg_t *)&cpu.gpr[6]._16);
	//rtl_push((rtlreg_t *)&cpu.gpr[7]._16);
  } else {
	t0 = cpu.esp;
	rtl_push((rtlreg_t *)&cpu.eax);
	rtl_push((rtlreg_t *)&cpu.ecx);
	rtl_push((rtlreg_t *)&cpu.edx);
	rtl_push((rtlreg_t *)&cpu.ebx);
	rtl_push(&t0);
	rtl_push((rtlreg_t *)&cpu.ebp);
	rtl_push((rtlreg_t *)&cpu.esi);
	rtl_push((rtlreg_t *)&cpu.edi);
  }
  print_asm("pusha");
}

make_EHelper(popa) {
  if (decoding.is_operand_size_16) {
	Assert(0, "popa operand size is 16");
  } else {
	rtl_pop((rtlreg_t *)&cpu.edi);
	rtl_pop((rtlreg_t *)&cpu.esi);
	rtl_pop((rtlreg_t *)&cpu.ebp);
	rtl_pop(&t0);
	rtl_pop((rtlreg_t *)&cpu.ebx);
	rtl_pop((rtlreg_t *)&cpu.edx);
	rtl_pop((rtlreg_t *)&cpu.ecx);
	rtl_pop((rtlreg_t *)&cpu.eax);
  }
  print_asm("popa");
}

make_EHelper(leave) {
  reg_l(R_ESP) = reg_l(R_EBP);
  rtl_pop(&t0);
  if (decoding.is_operand_size_16) {
	  reg_w(R_BP) = (uint16_t) t0;
  } else {
	  reg_l(R_EBP) = t0;
  }
  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
	  int32_t temp = (int32_t) (int16_t) (uint16_t) reg_w(R_AX);
	  if (temp < 0) {
		reg_w(R_DX) = 0xffff;
	  } else {
		reg_w(R_DX) = 0x0;
	  }
  }
  else {
	  int32_t temp = reg_l(R_EAX);
	  if (temp < 0) {
		reg_l(R_EDX) = 0xffffffff;
	  } else {
		reg_l(R_EDX) = 0x0;
	  }
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
	  int16_t temp = (int16_t) (int8_t) (uint8_t) reg_b(R_AL);
	  reg_w(R_AX) = temp;
  }
  else {
	  int32_t temp = (int32_t) (int16_t) (uint16_t) reg_w(R_AX);
	  reg_l(R_EAX) = temp;
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
