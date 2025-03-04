/*****************************************************************************************************************************
Created:   2022/10/8
Author:    ZhangHongtao,   1265008170@qq.com
FileName:  
******************************************************************************************************************************/
#ifndef XClientBase_h__
#define XClientBase_h__
#include "./NiceIDMg.h"
#include "./CGlobal.hpp"
#include "./CountersMg.hpp"
#include "./ClientServerIDMG.h"
#include "./TCPXSessionBase.hpp"

namespace peony
{
	namespace net
	{
		class IFaceXClientMg
		{
		protected:
			IFaceXClientMg(){}
		public:
			virtual ~IFaceXClientMg(){}
		public:
			virtual void     SetXAttrib(unsigned conid, unsigned xadd, unsigned xdel)=0;
			virtual bool     IsXAttrib(unsigned conid, unsigned xmark) = 0;
			virtual bool     IsValidConid(unsigned conid) = 0;
			virtual void     SetConData(unsigned conid, uint64_t udata) = 0;
			virtual uint64_t GetConData(unsigned conid) = 0;
			virtual string	 GetLocalIp(unsigned &uPort, unsigned conn_index) = 0;
			virtual string	 GetRemoteIp(unsigned &uPort, unsigned conn_index) = 0;
			virtual bool	 ClientS_GetSessionBufferInfo(unsigned uConnectid, TSessionBufferInfo &info) = 0;

		};

		template < class XClient >
		class XClientMgBase : public IFaceXClientMg, private boost::noncopyable
		{
		protected:
			typedef boost::shared_ptr<XClient>XClientSessionPtr;
			typedef map<unsigned, XClient*> ConnMap;
			typedef typename ConnMap::iterator ConnMap_It;

			ConnMap			        m_conn_map;
			class Scheduler *       m_pScheduler;
			TStatData				m_StatData;
		protected:
			XClientMgBase(class Scheduler* pScheduler) {
				m_pScheduler = pScheduler;
			}

		public:
			virtual ~XClientMgBase()
			{
				if(0 != m_conn_map.size())
				{
					NETLOG_ERROR("m_conn_map.size()=" << m_conn_map.size() << FUN_FILE_LINE);
				}
			}
			virtual XClientSessionPtr create_session(TNewClientParam &param) { return nullptr; }
			virtual int	SendWebSocketMsg( unsigned conn_index, const void *pdata, unsigned data_len, bool IsText) { return 0; }

			void				destroy_session(XClient * session);
			unsigned			connect_to(TNewClientParam &param);
			void				UnInit();
			void				disconnect(unsigned conn_index, string strWhy);
			int					send(unsigned conn_index, const void *data_ptr, size_t data_len);
			void                SetXAttrib(unsigned conid, unsigned xadd, unsigned xdel);
			bool                IsXAttrib(unsigned conid, unsigned xmark);

			bool                IsValidConid(unsigned conid);
			void                SetConData(unsigned conid, uint64_t udata);
			uint64_t            GetConData(unsigned conid);

			unsigned			GetSize();
			string				GetLocalIp(unsigned &uPort, unsigned conn_index);
			string				GetRemoteIp(unsigned &uPort, unsigned conn_index);
			bool				ClientS_GetSessionBufferInfo(unsigned uConnectid, TSessionBufferInfo &info);
			unsigned			GetMaxCounterCount(unsigned uConnectid);
			void                OnTimeCheckHeartTimeout(LIST_UInt &listConID);
			unsigned            GetClientTotalCount() { return (unsigned)m_conn_map.size(); }

		public:
			bool				hander_r_alloc(char *&buffer, unsigned usize, bool is_req_alloc, unsigned umark);
			bool				hander_s_alloc(char *&buffer, unsigned usize, bool is_req_alloc, unsigned umark);

		protected:
			void				imp_destroy_session(unsigned conn_index);

		};

		template < class XClient >
		unsigned peony::net::XClientMgBase<XClient>::connect_to(TNewClientParam &param)
		{
			//app线程调用
			//Boost_Scoped_Lock lock(m_mutex);
			XClientSessionPtr smart_ptr = this->create_session(param);
			if( nullptr == smart_ptr )
			{
				return INVALID_CONN_INDEX;
			}

			smart_ptr->StartSocket();
			return smart_ptr->GetConnectIndex();
		}

		template < class XClient >
		void peony::net::XClientMgBase<XClient>::destroy_session(XClient * session)
		{
			//放入到主线程里面删除,保障对象的创建和删除都在主线程里面
			unsigned conn_index = session->GetConnectIndex();
			on_netmsg_hand cbFunDel = boost::bind(&peony::net::XClientMgBase<XClient>::imp_destroy_session, this, conn_index);
			PNGB::m_pNiceMsgMG->PutMsgFun(cbFunDel, true);
		}

		template < class XClient >
		void peony::net::XClientMgBase<XClient>::UnInit()
		{
			//主线程调用
			CAutoLogDeadlock AutoLgDL(__FUNCTION__);
			{
				//Boost_Scoped_Lock lock(m_mutex);
				ConnMap_It it;
				for(it = m_conn_map.begin(); it != m_conn_map.end(); it++)
				{
					(it->second)->CloseSocketOut("[NetUnInit!]");
				}
			}
		}

		template < class XClient >
		void peony::net::XClientMgBase<XClient>::disconnect(unsigned conn_index, string strWhy)
		{
			//主线程调用
			//Boost_Scoped_Lock lock(m_mutex);
			ConnMap_It it = m_conn_map.find(conn_index);
			if(it != m_conn_map.end())
			{
				(it->second)->CloseSocketOut(strWhy);
			}
		}

		template < class XClient >
		int peony::net::XClientMgBase<XClient>::send(unsigned conn_index, const void *data_ptr, size_t data_len)
		{
			//主线程调用
			//Boost_Scoped_Lock lock(m_mutex);
			//NETLOG_SYSINFO("mapSize="<<m_conn_map.size()<<";conn_index="<<conn_index<<FUN_LINE );
			std::string data(reinterpret_cast<const char *>(data_ptr), data_len);
			ConnMap_It it = m_conn_map.find(conn_index);
			if(it == m_conn_map.end())
			{
				NETLOG_ERROR("()," << "XClientMg::send() invalid index: " << conn_index << FUN_LINE);
				return 90908101;
			}else{
				return (it->second)->SendMsg(data.c_str(), (unsigned)data.length());
			}
		}

		template < class XClient >
		void peony::net::XClientMgBase<XClient>::SetXAttrib(unsigned conid, unsigned xadd, unsigned xdel)
		{
			ConnMap_It it = m_conn_map.find(conid);
			if(it == m_conn_map.end())
			{
				NETLOG_ERROR("()," << "XClientMg::send() invalid index: " << conid << FUN_LINE);
				return;
			}
			(it->second)->SetXMark(xadd, xdel);
		}

		template < class XClient >
		bool peony::net::XClientMgBase<XClient>::IsXAttrib(unsigned conid, unsigned xmark)
		{
			ConnMap_It it = m_conn_map.find(conid);
			if(it == m_conn_map.end())
			{
				NETLOG_ERROR("()," << "XClientMg::send() invalid index: " << conid << FUN_LINE);
				return false;
			}
			return (it->second)->IsXMark(xmark);
		}

		template < class XClient >
		bool peony::net::XClientMgBase<XClient>::IsValidConid(unsigned conid)
		{
			ConnMap_It it = m_conn_map.find(conid);
			if(it == m_conn_map.end())
			{
				return false;
			}
			return true;
		}

		template < class XClient >
		void peony::net::XClientMgBase<XClient>::SetConData(unsigned conid, uint64_t udata)
		{
			ConnMap_It it = m_conn_map.find(conid);
			if(it == m_conn_map.end())
			{
				NETLOG_ERROR("()," << "XClientMg::send() invalid index: " << conid << FUN_LINE);
				return;
			}
			(it->second)->SetConData(udata);
		}

		template < class XClient >
		uint64_t peony::net::XClientMgBase<XClient>::GetConData(unsigned conid)
		{
			ConnMap_It it = m_conn_map.find(conid);
			if(it == m_conn_map.end())
			{
				NETLOG_ERROR("()," << "XClientMg::send() invalid index: " << conid << FUN_LINE);
				return 0;
			}
			return (it->second)->GetConData();
		}

		template < class XClient >
		unsigned peony::net::XClientMgBase<XClient>::GetSize()
		{
			//主线程调用
			//Boost_Scoped_Lock lock(m_mutex);
			return (unsigned)m_conn_map.size();
		}

		template < class XClient >
		string peony::net::XClientMgBase<XClient>::GetLocalIp(unsigned &uPort, unsigned conn_index)
		{
			//主线程调用
			//Boost_Scoped_Lock lock(m_mutex);
			uPort = 0;
			ConnMap_It it = m_conn_map.find(conn_index);
			if(it == m_conn_map.end())
				return "";
			return it->second->GetLocalIp(uPort);
		}

		template < class XClient >
		string peony::net::XClientMgBase<XClient>::GetRemoteIp(unsigned &uPort, unsigned conn_index)
		{
			//主线程调用
			//Boost_Scoped_Lock lock(m_mutex);
			uPort = 0;
			ConnMap_It it = m_conn_map.find(conn_index);
			if(it == m_conn_map.end())
				return "";
			return it->second->GetRemoteIp(uPort);
		}

		template < class XClient >
		bool peony::net::XClientMgBase<XClient>::ClientS_GetSessionBufferInfo(unsigned uConnectid, TSessionBufferInfo &info)
		{
			//Boost_Scoped_Lock lock(m_mutex);
			ConnMap_It it = m_conn_map.find(uConnectid);
			if(it == m_conn_map.end())
				return false;
			it->second->GetSessionBufferInfo(info);
			return true;
		}

		template < class XClient >
		unsigned peony::net::XClientMgBase<XClient>::GetMaxCounterCount(unsigned uConnectid)
		{
			//Boost_Scoped_Lock lock(m_mutex);
			ConnMap_It it = m_conn_map.find(uConnectid);
			if(it == m_conn_map.end())
				return false;
			return 0;
		}

		template < class XClient >
		void peony::net::XClientMgBase<XClient>::OnTimeCheckHeartTimeout(LIST_UInt &listConID)
		{
			ConnMap_It it = m_conn_map.begin();
			for(; it != m_conn_map.end(); it++)
			{
				if(it->second->CheckHeartTiimeout())
				{
					NETLOG_ERROR("XClientMg..TimeOut: " << FUN_LINE);
					listConID.push_back(it->first);
				}
			}
		}

		template < class XClient >
		bool peony::net::XClientMgBase<XClient>::hander_r_alloc(char *&buffer, unsigned usize, bool is_req_alloc, unsigned umark)
		{
			if(is_req_alloc)
			{
				buffer = new char[usize];
				return (buffer != 0);
			}
			else
			{
				if(buffer)
				{
					delete[]buffer;
					buffer = 0;
				}
			}
			return true;

		}

		template < class XClient >
		bool peony::net::XClientMgBase<XClient>::hander_s_alloc(char *&buffer, unsigned usize, bool is_req_alloc, unsigned umark)
		{
			if(is_req_alloc)
			{
				buffer = new char[usize];
				return (buffer != 0);
			}
			else
			{
				if(buffer)
				{
					delete[]buffer;
					buffer = 0;
				}
			}
			return true;
		}

		template < class XClient >
		void peony::net::XClientMgBase<XClient>::imp_destroy_session(unsigned conn_index)
		{
			ConnMap_It it = m_conn_map.find(conn_index);
			if(it == m_conn_map.end())
			{
				NETLOG_ERROR("[Logic error]: conn_index=" << conn_index << FUN_LINE);
				return;
			}
			XClient *pSs = it->second;
			m_conn_map.erase(conn_index);
			PNGB::m_pCSIDMG->FreeID(conn_index);
			delete pSs;
		}

	}
}
#endif // NetXServerBase_h__
