// test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "testCoroutine.h"
#include "../persian/ServerPersian.h"
#include "../persian/PersianDefine.h"


#include "../persian/DatabasePersian.cpp"
#include <Poco/Data/DataException.h>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/date_time.hpp>
//#pragma comment (lib,"oci.lib")

void run(ServicePtr pService) {
	try {
		std::cout <<"run:"<< boost::this_thread::get_id() << std::endl;
		boost::asio::io_service::work work(*pService);
		pService->run();
	}
	catch (Poco::Data::DataException& e) {
		auto c = e.displayText();
		std::cout << e.displayText() << std::endl;
	}
	catch (std::exception& e) {
		auto c = e.what();
		std::cout << e.what() << std::endl;
	}
};

int main()
{
	typedef std::shared_ptr<boost::property_tree::ptree>TreePtr; 
	ServicePtr pService(new ServicePtr::element_type);
	
	typedef std::shared_ptr<ServerPersian<TreePtr::element_type> > ServerPersianPtr;
	ServerPersianPtr pServerPersian(new ServerPersianPtr::element_type);
	pServerPersian->setPort(1234);
	pServerPersian->setService(pService);
	/*
	if (!pServerPersian->start() ||  !pServerTcp->start()) {
		return -1;
	}
	*/

	DatabasePersianPtr pDatabase(new DatabasePersianPtr::element_type);
	pDatabase->start();
	std::cout << "main:" << boost::this_thread::get_id() << std::endl;
	boost::asio::spawn(*pService, [pDatabase, pService](boost::asio::yield_context yield) {
		std::cout <<"spawn:"<< boost::this_thread::get_id() << std::endl;
		auto pData = pDatabase->query(yield, [](IDatabase::database_ptr pSession) {
			int count = 0;
			Poco::Nullable<int> c;
			(*pSession) << "select count(*) from city", into(c), now;
			if (c.isNull()) {
				std::cout <<boost::this_thread::get_id()<< " null" << std::endl;
			}
			else {
				std::cout << boost::this_thread::get_id() << c.value() << std::endl;
			}
			data_ptr pTree(new data_ptr::element_type);
			pTree->put("root.count", count);
			return pTree;
		});
		int count = pData->get("root.count", -1);
		std::cout << boost::this_thread::get_id() << " count:" << count << std::endl;

		pService->post([pDatabase, pService]() {
			std::cout << "post2:" << boost::this_thread::get_id() << std::endl;
			boost::asio::spawn(*pService, [pDatabase] (boost::asio::yield_context yield){
				std::cout << "spawn2:" << boost::this_thread::get_id() << std::endl;
				auto pData = pDatabase->query(yield, [](IDatabase::database_ptr pSession) {
					std::cout << "query2:" << boost::this_thread::get_id() << std::endl;
					return data_ptr();
				});
				std::cout << "data2:" << boost::this_thread::get_id() << std::endl;
			});
		});
	});

	boost::thread(std::bind(run, pService));
	
	boost::this_thread::sleep(boost::posix_time::seconds(10));
	pService->stop();
	pDatabase->stop();
	pDatabase->afterStop();
    return 0;
}

