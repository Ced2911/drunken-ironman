/***************************************************************************

    Copyright Olivier Galibert
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/

#include "emu.h"
#include "mfi_dsk.h"
#include <zlib.h>

/*
  Mess floppy image structure:

  - header with signature, number of cylinders, number of heads.  Min
    track and min head are considered to always be 0.

  - vector of track descriptions, looping on cylinders and sub-lopping
    on heads, each description composed of:
    - offset of the track data in bytes from the start of the file
    - size of the compressed track data in bytes (0 for unformatted)
    - size of the uncompressed track data in bytes (0 for unformatted)

  - track data

  All values are 32-bits lsb first.

  Track data is zlib-compressed independently for each track using the
  simple "compress" function.

  Track data consists of a series of 32-bits lsb-first values
  representing magnetic cells.  Bits 0-27 indicate the sizes, and bits
  28-31 the types.  Type can be:
  - 0, MG_A -> Magnetic orientation A
  - 1, MG_B -> Magnetic orientation B
  - 2, MG_N -> Non-magnetized zone (neutral)
  - 3, MG_D -> Damaged zone, reads as neutral but cannot be changed by writing

  Remember that the fdcs detect transitions, not absolute levels, so
  the actual physical significance of the orientation A and B is
  arbitrary.

  Tracks data is aligned so that the index pulse is at the start,
  whether the disk is hard-sectored or not.

  The size is the angular size in units of 1/200,000,000th of a turn.
  Such a size, not coincidentally at all, is also the flyover time in
  nanoseconds for a perfectly stable 300rpm drive.  That makes the
  standard cell size of a MFM 3.5" DD floppy at 2000 exactly for
  instance (2us).  Smallest expected cell size is 500 (ED density
  drives).

  The sum of all sizes must of course be 200,000,000.

  An unformatted track is equivalent to one big MG_N cell covering a
  whole turn, but is encoded as zero-size.

  The "track splice" information indicates where to start writing
  if you try to rewrite a physical disk with the data.  Some
  preservation formats encode that information, it is guessed for
  others.  The write track function of fdcs should set it.  The
  representation is the angular position relative to the index.

  The media type is divided in two parts.  The first half
  indicate the physical form factor, i.e. all medias with that
  form factor can be physically inserted in a reader that handles
  it.  The second half indicates the variants which are usually
  detectable by the reader, such as density and number of sides.

  TODO: big-endian support
*/

const char mfi_format::sign[16] = "MESSFLOPPYIMAGE"; // Includes the final \0

mfi_format::mfi_format() : floppy_image_format_t()
{
}

const char *mfi_format::name() const
{
	return "mfi";
}

const char *mfi_format::description() const
{
	return "MESS floppy image";
}

const char *mfi_format::extensions() const
{
	return "mfi";
}

bool mfi_format::supports_save() const
{
	return true;
}

int mfi_format::identify(io_generic *io, UINT32 form_factor)
{
	header h;

	io_generic_read(io, &h, 0, sizeof(header));
	if(memcmp( h.sign, sign, 16 ) == 0 &&
	   h.cyl_count <= 160 &&
	   h.head_count <= 2 &&
	   (!form_factor || h.form_factor == form_factor))
		return 100;
	return 0;
}

bool mfi_format::load(io_generic *io, UINT32 form_factor, floppy_image *image)
{
	header h;
	entry entries[84*2];
	io_generic_read(io, &h, 0, sizeof(header));
	io_generic_read(io, &entries, sizeof(header), h.cyl_count*h.head_count*sizeof(entry));

	image->set_variant(h.variant);

	UINT8 *compressed = 0;
	int compressed_size = 0;

	entry *ent = entries;
	for(unsigned int cyl=0; cyl != h.cyl_count; cyl++)
		for(unsigned int head=0; head != h.head_count; head++) {
			if(ent->uncompressed_size == 0) {
				// Unformatted track
				image->set_track_size(cyl, head, 0);
				ent++;
				continue;
			}

			if(ent->compressed_size > compressed_size) {
				if(compressed)
					global_free(compressed);
				compressed_size = ent->compressed_size;
				compressed = global_alloc_array(UINT8, compressed_size);
			}

			io_generic_read(io, compressed, ent->offset, ent->compressed_size);

			unsigned int cell_count = ent->uncompressed_size/4;
			image->set_track_size(cyl, head, cell_count);
			UINT32 *trackbuf = image->get_buffer(cyl, head);

			uLongf size = ent->uncompressed_size;
			if(uncompress((Bytef *)trackbuf, &size, compressed, ent->compressed_size) != Z_OK)
				return false;

			UINT32 cur_time = 0;
			for(unsigned int i=0; i != cell_count; i++) {
				UINT32 next_cur_time = cur_time + (trackbuf[i] & TIME_MASK);
				trackbuf[i] = (trackbuf[i] & MG_MASK) | cur_time;
				cur_time = next_cur_time;
			}
			if(cur_time != 200000000)
				return false;

			ent++;
		}

	if(compressed)
		global_free(compressed);

	return true;
}

bool mfi_format::save(io_generic *io, floppy_image *image)
{
	int tracks, heads;
	image->get_actual_geometry(tracks, heads);
	int max_track_size = 0;
	for(int track=0; track<tracks; track++)
		for(int head=0; head<heads; head++) {
			int tsize = image->get_track_size(track, head);
			if(tsize > max_track_size)
				 max_track_size = tsize;
		}

	header h;
	entry entries[84*2];
	memcpy(h.sign, sign, 16);
	h.cyl_count = tracks;
	h.head_count = heads;
	h.form_factor = image->get_form_factor();
	h.variant = image->get_variant();

	io_generic_write(io, &h, 0, sizeof(header));

	memset(entries, 0, sizeof(entries));

	int pos = sizeof(header) + tracks*heads*sizeof(entry);
	int epos = 0;
	UINT32 *precomp = global_alloc_array(UINT32, max_track_size);
	UINT8 *postcomp = global_alloc_array(UINT8, max_track_size*4 + 1000);

	for(int track=0; track<tracks; track++)
		for(int head=0; head<heads; head++) {
			int tsize = image->get_track_size(track, head);
			if(!tsize) {
				epos++;
				continue;
			}

			memcpy(precomp, image->get_buffer(track, head), tsize*4);
			for(int j=0; j<tsize-1; j++)
				precomp[j] = (precomp[j] & floppy_image::MG_MASK) |
					((precomp[j+1] & floppy_image::TIME_MASK) -
					 (precomp[j] & floppy_image::TIME_MASK));
			precomp[tsize-1] = (precomp[tsize-1] & floppy_image::MG_MASK) |
				(200000000 - (precomp[tsize-1] & floppy_image::TIME_MASK));

			uLongf csize = max_track_size*4 + 1000;
			if(compress(postcomp, &csize, (const Bytef *)precomp, tsize*4) != Z_OK)
				return false;

			entries[epos].offset = pos;
			entries[epos].uncompressed_size = tsize*4;
			entries[epos].compressed_size = csize;
			entries[epos].write_splice = image->get_write_splice_position(track, head);
			epos++;

			io_generic_write(io, postcomp, pos, csize);
			pos += csize;
		}

	io_generic_write(io, entries, sizeof(header), tracks*heads*sizeof(entry));
	return true;
}

const floppy_format_type FLOPPY_MFI_FORMAT = &floppy_image_format_creator<mfi_format>;
