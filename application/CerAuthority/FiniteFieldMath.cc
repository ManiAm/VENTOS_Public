
#include <FiniteFieldMath.h>

namespace VENTOS {

/* Add two numbers in a GF(2^8) finite field */
uint8_t FiniteFieldMath::gadd(uint8_t a, uint8_t b)
{
    return a ^ b;
}


/* Subtract two numbers in a GF(2^8) finite field */
uint8_t FiniteFieldMath::gsub(uint8_t a, uint8_t b)
{
    return a ^ b;
}


/* Multiply two numbers in the GF(2^8) finite field defined
   by the polynomial x^8 + x^4 + x^3 + x^2 + 1, hexadecimal 0x11d.*/
uint8_t FiniteFieldMath::gmul(uint8_t a, uint8_t b)
{
    uint8_t p = 0;
    uint8_t counter;
    uint8_t hi_bit_set;

    for (counter = 0; counter < 8; counter++)
    {
        if (b & 1) p ^= a;
        hi_bit_set = (a & 0x80);
        a <<= 1;
        if (hi_bit_set) a ^= 0x11D;
        b >>= 1;
    }

    return p;
}


uint8_t FiniteFieldMath::gpow(uint8_t a, uint8_t e)
{
    uint8_t tmp = 1;

    for(int i=1; i<=e; i++)
        tmp = gmul(a,tmp);

    return tmp;
}

}

