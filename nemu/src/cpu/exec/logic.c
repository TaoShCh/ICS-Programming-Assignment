#include "cpu/exec.h"

make_EHelper(test) {
  rtl_and(&t2, &id_dest->val, &id_src->val);

  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);

  print_asm_template2(test);
}

make_EHelper(and) {
  rtl_and(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);

  print_asm_template2(and);
}

make_EHelper(xor) {
  rtl_xor(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  print_asm_template2(xor);
}

make_EHelper(or) {
  rtl_or(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);

  print_asm_template2(or);
}

make_EHelper(sar) {
  if(id_dest->width == 4) {
//	printf("sar4 0x%-8x 0x%-2x\n", id_dest->val, id_src->val);
	rtl_sar(&t2, &id_dest->val, &id_src->val);
  } else if(id_dest->width == 2) {
//	printf("sar2 0x%-8x 0x%-2x\n", id_dest->val, id_src->val);
	t0 = (int16_t) id_dest->val;
	rtl_sar(&t2, &t0, &id_src->val);
	//t2 = ((int16_t)id_dest->val) >> id_src->val;
  } else {
//	printf("sar1 0x%-8x 0x%-2x\n", id_dest->val, id_src->val);
	t0 = (int8_t) id_dest->val;
	rtl_sar(&t2, &t0, &id_src->val);
	//t2 = ((int8_t)id_dest->val) >> id_src->val;
  }

  operand_write(id_dest, &t2);
  // unnecessary to update CF and OF in NEMU
  rtl_update_ZFSF(&t2, id_dest->width);

  print_asm_template2(sar);
}

make_EHelper(rol) {
	rtl_shri(&t2, &id_dest->val, id_dest->width * 8 - id_src->val);
	rtl_shl(&t3, &id_dest->val, &id_src->val);
	rtl_or(&t1, &t2, &t3);
	operand_write(id_dest, &t1);

//	if (id_src->val == 1) {
//		t2 = (t1 >> (id_dest->width * 8 -1)) & 0x1;
//		rtl_get_CF(&t0);
//		if (t2 == t0) {
//			rtl_set_OF(&tzero);
//		} else {
//			t2 = 1;
//			rtl_set_OF(&t2);
//		}
//	}
    
	print_asm_template2(rol);
}

make_EHelper(shl) {
  rtl_shl(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  // unnecessary to update CF and OF in NEMU
  rtl_update_ZFSF(&t2, id_dest->width);

  print_asm_template2(shl);
}

make_EHelper(shr) {
  rtl_shr(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  // unnecessary to update CF and OF in NEMU
  rtl_update_ZFSF(&t2, id_dest->width);

  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  t2 = id_dest->val;
  rtl_not(&t2);
  operand_write(id_dest, &t2);

  print_asm_template1(not);
}
