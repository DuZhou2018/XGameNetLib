/****************************************************************************************************
	created:	2010-8-15   20:04
	filename: 	f:\game_server_v3\publib\PeonyNetLib\PeonyNet\source\CountersMg.cpp
	author:		Zhanghongtao
	
	purpose:	

	<1> //stlport 不支持这样  这是张红涛 2010-8-15   20:04修改的地方，为了支持stlport
*****************************************************************************************************/
#include "../include/INiceNet.h"
#include "../header/CountersMg.hpp"
#include "../header/CGlobal.hpp"
#include "../header/ClientServerIDMG.h"
#include "../header/NiceLog.h"
#include "../header/INiceNetServer.h"

namespace peony {
    namespace net {	

        CCountersMg::CHelp_DelCounterInstallDr::CHelp_DelCounterInstallDr(unsigned uCounterid,string strDelCounter)
        {
            MAP_CUINSTALLDR::iterator itor = PNGB::m_pCountersMg->m_map_InstallDr.find(uCounterid);
            if( itor!=PNGB::m_pCountersMg->m_map_InstallDr.end() )
            {
                CCounterInstallDriver *pCuInstallDr = itor->second;
                PNGB::m_pCountersMg->m_map_InstallDr.erase(pCuInstallDr->m_uCounterID);
                PNGB::m_pCountersMg->m_JarCuInstallDr.ReleaseObj(pCuInstallDr);
            }
        }

        CCountersMg::CHelp_DelCounterInstallDr::CHelp_DelCounterInstallDr(unsigned uConnectid)
        {
            MAP_CUINSTALLDR::iterator itor = PNGB::m_pCountersMg->m_map_InstallDr.begin();
            for( ;itor!=PNGB::m_pCountersMg->m_map_InstallDr.end(); )
            {
                CCounterInstallDriver *pCuInstallDr = itor->second;
                pCuInstallDr->Del( uConnectid );
                if( 0==pCuInstallDr->GetCounterSize() )
                {
                    //itor = PNGB::m_pCountersMg->m_map_InstallDr.erase(itor); //stlport 不支持这样
					PNGB::m_pCountersMg->m_map_InstallDr.erase(itor);
                    PNGB::m_pCountersMg->m_JarCuInstallDr.ReleaseObj(pCuInstallDr);
					itor = PNGB::m_pCountersMg->m_map_InstallDr.begin(); //for support stlport
                }else
                {
                    itor++;
                }
            }
        }

        
        CCountersMg::CHelp_DelCounterInstallDr::CHelp_DelCounterInstallDr(unsigned uConnectid,unsigned uCounterid)
        {
            MAP_CUINSTALLDR::iterator itor = PNGB::m_pCountersMg->m_map_InstallDr.find(uCounterid);
            if( itor!=PNGB::m_pCountersMg->m_map_InstallDr.end() )
            {
                CCounterInstallDriver *pCuInstallDr = itor->second;
                pCuInstallDr->Del( uConnectid );
                if( 0==pCuInstallDr->GetCounterSize() )
                {
                    PNGB::m_pCountersMg->m_map_InstallDr.erase(pCuInstallDr->m_uCounterID);
                    PNGB::m_pCountersMg->m_JarCuInstallDr.ReleaseObj(pCuInstallDr);
                }
            }
        }
        CCountersMg::CHelp_DelBook::CHelp_DelBook(unsigned uCounterid,string strDelCounter)
        {
            CCountersMg::CHelp_DelCounterInstallDr helpDelDr(uCounterid,strDelCounter);

            MAP_UserBook::iterator itor = PNGB::m_pCountersMg->m_MapUser.begin();
            for( ;itor!= PNGB::m_pCountersMg->m_MapUser.end(); )
            {
                CUserBookCountersList *pUser = &(itor->second);
                pUser->DelBookItem( uCounterid );
                if( 0==pUser->GetCounterSize() )
				{
					//原来是这样的，会崩溃
					//PNGB::m_pCountersMg->m_MapUser.erase(pUser->m_uServer_subConnect_id);
					//itor = PNGB::m_pCountersMg->m_MapUser.erase(itor); //stlport 不支持这样的语法
					PNGB::m_pCountersMg->m_MapUser.erase(itor); //for stlport
					itor =PNGB::m_pCountersMg->m_MapUser.begin();
				}else
				{
					++itor;
				}
            }
        }

        CCountersMg::CHelp_DelBook::CHelp_DelBook(unsigned uConnectid,unsigned uCounterid)
        {
            CCountersMg::CHelp_DelCounterInstallDr helpDelDr(uConnectid,uCounterid);

            MAP_UserBook::iterator itor = PNGB::m_pCountersMg->m_MapUser.find(uConnectid);
            if( itor!= PNGB::m_pCountersMg->m_MapUser.end() )
            {
                CUserBookCountersList *pUser = &(itor->second);
                pUser->DelBookItem( uCounterid );
                if( 0==pUser->GetCounterSize() )
                    PNGB::m_pCountersMg->m_MapUser.erase(pUser->m_uServer_subConnect_id);
            }
        }

        CCountersMg::CHelp_DelBook::CHelp_DelBook(unsigned uConnectid)
        {
            CCountersMg::CHelp_DelCounterInstallDr helpDelDr(uConnectid);

            MAP_UserBook::iterator itor = PNGB::m_pCountersMg->m_MapUser.find(uConnectid);
            if( itor!= PNGB::m_pCountersMg->m_MapUser.end() )
            {
                CUserBookCountersList *pUser = &(itor->second);
                PNGB::m_pCountersMg->m_MapUser.erase(pUser->m_uServer_subConnect_id);
            }
        }

        unsigned CCountersMg::CuMg_UpdateValue(unsigned &u_oldvalue,unsigned& uValue,unsigned/*EmCounterUpdateKind*/ kind)
        {
            EmCounterUpdateKind emKind = (EmCounterUpdateKind)kind;
            switch(emKind)
            {
            case CuUpKind_Add:
                u_oldvalue += uValue;
                break;
            case CuUpKind_Sub:
                u_oldvalue -= uValue;
                break;
            case CuUpKind_Replace:
                u_oldvalue = uValue;
                break;
            case CuUpKind_Swap:
				{
					unsigned t =u_oldvalue;
					u_oldvalue = uValue;
					uValue = t;
					break;
				}
            default:
                break;
            }
            return 0;
        }

		INiceNetCounter *INiceNetCounter::CreateCounter(
			const char * strPath,
			const char * strName,
			const char * strDesc,
			bool   IsAutoReg /*= true*/,
		class INiceNetServer *pServer/*=0*/)
		{
			unsigned iServerID =0;
			if( pServer )
				iServerID = pServer->GetID();
			if(0==PNGB::m_pCountersMg)
				return 0;

			CNiceNetCounter *pCounter = PNGB::m_pCountersMg->GetCounterPool().malloc();
			if( pCounter )
			{
				new (pCounter) CNiceNetCounter(strPath,strName,strDesc,IsAutoReg,iServerID);
			}
			return pCounter;
		}
		void INiceNetCounter::DeleteCounter( INiceNetCounter *pCounter)
		{
			if( pCounter )
			{
				if( pCounter->IsRegrister() )
					pCounter->UnRegister();
				PNGB::m_pCountersMg->GetCounterPool().destroy( (CNiceNetCounter *)pCounter );
			}
		}

		CCountersMg::CCountersMg():m_NetCounter_pool()/*,m_mutex(*PNGB::m_globMutex)*/
        {
            //CAutoLogDeadlock AutoLgDL(__FUNCTION__);
            m_JarCuInstallDr.Init(sc_MAX_MAPINSTALLCOUNTER);
            m_uCounterIDMg = 1;

        }		

        CCountersMg::~CCountersMg()
        {
            //CAutoLogDeadlock AutoLgDL(__FUNCTION__);
            m_MapUser.clear();
            m_map_regcounters.clear();
            m_JarCuInstallDr.UnInit();
        }
        unsigned CCountersMg::GetNewCounterID()
        {
            while(1)
            {
                unsigned uNewID = m_uCounterIDMg++;
                if(m_uCounterIDMg>=sc_INVALID_COUNTERID-1)
                    m_uCounterIDMg = 1;
                if(m_map_regcounters.find(uNewID)==m_map_regcounters.end() )
                    return uNewID;
            }
            return 0;
        }

        CUserBookCountersList * CCountersMg::AddUserBookInfo(unsigned uServer_subConnect_id)
        {
            Boost_Scoped_Lock CounterMg_lock(m_mutex);
            if( m_MapUser.find(uServer_subConnect_id)!=m_MapUser.end() )
                return &( m_MapUser.find(uServer_subConnect_id)->second);
            return &( m_MapUser[uServer_subConnect_id] = CUserBookCountersList(uServer_subConnect_id) );
        }

        unsigned CCountersMg::GetCountersInfoListRegToServer( unsigned uServerInstanceID,INPROTOCOL::LIST_RepCounterList &listMsg )
        {
            Boost_Scoped_Lock CounterMg_lock(m_mutex);
 
            INPROTOCOL::TServerRepCounterList *pRepMsg              = 0;
            INPROTOCOL::TServerRepCounterList::COUNTER_ITEM *pCItem = 0;

            unsigned iInsertCount = 0;
            MAP_REGCOUNTER::const_iterator itor = m_map_regcounters.begin();
            for( ;itor!=m_map_regcounters.end();++itor )
            {
				//ostringstream strdd;
				//strdd<<"begin:";
                if( 0==itor->second->uServer_instanceid || itor->second->uServer_instanceid == uServerInstanceID )
                {
                    if( (!pCItem) || iInsertCount==INPROTOCOL::COUNTERLIST_MAXCOUNT)
                    {
                        if( pRepMsg )
                            pRepMsg->count = iInsertCount;
                        iInsertCount = 0;
                        pRepMsg = &(*(listMsg.insert(listMsg.end(),INPROTOCOL::TServerRepCounterList())));
                        pCItem  = pRepMsg->items;
                        memset(pCItem,0,sizeof(INPROTOCOL::TServerRepCounterList::COUNTER_ITEM)*INPROTOCOL::COUNTERLIST_MAXCOUNT );
                    }

                    pCItem->id = itor->second->uCounterID;
                    strncpy(pCItem->name,itor->second->strName,strlen(itor->second->strName) );
                    strncpy(pCItem->path,itor->second->strPath,strlen(itor->second->strPath) );
					strncpy(pCItem->desc,itor->second->strDesc,strlen(itor->second->strDesc) );
					
					//strdd<<iInsertCount<<" "<<pCItem->path<<" "<<pCItem->name;
					//NETLOG_NORMAL( strdd.str().c_str() );

					iInsertCount++;
					pRepMsg->count = iInsertCount;
                    pCItem++;
                }
            }
            return (unsigned)listMsg.size();
        }
        void CCountersMg::GetCounterStateList(MAP_UserBookCounterSate &mapListMsg)
        {
            Boost_Scoped_Lock CounterMg_lock(m_mutex);

            MAP_UserBook::iterator itor = PNGB::m_pCountersMg->m_MapUser.begin();
            for( ;itor!= PNGB::m_pCountersMg->m_MapUser.end();++itor )
            {
                itor->second.GetBookCuStateList(mapListMsg);
            }
        }
        unsigned CCountersMg::IsRegister(const CNiceNetCounter &reg_info)
        {
            MAP_REGCOUNTER::const_iterator itor = m_map_regcounters.begin();
            for(;itor!=m_map_regcounters.end();++itor)
            {
                if( itor->second->strName==reg_info.strName 
                    && itor->second->strPath==reg_info.strPath )
                {
                    if( 0==itor->second->uServer_instanceid )
                        return itor->second->uCounterID;
                    else if(itor->second->uServer_instanceid==reg_info.uServer_instanceid )
                        return itor->second->uCounterID;
                }
            }
            return sc_INVALID_COUNTERID;
        }
        unsigned CCountersMg::IsRegister(unsigned uCounterid)
        {
            MAP_REGCOUNTER::const_iterator itor = m_map_regcounters.find(uCounterid);
            if( itor==m_map_regcounters.end() )
                return sc_INVALID_COUNTERID;
            return itor->second->uCounterID;
        }

        unsigned CCountersMg::RegisterCounter(CNiceNetCounter &reg_info)
        {
            Boost_Scoped_Lock CounterMg_lock(m_mutex);
            if( unsigned iRet=IsRegister(reg_info) != sc_INVALID_COUNTERID )
                return iRet;

            //NETLOG_DBOTHER("RegCounter ok,Path:"<<reg_info.strPath<<"; name:"<<reg_info.strName<<";ID="<<reg_info.uCounterID<<"; "<<__FUNCTION__);
            
            reg_info.uCounterID = this->GetNewCounterID();
            reg_info.SetStateRegrister();
            m_map_regcounters[reg_info.uCounterID] = &reg_info;
            return reg_info.uCounterID;
        }

        void CCountersMg::UnRegisterCounter( CNiceNetCounter &reg_info,LIST_UInt &BookItUserList)
        {
            Boost_Scoped_Lock CounterMg_lock(m_mutex);
            NETLOG_DBOTHER(__FUNCTION__<<"(); regPath:"<<reg_info.strPath<<"; name:"<<reg_info.strName<<";ID="<<reg_info.uCounterID);

            MAP_UserBook::iterator itor = PNGB::m_pCountersMg->m_MapUser.begin();
            for( ;itor!= PNGB::m_pCountersMg->m_MapUser.end();++itor )
            {
                if( itor->second.FindBookItem( reg_info.uCounterID ) )
                {
                    BookItUserList.push_back( itor->second.m_uServer_subConnect_id );
                    break;
                }
            }
            CCountersMg::CHelp_DelBook helpDelBook(reg_info.uCounterID,"UnRegisterCounter");
            m_map_regcounters.erase( reg_info.uCounterID );
            reg_info.ClearStateInstall();
            reg_info.ClearStateRegrister();
        }
        void CCountersMg::UnRegisterAllCounter(LIST_UInt &BookItUserList,LIST_UInt &BookItCounteridList)
        {
            Boost_Scoped_Lock CounterMg_lock(m_mutex);

            MAP_REGCOUNTER::iterator itorRegCt=m_map_regcounters.begin();
            for(;itorRegCt!=m_map_regcounters.end();++itorRegCt)
            {
                CNiceNetCounter &reg_info   = *itorRegCt->second;
                MAP_UserBook::iterator itor = PNGB::m_pCountersMg->m_MapUser.begin();
                for( ;itor!= PNGB::m_pCountersMg->m_MapUser.end();++itor )
                {
                    if( itor->second.FindBookItem( reg_info.uCounterID ) )
                    {
                        BookItUserList.push_back( itor->second.m_uServer_subConnect_id );
                        BookItCounteridList.push_back( reg_info.uCounterID );
                        break;
                    }
                }
                CCountersMg::CHelp_DelBook helpDelBook(reg_info.uCounterID,"UnRegisterCounter");
                reg_info.ClearStateInstall();
                reg_info.ClearStateRegrister();
            }
            m_map_regcounters.clear();
            if(m_MapUser.size()!=0)
			{NETLOG_WARNING(FUN_FILE_LINE);}
            if(m_map_InstallDr.size()!=0)
			{NETLOG_WARNING(FUN_FILE_LINE);}
            m_MapUser.clear();
            m_map_InstallDr.clear();
        }
        void CCountersMg::ServerConnectClose(unsigned uConnectdid)
        {
            Boost_Scoped_Lock CounterMg_lock(m_mutex);
            CCountersMg::CHelp_DelBook helpDelBook(uConnectdid);
        }

        bool CCountersMg::InstallAndUnInstallCounter(unsigned uConnectid,unsigned uCounterid,unsigned interval)
        {
            Boost_Scoped_Lock CounterMg_lock(m_mutex);

            MAP_REGCOUNTER::iterator regItor = m_map_regcounters.find( uCounterid );
            if( regItor==m_map_regcounters.end() )
                return false;
            CNiceNetCounter *pCounter =regItor->second;

            CNiceNetCounter &reg_info = *pCounter;
            //NETLOG_NORMAL(__FUNCTION__<<"(); regPath:"<<reg_info.strPath<<"; name:"<<reg_info.strName<<";ID="<<reg_info.uCounterID);

            if( sc_INVALID_COUNTERID==this->IsRegister(uCounterid) )
                return (0==interval)?true:false;

            CUserBookCountersList *pUser = this->AddUserBookInfo( uConnectid );
            if( !pUser )
            {
                NETLOG_ERROR(__FUNCTION__<<"(); Add new user to map fail!");
                return (0==interval)?true:false;
            }

            TBookItem *pBook = pUser->FindBookItem( uCounterid );
            if( pBook )
            {
                if( 0==interval )
                {
                    CCountersMg::CHelp_DelBook helpDelBook(uConnectid,uCounterid);
                }else
                {
                    pBook->interval =interval;
                }
                return true;
            }

            pBook = pUser->AddBookItem(uCounterid, *reg_info.GetBaseValue() );
            //updatevalue_handler_type upValueHand=boost::bind(CuMg_UpdateValue,pBook->uCuValue,_1,_2);
            CCounterInstallDriver *pCuInstallDr = 0;
            if( pBook && (pCuInstallDr = this->AddCuInstallDr( uCounterid )) &&
                (pCuInstallDr->AddNew(uConnectid))
              )
            {
                pBook->interval = interval;
                pCounter->SetStateInsatall();
                return true;
            }else
            {
                CCountersMg::CHelp_DelBook helpDelBook(uConnectid,uCounterid);
                return false;
            }
        }

        void CCountersMg::UpdateCounterValue(CNiceNetCounter &reg_info,unsigned uValue,EmCounterUpdateKind opKind)
        {
            Boost_Scoped_Lock CounterMg_lock(m_mutex);                
            MAP_CUINSTALLDR::iterator itor = m_map_InstallDr.find(reg_info.uCounterID);
            if( itor!=m_map_InstallDr.end() )
                itor->second->Update(uValue,opKind);
        }
		/***********************************************************************
		unsigned CCountersMg::GetCounterCurValue( unsigned uCounterID )
		{
		Boost_Scoped_Lock CounterMg_lock(m_mutex);                
		MAP_REGCOUNTER::iterator itor = m_map_regcounters.find(uCounterID);
		if( itor!=m_map_regcounters.end() )
		return itor->second->GetCurValue();
		return 0XFFFFFFFF;
		}
		************************************************************************/
        CCounterInstallDriver * CCountersMg::AddCuInstallDr(unsigned uCounterID)
        {
            MAP_CUINSTALLDR::iterator itor = m_map_InstallDr.find(uCounterID);
            if( itor!=m_map_InstallDr.end() )
                return itor->second;

            CCounterInstallDriver *pNewDr = m_JarCuInstallDr.FetchObj();
            if( pNewDr )
                m_map_InstallDr[uCounterID] = pNewDr;
            return pNewDr;
        }

    }	// end of namespace net

}	// end of namespace peony
