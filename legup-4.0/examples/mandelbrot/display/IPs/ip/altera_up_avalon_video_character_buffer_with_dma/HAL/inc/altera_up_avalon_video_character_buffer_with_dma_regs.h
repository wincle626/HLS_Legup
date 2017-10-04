/******************************************************************************
*																			  *
* License Agreement															  *
*																			  *
* Copyright (c) 2003 Altera Corporation, San Jose, California, USA.			  *
* All rights reserved.														  *
*																			  *
* Permission is hereby granted, free of charge, to any person obtaining a	  *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation	  *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,	  *
* and/or sell copies of the Software, and to permit persons to whom the		  *
* Software is furnished to do so, subject to the following conditions:		  *
*																			  *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.						  *
*																			  *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  * 
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,	  *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER	  *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING	  *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER		  *
* DEALINGS IN THE SOFTWARE.													  *
*																			  *
* This agreement shall be governed in all respects by the laws of the State	  *
* of California and by the laws of the United States of America.		 	  *
*																			  *
******************************************************************************/

#ifndef __ALTERA_UP_AVALON_VIDEO_CHARACTER_BUFFER_WITH_DMA_REGS_H__
#define __ALTERA_UP_AVALON_VIDEO_CHARACTER_BUFFER_WITH_DMA_REGS_H__

#include <io.h>

/*
 * Control Register 
 */
#define ALT_UP_CHAR_BUFFER_CTRL_REG						0

#define IOADDR_ALT_UP_CHAR_BUFFER_CTRL_REG(base)		\
		__IO_CALC_ADDRESS_NATIVE(base, ALT_UP_CHAR_BUFFER_CTRL_REG)
#define IORD_ALT_UP_CHAR_BUFFER_CTRL_REG(base)			\
		IORD(base, ALT_UP_CHAR_BUFFER_CTRL_REG)
#define IOWR_ALT_UP_CHAR_BUFFER_CTRL_REG(base, data)	\
		IOWR(base, ALT_UP_CHAR_BUFFER_CTRL_REG, data)

#define ALT_UP_CHAR_BUFFER_CTRL_REG_CLR_SCRN_MSK		(0x00010000)
#define ALT_UP_CHAR_BUFFER_CTRL_REG_CLR_SCRN_OFST		(16)


#define ALT_UP_CHAR_BUFFER_CLR_SCRN						2

#define IORD_ALT_UP_CHAR_BUFFER_CLR_SCRN(base)			\
		IORD_8DIRECT(base, ALT_UP_CHAR_BUFFER_CLR_SCRN)
#define IOWR_ALT_UP_CHAR_BUFFER_CLR_SCRN(base, data)	\
		IOWR_8DIRECT(base, ALT_UP_CHAR_BUFFER_CLR_SCRN, data)

#define ALT_UP_CHAR_BUFFER_CLR_SCRN_MSK					(0x00000001)
#define ALT_UP_CHAR_BUFFER_CLR_SCRN_OFST				(0)

#endif /* __ALTERA_UP_AVALON_VIDEO_CHARACTER_BUFFER_WITH_DMA_REGS_H__ */
