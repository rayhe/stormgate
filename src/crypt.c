/*$Id: crypt.c,v 1.2 2005/02/22 23:55:16 ahsile Exp $*/

#include "crypt.h"

static void 
perm (a, e, pc, n)
     register char *a, *e;
     register char *pc;
     int n;
{
  for (; n--; pc++, a++)
    *a = e[*pc];
}

static void 
crypt_main (nachr_l, nachr_r, schl)
     register char *nachr_l, *nachr_r;
     register char *schl;
{
  char tmp[KS];
  register int sbval;
  register char *tp = tmp;
  register char *e = EE;
  register int i, j;

  for (i = 0; i < 8; i++)
    {
      for (j = 0, sbval = 0; j < 6; j++)
	sbval = (sbval << 1) | (nachr_r[*e++] ^ *schl++);
      sbval = S_BOX[i][sbval];
      for (tp += 4, j = 4; j--; sbval >>= 1)
	*--tp = sbval & 1;
      tp += 4;
    }

  e = PERM;
  for (i = 0; i < BS2; i++)
    *nachr_l++ ^= tmp[*e++];
}

void 
encrypt (char *nachr, int decr)
{
  char (*schl)[KS] = decr ? schluessel + 15 : schluessel;
  char tmp[BS];
  int i;

  perm (tmp, nachr, IP, BS);

  for (i = 8; i--;)
    {
      crypt_main (tmp, tmp + BS2, *schl);
      if (decr)
	schl--;
      else
	schl++;
      crypt_main (tmp + BS2, tmp, *schl);
      if (decr)
	schl--;
      else
	schl++;
    }

  perm (nachr, tmp, EP, BS);
}

void setkey (char *schl)
{
  char tmp1[IS];
  register unsigned int ls = 0x7efc;
  register int i, j, k;
  register int shval = 0;
  register char *akt_schl;

  memcpy (EE, E0, KS);
  perm (tmp1, schl, PC1, IS);

  for (i = 0; i < 16; i++)
    {
      shval += 1 + (ls & 1);
      akt_schl = schluessel[i];
      for (j = 0; j < KS; j++)
	{
	  if ((k = PC2[j]) >= IS2)
	    {
	      if ((k += shval) >= IS)
		k = (k - IS2) % IS2 + IS2;
	    }
	  else if ((k += shval) >= IS2)
	    k %= IS2;
	  *akt_schl++ = tmp1[k];
	}
      ls >>= 1;
    }
}

char *crypt (const char *pass, const char *salt)
{
  static char retkey[14];
  char key[BS + 2];
  char *k;
  int tmp, keybyte;
  int i, j;

  memset (key, 0, BS + 2);

  for (k = key, i = 0; i < BS; i++)
    {
      if (!(keybyte = *pass++))
	break;
      k += 7;
      for (j = 0; j < 7; j++, i++)
	{
	  *--k = keybyte & 1;
	  keybyte >>= 1;
	}
      k += 8;
    }

  setkey (key);
  memset (key, 0, BS + 2);

  for (k = EE, i = 0; i < 2; i++)
    {
      retkey[i] = keybyte = *salt++;
      if (keybyte > 'Z')
	keybyte -= 'a' - 'Z' - 1;
      if (keybyte > '9')
	keybyte -= 'A' - '9' - 1;
      keybyte -= '.';

      for (j = 0; j < 6; j++, keybyte >>= 1, k++)
	{
	  if (!(keybyte & 1))
	    continue;
	  tmp = *k;
	  *k = k[24];
	  k[24] = tmp;
	}
    }

  for (i = 0; i < 25; i++)
    encrypt (key, 0);

  for (k = key, i = 0; i < 11; i++)
    {
      for (j = keybyte = 0; j < 6; j++)
	{
	  keybyte <<= 1;
	  keybyte |= *k++;
	}

      keybyte += '.';
      if (keybyte > '9')
	keybyte += 'A' - '9' - 1;
      if (keybyte > 'Z')
	keybyte += 'a' - 'Z' - 1;
      retkey[i + 2] = keybyte;
    }

  retkey[i + 2] = 0;

  if (!retkey[1])
    retkey[1] = *retkey;

  return retkey;
}