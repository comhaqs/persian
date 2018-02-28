#ifndef __CONTEXT_PERSIAN_H__
#define __CONTEXT_PERSIAN_H__


#include "IContext.h"
#include "PersianDefine.h"
#include <boost/asio/spawn.hpp>

template<typename TData>
class ContextPersian : public IContext<TData>
{
public:
	typedef std::shared_ptr<IContextProxy<TData> > proxy_ptr;

	ContextPersian(SocketPtr pSocket, proxy_ptr pProxy, std::string route, boost::asio::yield_context& yield) 
		:m_pSocket(pSocket), m_pProxy(pProxy), m_Route(route), m_Yield(yield)
	{
	};

	ContextPersian(ContextPersian& context, boost::asio::yield_context& yield) 
		: ContextPersian(context.m_pSocket, context.m_pProxy, context.m_Route, yield)
	{
	};

	virtual bool write(data_ptr pData) {
		if (!m_pProxy) {
			return false;
		}
		return m_pProxy->write(m_Yield, pData, m_Route, m_pSocket);
	};

	virtual bool writeAndWait(data_ptr pDataWrite, data_ptr& pDataRead) {
		if (!m_pProxy) {
			return false;
		}
		return m_pProxy->writeAndWait(m_Yield, pDataWrite, m_Route, m_pSocket, pDataRead);
	};

	virtual bool write(data_ptr pData, const std::string route) {
		if (!m_pProxy) {
			return false;
		}
		return m_pProxy->write(m_Yield, pData, route);
	};

	virtual bool writeAndWait(data_ptr pDataWrite, const std::string route, data_ptr& pDataRead) {
		if (!m_pProxy) {
			return false;
		}
		return m_pProxy->writeAndWait(m_Yield, pDataWrite, route, pDataRead);
	};

	virtual bool wait(boost::posix_time::time_duration time) {
		if (!m_pProxy) {
			return false;
		}
		return m_pProxy->wait(m_Yield, time);
	};

	virtual bool waitBySeconds(const long time) {
		if (!m_pProxy) {
			return false;
		}
		return m_pProxy->wait(m_Yield, boost::posix_time::seconds(time));
	};
protected:
	SocketPtr m_pSocket;
	proxy_ptr m_pProxy;
	boost::asio::yield_context m_Yield;
	std::string m_Route;
};







#endif // !__CONTEXT_PERSIAN_H__



