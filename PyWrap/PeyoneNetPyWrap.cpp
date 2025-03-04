//
//copy "F:\ZtyServer\Vc9Bin\lib\PeyoneNetForPyRelease.dll" "D:\python279_64\Lib\site-packages\peyone\peyonenet.pyd"

#pragma warning(disable:4819)

#include <boost/python.hpp>
using namespace boost::python;
#include "./INiceNetWrap.h"

BOOST_PYTHON_MODULE( peyonenet )
{
    {//IAppClient        IAppClient_Wrap
        scope s = class_<IAppClient_Wrap,boost::noncopyable>("IAppClient")
            .def("OnConnect",	pure_virtual(&IAppClientPy::OnConnect))
            .def("OnReciveTcp", pure_virtual(&IAppClientPy::OnReciveTcp))
            ;
    }

	///// 下面是创建server使用到 IAppServer_Wrap
	{//INiceNetServer    IAppServer_Wrap 
		scope s = class_<IAppServer_Wrap,boost::noncopyable>("INiceNetServer")
			.def("OnHttpReq",		  pure_virtual(&IAppServer_Wrap::OnRecHttp))
			.def("OnReciveTcp",		  pure_virtual(&IAppServer_Wrap::OnReciveTcp))
			.def("OnReciveWebSocket", pure_virtual(&IAppServer_Wrap::OnReciveWebSocket))
			.def("ServerStatus", pure_virtual(&IAppServer_Wrap::ServerStatus))
			.def("OnSubConnect", pure_virtual(&IAppServer_Wrap::OnSubConnect))
			;
		{//Scheduler.options scope
		}
	}

    {//INiceNet_Wrap SaveLog
        scope s = class_<INiceNet_Wrap>("INiceNet")
			.def("Init",				&INiceNet_Wrap::Init)
			.def("UnInit",				&INiceNet_Wrap::UnInit)
			.def("Run",					&INiceNet_Wrap::Run)
			.def("PutOneLog",			&INiceNet_Wrap::PutOneLog)
			.def("SaveLog",				&INiceNet_Wrap::SaveLog)
			.def("AddClient",			&INiceNet_Wrap::AddClient)
			.def("DelClient",			&INiceNet_Wrap::DelClient)
			.def("AddTcpServer",		&INiceNet_Wrap::AddServer)
			.def("AddHttpServer",		&INiceNet_Wrap::AddHttpServer)
			.def("AddWebSocketServer",	&INiceNet_Wrap::AddWebSocketServer)
			.def("HttpRepMsg",			&INiceNet_Wrap::HttpRepMsg)
			.def("SendWebSocketMsg",	&INiceNet_Wrap::SendWebSocketMsg)
			.def("SendTcpMsg",          &INiceNet_Wrap::SendTcpMsg)
            .def("SendTcpMsgBb",        &INiceNet_Wrap::SendTcpMsgBb)
            ;
        {//Scheduler.options scope AddHttpServer
        }
    }
}
