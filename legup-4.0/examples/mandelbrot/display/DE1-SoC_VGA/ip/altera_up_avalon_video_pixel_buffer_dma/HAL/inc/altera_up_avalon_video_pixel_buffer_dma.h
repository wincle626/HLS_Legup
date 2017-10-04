#ifndef __ALTERA_UP_AVALON_VIDEO_PIXEL_BUFFER_DMA_H__
#define __ALTERA_UP_AVALON_VIDEO_PIXEL_BUFFER_DMA_H__

#include <stddef.h>
#include <alt_types.h>
#include <sys/alt_dev.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define ALT_UP_PIXEL_BUFFER_CONSECUTIVE_ADDRESS_MODE 1
#define ALT_UP_PIXEL_BUFFER_XY_ADDRESS_MODE 0

#define ALT_UP_8BIT_COLOR_MODE	1
#define ALT_UP_16BIT_COLOR_MODE 2
#define ALT_UP_24BIT_COLOR_MODE 3
#define ALT_UP_30BIT_COLOR_MODE 4

/*
 * Device structure definition. Each instance of the driver uses one
 * of these structures to hold its associated state.
 */
typedef struct alt_up_pixel_buffer_dma_dev {
	/// @brief character mode device structure 
	/// @sa Developing Device Drivers for the HAL in Nios II Software Developer's Handbook
	alt_dev dev;
	/// @brief the pixel buffer's slave base address
	unsigned int base;
	/// @brief the memory buffer's start address
	unsigned int buffer_start_address;
	/// @brief the memory back buffer's start address
	unsigned int back_buffer_start_address;
	/// @brief the addressing mode 
	unsigned int addressing_mode;
	/// @brief the color mode 
	unsigned int color_mode;
	/// @brief the resolution in x direction 
	unsigned int x_resolution;
	/// @brief the resolution in y direction 
	unsigned int y_resolution;
	/// @brief the x/y coordinate offset/masks
	unsigned int x_coord_offset;
	unsigned int x_coord_mask;
	unsigned int y_coord_offset;
	unsigned int y_coord_mask;
} alt_up_pixel_buffer_dma_dev;

///////////////////////////////////////////////////////////////////////////////
// HAL system functions

///////////////////////////////////////////////////////////////////////////////
// file-like operation functions

///////////////////////////////////////////////////////////////////////////////
// direct operation functions
/**
 * @brief Opens the pixel buffer device specified by <em> name </em>
 *
 * @param name -- the pixel buffer component name in SOPC Builder. 
 *
 * @return The corresponding device structure, or NULL if the device is not found
 **/
alt_up_pixel_buffer_dma_dev* alt_up_pixel_buffer_dma_open_dev(const char* name);

/**
 * @brief Draw a pixel at the location specified by <em>(x, y)</em> on the VGA monitor
 *
 * @param pixel_buffer -- the pointer to the VGA structure
 * @param color	-- the RGB color to be drawn
 * @param x -- the \em x coordinate
 * @param y -- the \em y coordinate
 *
 * @return 0 for success, -1 for error (such as out of bounds)
 **/
int alt_up_pixel_buffer_dma_draw(alt_up_pixel_buffer_dma_dev *pixel_buffer, unsigned int color, unsigned int x, unsigned int y);

/**
 * @brief Changes the back buffer's start address
 *
 * @param pixel_buffer -- the pointer to the VGA structure
 * @param new_address  -- the new start address of the back buffer
 *
 * @return 0 for success
 **/
int alt_up_pixel_buffer_dma_change_back_buffer_address(alt_up_pixel_buffer_dma_dev *pixel_buffer, unsigned int new_address);

/**
 * @brief Swaps which buffer is being sent to the VGA Controller
 *
 * @param pixel_buffer -- the pointer to the VGA structure
 *
 * @return 0 for success
 **/
int alt_up_pixel_buffer_dma_swap_buffers(alt_up_pixel_buffer_dma_dev *pixel_buffer);

/**
 * @brief Check if swapping buffers has completed
 *
 * @param pixel_buffer -- the pointer to the VGA structure
 *
 * @return 0 if complete, 1 if still processing 
 **/
int alt_up_pixel_buffer_dma_check_swap_buffers_status(alt_up_pixel_buffer_dma_dev *pixel_buffer);

/**
 * @brief This function clears the screen or the back buffer.
 *
 * @param pixel_buffer -- the pointer to the VGA structure
 * @param backbuffer -- set to 1 to clear the back buffer, otherwise set to 0 to clear the current screen.
 *
 * @return 0 if complete, 1 if still processing 
 **/
void alt_up_pixel_buffer_dma_clear_screen(alt_up_pixel_buffer_dma_dev *pixel_buffer, int backbuffer);

/**
 * @brief This function draws a box of a given color between points (x0,y0) and (x1,y1).
 *
 * @param pixel_buffer -- the pointer to the VGA structure
 * @param x0,x1,y0,y1 -- coordinates of the top left (x0,y0) and bottom right (x1,y1) corner of the box
 * @param color -- color of the box to be drawn
 * @param backbuffer -- set to 1 to select the back buffer, otherwise set to 0 to select the current screen.
 *
 * @return 0 if complete, 1 if still processing 
 **/
void alt_up_pixel_buffer_dma_draw_box(alt_up_pixel_buffer_dma_dev *pixel_buffer, int x0, int y0, int x1, int y1, int color, int backbuffer);

/**
 * @brief This function draws a horizontal line of a given color between points (x0,y) and (x1,y).
 *
 * @param pixel_buffer -- the pointer to the VGA structure
 * @param x0,x1,y -- coordinates of the left (x0,y) and the right (x1,y) end-points of the line
 * @param color -- color of the line to be drawn
 * @param backbuffer -- set to 1 to select the back buffer, otherwise set to 0 to select the current screen.
 *
 * @return 0 if complete, 1 if still processing 
 **/
void alt_up_pixel_buffer_dma_draw_hline(alt_up_pixel_buffer_dma_dev *pixel_buffer, int x0, int x1, int y, int color, int backbuffer);

/**
 * @brief This function draws a vertical line of a given color between points (x,y0) and (x,y1).
 *
 * @param pixel_buffer -- the pointer to the VGA structure
 * @param x,y0,y1 -- coordinates of the top (x,y0) and the bottom (x,y1) end-points of the line
 * @param color -- color of the line to be drawn
 * @param backbuffer -- set to 1 to select the back buffer, otherwise set to 0 to select the current screen.
 *
 * @return 0 if complete, 1 if still processing 
 **/
void alt_up_pixel_buffer_dma_draw_vline(alt_up_pixel_buffer_dma_dev *pixel_buffer, int x, int y0, int y1, int color, int backbuffer);

/**
 * @brief This function draws a rectangle of a given color between points (x0,y0) and (x1,y1).
 *
 * @param pixel_buffer -- the pointer to the VGA structure
 * @param x0,x1,y0,y1 -- coordinates of the top left (x0,y0) and bottom right (x1,y1) corner of the rectangle
 * @param color -- color of the rectangle to be drawn
 * @param backbuffer -- set to 1 to select the back buffer, otherwise set to 0 to select the current screen.
 *
 * @return 0 if complete, 1 if still processing 
 **/
void alt_up_pixel_buffer_dma_draw_rectangle(alt_up_pixel_buffer_dma_dev *pixel_buffer, int x0, int y0, int x1, int y1, int color, int backbuffer);

/**
 * @brief This function draws a line of a given color between points (x0,y0) and (x1,y1).
 *
 * @param pixel_buffer -- the pointer to the VGA structure
 * @param x0,x1,y0,y1 -- coordinates (x0,y0) and (x1,y1) correspond to end points of the line
 * @param color -- color of the line to be drawn
 * @param backbuffer -- set to 1 to select the back buffer, otherwise set to 0 to select the current screen.
 *
 * @return 0 if complete, 1 if still processing 
 **/
void alt_up_pixel_buffer_dma_draw_line(alt_up_pixel_buffer_dma_dev *pixel_buffer, int x0, int y0, int x1, int y1, int color, int backbuffer);

///////////////////////////////////////////////////////////////////////////////
// Macros used by alt_sys_init 
#define ALTERA_UP_AVALON_VIDEO_PIXEL_BUFFER_DMA_INSTANCE(name, device)	\
static alt_up_pixel_buffer_dma_dev device =							\
{															 	\
	{															\
		ALT_LLIST_ENTRY,										\
		name##_NAME,											\
		NULL, /* open  */										\
		NULL, /* close */										\
		NULL, /* read  */										\
		NULL, /* write */										\
		NULL, /* lseek */										\
		NULL, /* fstat */										\
		NULL, /* ioctl */										\
	},															\
	name##_BASE,												\
	0,		/* Default Buffer Starting Address		*/			\
	0,		/* Default Back Buf Starting Address	*/			\
	0,		/* Default to XY Addressing Mode		*/			\
	2,		/* Default Color Mode  		 			*/			\
	320,	/* Default X Resolution					*/			\
	240,	/* Default Y Resolution					*/			\
	1,		/* Default X Offset   		 			*/			\
	0x01FF,	/* Default X Mask      					*/			\
	10,		/* Default Y Offset    					*/			\
	0x00FF,	/* Default Y Mask      					*/			\
}

#define ALTERA_UP_AVALON_VIDEO_PIXEL_BUFFER_DMA_INIT(name, device)		\
{																\
	device.buffer_start_address = 								\
		(*((int *)(device.base)) & 0xFFFFFFFF);					\
	device.back_buffer_start_address = 							\
		(*((int *)(device.base) + 1) & 0xFFFFFFFF);				\
	device.x_resolution = 										\
		(*((int *)(device.base) + 2) & 0xFFFF);					\
	device.y_resolution = 										\
		((*((int *)(device.base) + 2) >> 16) & 0xFFFF);			\
	device.addressing_mode = 									\
		((*((int *)(device.base) + 3) >> 1) & 0x1);				\
	device.color_mode = 										\
		((*((int *)(device.base) + 3) >> 4) & 0xF);				\
																\
	alt_u8 alt_up_wiw = 										\
		((*((int *)(device.base) + 3) >> 16) & 0xFF);			\
	alt_u8 alt_up_hiw = 										\
		((*((int *)(device.base) + 3) >> 24) & 0xFF);			\
																\
																\
	if (device.color_mode == ALT_UP_8BIT_COLOR_MODE) {			\
		device.x_coord_offset = 0;								\
	} else if (device.color_mode == ALT_UP_16BIT_COLOR_MODE) {	\
		device.x_coord_offset = 1;								\
	} else {													\
		device.x_coord_offset = 2;								\
	}															\
	device.x_coord_mask = 0xFFFFFFFF >> (32 - alt_up_wiw);		\
	device.y_coord_offset = alt_up_wiw + device.x_coord_offset;	\
	device.y_coord_mask = 0xFFFFFFFF >> (32 - alt_up_hiw);		\
																\
	/* make the device available to the system */				\
	alt_dev_reg(&device.dev);									\
}

#define alt_up_pixel_buffer_dma_x_res(dev_struct_ptr)	(dev_struct_ptr->x_resolution)
#define alt_up_pixel_buffer_dma_y_res(dev_struct_ptr)	(dev_struct_ptr->y_resolution)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ALTERA_UP_AVALON_VIDEO_PIXEL_BUFFER_DMA_H__ */


