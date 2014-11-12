#include "windows.h"

#define MAXKEYBYTES 56	/* 448 bits */
#define bf_N			16
#define noErr           0
#define DATAERROR		-1
#define KEYBYTES		8

#define UWORD_32bits	unsigned long
#define UWORD_16bits	unsigned short
#define UBYTE_08bits	unsigned char

#define ORDER_DCBA		/* Intel byte ordering.

/* choose a byte order for your hardware */

/* ABCD - big endian - motorola */
#ifdef ORDER_ABCD
union aword {
  UWORD_32bits word;
  UBYTE_08bits byte [4];
  struct {
    unsigned int byte0:8;
    unsigned int byte1:8;
    unsigned int byte2:8;
    unsigned int byte3:8;
  } w;
};
#endif	/* ORDER_ABCD */

/* DCBA - little endian - intel */
#ifdef ORDER_DCBA
union aword {
  UWORD_32bits word;
  UBYTE_08bits byte [4];
  struct {
    unsigned int byte3:8;
    unsigned int byte2:8;
    unsigned int byte1:8;
    unsigned int byte0:8;
  } w;
};
#endif	/* ORDER_DCBA */

/* BADC - vax */
#ifdef ORDER_BADC
union aword {
  UWORD_32bits word;
  UBYTE_08bits byte [4];
  struct {
    unsigned int byte1:8;
    unsigned int byte0:8;
    unsigned int byte3:8;
    unsigned int byte2:8;
  } w;
};
#endif	/* ORDER_BADC */

#define S(x,i) (bf_S[i][x.w.byte##i])
#define bf_F(x) (((S(x,0) + S(x,1)) ^ S(x,2)) + S(x,3))
#define ROUND(a,b,n) (a.word ^= bf_F(b) ^ bf_P[n])

void Blowfish_encipher(UWORD_32bits *xl, UWORD_32bits *xr);
void Blowfish_decipher(UWORD_32bits *xl, UWORD_32bits *xr);

extern "C" __declspec(dllexport) void __stdcall Version(LPSTR lpszBuffer);
extern "C" __declspec(dllexport) short __stdcall BlowfishInitialize(UBYTE_08bits bKey[], short nKey);

extern "C" __declspec(dllexport) short __stdcall Encipher(UBYTE_08bits bByte[], long nBytes);
extern "C" __declspec(dllexport) short __stdcall Decipher(UBYTE_08bits bByte[], long nBytes);
