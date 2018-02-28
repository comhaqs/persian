#ifndef __CONTEXT_PERSIAN_H__
#define __CONTEXT_PERSIAN_H__


#include "IContext.h"
#include "PersianDefine.h"
#include <boost/asio/spawn.hpp>


class ContextPersian : public IContext
{
public:
	ContextPersian(SocketPtr pSocket, IContextProxyPtr pProxy, std::string route, boost::asio::yield_context& yield);
	ContextPersian(ContextPersian& context, boost::asio::yield_context& yield);

	virtual bool write(TreePtr pData);
	virtual bool writeAndWait(TreePtr pDataWrite, TreePtr& pDataRead);
	virtual bool write(TreePtr pData, const std::string route);
	virtual bool writeAndWait(TreePtr pDataWrite, const std::string route, TreePtr& pDataRead);
	virtual bool wait(boost::posix_time::time_duration time);
	virtual bool waitBySeconds(const long time);
protected:
	SocketPtr m_pSocket;
	IContextProxyPtr m_pProxy;
	boost::asio::yield_context& m_Yield;
	std::string m_Route;
};
typedef std::shared_ptr<ContextPersian> ContextPersianPtr;







#endif // !__CONTEXT_PERSIAN_H__



