/** @file
    Solight DY01 remote controlled socket

    Copyright (c) 2019 Martin Piatka

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "decoder.h"

static int solight_dy01_callback(r_device *decoder, bitbuffer_t *bitbuffer)
{
    int row;
    data_t *data;
    uint8_t *b;
    int dip[5] = {0};
    int outlet = -1;
    int state = -1;

    row = bitbuffer_find_repeated_row(bitbuffer, 2, 24); // expected are 4 rows, require 2
    if (row < 0)
        return DECODE_ABORT_EARLY;

    b = bitbuffer->bb[row];

    for(int i = 0; i < 5; i++){
    	dip[i] = (b[i/4] & (0xC0 >> ((i % 4) * 2))) ? 0 : 1;
    }

    for(int i = 0; i < 5; i++){
    	if(b[1 + i/3] & (0x40 >> (((i % 3) + 1 - i/3) * 2))){
		outlet = i;
	}
    }

    state = (b[2] & 0x01) ? 1 : 0;

    /* clang-format off */
    data = data_make(
            "model",            "",                 DATA_STRING,    _X("Solight-DY01","Solight-DY01"),
            "dip",               "Dip",             DATA_ARRAY, data_array(5, DATA_INT, dip),
            "outlet",       "Outlet",          DATA_INT,       outlet,
            "state",    "State",      DATA_INT,    state,
            NULL);
    /* clang-format on */

    decoder_output_data(decoder, data);
    return 1;

}

static char *output_fields[] = {
        "model",
        "dip",
	"outlet",
	"state",
        NULL,
};

r_device solight_dy01 = {
        .name        = "Solight DY01 remote outlet",
        .modulation  = OOK_PULSE_PPM,
        .short_width = 136,
        .long_width  = 428,
        .gap_limit   = 452,
        .reset_limit = 4516,
        .decode_fn   = &solight_dy01_callback,
        .disabled    = 0,
        .fields      = output_fields,
};
