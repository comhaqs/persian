#ifndef __SERVICE_TCP_H__
#define __SERVICE_TCP_H__

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/coroutine2/all.hpp>
#include <vector>
#include "PersianDefine.h"


class ServerTcp : public IServerNet, public std::enable_shared_from_this<ServerTcp> {
public:
	typedef std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_ptr;

	ServerTcp();
	virtual bool start();
	virtual void stop();

	virtual void setPort(const int port);
	virtual void setCallback(callback fun);
	virtual void setService(ServicePtr pService);
protected:
	static void createServer(boost::asio::yield_context yield, callback fun, acceptor_ptr pAcceptor);
	static void createSocket(boost::asio::yield_context yield, SocketPtr pSocket, callback fun);

	int m_Port;
	ServicePtr m_pService;
	callback m_Callback;
};
typedef std::shared_ptr<ServerTcp> ServerTcpPtr;





#endif // !__SERVICE_TCP_H__
