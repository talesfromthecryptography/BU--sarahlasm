#include <stdio.h>
#include <string.h>

#include "bu.h"


int main() {
  bigunsigned a,b,c,d;
  char s[BU_MAX_HEX+1];

  printf("Read hex tests:\n");
  bu_readhex(&a,"DEADBEEFDEAD4444");
  bu_readhex(&b,"111111111111");
  bu_readhex(&c, "1234");
  bu_dbg_printf(&a);
  printf("\n");

  printf("Shift left tests:\n");
  bu_shl(&d, &a, 32);
  bu_dbg_printf(&d);
  bu_shl(&d, &a, 37);
  bu_dbg_printf(&d);
  printf("\n");

  printf("Shift right tests:\n");
  bu_shr(&d, &a, 1);
  bu_dbg_printf(&d);
  bu_shr(&d, &a, 32);
  bu_dbg_printf(&d);
  bu_shr_ip(&a, 37);
  bu_dbg_printf(&a);
  printf("\n");

  bu_readhex(&a,"DEADBEEFDEADBEEF");
  printf("Multiply digit tests:\n");
  bu_mul_digit(&d, &a, 2);
  bu_dbg_printf(&d);
  bu_mul_digit_ip(&a, 9);
  bu_dbg_printf(&a);
  printf("\n");

  bu_readhex(&a,"DEADBEEFDEADBEEF");
  bu_readhex(&c, "1234");
  printf("Multiply tests:\n");
  bu_mul(&d, &a, &c);
  bu_dbg_printf(&d);
  bu_sqr(&d, &c);
  bu_dbg_printf(&d);
  bu_sqr_ip(&c);
  bu_dbg_printf(&c);

  return 0;
}
