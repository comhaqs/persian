#ifndef __ICONTEXT_H__
#define __ICONTEXT_H__



#include <memory>
#include <string>
#include <boost/date_time.hpp>
#include <boost/asio/spawn.hpp>
#include "PersianDefine.h"

class IContextProxy {
public:
	virtual bool write(boost::asio::yield_context yield, TreePtr pData, const std::string route, SocketPtr pSocket) = 0;
	virtual bool write(boost::asio::yield_context yield, TreePtr pData, const std::string route) = 0;
	virtual bool writeAndWait(boost::asio::yield_context yield, TreePtr pDataWrite, const std::string route, SocketPtr pSocket, TreePtr& pDataRead) = 0;
	virtual bool writeAndWait(boost::asio::yield_context yield, TreePtr pDataWrite, const std::string route, TreePtr& pDataRead) = 0;
	virtual bool wait(boost::asio::yield_context yield, boost::posix_time::time_duration time) = 0;
};
typedef std::shared_ptr<IContextProxy> IContextProxyPtr;

class IContext {
public:
	virtual bool write(TreePtr pData) = 0;
	virtual bool writeAndWait(TreePtr pDataWrite, TreePtr& pDataRead) = 0;
	virtual bool write(TreePtr pData, const std::string route) = 0;
	virtual bool writeAndWait(TreePtr pDataWrite, const std::string route, TreePtr& pDataRead) = 0;
	virtual bool wait(boost::posix_time::time_duration time) = 0;
	virtual bool waitBySeconds(const long time) = 0;
};
typedef std::shared_ptr<IContext> IContextPtr;






#endif // !__ICONTEXT_H__

