#ifndef __IO_H__
#define __IO_H__

#define IO_RD_WORD(BASE, REG) (*(volatile unsigned int*)(BASE + REG * 4))
#define IO_WR_WORD(BASE, REG, X) (*(volatile unsigned int*)(BASE + REG * 4) = X)

#endif
