#include "stdafx.h"
#include "ServerPersian.h"
#include "ContextPersian.h"


#define FRAME_MAX 256

bool ServerPersian::start()
{
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
		if (m_Address)
		{
			point = (boost::asio::ip::address::from_string(m_Address), m_Port);
		}else {
			point = (boost::asio::ip::tcp::v4(), m_Port);
		}
		acceptor_ptr pAcceptor(new acceptor_ptr::element_type(*pService, point));
		boost::asio::spawn(*pService, std::bind(&ServerTcp::createServer, shared_from_this(), std::placeholders::_1, pAcceptor));
	}
	catch (std::exception&) {
		return false;
	}
	return true;
}

void ServerPersian::stop()
{
	m_bStop = true;
}

bool ServerPersian::addListen(const std::string & tag, callback fun)
{
	auto Iter = m_MapCallback.find(tag);
	if (m_MapCallback.end() == Iter) {
		Iter = m_MapCallback.insert(std::make_pair(tag, std::vector<callback>())).first;
	}
	Iter->second.push_back(fun);
	return true;
}

void ServerPersian::setPort(const int port)
{
	m_Port = port;
}

void ServerPersian::setAddress(const std::string& address)
{
	m_Address = address;
}

void ServerPersian::setService(ServicePtr pService)
{
	m_pService = pService;
}

bool ServerPersian::write(boost::asio::yield_context yield, TreePtr pData, const std::string route, SocketPtr pSocket)
{
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
}

bool ServerPersian::write(boost::asio::yield_context yield, TreePtr pData, const std::string route)
{
	auto Iter = m_MapRoute.find(route);
	if (m_MapRoute.end() == Iter || !(Iter->second->pSocket)) {
		return false;
	}
	return write(yield, pData, route, Iter->second->pSocket);
}

bool ServerPersian::writeAndWait(boost::asio::yield_context yield, TreePtr pDataWrite, const std::string route, SocketPtr pSocket, TreePtr& pDataRead)
{
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
	pDataRead = wait<void(TreePtr), boost::asio::yield_context, TreePtr>(yield, pRequest->Seq);
	return true;
}

bool ServerPersian::writeAndWait(boost::asio::yield_context yield, TreePtr pDataWrite, const std::string route, TreePtr & pDataRead)
{
	auto Iter = m_MapRoute.find(route);
	if (m_MapRoute.end() == Iter || !(Iter->second->pSocket)) {
		return false;
	}
	return writeAndWait(yield, pDataWrite, route, Iter->second->pSocket, pDataRead);
}

bool ServerPersian::wait(boost::asio::yield_context yield, boost::posix_time::time_duration time)
{
	boost::asio::deadline_timer timer(*m_pService, time);
	boost::system::error_code ec;
	timer.async_wait(yield[ec]);
	return true;
}




void ServerPersian::readFrame(FramePtr pFrame, unsigned int count, SocketPtr pSocket)
{
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
		ContextPersianPtr pContext(new ContextPersianPtr::element_type(pSocket, that, pResponse->Route, yield));
		for (auto fun : funs) {
			fun(pContext, pResponse->pData);
		}
	});
}

unsigned int ServerPersian::getSeq()
{
	return ++m_Seq;
}

void ServerPersian::connectServer(boost::asio::yield_context yield, InfoRoutePtr pInfo)
{
	if (pInfo->pSocket) {
		return;
	}
	auto pSocket = SocketPtr(new SocketPtr::element_type(*m_pService));
	boost::system::error_code ec;
	pSocket->async_connect(pInfo->Point, yield[ec]);
	if (ec) {
		return;
	}
	pInfo->pSocket = pSocket;
	boost::asio::spawn(pSocket->get_io_service(), std::bind(&ServerPersian::readServer, shared_from_this(), std::placeholders::_1, pSocket));
}

void ServerPersian::readServer(boost::asio::yield_context yield, SocketPtr pSocket)
{
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
}

void ServerPersian::createServer(boost::asio::yield_context yield, acceptor_ptr pAcceptor)
{
	while (true) {
		SocketPtr pSocket(new SocketPtr::element_type(pAcceptor->get_io_service()));
		boost::system::error_code ec;
		pAcceptor->async_accept(*pSocket, yield[ec]);
		if (ec) {
			std::cout << ec.message() << std::endl;
			break;
		}
		boost::asio::spawn(pSocket->get_io_service(), std::bind(&ServerTcp::createSocket, shared_from_this(), std::placeholders::_1, pSocket));
	}
}

void ServerPersian::createSocket(boost::asio::yield_context yield, SocketPtr pSocket)
{
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
}
