#include "../header/ImpNetLog.h"
#include "../header/NormalThreadMg.h"

#ifndef WIN32
	#include <sys/stat.h>
#else
	#include <Windows.h>
#endif // WIN32

namespace peony
{
	namespace net
	{
		void ImpThread_WriteData( ImpLogToFile *pself )
		{
			while( !pself->m_writeQ.empty() )
			{
				bool isok = false;
				ImpLogToFile::TBuffer*pwb = pself->m_writeQ.pop( isok );
				if( !isok )
					return;

				pself->WriteLogToFile( pwb );
				pwb->cur_pos = 0;
				pself->m_freeQ.push( pwb );
			}
		}

		void ImpLogToFile::CallThreadWriteData()
		{
			PNet_HandVoid funWrite = boost::bind(ImpThread_WriteData,this );
			CPNetFunTaskMg::Single().PostFun( funWrite );
		}

		ImpLogToFile::ImpLogToFile( string filepath,string filename,
			unsigned filemaxsize,
			unsigned buffer_count,
			unsigned tb_size/*=1024*512*/):
			m_strdir(filepath),m_filename(filename),
			m_filemaxsize(filemaxsize),
			m_index_file(1),m_file(0)
		{
			m_LastWritLogTm = PNGB::m_server_curTm;
			m_isStop     = false;
			tb_size      =(tb_size<1024*10) ? (1024*10):tb_size;
			tb_size      =(tb_size<1024*1024*2) ? (1024*1024*2):tb_size;

			buffer_count =(buffer_count<1) ? 1:buffer_count;
			buffer_count =(buffer_count>10) ? 10:buffer_count;

			for( unsigned t=0; t<buffer_count; ++t ){
				m_freeQ.push( new TBuffer(tb_size) );
			}
			m_cur_tb = 0;
		}
		ImpLogToFile::~ImpLogToFile()
		{
			if( m_cur_tb )
			{
				m_writeQ.push( m_cur_tb );
				m_cur_tb = 0;
			}

			while( !m_writeQ.empty() )
			{
				bool isok = false;
				TBuffer*pwb = m_writeQ.pop(isok);
				if( !isok )
					return;
				WriteLogToFile( pwb );
				delete pwb;
			}

			while( !m_freeQ.empty() )
			{
				bool isok = false;
				TBuffer*pwb = m_freeQ.pop( isok );
				if( !isok )
					return;
				WriteLogToFile( pwb );
				delete pwb;
			}

			if( m_file )
				fclose(m_file);
		}
	
		void ImpLogToFile::stop()
		{
			m_isStop = true;
		}

		bool ImpLogToFile::is_have_buffer()
		{
			if( 0 == m_cur_tb )
			{
				bool isok = false;
				m_cur_tb = m_freeQ.pop( isok );
				if( !isok )
					m_cur_tb = 0;
			}
			return (0!=m_cur_tb);
		}

		void ImpLogToFile::RealtimeSaveLog()
		{
			if( PNGB::m_server_curTm<m_LastWritLogTm )
			{
				m_LastWritLogTm = PNGB::m_server_curTm;
				return;
			}
			if( PNGB::m_server_curTm-m_LastWritLogTm<0 )
				return;
			m_LastWritLogTm = PNGB::m_server_curTm;

			if( m_cur_tb && m_cur_tb->cur_pos>0 )
			{
				m_writeQ.push( m_cur_tb );
				m_cur_tb = 0;
				CallThreadWriteData();
			}
		}
		void ImpLogToFile::write_log(const char *buffer,unsigned size)
		{
			if( m_isStop )
				return;

			if( m_writeQ.size()>0 ){
				CallThreadWriteData();
			}

			if( !is_have_buffer() ){
				CallThreadWriteData();
			}

			if( 0==m_cur_tb )
				return;

			//如果这个缓冲区的整个大小还没有这个条log大，还写过屁；
			if( m_cur_tb->bf_size<size )
				return;

			if( (m_cur_tb->bf_size-m_cur_tb->cur_pos)<=size )
			{
				m_writeQ.push( m_cur_tb );
				CallThreadWriteData();
				m_cur_tb = 0;
			}

			if(	!is_have_buffer()  )
				return;
			if( !m_cur_tb )
				return;

			memcpy( &m_cur_tb->m_buffer[m_cur_tb->cur_pos], buffer,size );
			m_cur_tb->cur_pos += size;
		}

		typedef vector< string > NetSplitVector;
		static string get_selfExe_path()
		{
			char chTemp[512]={0};
			memset(chTemp,0,sizeof(chTemp) );

			#ifdef WIN32
				if( 0 == GetModuleFileName(NULL, &chTemp[0], sizeof(chTemp) ) )
					return "";
				int iPos =0;
				int i=sizeof(chTemp)-1;
				while(chTemp[i] != '\\' )
					i--;
				chTemp[i+1] = 0;	
				return string( chTemp );
			#else
				int count = readlink( "/proc/self/exe", chTemp, sizeof(chTemp) );
				if (count > 0 && count < ((int)sizeof(chTemp)) )
					chTemp[count] = 0;

				string strTta="";
				NetSplitVector SplitVec;
				boost::split( SplitVec, chTemp, boost::is_any_of("/") );
				for(int t=0;t<(int)(SplitVec.size()-1);t++)
				{
					if( SplitVec[t].empty() )
						continue;
					strTta+=SplitVec[t];
					strTta+="/";
				}
				return strTta;
			#endif // WIN32
			return "";
		}
		static string get_log_path( string strXmlPath )
		{
			boost::erase_all(strXmlPath, " ");
			bool isAbs = true; //是否是绝对路径
			string strLogPath="";
			#ifdef WIN32
				if( strXmlPath.size()<=1 )
					isAbs = false;
				else{
					char chFirstCc = strXmlPath[1];
					if( ':'!=chFirstCc )
						isAbs = false;
				}
			#else
			    if( strXmlPath.empty() )
					isAbs = false;
				else{
					char chFirstCc = strXmlPath[0];
					if( '/'!=chFirstCc )
						isAbs = false;
				}
			#endif // WIN32
				if( isAbs )
				{
					strLogPath = strXmlPath;
				}else
				{
					strLogPath = get_selfExe_path();
					strLogPath+="/";
					strLogPath+=strXmlPath;
				}

				strLogPath+="/";
				boost::replace_all(strLogPath, "\\\\", "/");
				boost::replace_all(strLogPath, "\\", "/");
				boost::replace_all(strLogPath, "//", "/");
				boost::replace_all(strLogPath, "//", "/");
				return strLogPath;
		}
		static FILE* create_log_file(const char* chfileName,const char* chdirpath,unsigned index_file)
		{
			string fileName(chfileName);
			string dirpath(chdirpath);
			string strlogfile;

			try{

				dirpath = get_log_path( dirpath ); //full_path.file_string();
				//printf( "[create_log_file___zht  aa] %s \n\r",dirpath.c_str() );
				if(1)
				{//create path
					NetSplitVector SplitVec;
					boost::split( SplitVec, dirpath, boost::is_any_of("/") );
					int iCount = (int)SplitVec.size();
					string strTempDir="";
					#ifndef WIN32
						strTempDir+="/";
					#endif
					for(int t=0;t<iCount;t++)
					{
						if( SplitVec[t].empty() )
							continue;

						strTempDir+=SplitVec[t];
						strTempDir+="/";

						//printf( "[create_log_file___zht  t=%d] %s \n\r",t,strTempDir.c_str() );
						if(1)
						{
						#ifdef WIN32
							boost::filesystem::path temp_Path(strTempDir);
							if( !boost::filesystem::is_directory( temp_Path ) )
							{boost::filesystem::create_directory(temp_Path);}
						#else
							if( 0!=access(strTempDir.c_str(),F_OK) )
							{
								mkdir(strTempDir.c_str(),0744);
							}
						#endif // WIN32
						}
					}
					SplitVec.clear();
					strlogfile = strTempDir;
				}
				strlogfile += fileName;

				if(1)
				{//create filename
					boost::posix_time::ptime curtime(boost::posix_time::second_clock::local_time());
					struct tm  tmRet   = boost::posix_time::to_tm( curtime );
					struct tm* ptmTemp = &tmRet;

					char chTempBuf[256]={0};
					strftime(chTempBuf, 256, "_%Y%m%d%H%M%S_", ptmTemp);
					strlogfile+=chTempBuf;
					strlogfile += boost::lexical_cast<string>( index_file );
					strlogfile += ".log";
				}

				//create file
				FILE *pfile = fopen(strlogfile.c_str(),"w");
				return pfile;

			}catch(...)
			{
				//ostringstream strLog;
				//strLog<<"dirpath,filename have error:"<<dirpath<<";"<<fileName<<endl;
				//std::cout<<strLog.str();
				return 0;
			}
		}

		void ImpLogToFile::WriteLogToFile( TBuffer *ptb )
		{
			if( 0==ptb )
				return;
			if( 0==ptb->cur_pos )
				return;

			if( m_file )
			{
				long pos = ftell( m_file );
				if(pos>=(long)m_filemaxsize )
				{
					fclose( m_file );
					m_file = 0;
				}
			}

			if( !m_file ){
				m_file = create_log_file( m_filename.c_str(),m_strdir.c_str(),m_index_file++ );
			}

			if(0==m_file)
				return;

			fwrite( ptb->m_buffer,ptb->cur_pos,1,m_file );
			fflush( m_file );
		}
	}
}