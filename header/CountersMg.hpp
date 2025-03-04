
#ifndef __PY_NET_COUNTERMG_HEADER__
#define __PY_NET_COUNTERMG_HEADER__

#include <boost/thread.hpp>

#include "./InPublicPreDefine.hpp"

#include "./SimpleObjJar.h"
#include "./InPublicPreDefine.hpp"
#include "./DefineCounter.h"
#include "./NiceNetCounter.h"
#include <boost/pool/object_pool.hpp>

namespace peony {
	namespace net {		

        typedef CSimpleObjJar<CCounterInstallDriver,true> JAR_CUINSTALLDR;
        typedef map<unsigned,CCounterInstallDriver*> MAP_CUINSTALLDR;
        typedef map<unsigned,CUserBookCountersList>MAP_UserBook;
		typedef boost::object_pool<CNiceNetCounter>	 COUNTER_POOL;

		class CCountersMg
		{
		public:
			static unsigned CuMg_UpdateValue(unsigned &u_oldvalue,unsigned& uValue,unsigned/*EmCounterUpdateKind*/ kind);

        private:
            class CHelp_DelCounterInstallDr
            {
            public:
                CHelp_DelCounterInstallDr(unsigned uCounterid,string strDelCounter);
                CHelp_DelCounterInstallDr(unsigned uConnectid);
                CHelp_DelCounterInstallDr(unsigned uConnectid,unsigned uCounterid);
            };
            friend class CHelp_DelCounterInstallDr;

            class CHelp_DelBook
            {
            public:
                CHelp_DelBook(unsigned uCounterid,string strDelCounter);
                CHelp_DelBook(unsigned uConnectid);
                CHelp_DelBook(unsigned uConnectid,unsigned uCounterid);
            };
            friend class CHelp_DelBook;

		protected:		
			friend class PNGB;
			CCountersMg();
		public:
            ~CCountersMg();

        public://importance interface
            //return 0 fail,or return counterid
            unsigned        RegisterCounter(   CNiceNetCounter &reg_info);
            void            UnRegisterCounter( CNiceNetCounter &reg_info, LIST_UInt &BookItUserList);
            void            UnRegisterAllCounter(LIST_UInt &BookItUserList,LIST_UInt &BookItCounteridList);
            bool            InstallAndUnInstallCounter(unsigned uConnectid,unsigned uCounterid,unsigned interval);
            void            UpdateCounterValue(CNiceNetCounter &reg_info,unsigned uValue,EmCounterUpdateKind opKind);
            void            ServerConnectClose(unsigned uConnectdid);

            //for net cmd
            unsigned    GetCountersInfoListRegToServer( unsigned uServerInstanceID,INPROTOCOL::LIST_RepCounterList &listMsg );
            void            GetCounterStateList(MAP_UserBookCounterSate &mapListMsg);

			COUNTER_POOL&   GetCounterPool(){ return m_NetCounter_pool;}
        private:
            unsigned            IsRegister(const CNiceNetCounter &reg_info);
            unsigned            IsRegister(unsigned uCounterid);
            CUserBookCountersList * AddUserBookInfo(unsigned uServer_subConnect_id);
            CCounterInstallDriver * AddCuInstallDr(unsigned uCounterID);

            unsigned            GetNewCounterID();

        private:
            boost::recursive_mutex	m_mutex;

            //regrister counter map
            typedef map<unsigned,CNiceNetCounter*> MAP_REGCOUNTER;
            MAP_REGCOUNTER                      m_map_regcounters;
            unsigned                        m_uCounterIDMg;

            //install counter map
            JAR_CUINSTALLDR                     m_JarCuInstallDr;
            MAP_CUINSTALLDR                     m_map_InstallDr;

            //book counter user map
            MAP_UserBook						m_MapUser;

			COUNTER_POOL					    m_NetCounter_pool;

		};		

	}	// end of namespace net

}	// end of namespace peony

#endif // end of #define __OMP_TCPCONNECTION_HEADER__
