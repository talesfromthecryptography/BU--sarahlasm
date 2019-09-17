#include <string.h> // for memset, etc.
#include <stdio.h>  // for printf

#include "bu.h"

// NOTE: In this code "word" always refers to a uint32_t

// Copy dest = src
void bu_cpy(bigunsigned *dest, bigunsigned *src) {
  uint16_t cnt = src->used;
  dest->used = cnt;
  dest->base = 0;

  // reset upper 0s in dest
  memset(dest->digit + cnt, 0, sizeof(uint32_t)*(BU_DIGITS-cnt));

  uint8_t i_dest = dest->base;
  uint8_t i_src = src->base;

  while (cnt-- > 0) {
    dest->digit[i_dest++] = src->digit[i_src++];
  }
}

// Set to 0
void bu_clear(bigunsigned *a_ptr) {
  memset(a_ptr->digit, 0, sizeof(uint32_t)*BU_DIGITS);
  a_ptr->used = 0;
  a_ptr->base = 0;
}

// Shift left, not in place
// Produce a = b << cnt
// DONE as far as I can tell
void bu_shl(bigunsigned* a_ptr, bigunsigned* b_ptr, uint16_t cnt)
{
  if (cnt == 0) return; //No shift necessary

  uint16_t wrds = cnt >> 5; // # of whole words to shift
  uint16_t bits = cnt &0x1f;// number of bits in a word to shift
  uint32_t mask = 0xffffffff << (32 - bits);

  bu_clear(a_ptr);
  a_ptr->used = b_ptr->used;
  a_ptr->base = b_ptr->base;

  uint32_t new_carry = 0;
  uint32_t old_carry = 0;
  // If you're shifting a full number of words:
  // Increase used by that number of words
  a_ptr->used += wrds;
  // Decrease base by that number of words (base should be least significant word)
  a_ptr->base -= wrds;
  // Starting at the least significant word:
  for (uint16_t i = 0; i < a_ptr->used; ++i) {
    // Calculate the new carry
    uint16_t position = i + a_ptr->base;
    if (bits != 0)
      new_carry = (b_ptr->digit[position % BU_DIGITS] & mask) >> (BU_BITS_PER_DIGIT - bits);
    // Shift the appropriate number of bits (bits < 32), taking in to account the old carry
    a_ptr->digit[position % BU_DIGITS] = (b_ptr->digit[position % BU_DIGITS] << bits);
    a_ptr->digit[position % BU_DIGITS] |= old_carry;
    // Old carry = new carry
    old_carry = new_carry;
  }
  // If you've reached the most significant word and you still have a carry:
  if (old_carry != 0) {
    // Increase used by one
    a_ptr->used++;
    // Stick the carry in the next index
    a_ptr->digit[(a_ptr->base + a_ptr->used - 1) % BU_DIGITS] = old_carry;
  }
}

// Shift in place a bigunsigned by cnt bits to the left
// Example: beef shifted by 4 results in beef0
void bu_shl_ip(bigunsigned* a_ptr, uint16_t cnt) {
  bigunsigned tmp;
  bu_clear(&tmp);
  bu_cpy(&tmp, a_ptr);
  bu_shl(a_ptr, &tmp, cnt);
}

// Shift right, not in place
// Produce a = b >> c
// DONE as far as I can tell
void bu_shr(bigunsigned* a_ptr, bigunsigned* b_ptr, uint16_t cnt) {
  uint16_t wrds = cnt >> 5; // # of whole words to shift
  uint16_t bits = cnt &0x1f;// number of bits in a word to shift
  uint32_t mask = 0xffffffff >> (32 - bits);

  bu_clear(a_ptr);
  if (wrds > b_ptr->used) return;
  a_ptr->used = b_ptr->used;
  a_ptr->base = b_ptr->base;


  uint32_t new_carry = 0;
  uint32_t old_carry = 0;
  // If you're shifting a full amount of words:
  // Decrease used by that number of words
  a_ptr->used -= wrds;
  printf("Used: %d\n", a_ptr->used);
  // Increase base by that number of words
  a_ptr->base += wrds;
  // Starting at the most significant word:
  for (uint16_t i = 0; i < a_ptr->used; ++i) {
    // Calculate the new carry
    uint16_t position = a_ptr->used + a_ptr->base - i - 1;
    new_carry = (b_ptr->digit[position % BU_DIGITS] & mask) << (BU_BITS_PER_DIGIT - bits);
    // Shift the appropriate number of bits (bits < 32), taking in to account the old carry
    a_ptr->digit[position % BU_DIGITS] = (b_ptr->digit[position % BU_DIGITS] >> bits) | old_carry;
    // Old carry = new carry
    old_carry = new_carry;
  }

}

// Shift right, in place
void bu_shr_ip(bigunsigned *a_ptr, uint16_t cnt) {
  bigunsigned tmp;
  bu_clear(&tmp);
  bu_cpy(&tmp, a_ptr);
  bu_shr(a_ptr, &tmp, cnt);
}


// Produce a = b + c
// DONE
void bu_add(bigunsigned *a_ptr, bigunsigned *b_ptr, bigunsigned *c_ptr) {
  uint8_t carry = 0;
  uint64_t nxt;
  uint16_t cnt = 0;
  uint16_t min_used = b_ptr->used <= c_ptr->used
                      ? b_ptr->used : c_ptr->used;
  uint8_t  b_dig = b_ptr->base;
  uint8_t  c_dig = c_ptr->base;
  uint8_t  a_dig = 0;

  while (cnt < min_used) {
    nxt = ((uint64_t)b_ptr->digit[b_dig++])
          + (uint64_t)(c_ptr->digit[c_dig++]) + carry;
    carry = 0 != (nxt&0x100000000);
    a_ptr->digit[a_dig++] = (uint32_t)nxt;
    cnt++;
  }

  while (cnt < b_ptr->used && carry) {
    nxt = ((uint64_t)b_ptr->digit[b_dig++]) + carry;
    carry = 0 != (nxt&0x100000000);
    a_ptr->digit[a_dig++] = (uint32_t)nxt;
    cnt++;
  }

  while (cnt < b_ptr->used) {
    a_ptr->digit[a_dig++] = b_ptr->digit[b_dig++];
    cnt++;
  }

  while (cnt < c_ptr->used && carry) {
    nxt = ((uint64_t)c_ptr->digit[c_dig++]) + carry;
    carry = 0 != (nxt&0x100000000);
    a_ptr->digit[a_dig++] = (uint32_t)nxt;
    cnt++;
  }

  while (cnt < c_ptr->used) {
    a_ptr->digit[a_dig++] = c_ptr->digit[c_dig++];
    cnt++;
  }

  while (cnt < BU_DIGITS && carry) {
    a_ptr->digit[a_dig++] = 1;
    carry = 0;
    cnt++;
  }

  a_ptr->base = 0;
  a_ptr->used = cnt;
}

void bu_add_ip(bigunsigned *a_ptr, bigunsigned *b_ptr) {
  bigunsigned tmp;
  bu_clear(&tmp);
  bu_cpy(&tmp, a_ptr);
  bu_add(a_ptr, &tmp, b_ptr);
}

//a = b * d
void bu_mul_digit(bigunsigned *a_ptr, bigunsigned *b_ptr, uint32_t d) {
  bigunsigned carry;
  bu_clear(&carry);

  bu_clear(a_ptr);
  a_ptr->used = b_ptr->used;
  a_ptr->base = b_ptr->base;
  carry.used = (b_ptr->used + b_ptr->base) % BU_DIGITS;
  carry.base = b_ptr->base;

  uint64_t res = 0;

  for (uint16_t i = 0; i < b_ptr->used + b_ptr->base; ++i) {
    res = b_ptr->digit[i + b_ptr->base] * (uint64_t)d;
    a_ptr->digit[i + a_ptr->base] = (uint32_t)(res); //res_lo
    carry.digit[i + a_ptr->base + 1] = (uint32_t)(res >> 32); //res_hi
  }
  a_ptr->used += b_ptr->base;
  bu_shl_ip(&carry, 32);
  bu_add_ip(a_ptr, &carry);
  return;

}

// a *= d
void bu_mul_digit_ip(bigunsigned *a_ptr, uint32_t d) {
  bigunsigned tmp;
  bu_clear(&tmp);
  bu_cpy(&tmp, a_ptr);
  bu_mul_digit(a_ptr, &tmp, d);
}

void bu_mul(bigunsigned *a_ptr, bigunsigned *b_ptr, bigunsigned *c_ptr) {
  bigunsigned carry;
  bu_clear(a_ptr);
  for (uint16_t i = 0; i < b_ptr->used; ++i) {
    bu_mul_digit(&carry, c_ptr, b_ptr->digit[b_ptr->base + i]);
    uint32_t shift = ((uint32_t)i) << 5;
    bu_shl_ip(&carry, shift);
    bu_add_ip(a_ptr, &carry);
  }
}

void bu_mul_ip(bigunsigned *a_ptr, bigunsigned *b_ptr) {
  bigunsigned tmp;
  bu_clear(&tmp);
  bu_cpy(&tmp, a_ptr);
  bu_mul(a_ptr, &tmp, b_ptr);
}

// a = b^2
void bu_sqr(bigunsigned *a_ptr, bigunsigned *b_ptr) {
  bigunsigned tmp;
  bu_clear(&tmp);
  bu_cpy(&tmp, b_ptr);
  bu_mul(a_ptr, &tmp, b_ptr);
}

// a *= a
void bu_sqr_ip(bigunsigned *a_ptr) {
  bigunsigned tmp, tmp2;
  bu_clear(&tmp);
  bu_cpy(&tmp, a_ptr);
  bu_clear(&tmp2);
  bu_cpy(&tmp2, a_ptr);
  bu_mul(a_ptr, &tmp, &tmp2);
}

// return the length in bits (should always be less or equal to 32*a->used)
uint16_t bu_len(bigunsigned *a_ptr) {
  uint16_t res = a_ptr->used<<5;
  uint32_t bit_mask = 0x80000000;
  uint32_t last_wrd = a_ptr->digit[a_ptr->base+a_ptr->used-1];

  while (bit_mask && !(last_wrd&bit_mask)) {
    bit_mask >>= 1;
    res--;
  }
  return res;
}

// Read from a string of hex digits
// DONE

void bu_readhex(bigunsigned * a_ptr, char *s) {
  bu_clear(a_ptr);

  unsigned pos = 0;
  char *s_ptr = s;
  int read = strlen(s_ptr)-1;
  while ((read >= 0) && pos < BU_MAX_HEX) {
    if ((s[read]) != ' ') {
      a_ptr->digit[pos>>3] |= (((uint32_t)hex2bin(s[read])) << ((pos & 0x7)<<2));
      pos++;
    }
    read--;
  }
  a_ptr->used = (pos>>3) + ((pos&0x7)!=0);
}

//
void bu_dbg_printf(bigunsigned *a_ptr) {
  printf("Used %x\n", a_ptr->used);
  printf("Base %x\n", a_ptr->base);
  uint16_t i = a_ptr->used;
  printf("Digits: ");
  while (i-- > 0)
    printf("%8x ", a_ptr->digit[(a_ptr->base+i) % BU_DIGITS]);
  printf("Length: %x\n", bu_len(a_ptr));
}
