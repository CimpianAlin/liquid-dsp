/*
 * Copyright (c) 2007 - 2014 Joseph Gaeddert
 *
 * This file is part of liquid.
 *
 * liquid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * liquid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with liquid.  If not, see <http://www.gnu.org/licenses/>.
 */

//
// modem_ask.c
//

// create an ask (amplitude-shift keying) modem object
MODEM() MODEM(_create_ask)(unsigned int _bits_per_symbol)
{
    MODEM() q = (MODEM()) malloc( sizeof(struct MODEM(_s)) );

    MODEM(_init)(q, _bits_per_symbol);

    switch (q->M) {
    case 2:     q->data.ask.alpha = ASK2_ALPHA;   q->scheme = LIQUID_MODEM_ASK2;   break;
    case 4:     q->data.ask.alpha = ASK4_ALPHA;   q->scheme = LIQUID_MODEM_ASK4;   break;
    case 8:     q->data.ask.alpha = ASK8_ALPHA;   q->scheme = LIQUID_MODEM_ASK8;   break;
    case 16:    q->data.ask.alpha = ASK16_ALPHA;  q->scheme = LIQUID_MODEM_ASK16;  break;
    case 32:    q->data.ask.alpha = ASK32_ALPHA;  q->scheme = LIQUID_MODEM_ASK32;  break;
    case 64:    q->data.ask.alpha = ASK64_ALPHA;  q->scheme = LIQUID_MODEM_ASK64;  break;
    case 128:   q->data.ask.alpha = ASK128_ALPHA; q->scheme = LIQUID_MODEM_ASK128; break;
    case 256:   q->data.ask.alpha = ASK256_ALPHA; q->scheme = LIQUID_MODEM_ASK256; break;
    default:
#if 0
        // calculate alpha dynamically
        q->data.ask.alpha = expf(-0.70735 + 0.63653*q->m);
#else
        fprintf(stderr,"error: modem_create_ask(), cannot support ASK with m > 8\n");
        exit(1);
#endif
    }

    unsigned int k;
    for (k=0; k<(q->m); k++)
        q->ref[k] = (1<<k) * q->data.ask.alpha;

    q->modulate_func = &MODEM(_modulate_ask);
    q->demodulate_func = &MODEM(_demodulate_ask);

    // initialize soft-demodulation look-up table
    if (q->m >= 2 && q->m < 8)
        MODEM(_demodsoft_gentab)(q, 2);

    // reset modem and return
    MODEM(_reset)(q);
    return q;
}

// modulate ASK
void MODEM(_modulate_ask)(MODEM()      _q,
                          unsigned int _sym_in,
                          TC *         _y)
{
    // 'encode' input symbol (actually gray decoding)
    _sym_in = gray_decode(_sym_in);

    // modulate symbol
    *_y = (2*(int)_sym_in - (int)(_q->M) + 1) * _q->data.ask.alpha;
}

// demodulate ASK
void MODEM(_demodulate_ask)(MODEM()        _q,
                            TC             _x,
                            unsigned int * _sym_out)
{
    // demodulate on linearly-spaced array
    unsigned int s;
    T res_i;
    MODEM(_demodulate_linear_array_ref)(crealf(_x), _q->m, _q->ref, &s, &res_i);

    // 'decode' output symbol (actually gray encoding)
    *_sym_out = gray_encode(s);

    // re-modulate symbol and store state
    MODEM(_modulate_ask)(_q, *_sym_out, &_q->x_hat);
    _q->r = _x;
}

