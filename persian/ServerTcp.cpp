#include "stdafx.h"
#include "ServerTcp.h"
#include <iostream>
#include "ILog.h"


#define FRAME_MAX 256

ServerTcp::ServerTcp()
	: m_Port(0)
{
}

bool ServerTcp::start()
{
	if (65536 < m_Port || 0 >= m_Port) {
		return false;
	}
	if (!m_Callback) {
		return false;
	}
	try {
		ServicePtr pService = m_pService;
		if (!pService) {
			return false;
		}
		boost::asio::ip::tcp::endpoint point(boost::asio::ip::tcp::v4(), m_Port);
		acceptor_ptr pAcceptor(new acceptor_ptr::element_type(*pService, point));
		boost::asio::spawn(*pService, std::bind(ServerTcp::createServer, std::placeholders::_1, m_Callback, pAcceptor));
		std::thread([](ServicePtr pService) {
			boost::asio::io_service::work work(*pService);
			pService->run();
		}, pService).detach();
	}
	catch (std::exception&) {
		return false;
	}
	return true;
}

void ServerTcp::stop()
{
}

void ServerTcp::setPort(const int port)
{
	m_Port = port;
}

void ServerTcp::setCallback(callback fun)
{
	m_Callback = fun;
}

void ServerTcp::setService(ServicePtr pService)
{
	m_pService = pService;
}

void ServerTcp::createSocket(boost::asio::yield_context yield, SocketPtr pSocket, callback fun)
{
	FramePtr pFrame(new FramePtr::element_type);
	pFrame->resize(FRAME_MAX);
	while (true) {
		boost::system::error_code ec;
		auto count = pSocket->async_read_some(boost::asio::buffer(*pFrame, FRAME_MAX), yield[ec]);
		if (ec) {
			break;
		}
		fun(pFrame, count, pSocket);
	}
}

void ServerTcp::createServer(boost::asio::yield_context yield,  callback fun, acceptor_ptr pAcceptor)
{
	while (true) {
		SocketPtr pSocket(new SocketPtr::element_type(pAcceptor->get_io_service()));
		boost::system::error_code ec;
		pAcceptor->async_accept(*pSocket, yield[ec]);
		if (ec) {
			std::cout << ec.message() << std::endl;
			break;
		}
		boost::asio::spawn(pSocket->get_io_service(), std::bind(ServerTcp::createSocket, std::placeholders::_1, pSocket, fun));
	}
}
