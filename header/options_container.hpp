#ifndef __OPTIONS_CONTAINER_HEADER_FILE__
#define __OPTIONS_CONTAINER_HEADER_FILE__

#include <map>
#include <string>
#include <boost/any.hpp>

namespace peony {
	namespace net {
		class options_container
		{
		public:
			bool set_option(const boost::any& option_item,bool init_param=false);
			bool get_option(boost::any& option_item);

			template<class option_type>
			bool get_option(option_type& dest)
			{
				boost::any any_value=option_type();
				get_option(any_value);
				option_type *ptr=boost::any_cast<option_type>(&any_value);
				if(ptr==NULL) 
					return false;
				dest=*ptr;
				return true;
			}
		private:
			typedef std::map<std::string,boost::any>	OptionsMap;
			typedef OptionsMap::iterator				OptionsMapIt;
			OptionsMap			m_options_map;
		};
	}
}

#endif