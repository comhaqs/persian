#ifndef __ICONTEXT_H__
#define __ICONTEXT_H__



#include <memory>
#include <string>
#include <boost/date_time.hpp>
#include <boost/asio/spawn.hpp>
#include "PersianDefine.h"

template<typename TData>
class IContextProxy {
public:
	typedef std::shared_ptr<TData> data_ptr;

	virtual bool write(boost::asio::yield_context yield, data_ptr pData, const std::string route, SocketPtr pSocket) = 0;
	virtual bool write(boost::asio::yield_context yield, data_ptr pData, const std::string route) = 0;
	virtual bool writeAndWait(boost::asio::yield_context yield, data_ptr pDataWrite, const std::string route, SocketPtr pSocket, data_ptr& pDataRead) = 0;
	virtual bool writeAndWait(boost::asio::yield_context yield, data_ptr pDataWrite, const std::string route, data_ptr& pDataRead) = 0;
	virtual bool wait(boost::asio::yield_context yield, boost::posix_time::time_duration time) = 0;
};

template<typename TData>
class IContext {
public:
	typedef std::shared_ptr<TData> data_ptr;

	virtual bool write(data_ptr pData) = 0;
	virtual bool writeAndWait(data_ptr pDataWrite, data_ptr& pDataRead) = 0;
	virtual bool write(data_ptr pData, const std::string route) = 0;
	virtual bool writeAndWait(data_ptr pDataWrite, const std::string route, data_ptr& pDataRead) = 0;
	virtual bool wait(boost::posix_time::time_duration time) = 0;
	virtual bool waitBySeconds(const long time) = 0;
};






#endif // !__ICONTEXT_H__

