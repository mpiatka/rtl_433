/** @file
    Solight DY01 remote controlled socket

    Copyright (c) 2019 Martin Piatka

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#include "decoder.h"

static int get_bit(uint8_t *data, int n){
    uint8_t res = data[n/8] & (0x80 >> (n % 8));

    return res != 0;
}

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

    //Check zero bits between dip bits
    for(int i = 0; i < 5; i++){
        if(get_bit(b, (i * 2) + 1))
            return DECODE_FAIL_SANITY;
    }

    //Check one bits between outlet bits
    for(int i = 0; i < 6; i++){
        if(!get_bit(b, (i * 2) + 10))
            return DECODE_FAIL_SANITY;
    }

    //Get dip switches
    for(int i = 0; i < 5; i++){
        dip[i] = !get_bit(b, i * 2);
    }

    //Get outlet
    for(int i = 0; i < 5; i++){
        if(get_bit(b, (i * 2) + 11)){
            if(outlet != -1)
                return DECODE_FAIL_SANITY;

            outlet = i;
        }
    }

    state = get_bit(b, 23);
    if(get_bit(b, 21) == state)
        return DECODE_FAIL_SANITY;

    /* clang-format off */
    data = data_make(
            "model",   "",        DATA_STRING,  _X("Solight-DY01","Solight-DY01"),
            "dip",     "Dip",     DATA_ARRAY,   data_array(5, DATA_INT, dip),
            "outlet",  "Outlet",  DATA_INT,     outlet,
            "state",   "State",   DATA_INT,     state,
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
