#ifndef __OMP_TCPPAKDEF_HEADER_FILE__
#define __OMP_TCPPAKDEF_HEADER_FILE__

//#include <cstdlib>
//#include <boost/static_assert.hpp>
#include "../include/NiceNetMsgHead.h"
#include "./CGlobal.hpp"

namespace peony {
	namespace net {

	    #pragma pack(push)
	    #pragma pack(push,1)
	    struct tcp_pak_header : public INPROTOCOL::TInNiceNetHead
	    {
		    tcp_pak_header()
		    {
			    reset();
		    }
		    inline void reset()
		    {
		    }
            inline bool check()
		    {
			    if( false == PNGB::m_IsCheckPackHead )
				    return true;
			    int iCheckCode = getCheckCode();
			    return (INPROTOCOL::sc_THttpCheckCodeIn==iCheckCode );
		    }
	    };
        #pragma pack(pop)

		//BOOST_STATIC_ASSERT( sizeof(char)==1 );
		//BOOST_STATIC_ASSERT( sizeof(int)==4 );
		//BOOST_STATIC_ASSERT( sizeof(tcp_pak_header)==16 );
        static const unsigned sc_pak_header_len = sizeof(tcp_pak_header);
	}	// end of namespace net

}	// end of namespace peony

#endif//#define __OMP_TCPPAKDEF_HEADER_FILE__