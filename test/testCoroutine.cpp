#include "stdafx.h"
#include "testCoroutine.h"
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <iostream>

#define FRAME_MAX  256

bool testCoroutine::start()
{
	service_ptr pService(new boost::asio::io_service());
	m_pService = pService;
	pService->post(std::bind(&testCoroutine::createServer, this, pService));
	pService->post(std::bind(&testCoroutine::createClient, this, pService));
	std::thread([](service_ptr pService) {
		boost::asio::io_service::work work(*pService);
		pService->run();
	}, pService).detach();
	return true;
}

void testCoroutine::createServer(service_ptr pService)
{
	boost::asio::spawn(*pService, [pService](boost::asio::yield_context yield) {
		boost::asio::ip::tcp::endpoint point(boost::asio::ip::tcp::v4(), 1234);
		acceptor_ptr pAcceptor(new acceptor_ptr::element_type(*pService, point));
		while (true) {
			socket_ptr pSocket(new socket_ptr::element_type(pAcceptor->get_io_service()));
			boost::system::error_code ec;
			pAcceptor->async_accept(*pSocket, yield[ec]);
			if (ec) {
				std::cout << ec.message() << std::endl;
				continue;
			}
			boost::asio::spawn(pSocket->get_io_service(), [pSocket](boost::asio::yield_context yield2) {
				auto pTmpSocket = pSocket;
				frame_ptr pFrame(new frame_ptr::element_type);
				pFrame->resize(FRAME_MAX);
				while (true) {
					boost::system::error_code ec2;
					auto count = pTmpSocket->async_read_some(boost::asio::buffer(*pFrame, FRAME_MAX), yield2[ec2]);
					if (ec2) {
						std::cout << ec2.message() << std::endl;
						break;
					}
					else {
						std::string data((const char*)(pFrame->data()), count);
						std::cout << data<<std::endl;
					}
				}
			});
		}
	});

	boost::asio::spawn(*pService, [pService, this](boost::asio::yield_context yield) {
		boost::asio::deadline_timer timer(*pService);
		timer.expires_from_now(boost::posix_time::seconds(5));
		boost::system::error_code ec;
		timer.async_wait(yield[ec]);

		std::vector<int> keys;
		for (auto keyvalue : m_MapCallback) {
			keys.push_back(keyvalue.first);
		}
		while (!keys.empty()) {
			int indexKey = (rand() % keys.size());
			auto Iter = m_MapCallback.find(keys[indexKey]);
			std::cout << "exe index:" << Iter->first << std::endl;
			(Iter->second)(Iter->first);

			keys.erase(keys.begin() + indexKey);
			m_MapCallback.erase(Iter);
		}
	});
}

void testCoroutine::createClient(service_ptr pService)
{
	static std::vector<socket_ptr> sSokcets;

	for (int i = 0; i < 10; ++i) {
		int index = i;
		socket_ptr pSocket(new socket_ptr::element_type(*pService));
		sSokcets.push_back(pSocket);
		boost::asio::spawn(*pService, [pSocket, index, this](boost::asio::yield_context yield) {
			boost::asio::ip::tcp::endpoint point(boost::asio::ip::address::from_string("127.0.0.1"), 1234);
			boost::system::error_code ec;
			pSocket->async_connect(point, yield[ec]);
			if (ec) {
				std::cout << ec.message() << std::endl;
				return;
			}
			boost::format tmpFormat;
			frame_ptr pFrame(new frame_ptr::element_type());
			tmpFormat.parse("test Coroutine-%d") % index;
			pFrame->resize(tmpFormat.str().size());
			memcpy(pFrame->data(), tmpFormat.str().c_str(), tmpFormat.str().size());
			pSocket->async_write_some(boost::asio::buffer(*pFrame, pFrame->size()), yield[ec]);

			int back = wait3<void (int)>(yield, index);
			std::cout << "exit "<<index<<" -> " << back<< std::endl;
		});
	}
}
