/*************************************************************************

    Diet Go Go

*************************************************************************/

class dietgo_state : public driver_device
{
public:
	dietgo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_pf1_rowscroll(*this, "pf1_rowscroll"),
		m_pf2_rowscroll(*this, "pf2_rowscroll"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_pf1_rowscroll;
	required_shared_ptr<UINT16> m_pf2_rowscroll;
	required_shared_ptr<UINT16> m_spriteram;
//  UINT16 *  m_paletteram;    // currently this uses generic palette handling (in decocomn.c)

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_deco_tilegen1;
};



/*----------- defined in video/dietgo.c -----------*/

SCREEN_UPDATE_IND16( dietgo );
