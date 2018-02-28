#ifndef __SERVER_PERSIAN_H__
#define __SERVER_PERSIAN_H__

#include <map>
#include <atomic>
#include <boost/asio.hpp>
#include "PersianDefine.h"
#include "IProcessor.h"
#include "IContext.h"
#include "ContextPersian.h"

class InfoRoute {
public:
	std::string Route;
	SocketPtr pSocket;
	boost::asio::ip::tcp::endpoint Point;
};
typedef std::shared_ptr<InfoRoute> InfoRoutePtr;

template<typename TData>
class ServerPersian : public IServerNet, public IPlugin, public IProcessor<TData>, public IContextProxy<TData>, public std::enable_shared_from_this<ServerPersian<TData> >
{
public:
	typedef std::shared_ptr<TData> data_ptr;
	typedef std::function<void (data_ptr)> call_seq;
	typedef std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_ptr;
	typedef std::shared_ptr<ContextPersian<TData> > context_ptr;
	#define FRAME_MAX 256

	virtual bool start() {
		m_bStop = false;

		if (65536 < m_Port || 0 >= m_Port) {
			return false;
		}
		try {
			ServicePtr pService = m_pService;
			if (!pService) {
				return false;
			}
			boost::asio::ip::tcp::endpoint point;
			if (!m_Address.empty())
			{
				point = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(m_Address), m_Port);
			}
			else {
				point = boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), m_Port);
			}
			acceptor_ptr pAcceptor(new acceptor_ptr::element_type(*pService, point));
			boost::asio::spawn(*pService, std::bind(&ServerPersian<TData>::createServer, shared_from_this(), std::placeholders::_1, pAcceptor));
		}
		catch (std::exception&) {
			return false;
		}
		return true;
	};

	virtual void stop() {
		m_bStop = true;
	};

	virtual bool addListen(const std::string& tag, fun_route fun) {
		auto Iter = m_MapCallback.find(tag);
		if (m_MapCallback.end() == Iter) {
			Iter = m_MapCallback.insert(std::make_pair(tag, std::vector<fun_route>())).first;
		}
		Iter->second.push_back(fun);
		return true;
	};

	virtual void setPort(const int port) {
		m_Port = port;
	};

	virtual void setAddress(const std::string& address) {
		m_Address = address;
	};

	virtual void setService(ServicePtr pService) {
		m_pService = pService;
	};
	
	virtual bool write(boost::asio::yield_context yield, data_ptr pData, const std::string route, SocketPtr pSocket) {
		RequestPersianPtr pRequest(new RequestPersianPtr::element_type);
		pRequest->pData = pData;
		if (m_pProtocol && !m_pProtocol->encode(pRequest)) {
			return false;
		}
		boost::system::error_code ec;
		pSocket->async_write_some(boost::asio::buffer(*(pRequest->pFrame), pRequest->pFrame->size()), yield[ec]);
		if (ec) {
			return false;
		}
		return true;
	};

	virtual bool write(boost::asio::yield_context yield, data_ptr pData, const std::string route) {
		auto Iter = m_MapRoute.find(route);
		if (m_MapRoute.end() == Iter || !(Iter->second->pSocket)) {
			return false;
		}
		return write(yield, pData, route, Iter->second->pSocket);
	};

	virtual bool writeAndWait(boost::asio::yield_context yield, data_ptr pDataWrite, const std::string route, SocketPtr pSocket, data_ptr& pDataRead) {
		RequestPersianPtr pRequest(new RequestPersianPtr::element_type);
		pRequest->pData = pDataWrite;
		pRequest->Seq = getSeq();
		if (m_pProtocol && !m_pProtocol->encode(pRequest)) {
			return false;
		}
		boost::system::error_code ec;
		pSocket->async_write_some(boost::asio::buffer(*(pRequest->pFrame), pRequest->pFrame->size()), yield[ec]);
		if (ec) {
			return false;
		}
		pDataRead = wait<void(data_ptr), boost::asio::yield_context, data_ptr::element_type>(yield, pRequest->Seq);
		return true;
	};

	virtual bool writeAndWait(boost::asio::yield_context yield, data_ptr pDataWrite, const std::string route, data_ptr& pDataRead) {
		auto Iter = m_MapRoute.find(route);
		if (m_MapRoute.end() == Iter || !(Iter->second->pSocket)) {
			return false;
		}
		return writeAndWait(yield, pDataWrite, route, Iter->second->pSocket, pDataRead);
	};

	virtual bool wait(boost::asio::yield_context yield, boost::posix_time::time_duration time) {
		boost::asio::deadline_timer timer(*m_pService, time);
		boost::system::error_code ec;
		timer.async_wait(yield[ec]);
		return true;
	};
	
	
protected:
	virtual void readFrame(FramePtr pFrame, unsigned int count, SocketPtr pSocket) {
		ResponsePersianPtr pResponse(new ResponsePersianPtr::element_type);
		pResponse->pFrame = FramePtr(new FramePtr::element_type(pFrame->begin(), pFrame->begin() + count));
		if (m_pProtocol && !m_pProtocol->decode(pResponse)) {
			return;
		}
		auto Iter = m_MapCallback.find(pResponse->Route);
		if (m_MapCallback.end() == Iter) {
			return;
		}
		auto that = shared_from_this();
		auto funs = Iter->second;
		boost::asio::spawn(pSocket->get_io_service(), [pSocket, that, funs, pResponse](boost::asio::yield_context yield) {
			context_ptr pContext(new context_ptr::element_type(pSocket, that, pResponse->Route, yield));
			for (auto fun : funs) {
				fun(pContext, pResponse->pData);
			}
		});
	};

	virtual unsigned int getSeq() {
		return ++m_Seq;
	};

	virtual void connectServer(boost::asio::yield_context yield, InfoRoutePtr pInfo) {
		FramePtr pFrame(new FramePtr::element_type);
		pFrame->resize(FRAME_MAX);
		while (!m_bStop) {
			boost::system::error_code ec;
			auto count = pInfo->pSocket->async_read_some(boost::asio::buffer(*pFrame, FRAME_MAX), yield[ec]);
			if (ec) {
				break;
			}
			RequestPersianPtr pRequest(new RequestPersianPtr::element_type);
			pRequest->pFrame = FramePtr(new FramePtr::element_type(pFrame->begin(), pFrame->begin() + count));
			if (m_pProtocol && !m_pProtocol->decode(pRequest)) {
				continue;
			}
			auto Iter = m_MapCallSeq.find(pRequest->Seq);
			if (m_MapCallSeq.end() == Iter) {
				continue;
			}
			(Iter->second)(pRequest->pData);
		}
	};

	virtual void readServer(boost::asio::yield_context yield, SocketPtr pSocket) {
		FramePtr pFrame(new FramePtr::element_type);
		pFrame->resize(FRAME_MAX);
		while (!m_bStop) {
			boost::system::error_code ec;
			auto count = pSocket->async_read_some(boost::asio::buffer(*pFrame, FRAME_MAX), yield[ec]);
			if (ec) {
				break;
			}
			RequestPersianPtr pRequest(new RequestPersianPtr::element_type);
			pRequest->pFrame = FramePtr(new FramePtr::element_type(pFrame->begin(), pFrame->begin() + count));
			if (m_pProtocol && !m_pProtocol->decode(pRequest)) {
				continue;
			}
			auto Iter = m_MapCallSeq.find(pRequest->Seq);
			if (m_MapCallSeq.end() == Iter) {
				continue;
			}
			(Iter->second)(pRequest->pData);
		}
	};

	virtual void createServer(boost::asio::yield_context yield, acceptor_ptr pAcceptor) {
		while (true) {
			SocketPtr pSocket(new SocketPtr::element_type(pAcceptor->get_io_service()));
			boost::system::error_code ec;
			pAcceptor->async_accept(*pSocket, yield[ec]);
			if (ec) {
				std::cout << ec.message() << std::endl;
				break;
			}
			boost::asio::spawn(pSocket->get_io_service(), std::bind(&ServerPersian<TData>::createSocket, shared_from_this(), std::placeholders::_1, pSocket));
		}
	};

	virtual void createSocket(boost::asio::yield_context yield, SocketPtr pSocket) {
		FramePtr pFrame(new FramePtr::element_type);
		pFrame->resize(FRAME_MAX);
		while (true) {
			boost::system::error_code ec;
			auto count = pSocket->async_read_some(boost::asio::buffer(*pFrame, FRAME_MAX), yield[ec]);
			if (ec) {
				break;
			}
			readFrame(pFrame, count, pSocket);
		}
	};

	template <typename Signature, typename CompletionToken, typename TD>
	auto wait(CompletionToken token, int index) {
		using completion_type = boost::asio::async_completion<CompletionToken, Signature>;
		completion_type completion{ token };
		auto handler = completion.completion_handler;
		std::function<void(std::shared_ptr<TD>)> fun = std::bind([handler](std::shared_ptr<TD> pd) {
			using boost::asio::asio_handler_invoke;
			asio_handler_invoke(std::bind(handler, pd), &handler);
		}, std::placeholders::_1);
		m_MapCallSeq.insert(std::make_pair(index, fun));
		return completion.result.get();
	};

	std::map<std::string, std::vector<fun_route> > m_MapCallback;
	std::map<std::string, InfoRoutePtr> m_MapRoute;
	IProtocolPtr m_pProtocol;
	std::atomic_uint16_t m_Seq;
	std::map<int, call_seq> m_MapCallSeq;
	ServicePtr m_pService;
	std::atomic_bool m_bStop;
	int m_Port;
	std::string m_Address;
};





#endif // !__SERVER_PERSIAN_H__



