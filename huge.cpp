#include "huge.h"
#include <string.h>

CSet::CSet()
{
	m_pu = 0;
	m_nSize = 0;
	m_nLast = 0;
}

CSet::~CSet()
{
	unsigned i = m_nSize;

	while(i)
		m_pu[--i] = 0;

	delete [] m_pu;;
}

void CSet::Reserve(unsigned n)
{
	if(n > m_nSize)
	{
		unsigned * pu = new unsigned[n];

		for(unsigned i = 0; i < m_nLast;i++)
			pu[i] = m_pu[i];

		delete [] m_pu;

		m_pu = pu;
		m_nSize = n;
	}
}

void CSet::Set(unsigned i, unsigned n)
{
	if(i < m_nLast)
	{
		m_pu[i] = n;
		if(n == 0)
			Normalise();
	}
	else if( n )
	{
		Reserve(i+1);

		for(unsigned j = m_nLast; j < i; j++)
			m_pu[j] = 0;

		m_pu[i] = n;
		m_nLast = i+1;
	}
}

void CSet::Normalise()
{
	while(m_nLast && !m_pu[m_nLast-1])
			m_nLast--;
}

#define INT_BITS	(8 * sizeof(unsigned))
#define LOW(n)		((n) & ((1<<(INT_BITS / 2)) - 1))
#define HIGH(n)		((n) >> (INT_BITS / 2))
#define SHIFT(n)	((n) << (INT_BITS / 2))

void CSet::FstMul(CSet &rCSet1, CSet &rCSet2, unsigned nKeep)
{
	unsigned i;
	unsigned nWords = (nKeep + INT_BITS - 1) / INT_BITS;

	Reserve(nWords);
	for(i = 0; i < nWords; i++)
		m_pu[i] = 0;

	unsigned nMin = rCSet1.m_nLast;
	if(nMin > nWords)
		nMin = nWords;

	for(i = 0; i < nMin; i++)
	{
		unsigned m = rCSet1.m_pu[i];
		unsigned c = 0;

		unsigned nMin = i + rCSet2.m_nLast;
		if(nMin > nWords)
			nMin = nWords;

		for(unsigned j = i; j < nMin; j++)
		{
			unsigned w;
			unsigned v = m_pu[j];
			unsigned p = rCSet2.m_pu[j-i];

			v += c;
			c = ( v < c );

			w = LOW(p)*LOW(m);
			v += w;
			c += ( v < w );

			w = LOW(p)*HIGH(m);
			c += HIGH(w);
			w = SHIFT(w);
			v += w;
			c += ( v < w );

			w = HIGH(p)*LOW(m);
			c += HIGH(w);
			w = SHIFT(w);
			v += w;
			c += ( v < w );

			c += HIGH(p) * HIGH(m);
			m_pu[j] = v;
		}

		while (c && j < nWords)
		{
			m_pu[j] += c;
			c = m_pu[j] < c;
			j++;
		}
	}

	// eliminate unwanted Bits
	nKeep %= INT_BITS;
	if(nKeep)
		m_pu[nWords-1] &= (1 << nKeep)-1;

	m_nLast = nWords;
	Normalise();
}

CInt::CInt()
{
	m_nShare = 0;
}

CInt::operator unsigned()
{
	return Get(0);
}

int CInt::BitTest(unsigned i) const
{
	return (Get(i/INT_BITS) & (1<<(i%INT_BITS))) != 0;
}

unsigned CInt::Bits() const
{
	unsigned n = m_nLast * INT_BITS;

	while(n && BitTest(n-1)==0)
			n--;

	return n;
}

int CInt::Comp(CInt& rCInt) const
{
	if(m_nLast > rCInt.m_nLast)
		return +1;

	if(m_nLast < rCInt.m_nLast)
		return -1;

	unsigned i = m_nLast;

	while (i)
	{
		i--;
		if(Get(i) > rCInt.Get(i))
			return +1;

		if(Get(i) < rCInt.Get(i))
			return -1;
	}

	return 0;
}

void CInt::ShiftL()
{
	unsigned c = 0;
	unsigned N = m_nLast;

	for(unsigned i = 0; i <= N; i++)
	{
		unsigned u = Get(i);

		Set(i,(u<<1) + c);
		c = u>>(INT_BITS - 1);
	}
}

void CInt::ShiftR()
{
	unsigned c = 0;
	unsigned i = m_nLast;

	while (i)
	{
		i--;
		unsigned u = Get(i);

		Set(i,(u >> 1) + c);
		c = u << (INT_BITS - 1);
	}
}

void CInt::ShiftR(unsigned n)
{
	unsigned d = n / INT_BITS; n %= INT_BITS;

	for(unsigned i = 0; i < m_nLast;i++)
	{
		unsigned u = Get(i + d);
		if(n)
		{
			u >>= n;
			u += Get(i + d + 1) << (INT_BITS - n);
		}

		Set(i,u);
	}
}

void CInt::Init(unsigned n)
{
	Clear();
	Set(0, n);
}

void CInt::Copy(CInt& rCInt)
{
	Clear();
	unsigned i = rCInt.m_nLast;

	while(i)
	{
		i--;
		Set(i, rCInt.Get(i));
	}
}

void CInt::Add(CInt& rCInt)
{
	unsigned c = 0;
	unsigned n = m_nLast;

	if(n < rCInt.m_nLast)
		n = rCInt.m_nLast;

	Reserve(n);

	for(unsigned i = 0; i < n + 1; i++)
	{
		unsigned u = Get(i);
		u = u + c;
		c = (u < c);

		unsigned ux = rCInt.Get(i);
		u = u + ux;
		c += ( u < ux );
		Set(i,u);
	}
}

void CInt::Sub(CInt & rCInt)
{
	unsigned c = 0;
	unsigned N = m_nLast;

	for(unsigned i = 0; i < N; i++)
	{
		unsigned ux = rCInt.Get(i);
		ux += c;

		if( ux >= c)
		{
			unsigned u = Get(i);
			unsigned nu = u - ux;

			c = nu > u;
			Set(i,nu);
		}
  }
}

void CInt::Mul(CInt& rCInt1, CInt& rCInt2)
{
	FstMul( rCInt1, rCInt2, rCInt1.Bits() + rCInt2.Bits() );
}

void CInt::Div(CInt& rCInt1, CInt& rCInt2, CInt& rCInt3 )
{
	Init(0);
	rCInt3.Copy(rCInt1);

	CInt m,s;
	m.Copy(rCInt2);
	s.Init(1);

	while(rCInt3.Comp(m) > 0)
	{
		m.ShiftL();
		s.ShiftL();
	}

	while (rCInt3.Comp(rCInt2) >= 0)
	{
		while ( rCInt3.Comp(m) < 0 )
		{
			m.ShiftR();
			s.ShiftR();
		}

		rCInt3.Sub(m);
		Add(s);
	}
}

void CHuge::DoCopy()
{
	if(m_pCInt->m_nShare)
	{
		m_pCInt->m_nShare -= 1;
		CInt *pCInt = new CInt;

		pCInt->Copy(*m_pCInt);
		m_pCInt = pCInt;
	}
}

int CHuge::Comp(const CHuge rCHuge) const
{
	int nSign = m_nSign && !m_pCInt->Is0();

	if(nSign == (rCHuge.m_nSign && !rCHuge.m_pCInt->Is0()))
		return m_pCInt->Comp(*rCHuge.m_pCInt);

	else if(nSign)
		return -1;

	else
		return +1;
}

CHuge::CHuge (int n)
{
	m_pCInt = new CInt;
	m_nSign = 0;

	m_pCInt->Init(n);
}

CHuge::CHuge (const CHuge& rCHuge)
{
	m_nSign = rCHuge.m_nSign;
	m_pCInt = rCHuge.m_pCInt;
	m_pCInt->m_nShare++;
}


CHuge::CHuge (const char *pstrzHuge)
{
	CInt rHot10;
	rHot10.Init(10);

	CInt rDigit;
	rDigit.Init(0);

	
	CInt rTmp;
	rTmp.Init(0);

	m_pCInt = new CInt;
	m_nSign = 0;

	m_pCInt->Init(0);

	if(*pstrzHuge == '-')
	{
		pstrzHuge++;
		m_nSign = 1;
	}

	while(*pstrzHuge)
	{
		rTmp.Mul(*m_pCInt, rHot10);

		rDigit.Init(*pstrzHuge - '0');
		rTmp.Add(rDigit);

		m_pCInt->Copy(rTmp);

		pstrzHuge++;
	}
}

CHuge& CHuge::operator =(const CHuge& rCHuge)
{
	if(m_pCInt->m_nShare)
		m_pCInt->m_nShare--;
	else
		delete m_pCInt;

	m_pCInt = rCHuge.m_pCInt;
	m_pCInt->m_nShare++;
	m_nSign = rCHuge.m_nSign;

	return *this;
}

CHuge::~CHuge()
{
	if(m_pCInt->m_nShare)
		m_pCInt->m_nShare--;
	else
		delete m_pCInt;
}

CHuge& CHuge::operator +=(const CHuge& rCHuge)
{
	if(m_nSign == rCHuge.m_nSign)
	{
		DoCopy();
		m_pCInt->Add(*rCHuge.m_pCInt);
	}
	else if(m_pCInt->Comp(*rCHuge.m_pCInt) >= 0)
	{
		DoCopy();
		m_pCInt->Sub(*rCHuge.m_pCInt);
	}
	else
	{
		CHuge rTmp = *this;
		*this = rCHuge;
		*this += rTmp;
	}

	return *this;
}

CHuge& CHuge::operator -=(const CHuge& rCHuge)
{
	if(m_nSign != rCHuge.m_nSign)
	{
		DoCopy();
		m_pCInt->Add(*rCHuge.m_pCInt);
	}
	else if(m_pCInt->Comp(*rCHuge.m_pCInt) >= 0 )
	{
		DoCopy();
		m_pCInt->Sub(*rCHuge.m_pCInt);
	}
	else
	{
		CHuge rTmp = *this;
		*this = rCHuge;
		*this -= rTmp;
		m_nSign= 1 - m_nSign;
	}

	return *this;
}

CHuge operator +(const CHuge& rCHuge1, const CHuge& rCHuge2)
{
	CHuge rTmp = rCHuge1;
	rTmp += rCHuge2;

	return rTmp;
}

CHuge operator -(const CHuge& rCHuge1, const CHuge& rCHuge2)
{
	CHuge rTmp = rCHuge1;
	rTmp -= rCHuge2;

	return rTmp;
}

CHuge operator *(const CHuge& rCHuge1, const CHuge& rCHuge2)
{
	CHuge rTmp;

	rTmp.m_pCInt->Mul(*rCHuge1.m_pCInt, *rCHuge2.m_pCInt);
	rTmp.m_nSign = rCHuge1.m_nSign ^ rCHuge2.m_nSign;

	return rTmp;
}

CHuge operator /(const CHuge& rCHuge1, const CHuge& rCHuge2)
{
	CHuge rTmp;

	CInt rRem;
	rTmp.m_pCInt->Div(*rCHuge1.m_pCInt, *rCHuge2.m_pCInt, rRem);
	rTmp.m_nSign = rCHuge1.m_nSign ^ rCHuge2.m_nSign;

	return rTmp;
}

CHuge operator %(const CHuge& rCHuge1, const CHuge& rCHuge2)
{
	CHuge rTmp;

	CInt rDiv;
	rDiv.Div(*rCHuge1.m_pCInt, *rCHuge2.m_pCInt, *rTmp.m_pCInt);
	rTmp.m_nSign = rCHuge1.m_nSign;

	return rTmp;
}

int CHuge::Int()
{
	return m_pCInt->Get(0);
}

void CHuge::String(char *pstrzBuffer)
{
	char *pstrzTmp = pstrzBuffer;
	CHuge rTmp = Abs(*this);
	CHuge rRem;

	do
	{
		rRem = rTmp % 10;
		rTmp = rTmp / 10;
		*pstrzTmp++ = (char)(rRem.Int() + '0');
	}while(rTmp > 0);

	if(m_nSign)
		*pstrzTmp++ = '-';

	*pstrzTmp = '\0';

	strrev(pstrzBuffer);
}

CHuge Gcd(const CHuge& rCHuge1, const CHuge& rCHuge2)
{
	CHuge n1 = rCHuge1;
	CHuge n2 = rCHuge2;

	for(;;)
	{
		if(n2 == 0)
			return n1;

		n1 = n1 % n2;

		if(n1 == 0)
			return n2;

		n2 = n2 % n1;
	}
}

CHuge ModInv(const CHuge &rCHuge1, const CHuge &m)
{
	CHuge j = 1;
	CHuge i = 0;
	CHuge b = m;
	CHuge c = rCHuge1;
	CHuge x;
	CHuge y;

	while (c != 0)
	{
		x = b / c;
		y = b - x*c;
		b = c;
		c = y;
		y = j;
		j = i - j*x;
		i = y;
	}

	if(i < 0)
		i += m;

	return i;
}


CHuge Abs(const CHuge &rCHuge)
{
	CHuge rTmp = rCHuge;

	rTmp.m_nSign = 0;
	return rTmp;
}

CHuge Hash(const char *s)
{
	CHuge n = 0;

	while (*s)
	{
		n = n * (CHuge)256 + (CHuge)((unsigned char)*s);
		s += 1;
	}

	return n;
}

//Montgomery modlar exponentiation.
class CMonty
{
	CHuge R,R1,m,n1;
	CHuge T,k;   // work registers

	unsigned N;  // Bits for R
	void Mul( CHuge &x, const CHuge &y );

public:
	CHuge Exp( const CHuge&, const CHuge&);
	CMonty(const CHuge&);
};

CMonty::CMonty(const CHuge &rCHuge)
{
	m = rCHuge;
	N = 0;
	R = 1;

	while (R < rCHuge)
	{
		R += R;
		N += 1;
	}

	R1 = ModInv(R - m, m);
	n1 = R - ModInv(m, R);
}

void CMonty::Mul(CHuge &rCHuge1, const CHuge &rCHuge2)
{
	// T = x*rCHuge2;
	T.m_pCInt->FstMul(*rCHuge1.m_pCInt, *rCHuge2.m_pCInt, N*2);

	// k = ( T * n1 ) % R;
	k.m_pCInt->FstMul(*T.m_pCInt, *n1.m_pCInt, N);

	// x = ( T + k*m ) / R;
	rCHuge1.m_pCInt->FstMul(*k.m_pCInt, *m.m_pCInt, N*2);
	rCHuge1 += T;

	rCHuge1.m_pCInt->ShiftR(N);

	if(rCHuge1 >= m)
		rCHuge1 -= m;
}

CHuge CMonty::Exp( const CHuge &rCHuge1, const CHuge &rCHuge2 )
{
	unsigned nBits;
	CHuge rTmp1;
	CHuge rTmp2;
  
	nBits = rCHuge2.m_pCInt->Bits();
	rTmp1 = R - m;
	rTmp2 = (rCHuge1 * R) % m;

	for(unsigned i = 0;;)
	{
		if(rCHuge2.m_pCInt->BitTest(i))
			Mul( rTmp1, rTmp2);

		i++;

		if(i == nBits)
			break;

		Mul(rTmp2, rTmp2);
  }

  return (rTmp1 * R1) % m;
}

CHuge ModExp(const CHuge &rCHuge1, const CHuge & rCHuge2, const CHuge &rCHuge3)
{
	CMonty rTmp(rCHuge3);

	return rTmp.Exp(rCHuge1, rCHuge2);
}
