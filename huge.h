#ifndef HUGE_INCL
#define HUGE_INCL

class CSet
{
	unsigned *m_pu;
	unsigned m_nSize;

public:
	unsigned m_nLast;
	CSet();
	~CSet();

	void Clear()										{ m_nLast = 0; }
	void Reserve(unsigned );

	unsigned Get(unsigned i) const						{ return (i < m_nLast)?m_pu[i]:0; }
	void Set(unsigned, unsigned);

	void Normalise();

	void FstMul(CSet&, CSet&, unsigned);
};

class CInt : public CSet
{
public:
	unsigned m_nShare;

	CInt();

	bool Is0() const									{ return m_nLast == 0; }
	int BitTest( unsigned i ) const;
	unsigned Bits() const;
	int Comp(CInt&) const;

	operator unsigned();

	void ShiftL();
	void ShiftR();
	void ShiftR(unsigned);

	void Init(unsigned);
	void Copy(CInt&);

	void Add(CInt&);
	void Sub(CInt&);
	void Mul(CInt&, CInt&);
	void Div(CInt&, CInt&, CInt&);
};

class CHuge
{
public:
	// Standard arithmetic operators
	friend CHuge operator +(const CHuge&, const CHuge&);
	friend CHuge operator -(const CHuge&, const CHuge&);
	friend CHuge operator *(const CHuge&, const CHuge&);
	friend CHuge operator /(const CHuge&, const CHuge&);
	friend CHuge operator %(const CHuge&, const CHuge&);

	// Atomic arithmetic operators
	CHuge& operator +=(const CHuge&);
	CHuge& operator -=(const CHuge&);

	// Standard comparison operators
	friend inline int operator !=(const CHuge& rCHuge1, const CHuge& rCHuge2){ return rCHuge1.Comp(rCHuge2) != 0; }
	friend inline int operator ==(const CHuge& rCHuge1, const CHuge& rCHuge2){ return rCHuge1.Comp(rCHuge2) == 0; }
	friend inline int operator >=(const CHuge& rCHuge1, const CHuge& rCHuge2){ return rCHuge1.Comp(rCHuge2) >= 0; }
	friend inline int operator <=(const CHuge& rCHuge1, const CHuge& rCHuge2){ return rCHuge1.Comp(rCHuge2) <= 0; }
	friend inline int operator > (const CHuge& rCHuge1, const CHuge& rCHuge2){ return rCHuge1.Comp(rCHuge2) > 0; }
	friend inline int operator < (const CHuge& rCHuge1, const CHuge& rCHuge2){ return rCHuge1.Comp(rCHuge2) < 0; }

	// Construction and conversion operations
	CHuge(const int n = 0);
	CHuge(const CHuge&);
	CHuge(const char *);
	~CHuge();

	CHuge& operator =(const CHuge& rCHuge);

	// Miscellaneous operators.
	int Int();
	void String(char *);	
	bool Is0()												{ return m_pCInt->Is0(); }						

	int m_nSign;

private:
	CInt *m_pCInt;
	int Comp(const CHuge) const;
	void DoCopy();
	friend class CMonty;
};

CHuge ModExp(const CHuge&, const CHuge&, const CHuge&);
CHuge Gcd(const CHuge&, const CHuge&);
CHuge ModInv(const CHuge&, const CHuge&);
CHuge Abs(const CHuge&);
CHuge Hash(const char*);

#endif
