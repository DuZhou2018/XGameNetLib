#include "../header/ClientServerIDMG.h"
#include "../header/CGlobal.hpp"
#include "../header/NiceLog.h"
#include "../header/InPublicPreDefine.hpp"
#include "../header/NiceNetLogicProtocol.h"


namespace peony
{
    namespace net
    {
		CClientServerIDMG::CClientServerIDMG(void):m_mutex(*PNGB::m_globMutex)
        {
            m_TcpClientConID.InitAllID(100000, 200000);
            m_UdpClientConID.InitAllID(200001, 300000);
			m_WsClientConID.InitAllID( 300001, 400000);
			m_XServerConID.InitAllID(10, 99);
            m_XServerSubConID.InitAllID(100000, 300000); //9位数
        }
        CClientServerIDMG::~CClientServerIDMG(void)
        {
            m_mapUseID.clear();
        }

        unsigned CClientServerIDMG::GetNewID(EmIDKind IDKind,unsigned uIndexF)
        {
            Boost_Scoped_Lock sendbuf_lock(m_mutex);
            unsigned uNewID               = 0;
            unsigned uCurLoopCount        = 0;
            unsigned uLoopMaxCount        = 1000;
            if(IDKIND_CLIENTTCP == IDKind)
            {
                while(uCurLoopCount<uLoopMaxCount)
                {
                    uNewID = m_TcpClientConID.NextConID();
                    if( !IsInvalidID(uNewID) )
                    {
                        m_mapUseID[uNewID]=1;
                        return uNewID;
                    }
                    ++uCurLoopCount;
                }
            }
            else if(IDKIND_CLIENTUDP==IDKind)
            {
                while(uCurLoopCount<uLoopMaxCount)
                {
                    uNewID = m_UdpClientConID.NextConID();
                    if(!IsInvalidID(uNewID))
                    {
                        m_mapUseID[uNewID]=1;
                        return uNewID;
                    }
                    ++uCurLoopCount;
                }
            }
			else if(IDKIND_CLIENTWS == IDKind)
			{
				while(uCurLoopCount < uLoopMaxCount)
				{
					uNewID = m_WsClientConID.NextConID();
					if(!IsInvalidID(uNewID))
					{
						m_mapUseID[uNewID] = 1;
						return uNewID;
					}
					++uCurLoopCount;
				}
			}
			else if(IDKIND_SERVER == IDKind)
            {
				while(uCurLoopCount<uLoopMaxCount)
				{
					uNewID = m_XServerConID.NextConID();
					if( !IsInvalidID(uNewID) )
					{
						m_mapUseID[uNewID]=1;
						return uNewID;
					}
					++uCurLoopCount;
				}
            }
            else if( IDKIND_SERVER_SUBCLIENT == IDKind )
            {
                while(uCurLoopCount<uLoopMaxCount)
                {
                    uNewID = m_XServerSubConID.NextConID();
                    uNewID+= (uIndexF*1000000);
                    if( !IsInvalidID(uNewID) )
                    {
                        m_mapUseID[uNewID]=1;
                        return uNewID;
                    }
                    ++uCurLoopCount;
                }
            }
            NETLOG_ERROR("[net.得到新的连接ID失败]"<<FUN_FILE_LINE );
            return INVALID_CONN_INDEX;
        }
        bool CClientServerIDMG::GetIndexIDByFactID(unsigned uFactID,EmIDKind &IDKind,unsigned &iMeServerID)
        {
            if( !IsInvalidID(uFactID) )
                return false;

            if( m_XServerConID.IsInMeRange(uFactID) )
            {
                IDKind  = IDKIND_SERVER;
                iMeServerID = uFactID;
                return true;
            }
            else if(m_TcpClientConID.IsInMeRange(uFactID))
            {
                iMeServerID = uFactID;
                IDKind  = IDKIND_CLIENTTCP;
                return true;
            }else if(m_UdpClientConID.IsInMeRange(uFactID))
            {
                iMeServerID = uFactID;
                IDKind  = IDKIND_CLIENTUDP;
                return true;
			}
			else if(m_WsClientConID.IsInMeRange(uFactID))
			{
				iMeServerID = uFactID;
				IDKind = IDKIND_CLIENTWS;
				return true;
			}
            else
            {
                iMeServerID = uFactID/1000000;
                if(m_XServerConID.IsInMeRange(iMeServerID))
                {
                    IDKind  = IDKIND_SERVER_SUBCLIENT;
                    return true;
                }
            }

            return false;
        }
        bool CClientServerIDMG::IsInvalidID(unsigned uID)
        {
            Boost_Scoped_Lock sendbuf_lock(m_mutex);
            return (m_mapUseID.find(uID)!=m_mapUseID.end());
        }
        void CClientServerIDMG::FreeID(unsigned uID)
        {
            Boost_Scoped_Lock sendbuf_lock(m_mutex);
            m_mapUseID.erase( uID );
       }
    }
}