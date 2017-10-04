#ifndef __ALTERA_UP_AVALON_VIDEO_CHARACTER_BUFFER_WITH_DMA_H__
#define __ALTERA_UP_AVALON_VIDEO_CHARACTER_BUFFER_WITH_DMA_H__

#include <stddef.h>
#include <alt_types.h>
#include <sys/alt_dev.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*
 * Device structure definition. Each instance of the driver uses one
 * of these structures to hold its associated state.
 */
typedef struct alt_up_char_buffer_dev {
	/// @brief character mode device structure 
	/// @sa Developing Device Drivers for the HAL in Nios II Software Developer's Handbook
	alt_dev dev;
	/// @brief the control register's slave base address
	unsigned int ctrl_reg_base;
	/// @brief the character buffer's slave base address
	unsigned int buffer_base;
	/// @brief the character resolution in x direction 
	alt_u32 x_resolution;
	/// @brief the character resolution in y direction 
	alt_u32 y_resolution;
	/// @brief the x/y coordinate offset/masks
	alt_u32 x_coord_offset;
	alt_u32 x_coord_mask;
	alt_u32 y_coord_offset;
	alt_u32 y_coord_mask;
} alt_up_char_buffer_dev;

///////////////////////////////////////////////////////////////////////////////
// HAL system functions
/**
 * @brief Initialize the name of the alt_up_char_buffer_dev structure
 *
 * @param char_buffer -- struct for the character buffer device 
 *
 **/
void alt_up_char_buffer_init(alt_up_char_buffer_dev *char_buffer);


///////////////////////////////////////////////////////////////////////////////
// file-like operation functions

///////////////////////////////////////////////////////////////////////////////
// direct operation functions
/**
 * @brief Opens the character buffer device specified by <em> name </em>
 *
 * @param name -- the character buffer component name in SOPC Builder. 
 *
 * @return The corresponding device structure, or NULL if the device is not found
 **/
alt_up_char_buffer_dev* alt_up_char_buffer_open_dev(const char* name);

/**
 * @brief Draw a character at the location specified by <em>(x, y)</em> on the
 * VGA monitor with white color and transparent background
 *
 * @param ch -- the character to draw
 * @param x	-- the \em x coordinate
 * @param y	-- the \em y coordinate
 *
 * @return 0 for success, -1 for error (such as out of bounds)
 **/
int alt_up_char_buffer_draw(alt_up_char_buffer_dev *char_buffer, unsigned char ch, 
	unsigned int x, unsigned int y);

/**
 * @brief Draw a NULL-terminated text string at the location specified by <em>(x, y)</em>
 *
 * @param ch -- the character to draw
 * @param x	-- the \em x coordinate
 * @param y	-- the \em y coordinate
 *
 * @return 0 for success, -1 for error (such as out of bounds)
 **/
int alt_up_char_buffer_string(alt_up_char_buffer_dev *char_buffer, const char *ptr, 
	unsigned int x, unsigned int y);

/**
 * @brief Clears the character buffer's memory
 *
 * @return 0 for success
 **/
int alt_up_char_buffer_clear(alt_up_char_buffer_dev *char_buffer);

///////////////////////////////////////////////////////////////////////////////
// Macros used by alt_sys_init 
#define ALTERA_UP_AVALON_VIDEO_CHARACTER_BUFFER_WITH_DMA_INSTANCE(name, device)\
static alt_up_char_buffer_dev device =							\
{															 	\
	{															\
		ALT_LLIST_ENTRY,										\
		name##_AVALON_CHAR_BUFFER_SLAVE_NAME,					\
		NULL, /* open  */										\
		NULL, /* close */										\
		NULL, /* read  */										\
		NULL, /* write */										\
		NULL, /* lseek */										\
		NULL, /* fstat */										\
		NULL, /* ioctl */										\
	},															\
	name##_AVALON_CHAR_CONTROL_SLAVE_BASE,						\
	name##_AVALON_CHAR_BUFFER_SLAVE_BASE,						\
	80,		/* Default X Resolution */							\
	60,		/* Default Y Resolution */							\
	0,		/* Default X Offset */								\
	0x007F,	/* Default X Mask */ 								\
	7,		/* Default Y Offset */								\
	0x003F	/* Default Y Mask */ 								\
}

#define ALTERA_UP_AVALON_VIDEO_CHARACTER_BUFFER_WITH_DMA_INIT(name, device)	\
{																\
	device.x_resolution = 										\
		(*((int *)(device.ctrl_reg_base) + 1) & 0xFFFF);		\
	device.y_resolution = 										\
		((*((int *)(device.ctrl_reg_base) + 1) >> 16) & 0xFFFF);\
																\
	if (device.x_resolution <= 0x40) {							\
		device.x_coord_mask = 0x003F;							\
		device.y_coord_offset = 6;								\
	}															\
																\
	if (device.y_resolution <= 0x20) {							\
		device.y_coord_mask = 0x001F;							\
	}															\
																\
	alt_up_char_buffer_init(&device);							\
	/* make the device available to the system */				\
	alt_dev_reg(&device.dev);									\
}



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ALTERA_UP_AVALON_VIDEO_CHARACTER_BUFFER_WITH_DMA_H__ */


