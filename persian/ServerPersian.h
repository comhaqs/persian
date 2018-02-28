#ifndef __SERVER_PERSIAN_H__
#define __SERVER_PERSIAN_H__

#include <map>
#include <atomic>
#include <boost/asio.hpp>
#include "PersianDefine.h"
#include "IProcessor.h"
#include "IContext.h"

class InfoRoute {
public:
	std::string Route;
	SocketPtr pSocket;
	boost::asio::ip::tcp::endpoint Point;
};
typedef std::shared_ptr<InfoRoute> InfoRoutePtr;


class ServerPersian : public IServerNet, public IPlugin, public IProcessor, public IContextProxy, public std::enable_shared_from_this<ServerPersian>
{
public:
	typedef std::function<void (TreePtr)> call_seq;
	typedef std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_ptr;

	virtual bool start();
	virtual void stop();
	virtual bool addListen(const std::string& tag, callback fun);
	virtual void setPort(const int port);
	virtual void setAddress(const std::string& address);
	virtual void setService(ServicePtr pService);
	
	virtual bool write(boost::asio::yield_context yield, TreePtr pData, const std::string route, SocketPtr pSocket);
	virtual bool write(boost::asio::yield_context yield, TreePtr pData, const std::string route);
	virtual bool writeAndWait(boost::asio::yield_context yield, TreePtr pDataWrite, const std::string route, SocketPtr pSocket, TreePtr& pDataRead);
	virtual bool writeAndWait(boost::asio::yield_context yield, TreePtr pDataWrite, const std::string route, TreePtr& pDataRead);
	virtual bool wait(boost::asio::yield_context yield, boost::posix_time::time_duration time);
	
	
protected:
	virtual void readFrame(FramePtr pFrame, unsigned int count, SocketPtr pSocket);
	virtual unsigned int getSeq();
	virtual void connectServer(boost::asio::yield_context yield, InfoRoutePtr pInfo);
	virtual void readServer(boost::asio::yield_context yield, SocketPtr pSocket);

	virtual void createServer(boost::asio::yield_context yield, acceptor_ptr pAcceptor);
	virtual void createSocket(boost::asio::yield_context yield, SocketPtr pSocket);

	template <typename Signature, typename CompletionToken, typename TData>
	auto wait(CompletionToken token, int index) {
		using completion_type = boost::asio::async_completion<CompletionToken, Signature>;
		completion_type completion{ token };
		auto handler = completion.completion_handler;
		std::function<void(std::shared_ptr<TData>)> fun = std::bind([handler](std::shared_ptr<TData> pd) {
			using boost::asio::asio_handler_invoke;
			asio_handler_invoke(std::bind(handler, pd), &handler);
		}, std::placeholders::_1);
		m_MapCallSeq.insert(std::make_pair(index, fun));
		return completion.result.get();
	};

	std::map<std::string, std::vector<callback> > m_MapCallback;
	std::map<std::string, InfoRoutePtr> m_MapRoute;
	IProtocolPtr m_pProtocol;
	std::atomic_uint16_t m_Seq;
	std::map<int, call_seq> m_MapCallSeq;
	ServicePtr m_pService;
	std::atomic_bool m_bStop;
	int m_Port;
	std::string m_Address;
};
typedef std::shared_ptr<ServerPersian> ServerPersianPtr;




#endif // !__SERVER_PERSIAN_H__



