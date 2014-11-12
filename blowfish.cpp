/* TODO: test with zero length key */
/* TODO: test with a through z as key and plain text */
/* TODO: make this byte order independent */
#include <memory.h>
#include "blowfish.h"
#include "bf_tab.h"

static UWORD_32bits bf_P[bf_N + 2];
static UWORD_32bits bf_S[4][256];

void Blowfish_encipher(UWORD_32bits *xl, UWORD_32bits *xr)
{
  union aword  Xl;
  union aword  Xr;

  Xl.word = *xl;
  Xr.word = *xr;

  Xl.word ^= bf_P[0];
  ROUND (Xr, Xl, 1);  ROUND (Xl, Xr, 2);
  ROUND (Xr, Xl, 3);  ROUND (Xl, Xr, 4);
  ROUND (Xr, Xl, 5);  ROUND (Xl, Xr, 6);
  ROUND (Xr, Xl, 7);  ROUND (Xl, Xr, 8);
  ROUND (Xr, Xl, 9);  ROUND (Xl, Xr, 10);
  ROUND (Xr, Xl, 11); ROUND (Xl, Xr, 12);
  ROUND (Xr, Xl, 13); ROUND (Xl, Xr, 14);
  ROUND (Xr, Xl, 15); ROUND (Xl, Xr, 16);
  Xr.word ^= bf_P[17];

  *xr = Xl.word;
  *xl = Xr.word;
}

void Blowfish_decipher(UWORD_32bits *xl, UWORD_32bits *xr)
{
   union aword  Xl;
   union aword  Xr;

   Xl.word = *xl;
   Xr.word = *xr;

   Xl.word ^= bf_P[17];
   ROUND (Xr, Xl, 16);  ROUND (Xl, Xr, 15);
   ROUND (Xr, Xl, 14);  ROUND (Xl, Xr, 13);
   ROUND (Xr, Xl, 12);  ROUND (Xl, Xr, 11);
   ROUND (Xr, Xl, 10);  ROUND (Xl, Xr, 9);
   ROUND (Xr, Xl, 8);   ROUND (Xl, Xr, 7);
   ROUND (Xr, Xl, 6);   ROUND (Xl, Xr, 5);
   ROUND (Xr, Xl, 4);   ROUND (Xl, Xr, 3);
   ROUND (Xr, Xl, 2);   ROUND (Xl, Xr, 1);
   Xr.word ^= bf_P[0];

   *xl = Xr.word;
   *xr = Xl.word;
}

void __stdcall Version(LPSTR lpszBuffer)
{
	char szBuffer[] = "Blowfish(16 round)  Version 1.0.0";

	strcpy(lpszBuffer, szBuffer);
}

short __stdcall BlowfishInitialize(UBYTE_08bits bKey[], short nKey)
{
  UWORD_32bits  data;
  UWORD_32bits  datal;
  UWORD_32bits  datar;
  union aword temp;

  if(nKey > MAXKEYBYTES)
	  return 0;

  memcpy(bf_P, P, sizeof(P));
  memcpy(bf_S, S, sizeof(S));

  unsigned j = 0;
  for (unsigned i = 0; i < bf_N + 2; ++i) {
    temp.word = 0;
    temp.w.byte0 = bKey[j];
    temp.w.byte1 = bKey[(j+1)%nKey];
    temp.w.byte2 = bKey[(j+2)%nKey];
    temp.w.byte3 = bKey[(j+3)%nKey];
    data = temp.word;
    bf_P[i] = bf_P[i] ^ data;
    j = (j + 4) % nKey;
  }

  datal = 0x00000000;
  datar = 0x00000000;

  for (i = 0; i < bf_N + 2; i += 2) {
    Blowfish_encipher(&datal, &datar);

    bf_P[i] = datal;
    bf_P[i + 1] = datar;
  }

  for (i = 0; i < 4; ++i) {
    for (j = 0; j < 256; j += 2) {

      Blowfish_encipher(&datal, &datar);
   
      bf_S[i][j] = datal;
      bf_S[i][j + 1] = datar;
    }
  }
  return 1;
}

short __stdcall Encipher(UBYTE_08bits bByte[], long nBytes)
{
	if(nBytes % 8)
		return 0;

	for(long i = 0; i < nBytes; i += 8)
		Blowfish_encipher((UWORD_32bits *)&bByte[i], (UWORD_32bits *)&bByte[i + 4]);

	return 1;
}

short __stdcall Decipher(UBYTE_08bits bByte[], long nBytes)
{
	if(nBytes % 8)
		return 0;

	for(long i = 0; i < nBytes; i += 8)
		Blowfish_decipher((UWORD_32bits *)&bByte[i], (UWORD_32bits *)&bByte[i + 4]);

	return 1;
}
