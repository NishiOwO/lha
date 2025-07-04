/* ------------------------------------------------------------------------ */
/* LHa for UNIX                                                             */
/*              bitio.c -- bit stream                                       */
/*                                                                          */
/*      Modified                Nobutaka Watazaki                           */
/*                                                                          */
/*  Ver. 1.14   Source All chagned              1995.01.14  N.Watazaki      */
/*              Separated from crcio.c          2002.10.26  Koji Arai       */
/* ------------------------------------------------------------------------ */
#include "lha.h"

static unsigned char subbitbuf, bitcount;

void
fillbuf(n)          /* Shift bitbuf n bits left, read n bits */
    int   n;
{
    while (n > bitcount) {
        n -= bitcount;
        bitbuf = (bitbuf << bitcount) + (subbitbuf >> (CHAR_BIT - bitcount));
        if (compsize != 0) {
            int c;
            compsize--;
            c = getc(infile);
            if (c == EOF) {
                fatal_error("cannot read stream");
            }
            subbitbuf = (unsigned char)c;
        }
        else
            subbitbuf = 0;
        bitcount = CHAR_BIT;
    }
    bitcount -= n;
    bitbuf = (bitbuf << n) + (subbitbuf >> (CHAR_BIT - n));
    subbitbuf <<= n;
}

unsigned short
getbits(n)
    int   n;
{
    unsigned short  x;

    x = bitbuf >> (2 * CHAR_BIT - n);
    fillbuf(n);
    return x;
}

void
putcode(n, x)           /* Write leftmost n bits of x */
    int   n;
    int   x;
{
    while (n >= bitcount) {
        n -= bitcount;
        subbitbuf += (unsigned short)x >> (USHRT_BIT - bitcount);
        x <<= bitcount;
        if (compsize < origsize) {
            if (fwrite(&subbitbuf, 1, 1, outfile) == 0) {
                fatal_error("Write error in bitio.c(putcode)");
            }
            compsize++;
        }
        else
            unpackable = 1;
        subbitbuf = 0;
        bitcount = CHAR_BIT;
    }
    subbitbuf += (unsigned short)x >> (USHRT_BIT - bitcount);
    bitcount -= n;
}

void
putbits(n, x)           /* Write rightmost n bits of x */
    int   n;
    int   x;
{
    x <<= USHRT_BIT - (unsigned short)n;
    putcode(n, x);
}

void
init_getbits( /* void */ )
{
    bitbuf = 0;
    subbitbuf = 0;
    bitcount = 0;
    fillbuf(2 * CHAR_BIT);
}

void
init_putbits( /* void */ )
{
    bitcount = CHAR_BIT;
    subbitbuf = 0;
}
