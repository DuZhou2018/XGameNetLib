#include "../include/NiceNetBase64.h"

//===================================
//    Base64 ½âÂë
//===================================
char Decode_GetByte(char c)
{
	if(c == '+')
		return 62;
	else if(c == '/')
		return 63;
	else if(c <= '9')
		return (char)(c - '0' + 52);
	else if(c == '=')
		return 64;
	else if(c <= 'Z')
		return (char)(c - 'A');
	else if(c <= 'z')
		return (char)(c - 'a' + 26);
	return 64;
}

//½âÂë
unsigned Base64::Base64_Decode(char *pDest, const char *pSrc, unsigned srclen)
{
	char input[4];
	unsigned i, index = 0;
	for(i = 0; i < srclen; i += 4)
	{
		//char[0]
		input[0] = Decode_GetByte(pSrc[i]);
		input[1] = Decode_GetByte(pSrc[i + 1]);
		pDest[index++] = (input[0] << 2) + (input[1] >> 4);

		//char[1]
		if(pSrc[i + 2] != '=')
		{
			input[2] = Decode_GetByte(pSrc[i + 2]);
			pDest[index++] = ((input[1] & 0x0f) << 4) + (input[2] >> 2);
		}

		//char[2]
		if(pSrc[i + 3] != '=')
		{
			input[3] = Decode_GetByte(pSrc[i + 3]);
			pDest[index++] = ((input[2] & 0x03) << 6) + (input[3]);
		}            
	}

	//null-terminator
	pDest[index] = 0;
	return index;
}

//===================================
//    Base64 ±àÂë
//===================================
char g_NetB64TempChar[100]={"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="};
char Encode_AaGetChar(char num)
{
	return g_NetB64TempChar[(int)num];
}

//±àÂë
unsigned Base64::Base64_Encode(char *pDest, const char *pSrc, unsigned srclen)
{
	unsigned char input[3], output[4];
	unsigned i, index_src = 0, index_dest = 0;
	for(i = 0; i < srclen; i += 3)
	{
		//char [0]
		input[0]  = pSrc[index_src++];
		output[0] = (unsigned char)(input[0] >> 2);
		char chtemps        = Encode_AaGetChar(output[0]);
		pDest[index_dest++] = chtemps;
		
		//char [1]
		if(index_src < srclen)
		{
			input[1] = pSrc[index_src++];
			output[1] = (unsigned char)(((input[0] & 0x03) << 4) + (input[1] >> 4));
			chtemps             = Encode_AaGetChar(output[1]);
			pDest[index_dest++] = chtemps;
		}
		else
		{
			output[1] = (unsigned char)((input[0] & 0x03) << 4);
			chtemps             = Encode_AaGetChar(output[1]);
			pDest[index_dest++] = Encode_AaGetChar(output[1]);
			pDest[index_dest++] = '=';
			pDest[index_dest++] = '=';
			break;
		}

		//char [2]
		if(index_src < srclen)
		{
			input[2] = pSrc[index_src++];
			output[2] = (unsigned char)(((input[1] & 0x0f) << 2) + (input[2] >> 6));
			chtemps             = Encode_AaGetChar(output[2]);
			pDest[index_dest++] = Encode_AaGetChar(output[2]);
		}
		else
		{ 
			output[2] = (unsigned char)((input[1] & 0x0f) << 2);
			chtemps             = Encode_AaGetChar(output[2]);
			pDest[index_dest++] = Encode_AaGetChar(output[2]);
			pDest[index_dest++] = '=';
			break;
		}

		//char [3]
		output[3] = (unsigned char)(input[2] & 0x3f);
		chtemps             = Encode_AaGetChar(output[3]);
		pDest[index_dest++] = Encode_AaGetChar(output[3]);
	}
	//null-terminator
	pDest[index_dest] = 0;
	return index_dest;
} 