#include "../header/XXClientMg.hpp"
#include "../header/Scheduler.hpp"
#include "../header/NiceIDMg.h"

#include "../header/CGlobal.hpp"
#include "../header/ClientServerIDMG.h"
#include "../header/NetCountersInstanceMg.h"
#include <new>

namespace peony{
	namespace net{

		TCPClientSessionPtr TCPClientMg::create_session(TNewClientParam &param)
		{
            unsigned id= PNGB::m_pCSIDMG->GetNewID(IDKIND_CLIENTTCP,0);
			if(id==INVALID_CONN_INDEX)
			{
				NETLOG_ERROR("create session fail! ip="<<param.strServerIp<<",prot="<<param.strServerPort<<FUN_FILE_LINE );
				return TCPClientSessionPtr();
			}

			io_context& io=m_pScheduler->get_impl()->get_ioservice();
			AsioStrand& strand=m_pScheduler->get_impl()->get_strand();

			TXZTBaseInitPm tInitPm;
			tInitPm.mXServerID	= IDKIND_CLIENTTCP;
			tInitPm.r_size		= param.uReciveBuffSize;
			tInitPm.s_size		= param.uSendBuffSize;
			tInitPm.pStatData	= &m_StatData;
			tInitPm.r_alloc		= boost::bind(&TCPClientMg::hander_r_alloc,this,_1,_2,_3,_4);
			tInitPm.s_alloc		= boost::bind(&TCPClientMg::hander_s_alloc,this,_1,_2,_3,_4);

			TCPClientSession* session_ptr = new(std::nothrow) TCPClientSession(id,param,tInitPm,io,strand );
			if(session_ptr==NULL)
			{
				NETLOG_ERROR("create session fail! ip="<<param.strServerIp<<",prot="<<param.strServerPort<<FUN_FILE_LINE );
                PNGB::m_pCSIDMG->FreeID(id);
				return TCPClientSessionPtr();
			}

			m_conn_map.insert(std::make_pair(id,session_ptr));
			return TCPClientSessionPtr(session_ptr,strand.wrap(boost::bind(&TCPClientMg::destroy_session,this,_1)));
		}

		UDPClientSessionPtr UDPClientMg::create_session(TNewClientParam &param)
		{
			unsigned id = PNGB::m_pCSIDMG->GetNewID(IDKIND_CLIENTUDP, 0);
			if(id == INVALID_CONN_INDEX)
			{
				NETLOG_ERROR("create session fail! ip=" << param.strServerIp << ",prot=" << param.strServerPort << FUN_FILE_LINE);
				return UDPClientSessionPtr();
			}

			io_context& io = m_pScheduler->get_impl()->get_ioservice();
			AsioStrand& strand = m_pScheduler->get_impl()->get_strand();

			TXZTBaseInitPm tInitPm;
			tInitPm.mXServerID = IDKIND_CLIENTUDP;
			tInitPm.r_size = param.uReciveBuffSize;
			tInitPm.s_size = param.uSendBuffSize;
			tInitPm.pStatData = &m_StatData;
			tInitPm.r_alloc = boost::bind(&UDPClientMg::hander_r_alloc, this, _1, _2, _3, _4);
			tInitPm.s_alloc = boost::bind(&UDPClientMg::hander_s_alloc, this, _1, _2, _3, _4);

			UDPClientSession* session_ptr = new(std::nothrow) UDPClientSession(id, param, tInitPm, io, strand);
			if(session_ptr == NULL)
			{
				NETLOG_ERROR("create session fail! ip=" << param.strServerIp << ",prot=" << param.strServerPort << FUN_FILE_LINE);
				PNGB::m_pCSIDMG->FreeID(id);
				return UDPClientSessionPtr();
			}

			m_conn_map.insert(std::make_pair(id, session_ptr));
			return UDPClientSessionPtr(session_ptr, strand.wrap(boost::bind(&UDPClientMg::destroy_session, this, _1)));
		}

		WSClientSessionPtr WSClientMg::create_session(TNewClientParam &param)
		{
			unsigned id = PNGB::m_pCSIDMG->GetNewID(IDKIND_CLIENTWS, 0);
			if(id == INVALID_CONN_INDEX)
			{
				NETLOG_ERROR("create session fail! ip=" << param.strServerIp << ",prot=" << param.strServerPort << FUN_FILE_LINE);
				return WSClientSessionPtr();
			}

			//io_context& io = m_pScheduler->get_ioservice();
			AsioStrand& strand = m_pScheduler->get_strand();

			TXZTBaseInitPm tInitPm;
			tInitPm.mXServerID = IDKIND_CLIENTWS;
			tInitPm.r_size     = param.uReciveBuffSize;
			tInitPm.s_size     = param.uSendBuffSize;
			tInitPm.pStatData  = &m_StatData;
			tInitPm.r_alloc = boost::bind(&WSClientMg::hander_r_alloc, this, _1, _2, _3, _4);
			tInitPm.s_alloc = boost::bind(&WSClientMg::hander_s_alloc, this, _1, _2, _3, _4);

			//WSClientSession* session_ptr = new(std::nothrow) WSClientSession(id, param, tInitPm, io, strand);
			WSClientSession* session_ptr = new(std::nothrow) WSClientSession(id, param, tInitPm, m_pScheduler);
			if(session_ptr == NULL)
			{
				NETLOG_ERROR("create session fail! ip=" << param.strServerIp << ",prot=" << param.strServerPort << FUN_FILE_LINE);
				PNGB::m_pCSIDMG->FreeID(id);
				return WSClientSessionPtr();
			}

			m_conn_map.insert(std::make_pair(id, session_ptr));
			return WSClientSessionPtr(session_ptr, strand.wrap(boost::bind(&WSClientMg::destroy_session, this, _1)));
		}

		int WSClientMg::SendWebSocketMsg(unsigned conn_index, const void *pdata, unsigned data_len, bool IsText)
		{
			std::string data(reinterpret_cast<const char *>(pdata), data_len);
			ConnMap_It it = m_conn_map.find(conn_index);
			if(it == m_conn_map.end())
			{
				NETLOG_ERROR("()," << "XClientMg::send() invalid index: " << conn_index << FUN_LINE);
				return 90908101;
			}
			else {
				return (it->second)->SendWebSocketMsg(data.c_str(), (unsigned)data.length(),IsText);
			}
		}

	}
}