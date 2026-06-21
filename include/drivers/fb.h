#ifndef _DRIVERS_VIDEO_FB_H
#define _DRIVERS_VIDEO_FB_H

#include <kernel/types.h>
#include <fs/vfs.h>

#define FBIOGET_FSCREENINFO 0x4600
#define FBIOGET_VSCREENINFO 0x4601
#define FBIOPUT_VSCREENINFO 0x4602
#define FBIOPUTCMAP         0x4605

#define FB_TYPE_PACKED_PIXELS 0
#define FB_VISUAL_TRUECOLOR   2

struct fb_bitfield {
	uint32_t offset;
	uint32_t length;
	uint32_t msb_right;
};

struct fb_fix_screeninfo {
	char id[16];
	unsigned long smem_start;
	uint32_t smem_len;
	uint32_t type;
	uint32_t type_aux;
	uint32_t visual;
	uint16_t xpanstep;
	uint16_t ypanstep;
	uint16_t ywrapstep;
	uint32_t line_length;
	unsigned long mmio_start;
	uint32_t mmio_len;
	uint32_t capabilities;
	uint16_t reserved[2];
};

struct fb_var_screeninfo {
	uint32_t xres;
	uint32_t yres;
	uint32_t xres_virtual;
	uint32_t yres_virtual;
	uint32_t xoffset;
	uint32_t yoffset;
	uint32_t bits_per_pixel;
	uint32_t grayscale;
	struct fb_bitfield red;
	struct fb_bitfield green;
	struct fb_bitfield blue;
	struct fb_bitfield transp;
	uint32_t nonstd;
	uint32_t activate;
	uint32_t height;
	uint32_t width;
	uint32_t accel_flags;
};

typedef struct {
	uintptr_t phys;
	uint8_t *virt;
	uint32_t width;
	uint32_t height;
	uint32_t pitch;
	uint32_t bpp;
	uint8_t red_shift;
	uint8_t red_size;
	uint8_t green_shift;
	uint8_t green_size;
	uint8_t blue_shift;
	uint8_t blue_size;
} blueos_fb_t;

/* Declaraciones globales expuestas para vfs.c */
extern blueos_fb_t global_fb;
extern vfs_ops_t fb0_ops;

void fb_init(uintptr_t phys_addr, uint32_t width, uint32_t height, uint32_t bpp);
void fb_test_pattern(void);
void fb_puts(uint32_t x, uint32_t y, const char *str, uint32_t color);

#endif /* _DRIVERS_VIDEO_FB_H */