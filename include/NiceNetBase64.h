

#ifndef base64_h__
#define base64_h__

class Base64
{
public:
	static unsigned Base64_Decode(char *pDest, const char *pSrc, unsigned srclen);
	static unsigned Base64_Encode(char *pDest, const char *pSrc, unsigned srclen);
};

#endif // base64_h__

