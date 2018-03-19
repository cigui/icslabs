/* 
 * CS:APP Data Lab 
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#include "btest.h"
#include <limits.h>

/*
 * Instructions to Students:
 *
 * STEP 1: Fill in the following struct with your identifying info.
 */
team_struct team =
{
   /* Team name: Replace with either:
      Your login ID if working as a one person team
      or, ID1+ID2 where ID1 is the login ID of the first team member
      and ID2 is the login ID of the second team member */
   "515030910073", 
   /* Student name 1: Replace with the full name of first team member */
   "Zhou Xin",
   /* Login ID 1: Replace with the login ID of first team member */
   "515030910073",

   /* The following should only be changed if there are two team members */
   /* Student name 2: Full name of the second team member */
   "",
   /* Login ID 2: Login ID of the second team member */
   ""
};

#if 0
/*
 * STEP 2: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.
#endif

/*
 * STEP 3: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the btest test harness to check that your solutions produce 
 *      the correct answers. Watch out for corner cases around Tmin and Tmax.
 */
/* Copyright (C) 1991-2014 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses ISO/IEC 10646 (2nd ed., published 2011-03-15) /
   Unicode 6.0.  */
/* We do not support C11 <threads.h>.  */
/* 
 * abs - absolute value of x (except returns TMin for TMin)
 *   Example: abs(-1) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 10
 *   Rating: 4
 */
int abs(int x) {
/* if x < 0, then the left term is 0, the right term is x's 
 * absolute value; if x > 0, then the left term is x, the right
 * term is x's absolute value.
 */
  return ((~(x>>31))&x) + ((x>>31)&(~0)&(~x+1));
}
/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
/* NOT(NOT a OR NOT b) = a AND b */ 
  int result = ~ (~x | ~y);
  return result;
}
/* 
 * bitMask - Generate a mask consisting of all 1's 
 *   lowbit and highbit
 *   Examples: bitMask(5,3) = 0x38
 *   Assume 0 <= lowbit <= 31, and 0 <= highbit <= 31
 *   If lowbit > highbit, then mask should be all 0's
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int bitMask(int highbit, int lowbit) {
/* firstly, get the length of the "1"s, then use a 
 * mask ~0 and left shift it to get some "0"s and then
 * "~" it to turn it into a number that has the 
 * required number of "0"s and "1"s, with all the "0"s
 * at the left. then left shift it to make it become
 * the result.
 */
  int msk = ~0;
  int len = highbit + (~lowbit) + 1;
  int result = msk << len;
  result <<= 1;
  result = ~result;
  result <<= lowbit;
  result &= ~(!!(~(len>>31)))+1;
  return result;
}
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 2
 */
int bitXor(int x, int y) {
/* a Xor b = (NOT a AND b) OR (NOT b AND a)
 * a OR b = NOT (a AND b)
 */
  int i = ~x & y;
  int j = ~y & x;
  int result = ~ (~i & ~j);
  return result;
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
/* note that ~0 + 1 = 0, ~1 + 1 = ~0, so just use them as the
 * mask to choose the number we need.
 */
  return ((~(!!x)+1)&y)+((~(!x)+1)&z);
}
/* 
 * evenBits - return word with all even-numbered bits set to 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 2
 */
int evenBits(void) {
/* use a mask(0x55), shift it and use OR to add it into the 
 * result.
 */
  int start = 0x55;
  int tmp = start << 8;
  int result = start | tmp;
  tmp <<= 8;
  result |= tmp;
  tmp <<= 8;
  result |= tmp;
  return result;
}
/* 
 * isEqual - return 1 if x == y, and 0 otherwise 
 *   Examples: isEqual(5,5) = 1, isEqual(4,5) = 0
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int isEqual(int x, int y) {
/* if x = y, then ~x^y should be a binary string
 * with all bits set to 1, so ~(~x^y) should be 0
 * and !0 = 1.
 */
  return !(~(~x^y));
}
/* 
 * isLess - if x < y  then return 1, else return 0 
 *   Example: isLess(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLess(int x, int y) {
/* in this function, x - y = x + ny. firstly judge if x + ny
 * is negative, and use "rst" to store the result. then, i 
 * use "ofl" to judge if it overflows. if it overflows, then
 * "ofl" should be 1, and the result should be different from
 * the one that has been stored. but when debugging i found 
 * that this solution is wrong everytime when y = 1 << 31,
 * so i just use "flg" to judge if y = 1 << 31, and if so,
 * i simply change the result again. by the way, i think this
 * one is quite hard and should be a 4-rating one.
 */
  int ny = ~y + 1;
  int bal = x + ny;
  int ofl = (!(((((x & ny) ^ bal) >> 31)) + 1)) & (!((x ^ ny)>> 31));// return 1 if overflow
  int rst = !!(bal >> 31);
  int flg = !(~(~y^(1<<31)));
  rst = rst ^ ofl;
  rst = rst ^ flg;
  return rst;
}
/* 
 * isNegative - return 1 if x < 0, return 0 otherwise 
 *   Example: isNegative(-1) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 3
 */
int isNegative(int x) {
/* exploit right shift (arithmetically) to judge.*/
  return !!(x>>31);
}
/* 
 * isNonZero - Check whether x is nonzero using
 *              the legal operators except !
 *   Examples: isNonZero(3) = 1, isNonZero(0) = 0
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 10
 *   Rating: 4 
 */
int isNonZero(int x) {
/* get an opposite number of x, if x is 0 then nx|x=0;
 * if x isn't 0 then nx|x=-1, so just return nx|x+1.
 */
  int nx = ~x + 1;
  int result = 0;
  x>>=31;
  nx>>=31;
  result = ~(x | nx)+1;
  return result;
}
/*
 * isPower2 - returns 1 if x is a power of 2, and 0 otherwise
 *   Examples: isPower2(5) = 0, isPower2(8) = 1, isPower2(0) = 0
 *   Note that no negative number is a power of 2.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 60
 *   Rating: 4
 */
int isPower2(int x) {
/* "msk" is a mask of x produced by "leastBitPos". and if x is a 
 * power of 2, x should be the same as "msk". 0 and negative 
 * number are not power of 2, so we should check if x is a 
 * positive number.
 */
  int msk = (~x + 1) & x;
  int rst = (~(!((x + ~1 + 1) >> 31)) + 1) & (!(msk ^ x));
  rst &= ((~(!(x >> 31))) + 1);
  return rst;
}
/* 
 * leastBitPos - return a mask that marks the position of the
 *               least significant 1 bit. If x == 0, return 0
 *   Example: leastBitPos(96) = 0x20
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 4 
 */
int leastBitPos(int x) {
/* r is an opposite number of x, and starting from the least
 * significant bit(if it is 0), r is the same as x by every 
 * bit, until the bit becomes 1 for the first time.
 */
  int r = ~x + 1;
  return r & x;
}
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
/* the exactly same strategy as is used in "isNonZero". */
  int nx = ~x + 1;
  x >>= 31;
  nx >>= 31;
  return (x | nx) + 1;
}
/* 
 * reverseBytes - reverse the bytes of x
 *   Example: reverseBytes(0x01020304) = 0x04030201
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 25
 *   Rating: 3
 */
int reverseBytes(int x) {
/* use "tmp" and shift it to make sure the needed byte
 * is in the least significant position, and use a 
 * mask(0xff) to save the needed byte, then shift it 
 * again to make that byte in the new position and add
 * it into result.
 */
  int result = 0;
  int tmp = x;
  int msk = 0xff;
  tmp <<= 24;
  result |= tmp;
  tmp = x;
  tmp >>= 8;
  tmp &= msk;
  tmp <<= 16;
  result |= tmp;
  tmp = x;
  tmp >>= 16;
  tmp &= msk;
  tmp <<= 8;
  result |= tmp;
  tmp = x;
  tmp >>= 24;
  tmp &= msk;
  result |= tmp;
  return result;
}
/* 
 * sum3 - x+y+z using only a single '+'
 *   Example: sum3(3, 4, 5) = 12
 *   Legal ops: ! ~ & ^ | << >>
 *   Max ops: 16
 *   Rating: 3
 */
/* A helper routine to perform the addition.  Don't change this code */
static int sum(int x, int y) {
  return x+y;
}
int sum3(int x, int y, int z) {
/* use word1 to record the result of the sum(without carrying), 
 * and use word2 to record the result of carrying in the sum.
 */
  int word1 = 0;
  int word2 = 0;
  /**************************************************************
     Fill in code below that computes values for word1 and word2
     without using any '+' operations 
  ***************************************************************/
  word1 = x ^ y ^ z;
  word2 = (x & y) | (y & z) | (x & z);
  word2 <<= 1;
  /**************************************************************
     Don't change anything below here
  ***************************************************************/
  return sum(word1,word2);
}
