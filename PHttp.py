# coding=utf-8
# this is file write by zhanghongtao,  qq: 1265008170
# purpose:  easy create httpserver,  big concurrent, asynchronous

import peyone
import time
import json
import datetime

#define global ver....
pNiceNet    = 0;
pNiceServer = 0;

class CNetHttpServer(peyone.peyonenet.INiceNetServer):
    def __init__(self):
        peyone.peyonenet.INiceNetServer.__init__(self);
        return;

    #def ServerStatus( self,status ):
    #    return True;

    #def OnSubConnect( self,uServerID, uSubConID, status):
    #    #print "OnSubConnect: uSubConID="+str(uSubConID)+";status="+str(status);
    #    return True;

    #websocket, tcp
    def OnRecive( self,uSubConID,pData,data_len ):
        print( "OnRecive ReqMsg=" + str(pData) + ";" + str(len(pData))+", CurTime="+self.GetCurTimeStr() )
        strRep = str("this is pyrepmsg websocket RepMsg, curtime= ")+self.GetCurTimeStr();
        global pNiceNet;
        pNiceNet.SendWebSocketMsg(uSubConID,strRep,len(strRep),True );
        return True;

    #pData is json;
    #if http req is get: is json
    #if http req is post: json["reqBData"] = postData
    def OnHttpReq( self,uSubConID,pData,data_len):
        print( "ReqMsg=" + str(pData) + ";" + str(len(pData)) )
        strRep = str("this is pyrepmsg ,curtime= ")+self.GetCurTimeStr();
        global pNiceNet;
        pNiceNet.HttpRepMsg(uSubConID,strRep,len(strRep) );
        return True;

    def GetCurTimeStr(self):
        today_time = datetime.datetime.now()
        tomorrow_time = datetime.datetime.strftime(today_time, '%Y-%m-%d %H:%M:%S');
        return str(tomorrow_time);

#global class obj, this is init PNet, and Create one HttpServer;
#http://192.168.3.252:30280/TestFun?Aa=ThisIsTest001
class CMainNetMg:
    def __init__(self):
        return;

    # jsParam = {"ThreadCount": 3, "LogGrade": 5, "LogDir": "D:\\PNetPyExLog", "LogFileName": "TestPyEx"}
    def InitPeonyNet(self):
        jsInitParam = json.loads('{ }')
        jsInitParam["ThreadCount"]  = 3
        jsInitParam["LogGrade"]     = 0
        jsInitParam["LogDir"]       = "D:\\TestPyNetExLog"
        jsInitParam["LogFileName"]  = "Test"
        strJsOne = json.dumps(jsInitParam, ensure_ascii=True)

        global pNiceNet;
        pNiceNet = peyone.peyonenet.INiceNet();
        pNiceNet.Init( strJsOne );

    def RunMainNet(self):
        myLoop = 1;
        while  myLoop>0 :
            time.sleep(0.1);
            pNiceNet.Run(100,10);
            myLoop = myLoop+1;
            pNiceNet.PutOneLog( 1, "This is py mainrun...... myLoop="+str(myLoop) )
            if 0==myLoop%10:
                pNiceNet.SaveLog()
        return

    def StratHttpServer(self):
        global pNiceServer;
        pNiceServer = CNetHttpServer();
        # print str(pNiceServer);
        pNiceNet.AddHttpServer( str("192.168.3.252"),30280,100,1024*1024,pNiceServer );
        return 0;

    def StratWebSocketServer(self):
        global pNiceServer;
        pNiceServer = CNetHttpServer();
        print str(pNiceServer);
        pNiceNet.AddWebSocketServer( str("0.0.0.0"),30282,100,1024*1024,pNiceServer );
        return 0;

if 1:
    print globals().keys();
    print locals().keys();
    mNetMg = CMainNetMg();
    mNetMg.InitPeonyNet();
    mNetMg.StratWebSocketServer();
    mNetMg.RunMainNet();
