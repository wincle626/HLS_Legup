#ifndef __REGDEF_H
#define __REGDEF_H

#define zero    $0      /* wired zero */
#define at      $1      /* assembler temp  - uppercase because of ".set at" */
#define v0      $2      /* return value */
#define v1      $3
#define a0      $4      /* argument registers */
#define a1      $5
#define a2      $6
#define a3      $7
#define t0      $8      /* caller saved */
#define t1      $9
#define t2      $10
#define t3      $11
#define t4      $12
#define t5      $13
#define t6      $14
#define t7      $15
#define s0      $16     /* callee saved */
#define s1      $17
#define s2      $18
#define s3      $19
#define s4      $20
#define s5      $21
#define s6      $22
#define s7      $23
#define t8      $24     /* caller saved */
#define t9      $25
#define k0      $26     /* kernel scratch */
#define k1      $27
#define gp      $28     /* global pointer */
#define sp      $29     /* stack pointer */
#define fp      $30     /* frame pointer */
#define s8	    $30		/* same like fp! */
#define ra      $31     /* return address */

#define cause   $0
#define status  $1
#define epc     $2
#define cachectrl $3

// Orig Stack: 0x01000000
// Assume max size of profiler data is 256 Functions * 32bytes: 0x01000000-(32*256) = 0x00FFE000	- i think this should be 32BITS = 4 bytes
// Allow for 4 bytes of communication: 0x00FFE000 - (4 bytes) = 0x00FFDFFC
// ** KEEP CONSISTENT WITH legup/tiger/processor/tiger_mips/prof_defines.v **
#define STACK_ADDR 0x00FFDFFC
#define PROF_ADDR  0x00FFE000

#endif
