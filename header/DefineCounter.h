#ifndef _INICE_COUNTER
#define _INICE_COUNTER

#include "../header/NiceNetLogicProtocol.h"
#include "../include/INiceNetCounter.h"

// #include <string>
// #include <map>
// using namespace std;

// #include <boost/bind.hpp>
// #include <boost/function.hpp>
#include "./CGlobal.hpp"

namespace peony
{
    namespace net
    {
        static const unsigned sc_INVALID_COUNTERID  = 0X0FFFFFFF;
        static const unsigned sc_MAX_COUNTERBOOKER  = 10;  //one counter max book user
        static const unsigned sc_INVALID_CONNECTID  = 0;
        static const unsigned sc_MAX_MAPINSTALLCOUNTER = 100;
        //for regrister

        //for install counter map
        //typedef boost::function<unsigned(unsigned,unsigned,unsigned,EmCounterUpdateKind)>updatevalue_handler_type;
        typedef boost::function<unsigned(unsigned,unsigned)>updatevalue_handler_type;
        class CCounterInstallDriver
        {
            struct TOneBooker 
            {
                unsigned             m_uServerSubConnectID;
                //updatevalue_handler_type m_UpdateValueHand;
            };
        public:
            CCounterInstallDriver(unsigned uCounterID=sc_INVALID_CONNECTID)
            {
                m_uCounterID = uCounterID;
                for(unsigned t=0;t<sc_MAX_COUNTERBOOKER;++t)
                    m_BookerArr[t].m_uServerSubConnectID = sc_INVALID_CONNECTID;
            }
            bool AddNew(unsigned uServerSubConnectID)
            {
                if( this->Find(uServerSubConnectID) )
                    return false;

                for( unsigned t=0;t<sc_MAX_COUNTERBOOKER;++t )
                {
                    if( sc_INVALID_CONNECTID == m_BookerArr[t].m_uServerSubConnectID )
                    {
                        m_BookerArr[t].m_uServerSubConnectID = uServerSubConnectID;
                        //m_BookerArr[t].m_UpdateValueHand = upValueHand;
                        return true;
                    }
                }
                return false;
            }
            void Del(unsigned uConnectid)
            {
                for( unsigned t=0;t<sc_MAX_COUNTERBOOKER;++t )
                {
                    if( uConnectid == m_BookerArr[t].m_uServerSubConnectID )
                    {
                        m_BookerArr[t].m_uServerSubConnectID = sc_INVALID_CONNECTID;
                        return;
                    }
                }               
            }
            void Update(unsigned uValue,EmCounterUpdateKind kind)
            {
				return;
            }
            unsigned GetCounterSize()
            {
                unsigned iUserCount = 0;
                for( unsigned t=0;t<sc_MAX_COUNTERBOOKER;++t )
                {
                    if( sc_INVALID_CONNECTID != m_BookerArr[t].m_uServerSubConnectID )
                        iUserCount++;
                }
                return iUserCount;
            }
        private:
            TOneBooker *Find(unsigned uServerSubConnectID)
            {
                for( unsigned t=0;t<sc_MAX_COUNTERBOOKER;++t )
                {
                    if( m_BookerArr[t].m_uServerSubConnectID==uServerSubConnectID )
                        return &m_BookerArr[t];
                }
                return 0;
            }

        public:
            unsigned            m_uCounterID;
            TOneBooker              m_BookerArr[sc_MAX_COUNTERBOOKER];

        };

        struct TBookItem
        {
            unsigned   * pCuValue;
            unsigned     interval;
			unsigned     last_tm;  //上次更新的时间

			explicit TBookItem(unsigned  *pCuV)
            {
				pCuValue = pCuV;
                interval = 0;
				this->restart();
            }
			TBookItem()
			{
				pCuValue = 0;
				interval = 0;
				last_tm  = 0;
			}
			unsigned elapsed(){ return (PNGB::m_server_relativeTm-last_tm)/100; }
			void     restart(){ last_tm = PNGB::m_server_relativeTm;}
        };
        typedef std::map<unsigned,INPROTOCOL::LIST_RepCounterState>MAP_UserBookCounterSate;
        class CUserBookCountersList
        {
            typedef std::map<unsigned,TBookItem> Map_BookItem;

        public:
            CUserBookCountersList(const CUserBookCountersList &obj)
            { m_uServer_subConnect_id=obj.m_uServer_subConnect_id;}
            CUserBookCountersList(unsigned uServer_subConnect_id)
            { m_uServer_subConnect_id=uServer_subConnect_id;}
            CUserBookCountersList(){m_uServer_subConnect_id=0;m_mapBook.clear();}
            ~CUserBookCountersList(){m_mapBook.clear();}
        public:
            TBookItem *AddBookItem(unsigned uCounterid,unsigned  &uCuV)
            {
                if(0!=FindBookItem(uCounterid))
				{
					NETLOG_ERROR( FUN_FILE_LINE);
				}
                return &( m_mapBook[uCounterid] = TBookItem(&uCuV) );
            }
            TBookItem *FindBookItem(unsigned uCounterid)
            {
                map<unsigned,TBookItem>::iterator itor = m_mapBook.find(uCounterid);
                if( itor==m_mapBook.end() )
                    return 0;
                return &itor->second;
            }
            void DelBookItem(unsigned uCounterID)
            {
                m_mapBook.erase(uCounterID);
            }
            unsigned GetCounterSize()
            {
                return (unsigned)m_mapBook.size();
            }
            void GetBookCuStateList( MAP_UserBookCounterSate &mapListMsg )
            {
                INPROTOCOL::LIST_RepCounterState                   *pListMsgs  = 0;
                INPROTOCOL::TServerNoticeState                     *pMsg       = 0;
                INPROTOCOL::TServerNoticeState::COUNTER_ITEM_STATE *pStateItem = 0;
                unsigned iCount = 0;
                for(Map_BookItem::iterator itor=m_mapBook.begin();itor!=m_mapBook.end();++itor)
                {
                    if( itor->second.elapsed()>=itor->second.interval )
                    {
                        itor->second.restart();

                        if( !pStateItem || (iCount>=INPROTOCOL::COUNTERLIST_MAXCOUNT) )
                        {
                            if( pMsg )
                                pMsg->count = iCount;

                            if( !pListMsgs )
                                pListMsgs = &(mapListMsg[m_uServer_subConnect_id]=INPROTOCOL::LIST_RepCounterState());

                            pMsg = &(*pListMsgs->insert(pListMsgs->end(),INPROTOCOL::TServerNoticeState() ));
                            if( !pMsg )
							{
								NETLOG_ERROR(FUN_FILE_LINE);
							}

                            iCount = 0;
                            pStateItem = pMsg->items;
                        }

                        pStateItem->counter_id = itor->first;
                        pStateItem->value      = *(itor->second.pCuValue);
                        iCount++;
                        pStateItem++;
                    }
                }
                if( pMsg )
                    pMsg->count = iCount;
            }
        public:
            Map_BookItem                    m_mapBook;
            unsigned                    m_uServer_subConnect_id;
        };
        
    }
}

#endif
