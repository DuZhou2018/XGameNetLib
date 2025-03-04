#pragma once
#include "./QueueSafe.h"
// #include <boost/thread.hpp>
// #include <boost/filesystem.hpp>
// #include <boost/algorithm/string.hpp>
// #include <boost/lexical_cast.hpp>

namespace peony
{
	namespace net
	{
		class ImpLogToFile
		{
		public:
			class TBuffer
			{
			public:
				char      *m_buffer;
				unsigned   bf_size;
				unsigned   cur_pos;

				TBuffer( unsigned bsize )
				{
					m_buffer = new char[bsize];
					bf_size  = bsize;
					cur_pos  = 0;
				}
				~TBuffer()
				{
					if( m_buffer )
						delete []m_buffer;
					m_buffer = 0;
					bf_size  = 0;
					cur_pos  = 0;
				}
			};

			typedef peony::net::TSaveQueue<TBuffer*> BufferQU;

		public:
			ImpLogToFile(	string filepath,string filename,
							unsigned filemaxsize,
							unsigned buffer_count,
							unsigned tb_size=1024*512);
			~ImpLogToFile();

			void write_log(const char *buffer,unsigned size);
			void RealtimeSaveLog();

		public:
			void stop();

		private:
			friend void ImpThread_WriteData( ImpLogToFile *pself );
			void WriteLogToFile( TBuffer *ptb );
			bool is_have_buffer();
			void CallThreadWriteData();

		private:
			TBuffer					*m_cur_tb;
			BufferQU				 m_freeQ;
			BufferQU				 m_writeQ;
			bool                     m_isStop;
			string				 	 m_strdir;
			string					 m_filename;

			unsigned                 m_LastWritLogTm; //最近一次写入文件时间
			unsigned			     m_filemaxsize;
			unsigned			     m_index_file;
			FILE				    *m_file;
		};
	}
}