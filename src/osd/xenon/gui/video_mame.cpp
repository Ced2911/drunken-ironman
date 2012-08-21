#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <xetypes.h>

#include <input/input.h>
#include <console/console.h>

#include <xenos/xe.h>
#include <xenos/xenos.h>
#include <xenos/edram.h>
#include <xenos/xenos.h>

#include <debug.h>

// MAME headers
#include "emu.h"
#include "render.h"
#include "ui.h"
#include "rendutil.h"
#include "options.h"
#include "emuopts.h"
#include "aviio.h"
#include "png.h"

// wiigui stuff
#include "video.h"

// blitter func
#include "blit.inl.h"

typedef unsigned int DWORD;

#include "../hlsl/primary.ps.h"
#include "../hlsl/primary.vs.h"


#define ENABLE_BORDER_PIX	(1)

#define r32(o) g_pVideoDevice->regs[(o)/4]
#define w32(o, v) g_pVideoDevice->regs[(o)/4] = (v)

enum
{
	TEXTURE_TYPE_PLAIN,
	TEXTURE_TYPE_DYNAMIC,
	TEXTURE_TYPE_SURFACE
};

typedef struct
{
	float x, y, z; // 12
	unsigned int color; // 16
	float u, v; // 24
	/** padding **/
	float p1, p2; // 32
}
__attribute__((packed, aligned(32))) MameVerticeFormats;

struct xe_tex
{
	xe_tex *next;
	void *addr;
	struct XenosSurface *surface;
};

/* line_aa_step is used for drawing antialiased lines */
typedef struct _line_aa_step line_aa_step;

struct _line_aa_step
{
	float xoffs, yoffs; // X/Y deltas
	float weight; // weight contribution
};

struct texture_info
{
	struct XenosSurface * surf;

	texture_info * next; // next texture in the list
	texture_info * prev; // prev texture in the list
	unsigned int hash; // hash value for the texture
	unsigned int flags; // rendering flags
	render_texinfo texinfo; // copy of the texture info
	float ustart, ustop; // beginning/ending U coordinates
	float vstart, vstop; // beginning/ending V coordinates
	int rawwidth, rawheight; // raw width/height of the texture
	int type; // what type of texture are we?
	int xborderpix; // number of border pixels in X
	int yborderpix; // number of border pixels in Y
	int xprescale; // what is our X prescale factor?
	int yprescale; // what is our Y prescale factor?
	int cur_frame; // what is our current frame?
	int prev_frame; // what was our last frame? (used to determine pause state)
	struct XenosSurface * d3dtex; // Direct3D texture pointer
	struct XenosSurface * d3dsurface; // Direct3D offscreen plain surface pointer
	struct XenosSurface * d3dfinaltex; // Direct3D final (post-scaled) texture
	int target_index; // Direct3D target index
};

struct _xe_info
{
	int						width, height;				// current width, height

	texture_info *			texlist;					// list of active textures
	int						dynamic_supported;			// are dynamic textures supported?
	int						stretch_supported;			// is StretchRect with point filtering supported?
	int						mod2x_supported;			// is D3DTOP_MODULATE2X supported?
	int						mod4x_supported;			// is D3DTOP_MODULATE4X supported?
	unsigned int			screen_format;				// format to use for screen textures
	unsigned int			yuv_format;					// format to use for YUV textures

	DWORD					texture_max_aspect;			// texture maximum aspect ratio
	DWORD					texture_max_width;			// texture maximum width
	DWORD					texture_max_height;			// texture maximum height

	texture_info *			last_texture;				// previous texture
	UINT32					last_texture_flags;			// previous texture flags
	int						last_blendenable;			// previous blendmode
	int						last_blendop;				// previous blendmode
	int						last_blendsrc;				// previous blendmode
	int						last_blenddst;				// previous blendmode
	int						last_filter;				// previous texture filter
	int						last_wrap;					// previous wrap state
	DWORD					last_modmode;				// previous texture modulation

	texture_info *			vector_texture;				// experimental: texture for vectors
	texture_info *			default_texture;			// experimental: default texture
	
	struct XenosSurface * whitetexture; // Direct3D final (post-scaled) texture
};

// tmp
extern void SetRS();

extern int nb_vertices;
extern XenosDevice * g_pVideoDevice;
extern XenosVertexBuffer *vb;

// used to know which fb to use
static int ibuffer = 0;

static XenosShader * g_pVertexShader = NULL;
static XenosShader * g_pPixelShader = NULL;
static struct XenosSurface * g_framebuffer[2] = {NULL};

static _xe_info xe_info = { 0 };

static const line_aa_step line_aa_1step[] =
{
	{  0.00f,  0.00f,  1.00f  },
	{ 0 }
};

static const line_aa_step line_aa_4step[] =
{
	{ -0.25f,  0.00f,  0.25f  },
	{  0.25f,  0.00f,  0.25f  },
	{  0.00f, -0.25f,  0.25f  },
	{  0.00f,  0.25f,  0.25f  },
	{ 0 }
};

uint32_t texture_max_aspect = 2048;
uint32_t texture_max_width = 2048;
uint32_t texture_max_height = 2048;


int prescale = 0;
int stretch_supported = 1;

//============================================================
//  Textures
//============================================================

static inline struct XenosSurface * My_CreateTexture(struct XenosDevice *xe, unsigned int width, unsigned int height, unsigned int levels, int format, int tiled);
static void texture_compute_size(_xe_info *d3d, int texwidth, int texheight, texture_info *texture);
static void texture_set_data(_xe_info *d3d, texture_info *texture, const render_texinfo *texsource, UINT32 flags);
static void texture_prescale(_xe_info *d3d, texture_info *texture);
static texture_info *texture_find(_xe_info *d3d, const render_primitive *prim);
static void texture_update(_xe_info *d3d, const render_primitive *prim);
static texture_info *texture_create(_xe_info *d3d, const render_texinfo *texsource, UINT32 flags);


void init_xe_info(){
	xe_info.texture_max_aspect = 2048;
	xe_info.texture_max_width = 2048;
	xe_info.texture_max_height = 2048;
	xe_info.screen_format = XE_FMT_ARGB | XE_FMT_8888;
	xe_info.yuv_format = XE_FMT_ARGB | XE_FMT_8888;
	
	xe_info.whitetexture = My_CreateTexture(g_pVideoDevice, 256, 256, 0, XE_FMT_ARGB | XE_FMT_8888, 1);
	
	void * base = Xe_Surface_LockRect(g_pVideoDevice, xe_info.whitetexture, 0, 0, 0, 0, XE_LOCK_WRITE);
	memset(base, 0xFF, 256*256*4);
	Xe_Surface_Unlock(g_pVideoDevice, xe_info.whitetexture);	
	
	base = Xe_Surface_LockRect(g_pVideoDevice, g_framebuffer[0], 0, 0, 0, 0, XE_LOCK_WRITE);
	memset(base, 0xFF, g_framebuffer[0]->width*g_framebuffer[0]->height*4);
	Xe_Surface_Unlock(g_pVideoDevice, g_framebuffer[0]);
	
	base = Xe_Surface_LockRect(g_pVideoDevice, g_framebuffer[1], 0, 0, 0, 0, XE_LOCK_WRITE);
	memset(base, 0xFF, g_framebuffer[1]->width*g_framebuffer[1]->height*4);
	Xe_Surface_Unlock(g_pVideoDevice, g_framebuffer[1]);
}

//============================================================
//  Textures
//============================================================

INLINE uint32_t texture_compute_hash(const render_texinfo *texture, uint32_t flags)
{
	return (uint32_t) texture->base ^ (flags & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK));
}

//============================================================
//  texture_compute_size
//============================================================

static void texture_compute_size(_xe_info *d3d, int texwidth, int texheight, texture_info *texture)
{
	int finalheight = texheight;
	int finalwidth = texwidth;

	// if we're not wrapping, add a 1-2 pixel border on all sides
	texture->xborderpix = 0;
	texture->yborderpix = 0;
	if (ENABLE_BORDER_PIX && !(texture->flags & PRIMFLAG_TEXWRAP_MASK)) {
		// note we need 2 pixels in X for YUY textures
		texture->xborderpix = (PRIMFLAG_GET_TEXFORMAT(texture->flags) == TEXFORMAT_YUY16) ? 2 : 1;
		texture->yborderpix = 1;
	}

	// compute final texture size
	finalwidth += 2 * texture->xborderpix;
	finalheight += 2 * texture->yborderpix;

	// round width/height up to nearest power of 2 if we need to
	if (1) {
		// first the width
		if (finalwidth & (finalwidth - 1)) {
			finalwidth |= finalwidth >> 1;
			finalwidth |= finalwidth >> 2;
			finalwidth |= finalwidth >> 4;
			finalwidth |= finalwidth >> 8;
			finalwidth++;
		}

		// then the height
		if (finalheight & (finalheight - 1)) {
			finalheight |= finalheight >> 1;
			finalheight |= finalheight >> 2;
			finalheight |= finalheight >> 4;
			finalheight |= finalheight >> 8;
			finalheight++;
		}
	}

	// round up to square if we need to
	if (0) {
		if (finalwidth < finalheight)
			finalwidth = finalheight;
		else
			finalheight = finalwidth;
	}

	// adjust the aspect ratio if we need to
	while (finalwidth < finalheight && finalheight / finalwidth > texture_max_aspect)
		finalwidth *= 2;
	while (finalheight < finalwidth && finalwidth / finalheight > texture_max_aspect)
		finalheight *= 2;

	// if we added pixels for the border, and that just barely pushed us over, take it back
	if ((finalwidth > texture_max_width && finalwidth - 2 * texture->xborderpix <= texture_max_width) ||
		(finalheight > texture_max_height && finalheight - 2 * texture->yborderpix <= texture_max_height)) {
		finalwidth -= 2 * texture->xborderpix;
		finalheight -= 2 * texture->yborderpix;
		texture->xborderpix = 0;
		texture->yborderpix = 0;
	}

	// if we're above the max width/height, do what?
	if (finalwidth > texture_max_width || finalheight > texture_max_height) {
		static int printed = FALSE;
		if (!printed) mame_printf_warning("Texture too big! (wanted: %dx%d, max is %dx%d)\n", finalwidth, finalheight, (int) texture_max_width, (int) texture_max_height);
		printed = TRUE;
	}

	// compute the U/V scale factors
	texture->ustart = (float) texture->xborderpix / (float) finalwidth;
	texture->ustop = (float) (texwidth + texture->xborderpix) / (float) finalwidth;
	texture->vstart = (float) texture->yborderpix / (float) finalheight;
	texture->vstop = (float) (texheight + texture->yborderpix) / (float) finalheight;

	// set the final values
	texture->rawwidth = finalwidth;
	texture->rawheight = finalheight;
}

static inline struct XenosSurface * My_CreateTexture(struct XenosDevice *xe, unsigned int width, unsigned int height, unsigned int levels, int format, int tiled) {
	struct XenosSurface * s = Xe_CreateTexture(xe, width, height, levels, format, tiled);

	// disable filtering
	s->use_filtering = 0;
	return s;
}

static inline void handle_small_surface(struct XenosSurface * surf){
	int width;
	int height;
	int wpitch;
	int hpitch;
	uint32_t * surf_data;
	uint32_t * data;
	uint32_t * src;	
	
	// don't handle big texture
	if( surf->width>128 && surf->height>32) {
		return;
	}	
	
	width = surf->width;
	height = surf->height;
	wpitch = surf->wpitch / 4;
	hpitch = surf->hpitch;	
	
	surf_data = (uint32_t *)Xe_Surface_LockRect(g_pVideoDevice, surf, 0, 0, 0, 0, XE_LOCK_WRITE);
	
	src = data = surf_data;
		
	for(int yp=0; yp<hpitch;yp+=height) {
		int max_h = height;
		if (yp + height> hpitch)
				max_h = hpitch % height;
		for(int y = 0; y<max_h; y++){
			//x order
			for(int xp = 0;xp<wpitch;xp+=width) {
				int max_w = width;
				if (xp + width> wpitch)
					max_w = wpitch % width;

				for(int x = 0; x<max_w; x++) {
					data[x+xp + ((y+yp)*wpitch)]=src[x+ (y*wpitch)];
				}
			}
		}
	}
	
	Xe_Surface_Unlock(g_pVideoDevice, surf);
}

//============================================================
//  texture_create
//============================================================

texture_info * texture_create(_xe_info *d3d, const render_texinfo *texsource, UINT32 flags)
{
	texture_info *texture;
	// allocate a new texture
	texture = global_alloc_clear(texture_info);

	// fill in the core data
	texture->hash = texture_compute_hash(texsource, flags);
	texture->flags = flags;
	texture->texinfo = *texsource;
	texture->xprescale = prescale;
	texture->yprescale = prescale;

	// compute the size
	texture_compute_size(d3d, texsource->width, texsource->height, texture);
	
	// non-screen textures are easy
	if (!PRIMFLAG_GET_SCREENTEX(flags)) {
		assert(PRIMFLAG_TEXFORMAT(flags) != TEXFORMAT_YUY16);
		texture->d3dtex = My_CreateTexture(g_pVideoDevice, texture->rawwidth, texture->rawheight, 0, XE_FMT_ARGB | XE_FMT_8888, 0);

		texture->d3dfinaltex = texture->d3dtex;
		texture->type = TEXTURE_TYPE_PLAIN;
	}		
	// screen textures are allocated differently
	else {
		int maxdim = MAX(1270, 720);
			
		// don't prescale above screen size
		while (texture->xprescale > 1 && texture->rawwidth * texture->xprescale >= 2 * maxdim)
			texture->xprescale--;
		while (texture->xprescale > 1 && texture->rawwidth * texture->xprescale > texture_max_width)
			texture->xprescale--;
		while (texture->yprescale > 1 && texture->rawheight * texture->yprescale >= 2 * maxdim)
			texture->yprescale--;
		while (texture->yprescale > 1 && texture->rawheight * texture->yprescale > texture_max_height)
			texture->yprescale--;
		if (texture->xprescale != prescale || texture->yprescale != prescale)
			mame_printf_verbose("Direct3D: adjusting prescale from %dx%d to %dx%d\n", prescale, prescale, texture->xprescale, texture->yprescale);
			
		texture->d3dtex = My_CreateTexture(g_pVideoDevice, texture->rawwidth, texture->rawheight, 0, XE_FMT_ARGB | XE_FMT_8888, 0);

		texture->d3dfinaltex = texture->d3dtex;
		texture->type = TEXTURE_TYPE_PLAIN;
	}

	// copy the data to the texture
	texture_set_data(d3d, texture, texsource, flags);

	// add us to the texture list
	if (d3d->texlist != NULL)
		d3d->texlist->prev = texture;
	
	texture->prev = NULL;
	texture->next = d3d->texlist;
	d3d->texlist = texture;
	return texture;
}


//============================================================
//  texture_set_data
//============================================================

static void texture_set_data(_xe_info *d3d, texture_info *texture, const render_texinfo *texsource, UINT32 flags)
{
	void * pBits = NULL;
	int Pitch = 0;
	int miny, maxy;
	int dsty;

	// lock the texture
	switch (texture->type) {
	default:
	case TEXTURE_TYPE_PLAIN:
		pBits = Xe_Surface_LockRect(g_pVideoDevice, texture->d3dtex, 0, 0, 0, 0, XE_LOCK_WRITE);
		Pitch = texture->d3dtex->wpitch;
		break;
	case TEXTURE_TYPE_DYNAMIC:
		pBits = Xe_Surface_LockRect(g_pVideoDevice, texture->d3dtex, 0, 0, 0, 0, XE_LOCK_WRITE);
		Pitch = texture->d3dtex->wpitch;
		break;
	case TEXTURE_TYPE_SURFACE:
		pBits = Xe_Surface_LockRect(g_pVideoDevice, texture->d3dsurface, 0, 0, 0, 0, XE_LOCK_WRITE);
		Pitch = texture->d3dsurface->wpitch;
		break;
	}

	// loop over Y
	miny = 0 - texture->yborderpix;
	maxy = texsource->height + texture->yborderpix;

	for (dsty = miny; dsty < maxy; dsty++) {
		int srcy = (dsty < 0) ? 0 : (dsty >= texsource->height) ? texsource->height - 1 : dsty;
		void *dst = (unsigned char *) pBits + (dsty + texture->yborderpix) * Pitch;

		// switch off of the format and
		switch (PRIMFLAG_GET_TEXFORMAT(flags)) {
		case TEXFORMAT_PALETTE16:
			copyline_palette16((UINT32 *) dst, (UINT16 *) texsource->base + srcy * texsource->rowpixels, texsource->width, texsource->palette, texture->xborderpix);
			break;

		case TEXFORMAT_PALETTEA16:
			copyline_palettea16((UINT32 *) dst, (UINT16 *) texsource->base + srcy * texsource->rowpixels, texsource->width, texsource->palette, texture->xborderpix);
			break;

		case TEXFORMAT_RGB32:
			copyline_rgb32((UINT32 *) dst, (UINT32 *) texsource->base + srcy * texsource->rowpixels, texsource->width, texsource->palette, texture->xborderpix);
			break;

		case TEXFORMAT_ARGB32:
			copyline_argb32((UINT32 *) dst, (UINT32 *) texsource->base + srcy * texsource->rowpixels, texsource->width, texsource->palette, texture->xborderpix);
			break;

		case TEXFORMAT_YUY16:
			copyline_yuy16_to_argb((UINT32 *) dst, (UINT16 *) texsource->base + srcy * texsource->rowpixels, texsource->width, texsource->palette, texture->xborderpix);
			break;

		default:
			mame_printf_error("Unknown texture blendmode=%d format=%d\n", PRIMFLAG_GET_BLENDMODE(flags), PRIMFLAG_GET_TEXFORMAT(flags));
			break;
		}
	}

	// unlock
	switch (texture->type) {
		default:
		case TEXTURE_TYPE_PLAIN:
			Xe_Surface_Unlock(g_pVideoDevice, texture->d3dtex);
			break;
		case TEXTURE_TYPE_DYNAMIC:
			Xe_Surface_Unlock(g_pVideoDevice, texture->d3dtex);
			break;
		case TEXTURE_TYPE_SURFACE:
			Xe_Surface_Unlock(g_pVideoDevice, texture->d3dsurface);
			break;
	}

	// prescale
	texture_prescale(d3d, texture);
	
	handle_small_surface(texture->d3dtex);
}



//============================================================
//  texture_prescale
//============================================================

static void texture_prescale (_xe_info *d3d, texture_info *texture)
{
}

//============================================================
//  texture_find
//============================================================

static texture_info * texture_find(_xe_info *d3d, const render_primitive *prim)
{
	UINT32 texhash = texture_compute_hash(&prim->texture, prim->flags);
	texture_info *texture;

	// find a match
	for (texture = d3d->texlist; texture != NULL; texture = texture->next) {
		if (texture->hash == texhash &&
			texture->texinfo.base == prim->texture.base &&
			texture->texinfo.width == prim->texture.width &&
			texture->texinfo.height == prim->texture.height &&
			((texture->flags ^ prim->flags) & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK)) == 0) {
				//TR;
				return texture;
		}

	}
	// nothing found
	return NULL;
}

//============================================================
//  texture_update
//============================================================

static void texture_update(_xe_info *d3d, const render_primitive *prim)
{
	texture_info *texture = texture_find(d3d, prim);

	// if we didn't find one, create a new texture
	if (texture == NULL) {
		texture = texture_create(d3d, &prim->texture, prim->flags);
	}

	// if we found it, but with a different seqid, copy the data
	if (texture->texinfo.seqid != prim->texture.seqid) {
		texture_set_data(d3d, texture, &prim->texture, prim->flags);
		texture->texinfo.seqid = prim->texture.seqid;
	}
}

void SetRS(render_primitive * prim)
{
	int blendmode = PRIMFLAG_GET_BLENDMODE(prim->flags);

	int blendenable;
	int blendop;
	int blendsrc;
	int blenddst;

	switch (blendmode) {
		default:
		case BLENDMODE_NONE: 
			blendenable = FALSE;
			blendop = XE_BLENDOP_ADD;
			blendsrc = XE_BLEND_SRCALPHA;
			blenddst = XE_BLEND_INVSRCALPHA;
			break;
		case BLENDMODE_ALPHA: 
			blendenable = TRUE;
			blendop = XE_BLENDOP_ADD;
			blendsrc = XE_BLEND_SRCALPHA;
			blenddst = XE_BLEND_INVSRCALPHA;
			break;
		case BLENDMODE_RGB_MULTIPLY: 
			blendenable = TRUE;
			blendop = XE_BLENDOP_ADD;
			blendsrc = XE_BLEND_DESTCOLOR;
			blenddst = XE_BLEND_ZERO;
			break;
		case BLENDMODE_ADD: 
			blendenable = TRUE;
			blendop = XE_BLENDOP_ADD;
			blendsrc = XE_BLEND_SRCALPHA;
			blenddst = XE_BLEND_ONE;
			break;
	}

	Xe_SetBlendOp(g_pVideoDevice, blendop);
	Xe_SetSrcBlend(g_pVideoDevice, blendsrc);
	Xe_SetDestBlend(g_pVideoDevice, blenddst);
	Xe_SetAlphaTestEnable(g_pVideoDevice, blendenable);

	Xe_SetCullMode(g_pVideoDevice, XE_CULL_NONE);
	Xe_SetStreamSource(g_pVideoDevice, 0, vb, nb_vertices, 12);
}

/**
 *
 * @param prim
 */
void DrawQuad(render_primitive * prim)
{
	/* from shader */
	XenosSurface * fb = Xe_GetFramebufferSurface(g_pVideoDevice);
	float TargetWidth = fb->width;
	float TargetHeight = fb->height;
	float PostPass = 0.f;
	texture_info *texture = NULL;
	
	if (prim->texture.base != NULL){
		texture_update(&xe_info, prim);
		texture = texture_find (&xe_info, prim);
	}

	XeColor color;

	// Swap R => B
	color.a = (unsigned char) prim->color.a * 255.f;
	color.b = (unsigned char) prim->color.r * 255.f;
	color.g = (unsigned char) prim->color.g * 255.f;
	color.r = (unsigned char) prim->color.b * 255.f;

	MameVerticeFormats* Rect = (MameVerticeFormats*) Xe_VB_Lock(g_pVideoDevice, vb, nb_vertices, 4096, XE_LOCK_WRITE);
	{
		Rect[0].x = prim->bounds.x0 - 0.5f;
		Rect[0].y = prim->bounds.y0 - 0.5f;
		Rect[1].x = prim->bounds.x1 - 0.5f;
		Rect[1].y = prim->bounds.y0 - 0.5f;
		Rect[2].x = prim->bounds.x0 - 0.5f;
		Rect[2].y = prim->bounds.y1 - 0.5f;
		Rect[3].x = prim->bounds.x1 - 0.5f;
		Rect[3].y = prim->bounds.y1 - 0.5f;

		// set the texture coordinates
		if(texture != NULL && texture->d3dtex != NULL)
		{
			float du = texture->ustop - texture->ustart;
			float dv = texture->vstop - texture->vstart;
			Rect[0].u = texture->ustart + du * prim->texcoords.tl.u;
			Rect[0].v = texture->vstart + dv * prim->texcoords.tl.v;
			Rect[1].u = texture->ustart + du * prim->texcoords.tr.u;
			Rect[1].v = texture->vstart + dv * prim->texcoords.tr.v;
			Rect[2].u = texture->ustart + du * prim->texcoords.bl.u;
			Rect[2].v = texture->vstart + dv * prim->texcoords.bl.v;
			Rect[3].u = texture->ustart + du * prim->texcoords.br.u;
			Rect[3].v = texture->vstart + dv * prim->texcoords.br.v;
		}
		else {
			// bottom left
			Rect[0].u = 0;
			Rect[0].v = 1;

			// bottom right
			Rect[1].u = 1;
			Rect[1].v = 1;

			// top right
			Rect[2].u = 1;
			Rect[2].v = 0;

			// Top left
			Rect[3].u = 0;
			Rect[3].v = 0;
		}
		
		int i = 0;
		for (i = 0; i < 4; i++) {
			Rect[i].z = 0.0;
			Rect[i].color = color.lcol;
		}
	}
	Xe_VB_Unlock(g_pVideoDevice, vb);

	Xe_SetShader(g_pVideoDevice, SHADER_TYPE_PIXEL, g_pPixelShader, 0);
	Xe_SetShader(g_pVideoDevice, SHADER_TYPE_VERTEX, g_pVertexShader, 0);

	// primary.fx
	// Registers:
	//
	//   Name         Reg   Size
	//   ------------ ----- ----
	//   TargetWidth  c0       1
	//   TargetHeight c1       1
	//   PostPass     c2       1
	Xe_SetVertexShaderConstantF(g_pVideoDevice, 0, (float*) &TargetWidth, 1);
	Xe_SetVertexShaderConstantF(g_pVideoDevice, 1, (float*) &TargetHeight, 1);
	Xe_SetVertexShaderConstantF(g_pVideoDevice, 2, (float*) &PostPass, 1);	
	
	if(texture != NULL && texture->d3dtex != NULL)
		Xe_SetTexture(g_pVideoDevice, 0, texture->d3dtex);
	else
		Xe_SetTexture(g_pVideoDevice, 0, xe_info.whitetexture);		

	SetRS(prim);

	Xe_DrawPrimitive(g_pVideoDevice, XE_PRIMTYPE_RECTLIST, 0, 1);
	//nb_vertices += 4 * sizeof (DrawVerticeFormats);
	nb_vertices += 256; // fixe aligement
}

void DrawLine(render_primitive * prim)
{
	render_bounds b0, b1;
	const line_aa_step *step = line_aa_4step;
	
	if (prim->texture.base != NULL)
		texture_update(&xe_info, prim);

	/* from shader */
	XenosSurface * fb = Xe_GetFramebufferSurface(g_pVideoDevice);
	float TargetWidth = fb->width;
	float TargetHeight = fb->height;

	float PostPass = 0.f;

	XeColor color;
	
	// compute the effective width based on the direction of the line
	float effwidth = prim->width;
	if (effwidth < 0.5f)
		effwidth = 0.5f;
	
	// determine the bounds of a quad to draw this line
	render_line_to_quad(&prim->bounds, effwidth, &b0, &b1);
	
	for (step = PRIMFLAG_GET_ANTIALIAS(prim->flags) ? line_aa_4step : line_aa_1step; step->weight != 0; step++)
	{
		// get a pointer to the vertex buffer
		MameVerticeFormats* vertex = (MameVerticeFormats*) Xe_VB_Lock(g_pVideoDevice, vb, nb_vertices, 4096, XE_LOCK_WRITE);
		if (vertex == NULL)
			return;

		// rotate the unit vector by 135 degrees and add to point 0
		vertex[0].x = b0.x0 + step->xoffs;
		vertex[0].y = b0.y0 + step->yoffs;

		// rotate the unit vector by -135 degrees and add to point 0
		vertex[1].x = b0.x1 + step->xoffs;
		vertex[1].y = b0.y1 + step->yoffs;

		// rotate the unit vector by 45 degrees and add to point 1
		vertex[2].x = b1.x0 + step->xoffs;
		vertex[2].y = b1.y0 + step->yoffs;

		// rotate the unit vector by -45 degrees and add to point 1
		vertex[3].x = b1.x1 + step->xoffs;
		vertex[3].y = b1.y1 + step->yoffs;

		// determine the color of the line
		color.a = (unsigned char) prim->color.a * 255.f;
		color.b = (unsigned char) prim->color.r * 255.f;
		color.g = (unsigned char) prim->color.g * 255.f;
		color.r = (unsigned char) prim->color.b * 255.f;

		// set the color, Z parameters to standard values
		for (int i = 0; i < 4; i++)
		{
			vertex[i].z = 0.0f;
			//vertex[i].w = 1.0f;
			vertex[i].color = color.lcol;
		}

		// now add a polygon entry
		Xe_VB_Unlock(g_pVideoDevice, vb);
		
		Xe_SetTexture(g_pVideoDevice, 0, xe_info.whitetexture);

		Xe_SetShader(g_pVideoDevice, SHADER_TYPE_PIXEL, g_pPixelShader, 0);
		Xe_SetShader(g_pVideoDevice, SHADER_TYPE_VERTEX, g_pVertexShader, 0);

		// primary.fx
		// Registers:
		//
		//   Name         Reg   Size
		//   ------------ ----- ----
		//   TargetWidth  c0       1
		//   TargetHeight c1       1
		//   PostPass     c2       1

		Xe_SetVertexShaderConstantF(g_pVideoDevice, 0, (float*) &TargetWidth, 1);
		Xe_SetVertexShaderConstantF(g_pVideoDevice, 1, (float*) &TargetHeight, 1);
		Xe_SetVertexShaderConstantF(g_pVideoDevice, 2, (float*) &PostPass, 1);

		SetRS(prim);

		Xe_DrawPrimitive(g_pVideoDevice, XE_PRIMTYPE_TRIANGLESTRIP, 0, 2);
		nb_vertices += 256;
	}

}

void MameRender()
{
	// double buffering
	ibuffer ^= 1;	
	
	Xe_SetClearColor(g_pVideoDevice, 0xFF000000);
	Xe_Resolve(g_pVideoDevice);
	
	// @todo implement double buffering
	// must be called between vblank and vsync
	if(ibuffer ==0) {
		// hack !!
		w32(0x6110, g_framebuffer[0]->base);
		g_pVideoDevice->tex_fb.base = g_framebuffer[0]->base;
		Xe_SetRenderTarget(g_pVideoDevice,g_framebuffer[0]);
	}
	else{
		w32(0x6110, g_framebuffer[1]->base);
		g_pVideoDevice->tex_fb.base = g_framebuffer[1]->base;
		Xe_SetRenderTarget(g_pVideoDevice,g_framebuffer[1]);
	}

    Xe_Sync(g_pVideoDevice);

    Xe_InvalidateState(g_pVideoDevice);

    nb_vertices = 0;
}

void MameFrame()
{

}

void InitMameShaders()
{
	void* vs_program = NULL;
	void* ps_program = NULL;
	
	static const struct XenosVBFFormat vbf = {
		4,
		{
			{XE_USAGE_POSITION, 0, XE_TYPE_FLOAT3},
			{XE_USAGE_COLOR, 0, XE_TYPE_UBYTE4},
			{XE_USAGE_TEXCOORD, 0, XE_TYPE_FLOAT2},
			{XE_USAGE_TEXCOORD, 1, XE_TYPE_FLOAT2}, //padding
		}
	};
	vs_program = (void*) g_xvs_vs_main;
	ps_program = (void*) g_xps_ps_main;

	g_pPixelShader = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) ps_program);
	Xe_InstantiateShader(g_pVideoDevice, g_pPixelShader, 0);

	g_pVertexShader = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) vs_program);
	Xe_InstantiateShader(g_pVideoDevice, g_pVertexShader, 0);

	Xe_ShaderApplyVFetchPatches(g_pVideoDevice, g_pVertexShader, 0, &vbf);
	
	XenosSurface * fb = Xe_GetFramebufferSurface(g_pVideoDevice);
	
	// Create surface for double buffering
	g_framebuffer[0] = Xe_CreateTexture(g_pVideoDevice, fb->width, fb->height, 0, XE_FMT_8888 | XE_FMT_BGRA, 1);
	g_framebuffer[1] = Xe_CreateTexture(g_pVideoDevice, fb->width, fb->height, 0, XE_FMT_8888 | XE_FMT_BGRA, 1);
	
	init_xe_info();
}