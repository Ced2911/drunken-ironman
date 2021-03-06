/***************************************************************************

    Sega System 18 hardware

***************************************************************************/

#include "emu.h"
#include "video/segaic16.h"
#include "includes/genesis.h"
#include "includes/segas16.h"



/*************************************
 *
 *  Debugging
 *
 *************************************/

#define DEBUG_VDP				(0)


/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START( system18 )
{
	segas1x_state *state = machine.driver_data<segas1x_state>();
	int width, height;

	state->m_grayscale_enable = 0;
	state->m_vdp_enable = 0;
	state->m_vdp_mixing = 0;

	/* compute palette info */
	segaic16_palette_init(0x800);

	/* initialize the tile/text layers */
	segaic16_tilemap_init(machine, 0, SEGAIC16_TILEMAP_16B, 0x000, 0, 8);

	/* create the VDP */
	system18_vdp_start(machine);

	/* create a temp bitmap to draw the VDP data into */
	width = machine.primary_screen->width();
	height = machine.primary_screen->height();
	state->m_tmp_bitmap = auto_bitmap_ind16_alloc(machine, width, height);


	state->save_item(NAME(state->m_grayscale_enable));
	state->save_item(NAME(state->m_vdp_enable));
	state->save_item(NAME(state->m_vdp_mixing));
	state->save_item(NAME(*state->m_tmp_bitmap));
}



/*************************************
 *
 *  Miscellaneous setters
 *
 *************************************/

void system18_set_grayscale(running_machine &machine, int enable)
{
	segas1x_state *state = machine.driver_data<segas1x_state>();

	enable = (enable != 0);
	if (enable != state->m_grayscale_enable)
	{
		machine.primary_screen->update_partial(machine.primary_screen->vpos());
		state->m_grayscale_enable = enable;
//      mame_printf_debug("Grayscale = %02X\n", enable);
	}
}


void system18_set_vdp_enable(running_machine &machine, int enable)
{
	segas1x_state *state = machine.driver_data<segas1x_state>();

	enable = (enable != 0);
	if (enable != state->m_vdp_enable)
	{
		machine.primary_screen->update_partial(machine.primary_screen->vpos());
		state->m_vdp_enable = enable;
#if DEBUG_VDP
		mame_printf_debug("VDP enable = %02X\n", enable);
#endif
	}
}


void system18_set_vdp_mixing(running_machine &machine, int mixing)
{
	segas1x_state *state = machine.driver_data<segas1x_state>();

	if (mixing != state->m_vdp_mixing)
	{
		machine.primary_screen->update_partial(machine.primary_screen->vpos());
		state->m_vdp_mixing = mixing;
#if DEBUG_VDP
		mame_printf_debug("VDP mixing = %02X\n", mixing);
#endif
	}
}



/*************************************
 *
 *  VDP drawing
 *
 *************************************/

static void draw_vdp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority)
{
	segas1x_state *state = screen.machine().driver_data<segas1x_state>();
	int x, y;
	bitmap_ind8 &priority_bitmap = screen.machine().priority_bitmap;

	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT16 *src = &state->m_tmp_bitmap->pix16(y);
		UINT16 *dst = &bitmap.pix16(y);
		UINT8 *pri = &priority_bitmap.pix8(y);

		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			UINT16 pix = src[x];
			if (pix != 0xffff)
			{
				dst[x] = pix;
				pri[x] |= priority;
			}
		}
	}
}



/*************************************
 *
 *  Video update
 *
 *************************************/

SCREEN_UPDATE_IND16( system18 )
{
	segas1x_state *state = screen.machine().driver_data<segas1x_state>();
	int vdppri, vdplayer;

/*
    Current understanding of VDP mixing:

    mixing = 0x00:
        astorm: layer = 0, pri = 0x00 or 0x01
        lghost: layer = 0, pri = 0x00 or 0x01

    mixing = 0x01:
        ddcrew: layer = 0, pri = 0x00 or 0x01 or 0x02

    mixing = 0x02:
        never seen

    mixing = 0x03:
        ddcrew: layer = 1, pri = 0x00 or 0x01 or 0x02

    mixing = 0x04:
        astorm: layer = 2 or 3, pri = 0x00 or 0x01 or 0x02
        mwalk:  layer = 2 or 3, pri = 0x00 or 0x01 or 0x02

    mixing = 0x05:
        ddcrew: layer = 2, pri = 0x04
        wwally: layer = 2, pri = 0x04

    mixing = 0x06:
        never seen

    mixing = 0x07:
        cltchitr: layer = 1 or 2 or 3, pri = 0x02 or 0x04 or 0x08
        mwalk:    layer = 3, pri = 0x04 or 0x08
*/
	vdplayer = (state->m_vdp_mixing >> 1) & 3;
	vdppri = (state->m_vdp_mixing & 1) ? (1 << vdplayer) : 0;

#if DEBUG_VDP
	if (screen.machine().input().code_pressed(KEYCODE_Q)) vdplayer = 0;
	if (screen.machine().input().code_pressed(KEYCODE_W)) vdplayer = 1;
	if (screen.machine().input().code_pressed(KEYCODE_E)) vdplayer = 2;
	if (screen.machine().input().code_pressed(KEYCODE_R)) vdplayer = 3;
	if (screen.machine().input().code_pressed(KEYCODE_A)) vdppri = 0x00;
	if (screen.machine().input().code_pressed(KEYCODE_S)) vdppri = 0x01;
	if (screen.machine().input().code_pressed(KEYCODE_D)) vdppri = 0x02;
	if (screen.machine().input().code_pressed(KEYCODE_F)) vdppri = 0x04;
	if (screen.machine().input().code_pressed(KEYCODE_G)) vdppri = 0x08;
#endif

	/* if no drawing is happening, fill with black and get out */
	if (!segaic16_display_enable)
	{
		bitmap.fill(get_black_pen(screen.machine()), cliprect);
		return 0;
	}

	/* if the VDP is enabled, update our tmp_bitmap */
	if (state->m_vdp_enable)
		system18_vdp_update(*state->m_tmp_bitmap, cliprect);

	/* reset priorities */
	screen.machine().priority_bitmap.fill(0, cliprect);

	/* draw background opaquely first, not setting any priorities */
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 0 | TILEMAP_DRAW_OPAQUE, 0x00);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 1 | TILEMAP_DRAW_OPAQUE, 0x00);
	if (state->m_vdp_enable && vdplayer == 0) draw_vdp(screen, bitmap, cliprect, vdppri);

	/* draw background again to draw non-transparent pixels over the VDP and set the priority */
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 0, 0x01);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_BACKGROUND, 1, 0x02);
	if (state->m_vdp_enable && vdplayer == 1) draw_vdp(screen, bitmap, cliprect, vdppri);

	/* draw foreground */
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_FOREGROUND, 0, 0x02);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_FOREGROUND, 1, 0x04);
	if (state->m_vdp_enable && vdplayer == 2) draw_vdp(screen, bitmap, cliprect, vdppri);

	/* text layer */
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_TEXT, 0, 0x04);
	segaic16_tilemap_draw(screen, bitmap, cliprect, 0, SEGAIC16_TILEMAP_TEXT, 1, 0x08);
	if (state->m_vdp_enable && vdplayer == 3) draw_vdp(screen, bitmap, cliprect, vdppri);

	/* draw the sprites */
	segaic16_sprites_draw(screen, bitmap, cliprect, 0);

#if DEBUG_VDP
	if (state->m_vdp_enable && screen.machine().input().code_pressed(KEYCODE_V))
	{
		bitmap.fill(get_black_pen(screen.machine()), cliprect);
		update_system18_vdp(bitmap, cliprect);
	}
	if (vdp_enable && screen.machine().input().code_pressed(KEYCODE_B))
	{
		FILE *f = fopen("vdp.bin", "w");
		fwrite(state->m_tmp_bitmap->base, 1, state->m_tmp_bitmap->rowpixels * (state->m_tmp_bitmap->bpp / 8) * state->m_tmp_bitmap->height, f);
		fclose(f);
	}
#endif
	return 0;
}
