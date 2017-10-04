#ifndef	DEVICES_H
#define	DEVICES_H

/*
 *  Copyright (C) 2003-2010  Anders Gavare.  All rights reserved.
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
 *  _LEGACY_ memory mapped devices.
 *
 *  NOTE:  Many of these devices are legacy devices, that are here for one
 *         of these two reasons:
 *
 *		A)  Devices introduced before the DEVINIT system had to
 *		    be declared somewhere.
 *
 *		B)  The way interrupt controllers and such were implemented
 *		    up until release 0.4.3 requires that several parts of
 *		    the program access internal fields of interrupt
 *		    controllers' structs.
 *
 *         Both A and B need to be solved.
 */

#include <sys/types.h>
#include <inttypes.h>

#include "interrupt.h"

struct cpu;
struct machine;
struct memory;
struct pci_data;
struct timer;

/* #ifdef WITH_X11
#include <X11/Xlib.h>
#endif */

/*  dev_8259.c:  */
struct pic8259_data {
	struct interrupt irq;

	int		irq_base;
	int		current_command;

	int		init_state;

	int		priority_reg;
	uint8_t		irr;		/*  interrupt request register  */
	uint8_t		isr;		/*  interrupt in-service register  */
	uint8_t		ier;		/*  interrupt enable register  */
};

/*  dev_dec_ioasic.c:  */
#define	DEV_DEC_IOASIC_LENGTH		0x80100
#define	N_DEC_IOASIC_REGS	(0x1f0 / 0x10)
#define	MAX_IOASIC_DMA_FUNCTIONS	8
struct dec_ioasic_data {
	uint32_t	reg[N_DEC_IOASIC_REGS];
	int		(*(dma_func[MAX_IOASIC_DMA_FUNCTIONS]))(struct cpu *, void *, uint64_t addr, size_t dma_len, int tx);
	void		*dma_func_extra[MAX_IOASIC_DMA_FUNCTIONS];
	int		rackmount_flag;
};
int dev_dec_ioasic_access(struct cpu *cpu, struct memory *mem, uint64_t relative_addr, unsigned char *data, size_t len, int writeflag, void *);
struct dec_ioasic_data *dev_dec_ioasic_init(struct cpu *cpu, struct memory *mem, uint64_t baseaddr, int rackmount_flag);

/*  dev_asc.c:  */
#define	DEV_ASC_DEC_LENGTH		0x40000
#define	DEV_ASC_PICA_LENGTH		0x1000
#define	DEV_ASC_DEC		1
#define	DEV_ASC_PICA		2
int dev_asc_access(struct cpu *cpu, struct memory *mem, uint64_t relative_addr, unsigned char *data, size_t len, int writeflag, void *);
void dev_asc_init(struct machine *machine, struct memory *mem, uint64_t baseaddr,
	const char *irq_path, void *turbochannel, int mode,
	size_t (*dma_controller)(void *dma_controller_data,
		unsigned char *data, size_t len, int writeflag),
	void *dma_controller_data);

/*  dev_bt431.c:  */
#define	DEV_BT431_LENGTH		0x20
#define	DEV_BT431_NREGS			0x800	/*  ?  */
int dev_bt431_access(struct cpu *cpu, struct memory *mem,
	uint64_t relative_addr, unsigned char *data, size_t len,
	int writeflag, void *);
struct vfb_data;
void dev_bt431_init(struct memory *mem, uint64_t baseaddr,
	struct vfb_data *vfb_data, int color_fb_flag);

/*  dev_bt455.c:  */
#define	DEV_BT455_LENGTH		0x20
int dev_bt455_access(struct cpu *cpu, struct memory *mem,
	uint64_t relative_addr, unsigned char *data, size_t len,
	int writeflag, void *);
struct vfb_data;
void dev_bt455_init(struct memory *mem, uint64_t baseaddr,
	struct vfb_data *vfb_data);

/*  dev_bt459.c:  */
#define	DEV_BT459_LENGTH		0x20
#define	DEV_BT459_NREGS			0x1000
#define	BT459_PX		1	/*  px[g]  */
#define	BT459_BA		2	/*  cfb  */
#define	BT459_BBA		3	/*  sfb  */
int dev_bt459_access(struct cpu *cpu, struct memory *mem,
	uint64_t relative_addr, unsigned char *data, size_t len,
	int writeflag, void *);
struct vfb_data;
void dev_bt459_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, uint64_t baseaddr_irq, struct vfb_data *vfb_data,
	int color_fb_flag, const char *irq_path, int type);

/*  dev_colorplanemask.c:  */
#define	DEV_COLORPLANEMASK_LENGTH	0x0000000000000010
int dev_colorplanemask_access(struct cpu *cpu, struct memory *mem,
	uint64_t relative_addr, unsigned char *data, size_t len,
	int writeflag, void *);
void dev_colorplanemask_init(struct memory *mem, uint64_t baseaddr,
	unsigned char *color_plane_mask);

/*  dev_dc7085.c:  */
#define	DEV_DC7085_LENGTH		0x0000000000000080
/*  see dc7085.h for more info  */
void dev_dc7085_tick(struct cpu *cpu, void *);
int dev_dc7085_access(struct cpu *cpu, struct memory *mem,
	uint64_t relative_addr, unsigned char *data, size_t len,
	int writeflag, void *);
int dev_dc7085_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, char *irq_path, int use_fb);

/*  dev_dec5800.c:  */
#define	DEV_DECCCA_LENGTH			0x10000	/*  ?  */
#define	DEC_DECCCA_BASEADDR			0x19000000	/*  ?  I just made this up  */
int dev_deccca_access(struct cpu *cpu, struct memory *mem, uint64_t relative_addr, unsigned char *data, size_t len, int writeflag, void *);
void dev_deccca_init(struct memory *mem, uint64_t baseaddr);
#define	DEV_DECXMI_LENGTH			0x800000
int dev_decxmi_access(struct cpu *cpu, struct memory *mem, uint64_t relative_addr, unsigned char *data, size_t len, int writeflag, void *);
void dev_decxmi_init(struct memory *mem, uint64_t baseaddr);

/*  dev_fb.c:  */
#define	DEV_FB_LENGTH		0x3c0000	/*  3c0000 to not colide with */
						/*  turbochannel rom,         */
						/*  otherwise size = 4MB      */
/*  Type:  */
#define	VFB_GENERIC		0
#define	VFB_HPC			1
#define	VFB_DEC_VFB01		2
#define	VFB_DEC_VFB02		3
#define	VFB_DEC_MAXINE		4
#define	VFB_PLAYSTATION2	5
/*  Extra flags:  */
#define	VFB_REVERSE_START	0x10000
struct vfb_data {
	struct memory	*memory;
	int		vfb_type;

	int		vfb_scaledown;

	int		xsize;
	int		ysize;
	int		bit_depth;
	int		color32k;	/*  hack for 16-bit HPCmips  */
	int		psp_15bit;	/*  playstation portable hack  */

	unsigned char	color_plane_mask;

	int		bytes_per_line;		/*  cached  */

	int		visible_xsize;
	int		visible_ysize;

	size_t		framebuffer_size;
	int		x11_xsize, x11_ysize;

	int		update_x1, update_y1, update_x2, update_y2;

	/*  RGB palette for <= 8 bit modes:  (r,g,b bytes for each)  */
	unsigned char	rgb_palette[256 * 3];

	char		*name;
	char		title[100];

	void (*redraw_func)(struct vfb_data *, int, int);

	/*  These should always be in sync:  */
	unsigned char	*framebuffer;
	struct fb_window *fb_window;
};
#define	VFB_MFB_BT455			0x100000
#define	VFB_MFB_BT431			0x180000
#define	VFB_MFB_VRAM			0x200000
#define	VFB_CFB_BT459			0x200000
void set_grayscale_palette(struct vfb_data *d, int ncolors);
void dev_fb_resize(struct vfb_data *d, int new_xsize, int new_ysize);
void dev_fb_setcursor(struct vfb_data *d, int cursor_x, int cursor_y, int on, 
        int cursor_xsize, int cursor_ysize);
void framebuffer_blockcopyfill(struct vfb_data *d, int fillflag, int fill_r,
	int fill_g, int fill_b, int x1, int y1, int x2, int y2,
	int from_x, int from_y);
void dev_fb_tick(struct cpu *, void *);
int dev_fb_access(struct cpu *cpu, struct memory *mem, uint64_t relative_addr,
	unsigned char *data, size_t len, int writeflag, void *);
struct vfb_data *dev_fb_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, int vfb_type, int visible_xsize, int visible_ysize,
	int xsize, int ysize, int bit_depth, const char *name);

/*  dev_gt.c:  */
#define	DEV_GT_LENGTH			0x1000
int dev_gt_access(struct cpu *cpu, struct memory *mem, uint64_t relative_addr,
	unsigned char *data, size_t len, int writeflag, void *);
struct pci_data *dev_gt_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, const char *timer_irq_path, const char *isa_irq_path, int type);

/*  dev_jazz.c:  */
size_t dev_jazz_dma_controller(void *dma_controller_data,
	unsigned char *data, size_t len, int writeflag);

/*  dev_kn01.c:  */
#define	DEV_KN01_LENGTH			4
int dev_kn01_access(struct cpu *cpu, struct memory *mem,
	uint64_t relative_addr, unsigned char *data, size_t len,
	int writeflag, void *);
void dev_kn01_init(struct memory *mem, uint64_t baseaddr, int color_fb);
#define	DEV_VDAC_LENGTH			0x20
#define	DEV_VDAC_MAPWA			    0x00
#define	DEV_VDAC_MAP			    0x04
#define	DEV_VDAC_MASK			    0x08
#define	DEV_VDAC_MAPRA			    0x0c
#define	DEV_VDAC_OVERWA			    0x10
#define	DEV_VDAC_OVER			    0x14
#define	DEV_VDAC_OVERRA			    0x1c
int dev_vdac_access(struct cpu *cpu, struct memory *mem,
	uint64_t relative_addr, unsigned char *data, size_t len,
	int writeflag, void *);
void dev_vdac_init(struct memory *mem, uint64_t baseaddr,
	unsigned char *rgb_palette, int color_fb_flag);

/*  dev_kn220.c:  */
#define	DEV_DEC5500_IOBOARD_LENGTH		0x100000
int dev_dec5500_ioboard_access(struct cpu *cpu, struct memory *mem, uint64_t relative_addr, unsigned char *data, size_t len, int writeflag, void *);
struct dec5500_ioboard_data *dev_dec5500_ioboard_init(struct cpu *cpu, struct memory *mem, uint64_t baseaddr);
#define	DEV_SGEC_LENGTH		0x1000
int dev_sgec_access(struct cpu *cpu, struct memory *mem, uint64_t relative_addr, unsigned char *data, size_t len, int writeflag, void *);
void dev_sgec_init(struct memory *mem, uint64_t baseaddr, int irq_nr);

/*  dev_le.c:  */
#define	DEV_LE_LENGTH			0x1c0200
int dev_le_access(struct cpu *cpu, struct memory *mem,
	uint64_t relative_addr, unsigned char *data, size_t len,
	int writeflag, void *);
void dev_le_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, uint64_t buf_start, uint64_t buf_end,
	const char *irq_path, int len);

/*  dev_mc146818.c:  */
#define	DEV_MC146818_LENGTH		0x0000000000000100
#define	MC146818_DEC		0
#define	MC146818_PC_CMOS	1
#define	MC146818_ARC_NEC	2
#define	MC146818_ARC_JAZZ	3
#define	MC146818_SGI		4
#define	MC146818_CATS		5
#define	MC146818_ALGOR		6
#define	MC146818_PMPPC		7
/*  see mc146818reg.h for more info  */
void dev_mc146818_tick(struct cpu *cpu, void *);
int dev_mc146818_access(struct cpu *cpu, struct memory *mem,
	uint64_t relative_addr, unsigned char *data, size_t len,
	int writeflag, void *);
void dev_mc146818_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, char *irq_path, int access_style, int addrdiv);

/*  dev_pckbc.c:  */
#define	DEV_PCKBC_LENGTH		0x10
#define	PCKBC_8042		0
#define	PCKBC_8242		1
#define	PCKBC_JAZZ		3
int dev_pckbc_access(struct cpu *cpu, struct memory *mem,
	uint64_t relative_addr, unsigned char *data, size_t len,
	int writeflag, void *);
int dev_pckbc_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, int type, char *keyboard_irqpath,
	char *mouse_irqpath, int in_use, int pc_style_flag);

/*  dev_pmagja.c:  */
#define	DEV_PMAGJA_LENGTH		0x3c0000
int dev_pmagja_access(struct cpu *cpu, struct memory *mem,
	uint64_t relative_addr, unsigned char *data, size_t len,
	int writeflag, void *);
void dev_pmagja_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, const char *irq_path);

/*  dev_px.c:  */
struct px_data {
	struct memory		*fb_mem;
	struct vfb_data		*vfb_data;
	int			type;
	const char		*px_name;
	struct interrupt	irq;
	int			bitdepth;
	int			xconfig;
	int			yconfig;

	uint32_t		intr;
	unsigned char		sram[128 * 1024];
};
/*  TODO: perhaps these types are wrong?  */
#define	DEV_PX_TYPE_PX			0
#define	DEV_PX_TYPE_PXG			1
#define	DEV_PX_TYPE_PXGPLUS		2
#define	DEV_PX_TYPE_PXGPLUSTURBO	3
#define	DEV_PX_LENGTH			0x3c0000
int dev_px_access(struct cpu *cpu, struct memory *mem, uint64_t relative_addr,
	unsigned char *data, size_t len, int writeflag, void *);
void dev_px_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, int px_type, const char *irq_path);

/*  dev_ram.c:  */
#define	DEV_RAM_RAM				0
#define	DEV_RAM_MIRROR				1
#define	DEV_RAM_MIGHT_POINT_TO_DEVICES		0x10
int dev_ram_access(struct cpu *cpu, struct memory *mem, uint64_t relative_addr,
	unsigned char *data, size_t len, int writeflag, void *);
void dev_ram_init(struct machine *machine, uint64_t baseaddr, uint64_t length,
	int mode, uint64_t otheraddr);

/*  dev_scc.c:  */
#define	DEV_SCC_LENGTH			0x1000
int dev_scc_access(struct cpu *cpu, struct memory *mem, uint64_t relative_addr,
	unsigned char *data, size_t len, int writeflag, void *);
int dev_scc_dma_func(struct cpu *cpu, void *extra, uint64_t addr,
	size_t dma_len, int tx);
void *dev_scc_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, int irq_nr, int use_fb, int scc_nr, int addrmul);

/*  dev_sfb.c:  */
#define	DEV_SFB_LENGTH		0x400000
int dev_sfb_access(struct cpu *cpu, struct memory *mem, uint64_t relative_addr,
	unsigned char *data, size_t len, int writeflag, void *);
void dev_sfb_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, struct vfb_data *vfb_data);

/*  dev_sgi_gbe.c:  */
#define	DEV_SGI_GBE_LENGTH		0x1000000
int dev_sgi_gbe_access(struct cpu *cpu, struct memory *mem,
	uint64_t relative_addr, unsigned char *data, size_t len, int writeflag,
	void *);
void dev_sgi_gbe_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr);

/*  dev_sgi_ip20.c:  */
#define	DEV_SGI_IP20_LENGTH		0x40
#define	DEV_SGI_IP20_BASE		0x1fb801c0
struct sgi_ip20_data {
	int		dummy;
};
int dev_sgi_ip20_access(struct cpu *cpu, struct memory *mem, uint64_t relative_addr, unsigned char *data, size_t len, int writeflag, void *);
struct sgi_ip20_data *dev_sgi_ip20_init(struct cpu *cpu, struct memory *mem, uint64_t baseaddr);

/*  dev_sgi_ip22.c:  */
#define	DEV_SGI_IP22_LENGTH		0x100
#define	DEV_SGI_IP22_IMC_LENGTH		0x100
#define	DEV_SGI_IP22_UNKNOWN2_LENGTH	0x100
#define	IP22_IMC_BASE			0x1fa00000
#define	IP22_UNKNOWN2_BASE		0x1fb94000
struct sgi_ip22_data {
	int		guiness_flag;
	uint32_t	reg[DEV_SGI_IP22_LENGTH / 4];
	uint32_t	imc_reg[DEV_SGI_IP22_IMC_LENGTH / 4];
	uint32_t	unknown2_reg[DEV_SGI_IP22_UNKNOWN2_LENGTH / 4];
	uint32_t	unknown_timer;
};
int dev_sgi_ip22_access(struct cpu *cpu, struct memory *mem, uint64_t relative_addr, unsigned char *data, size_t len, int writeflag, void *);
struct sgi_ip22_data *dev_sgi_ip22_init(struct machine *machine, struct memory *mem, uint64_t baseaddr, int guiness_flag);

/*  dev_sgi_ip32.c:  */
void dev_crime_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, char *irq_path, int use_fb);
#define	DEV_MACEPCI_LENGTH		0x1000
int dev_macepci_access(struct cpu *cpu, struct memory *mem,
	uint64_t relative_addr, unsigned char *data, size_t len,
	int writeflag, void *);
struct pci_data *dev_macepci_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, char *irq_path);
#define	DEV_SGI_MEC_LENGTH		0x1000
int dev_sgi_mec_access(struct cpu *cpu, struct memory *mem,
	uint64_t relative_addr, unsigned char *data, size_t len,
	int writeflag, void *);
void dev_sgi_mec_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, char *irq_path, unsigned char *macaddr);
#define	DEV_SGI_UST_LENGTH		0x10000
int dev_sgi_ust_access(struct cpu *cpu, struct memory *mem,
	uint64_t relative_addr, unsigned char *data, size_t len,
	int writeflag, void *);
void dev_sgi_ust_init(struct memory *mem, uint64_t baseaddr);
#define	DEV_SGI_MTE_LENGTH		0x10000
int dev_sgi_mte_access(struct cpu *cpu, struct memory *mem,
	uint64_t relative_addr, unsigned char *data, size_t len,
	int writeflag, void *);
void dev_sgi_mte_init(struct memory *mem, uint64_t baseaddr);

/*  dev_sii.c:  */
#define	DEV_SII_LENGTH			0x100
void dev_sii_tick(struct cpu *cpu, void *);
int dev_sii_access(struct cpu *cpu, struct memory *mem, uint64_t relative_addr,
	unsigned char *data, size_t len, int writeflag, void *);
void dev_sii_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, uint64_t buf_start, uint64_t buf_end,
	char *irq_path);

/*  dev_ssc.c:  */
#define	DEV_SSC_LENGTH			0x1000
int dev_ssc_access(struct cpu *cpu, struct memory *mem, uint64_t relative_addr,
	unsigned char *data, size_t len, int writeflag, void *);
void dev_ssc_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, const char *irq_path, int use_fb);

/*  dev_turbochannel.c:  */
#define	DEV_TURBOCHANNEL_LEN		0x0470
int dev_turbochannel_access(struct cpu *cpu, struct memory *mem,
	uint64_t relative_addr, unsigned char *data, size_t len,
	int writeflag, void *);
void dev_turbochannel_init(struct machine *machine, struct memory *mem,
	int slot_nr, uint64_t baseaddr, uint64_t endaddr, const char *device_name,
	const char *irq_path);

/*  dev_uninorth.c:  */
struct pci_data *dev_uninorth_init(struct machine *machine, struct memory *mem,
	uint64_t addr, int irqbase, int pciirq);

/*  dev_vga.c:  */
int dev_vga_access(struct cpu *cpu, struct memory *mem, uint64_t relative_addr,
	unsigned char *data, size_t len, int writeflag, void *);
void dev_vga_init(struct machine *machine, struct memory *mem,
	uint64_t videomem_base, uint64_t control_base, char *name);

/*  dev_vr41xx.c:  */
struct vr41xx_data *dev_vr41xx_init(struct machine *machine,
	struct memory *mem, int cpumodel);

/*  lk201.c:  */
struct lk201_data {
        int                     use_fb;
	int			console_handle;

        void                    (*add_to_rx_queue)(void *,int,int);
	void			*add_data;
                
        unsigned char           keyb_buf[8];
        int                     keyb_buf_pos;
                        
        int                     mouse_mode;
        int                     mouse_revision;         /*  0..15  */  
        int                     mouse_x, mouse_y, mouse_buttons;
};
void lk201_tick(struct machine *, struct lk201_data *); 
void lk201_tx_data(struct lk201_data *, int port, int idata);
void lk201_init(struct lk201_data *d, int use_fb,
	void (*add_to_rx_queue)(void *,int,int), int console_handle, void *);


#endif	/*  DEVICES_H  */

