void gticlub_led_setreg(int offset, UINT8 data);

void K001005_draw(bitmap_rgb32 &bitmap, const rectangle &cliprect);
void K001005_swap_buffers(running_machine &machine);
void K001005_init(running_machine &machine);
void K001005_preprocess_texture_data(UINT8 *rom, int length, int gticlub);

READ32_HANDLER(K001005_r);
WRITE32_HANDLER(K001005_w);

void K001006_init(running_machine &machine);
READ32_HANDLER(K001006_0_r);
WRITE32_HANDLER(K001006_0_w);
READ32_HANDLER(K001006_1_r);
WRITE32_HANDLER(K001006_1_w);

VIDEO_START( gticlub );
SCREEN_UPDATE_RGB32( gticlub );
SCREEN_UPDATE_RGB32( hangplt );
