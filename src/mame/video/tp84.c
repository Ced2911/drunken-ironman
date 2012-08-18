/***************************************************************************

    video.c

    Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "includes/tp84.h"





/*
-The colortable is divided in 2 part:
 -The characters colors
 -The sprites colors

-The characters colors are indexed like this:
 -2 bits from the characters
 -4 bits from the attribute in tp84_bg_colorram
 -2 bits from palette_bank (d3-d4)
 -3 bits from palette_bank (d0-d1-d2)
-So, there is 2048 bytes for the characters

-The sprites colors are indexed like this:
 -4 bits from the sprites (16 colors)
 -4 bits from the attribute of the sprites
 -3 bits from palette_bank (d0-d1-d2)
-So, there is 2048 bytes for the sprites

*/
/*
     The RGB signals are generated by 3 proms 256X4 (prom 2C, 2D and 1E)
        The resistors values are:
            1K  ohm
            470 ohm
            220 ohm
            100 ohm
*/
PALETTE_INIT( tp84 )
{
	const UINT8 *color_prom = machine.root_device().memregion("proms")->base();
	static const int resistances[4] = { 1000, 470, 220, 100 };
	double weights[4];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0,	255, -1.0,
			4, resistances, weights, 470, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0);

	/* allocate the colortable */
	machine.colortable = colortable_alloc(machine, 0x100);

	/* create a lookup table for the palette */
	for (i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2, bit3;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i + 0x000] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x000] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x000] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x000] >> 3) & 0x01;
		r = combine_4_weights(weights, bit0, bit1, bit2, bit3);

		/* green component */
		bit0 = (color_prom[i + 0x100] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x100] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x100] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x100] >> 3) & 0x01;
		g = combine_4_weights(weights, bit0, bit1, bit2, bit3);

		/* blue component */
		bit0 = (color_prom[i + 0x200] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x200] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x200] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x200] >> 3) & 0x01;
		b = combine_4_weights(weights, bit0, bit1, bit2, bit3);

		colortable_palette_set_color(machine.colortable, i, MAKE_RGB(r, g, b));
	}

	/* color_prom now points to the beginning of the lookup table */
	color_prom += 0x300;

	/* characters use colors 0x80-0xff, sprites use colors 0-0x7f */
	for (i = 0; i < 0x200; i++)
	{
		int j;

		for (j = 0; j < 8; j++)
		{
			UINT8 ctabentry = ((~i & 0x100) >> 1) | (j << 4) | (color_prom[i] & 0x0f);
			colortable_entry_set_value(machine.colortable, ((i & 0x100) << 3) | (j << 8) | (i & 0xff), ctabentry);
		}
	}
}


WRITE8_MEMBER(tp84_state::tp84_spriteram_w)
{
	/* the game multiplexes the sprites, so update now */
	machine().primary_screen->update_now();
	m_spriteram[offset] = data;
}


READ8_MEMBER(tp84_state::tp84_scanline_r)
{
	/* reads 1V - 128V */
	return machine().primary_screen->vpos();
}


static TILE_GET_INFO( get_bg_tile_info )
{
	tp84_state *state = machine.driver_data<tp84_state>();
	int code = ((state->m_bg_colorram[tile_index] & 0x30) << 4) | state->m_bg_videoram[tile_index];
	int color = ((*state->m_palette_bank & 0x07) << 6) |
				((*state->m_palette_bank & 0x18) << 1) |
				(state->m_bg_colorram[tile_index] & 0x0f);
	int flags = TILE_FLIPYX(state->m_bg_colorram[tile_index] >> 6);

	SET_TILE_INFO(0, code, color, flags);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	tp84_state *state = machine.driver_data<tp84_state>();
	int code = ((state->m_fg_colorram[tile_index] & 0x30) << 4) | state->m_fg_videoram[tile_index];
	int color = ((*state->m_palette_bank & 0x07) << 6) |
				((*state->m_palette_bank & 0x18) << 1) |
				(state->m_fg_colorram[tile_index] & 0x0f);
	int flags = TILE_FLIPYX(state->m_fg_colorram[tile_index] >> 6);

	SET_TILE_INFO(0, code, color, flags);
}


VIDEO_START( tp84 )
{
	tp84_state *state = machine.driver_data<tp84_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info, tilemap_scan_rows, 8, 8, 32, 32);
}


static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	tp84_state *state = machine.driver_data<tp84_state>();
	int offs;
	int palette_base = ((*state->m_palette_bank & 0x07) << 4);

	for (offs = 0x5c; offs >= 0; offs -= 4)
	{
		int x = state->m_spriteram[offs];
		int y = 240 - state->m_spriteram[offs + 3];

		int code = state->m_spriteram[offs + 1];
		int color = palette_base | (state->m_spriteram[offs + 2] & 0x0f);
		int flip_x = ~state->m_spriteram[offs + 2] & 0x40;
		int flip_y =  state->m_spriteram[offs + 2] & 0x80;

		drawgfx_transmask(bitmap, cliprect, machine.gfx[1], code, color, flip_x, flip_y, x, y,
				colortable_get_transpen_mask(machine.colortable, machine.gfx[1], color, palette_base));

	}
}


SCREEN_UPDATE_IND16( tp84 )
{
	tp84_state *state = screen.machine().driver_data<tp84_state>();
	rectangle clip = cliprect;
	const rectangle &visarea = screen.visible_area();

	if (cliprect.min_y == screen.visible_area().min_y)
	{
		screen.machine().tilemap().mark_all_dirty();

		state->m_bg_tilemap->set_scrollx(0, *state->m_scroll_x);
		state->m_bg_tilemap->set_scrolly(0, *state->m_scroll_y);

		screen.machine().tilemap().set_flip_all(((*state->m_flipscreen_x & 0x01) ? TILEMAP_FLIPX : 0) |
									   ((*state->m_flipscreen_y & 0x01) ? TILEMAP_FLIPY : 0));
	}

	state->m_bg_tilemap->draw(bitmap, cliprect, 0, 0);
	draw_sprites(screen.machine(), bitmap, cliprect);

	/* draw top status region */
	clip.min_x = visarea.min_x;
	clip.max_x = visarea.min_x + 15;
	state->m_fg_tilemap->draw(bitmap, clip, 0, 0);

	/* draw bottom status region */
	clip.min_x = visarea.max_x - 15;
	clip.max_x = visarea.max_x;
	state->m_fg_tilemap->draw(bitmap, clip, 0, 0);

	return 0;
}