#ifndef NICE_NET_CLIENT_SERVER_ID_SCS_MG
#define NICE_NET_CLIENT_SERVER_ID_SCS_MG
#include "./InPublicPreDefine.hpp"

namespace peony
{
    namespace net
    {
		enum EmIDKind {
			IDKIND_NODEF = 0,
			IDKIND_CLIENTTCP = 1,
			IDKIND_CLIENTUDP,
			IDKIND_CLIENTWS,
			IDKIND_SERVER,
			IDKIND_SERVER_SUBCLIENT,
		};

        class CClientServerIDMG
        {
        public:
            struct TXxConIdBeginEndPos
            {
                TXxConIdBeginEndPos()
                {
                    beginID=endID=curID=0;
                }

                void InitAllID(unsigned b, unsigned e)
                {
                    beginID=b; endID=e; curID=beginID;
                }
                bool IsInMeRange(unsigned v)
                {
                    if( v>beginID && v<endID )
                        return true;
                    return false;
                }
                unsigned NextConID()
                {
                    curID++;
                    if( curID>=endID )
                        curID = beginID;
                    return curID;
                }
            private:
                unsigned  beginID;  //开始
                unsigned  endID;    //结束
                unsigned  curID;    //当前
            };

        public:
            CClientServerIDMG(void);
            ~CClientServerIDMG(void);

        public:
            bool         IsInvalidID(unsigned uID);
            unsigned     GetNewID(EmIDKind IDKind,unsigned uIndexF);
            void         FreeID(unsigned uID);
            /*************************************************************************
            return true, if is valid FactID,
            return false, if is invalid factID,
            unsigned &iMeServerID 返回我的ServerID
            *************************************************************************/
            bool        GetIndexIDByFactID(unsigned uFactID,EmIDKind &IDKind,unsigned &iMeServerID);

        private:
            TXxConIdBeginEndPos  m_TcpClientConID;
            TXxConIdBeginEndPos  m_UdpClientConID;
			TXxConIdBeginEndPos  m_WsClientConID;
			TXxConIdBeginEndPos  m_XServerConID;
            TXxConIdBeginEndPos  m_XServerSubConID;

            typedef map<unsigned,int> MAP_UIntToUInt;
            MAP_UIntToUInt          m_mapUseID;
            boost::recursive_mutex &m_mutex;

        };        

    }
}
#endif

