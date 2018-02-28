#include "stdafx.h"
#include "ContextPersian.h"



ContextPersian::ContextPersian(SocketPtr pSocket, IContextProxyPtr pProxy, std::string route, boost::asio::yield_context & yield)
	:m_pSocket(pSocket), m_pProxy(pProxy), m_Route(route), m_Yield(yield)
{

}

ContextPersian::ContextPersian(ContextPersian & context, boost::asio::yield_context & yield)
	: ContextPersian(context.m_pSocket, context.m_pProxy, context.m_Route, yield)
{
}

bool ContextPersian::write(TreePtr pData)
{
	if (!m_pProxy) {
		return false;
	}
	return m_pProxy->write(m_Yield, pData, m_Route, m_pSocket);
}

bool ContextPersian::writeAndWait(TreePtr pDataWrite, TreePtr& pDataRead)
{
	if (!m_pProxy) {
		return false;
	}
	return m_pProxy->writeAndWait(m_Yield, pDataWrite, m_Route, m_pSocket, pDataRead);
}

bool ContextPersian::write(TreePtr pData, const std::string route)
{
	if (!m_pProxy) {
		return false;
	}
	return m_pProxy->write(m_Yield, pData, route);
}

bool ContextPersian::writeAndWait(TreePtr pDataWrite, const std::string route, TreePtr& pDataRead)
{
	if (!m_pProxy) {
		return false;
	}
	return m_pProxy->writeAndWait(m_Yield, pDataWrite, route, pDataRead);
}

bool ContextPersian::wait(boost::posix_time::time_duration time)
{
	if (!m_pProxy) {
		return false;
	}
	return m_pProxy->wait(time);
}

bool ContextPersian::waitBySeconds(const long time)
{
	if (!m_pProxy) {
		return false;
	}
	return m_pProxy->wait(boost::posix_time::seconds(time));
}