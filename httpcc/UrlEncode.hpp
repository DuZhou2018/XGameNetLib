
#ifndef __PEYONE_NET_URLENCODE_
#define __PEYONE_NET_URLENCODE_
#include "../header/InPublicPreDefine.hpp"

namespace peony
{
	namespace net
	{
		class CUrlEncode
		{
		public:
			static string	UrlEncode(const std::string& str);
			static string	UrlDecode(const std::string& str);

		private:
			static unsigned char ToHex(unsigned char x);
			static unsigned char FromHex(unsigned char x);

		};
	}
}

#endif
