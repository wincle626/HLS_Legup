#ifndef	VGA_H
#define	VGA_H

/*
 *  Copyright (C) 2005-2010  Anders Gavare.  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright  
 *     notice, this list of conditions and the following disclaimer in the 
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE   
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 *
 *
 *  VGA register definitions, used by src/devices/dev_vga.c.
 */

/*
 *  Registers (offset from 0x3C0):
 */

#define	VGA_ATTRIBUTE_ADDR		0	/*  R/W  */
#define	VGA_ATTRIBUTE_DATA_WRITE	0	/*  W  */

#define	VGA_ATTRIBUTE_DATA_READ		1	/*  R  */

#define	VGA_INPUT_STATUS_0		2	/*  R  */

#define	VGA_MISC_OUTPUT_W		2	/*  W  */
#define	   VGA_MISC_OUTPUT_CS360	   0x04
#define	   VGA_MISC_OUTPUT_REN		   0x02
#define	   VGA_MISC_OUTPUT_IOAS		   0x01

#define	VGA_SEQUENCER_ADDR		4	/*  R/W  */
#define	VGA_SEQUENCER_DATA		5	/*  R/W  */
#define	   VGA_SEQ_RESET		   0
#define	   VGA_SEQ_CLOCKING_MODE	   1
#define	   VGA_SEQ_MAP_MASK		   2
#define	   VGA_SEQ_CHARACTER_MAP_SELECT	   3
#define	   VGA_SEQ_SEQUENCER_MEMORY_MODE   4

#define	VGA_DAC_STATE			7	/*  R  */
#define	VGA_DAC_ADDR_READ		7	/*  W  */
#define	VGA_DAC_ADDR_WRITE		8	/*  W  */
#define	VGA_DAC_DATA			9	/*  R/W  */

#define	VGA_FEATURE_CONTROL		0xA	/*  R/W?  */

#define	VGA_MISC_OUTPUT_R		0xC	/*  R  */

#define	VGA_GRAPHCONTR_ADDR		0xE	/*  R/W  */
#define	VGA_GRAPHCONTR_DATA		0xF	/*  R/W  */
#define	   VGA_GRAPHCONTR_SETRESET	   0
#define	   VGA_GRAPHCONTR_ENABLE	   1
#define	   VGA_GRAPHCONTR_COLORCMP	   2
#define	   VGA_GRAPHCONTR_DATAROTATE	   3
#define	   VGA_GRAPHCONTR_READMAPSELECT	   4
#define	   VGA_GRAPHCONTR_GRAPHICSMODE	   5
#define	   VGA_GRAPHCONTR_MISC		   6
#define	   VGA_GRAPHCONTR_COLORDONTCARE	   7
#define	   VGA_GRAPHCONTR_MASK		   8

#define	VGA_CRTC_ADDR			0x14	/*  R/W  */
#define	VGA_CRTC_DATA			0x15	/*  R/W  */
#define	   VGA_CRTC_CURSOR_SCANLINE_START  0x0a
#define	   VGA_CRTC_CURSOR_SCANLINE_END    0x0b
#define	   VGA_CRTC_START_ADDR_HIGH	   0x0c
#define	   VGA_CRTC_START_ADDR_LOW	   0x0d
#define	   VGA_CRTC_CURSOR_LOCATION_HIGH   0x0e
#define	   VGA_CRTC_CURSOR_LOCATION_LOW    0x0f

#define	VGA_INPUT_STATUS_1		0x1A	/*  R  */
#define	   VGA_IS1_DISPLAY_VRETRACE	   0x08
#define	   VGA_IS1_DISPLAY_DISPLAY_DISABLE 0x01

#endif	/*  VGA_H  */
