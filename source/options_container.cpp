#include "../header/options_container.hpp"
#include "../header/Scheduler.hpp"

namespace peony{
	namespace net{
		bool options_container::set_option(const boost::any& option_item,bool init_param/*=false*/)
		{
			if(option_item.empty())
				return false;
			const std::type_info& param_info=option_item.type();
			OptionsMapIt it=m_options_map.find(param_info.name());

			if(init_param)
			{
				if(it!=m_options_map.end())
				{
					//OMP_LOG_ERROR(Scheduler::getInstance().get_logger(),__FUNCTION__ << " option-item was initialized repeatlly: " << param_info.name()
					//	<< " file: " << __FILE__ << " line: " << __LINE__ );
					return false;
				}

				m_options_map.insert(std::make_pair(param_info.name(),option_item));
				/*因Scheduler的构造函数里会调用到set_option函数,所以这里会有重入现象.gcc下的boost::call_once会出现wait死锁.故先注释掉.
				OMP_LOG_DEBUG(Scheduler::getInstance().get_logger(),__FUNCTION__ << " option-item was initialized: " << param_info.name()
					<< " file: " << __FILE__ << " line: " << __LINE__ );
				*/
				return true;
			}

			if(it==m_options_map.end())
			{
				//OMP_LOG_ERROR(Scheduler::getInstance().get_logger(),__FUNCTION__ << " wrong option-item name: " << param_info.name()
				//	<< " file: " << __FILE__ << " line: " << __LINE__ );
				return false;
			}

			it->second=option_item;
			return true;
		}

		bool options_container::get_option(boost::any& option_item)
		{
			if(option_item.empty())
				return false;

			const std::type_info& info=option_item.type();
			OptionsMapIt it=m_options_map.find(info.name());
			if(it==m_options_map.end())
			{
				//OMP_LOG_ERROR(Scheduler::getInstance().get_logger(),__FUNCTION__ << " wrong option type_name: " << info.name()
				//	<< " file: " << __FILE__ << " line: " << __LINE__ );
				return false;
			}

			option_item=it->second;
			return true;
		}
	}
}
