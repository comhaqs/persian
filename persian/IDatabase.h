#ifndef __IDATABASE_H__
#define __IDATABASE_H__

#include <Poco/Data/Session.h>
#include "PersianDefine.h"

template<typename TData>
class IDatabase {
public:
	typedef std::shared_ptr<Poco::Data::Session> database_ptr;
	typedef std::function<std::shared_ptr<TData>(database_ptr)> fun_type;

	virtual std::shared_ptr<TData> query(boost::asio::yield_context yield, fun_type fun) = 0;
	virtual void exec(boost::asio::yield_context yield, const std::string sql) = 0;
};




#endif
