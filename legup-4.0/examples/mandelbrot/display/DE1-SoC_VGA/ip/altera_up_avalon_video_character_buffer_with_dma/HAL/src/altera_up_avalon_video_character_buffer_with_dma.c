/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2006 Altera Corporation, San Jose, California, USA.           *
* All rights reserved.                                                        *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
* This agreement shall be governed in all respects by the laws of the State   *
* of California and by the laws of the United States of America.              *
*                                                                             *
******************************************************************************/

#include <errno.h>
#include <string.h>

#include <priv/alt_file.h>

#include "altera_up_avalon_video_character_buffer_with_dma.h"
#include "altera_up_avalon_video_character_buffer_with_dma_regs.h"

void alt_up_char_buffer_init(alt_up_char_buffer_dev *char_buffer) {
	char * name;
	name = (char *) char_buffer->dev.name;

	for ( ; (*name) != '\0'; name++) {
		if (strcmp(name, "_avalon_char_buffer_slave") == 0) {
			(*name) = '\0';
			break;
		}
	}
	
	return;
}

alt_up_char_buffer_dev* alt_up_char_buffer_open_dev(const char* name) {
  // find the device from the device list 
  // (see altera_hal/HAL/inc/priv/alt_file.h 
  // and altera_hal/HAL/src/alt_find_dev.c 
  // for details)
  alt_up_char_buffer_dev *dev = (alt_up_char_buffer_dev *)alt_find_dev(name, &alt_dev_list);

  return dev;
}

int alt_up_char_buffer_draw(alt_up_char_buffer_dev *char_buffer, unsigned char ch, 
	unsigned int x, unsigned int y) {
	// boundary check
	if (x >= char_buffer->x_resolution || y >= char_buffer->y_resolution )
		return -1;
	
	unsigned int addr = 0;
	addr |= ((x & char_buffer->x_coord_mask) << char_buffer->x_coord_offset);
	addr |= ((y & char_buffer->y_coord_mask) << char_buffer->y_coord_offset);
	IOWR_8DIRECT(char_buffer->buffer_base, addr, ch);

	return 0;
}

int alt_up_char_buffer_string(alt_up_char_buffer_dev *char_buffer, const char *ptr, 
	unsigned int x, unsigned int y) {
	// boundary check
	if (x >= char_buffer->x_resolution || y >= char_buffer->y_resolution )
		return -1;
	
	unsigned int offset = 0;
	offset = (y << char_buffer->y_coord_offset) + x;

	while ( *ptr )
	{
		IOWR_8DIRECT(char_buffer->buffer_base, offset, *ptr);
		++ptr;
		if (++x >= char_buffer->x_resolution)
			return -1;
		++offset;
	}
	return 0;
}

int alt_up_char_buffer_clear(alt_up_char_buffer_dev *char_buffer) {
	IOWR_ALT_UP_CHAR_BUFFER_CLR_SCRN(char_buffer->ctrl_reg_base, 1);
	while ((IORD_ALT_UP_CHAR_BUFFER_CLR_SCRN(char_buffer->ctrl_reg_base) & ALT_UP_CHAR_BUFFER_CLR_SCRN_MSK) >> ALT_UP_CHAR_BUFFER_CLR_SCRN_OFST);
	return 0;
}

