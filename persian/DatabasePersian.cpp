#pragma warning(disable:4996)
#include "stdafx.h"
#include "DatabasePersian.h"
#include <boost/date_time.hpp>
#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/DataException.h>


using namespace Poco::Data::Keywords;

bool DatabasePersian::start()
{
	m_pQueue = queue_ptr(new queue_ptr::element_type);
	m_Thread = boost::thread(std::bind(DatabasePersian::handleThread, m_pQueue));
	return IPlugin::start();
}

void DatabasePersian::stop()
{
	m_Thread.interrupt();
}

void DatabasePersian::afterStop()
{
	m_Thread.timed_join(boost::posix_time::seconds(2));
}

TreePtr DatabasePersian::query(boost::asio::yield_context yield, fun_type fun)
{
	return queryDatabase(yield, fun, m_pQueue);
}

void DatabasePersian::exec(boost::asio::yield_context yield, const std::string sql)
{
	queryDatabase<TreePtr::element_type, database_ptr::element_type, queue_ptr::element_type>(
		yield, std::bind(DatabasePersian::funSql, std::placeholders::_1, sql), m_pQueue);
}

TreePtr DatabasePersian::funSql(database_ptr pSession, const std::string sql)
{
	if (!pSession)
	{
		return TreePtr();
	}
	(*pSession) << sql;
	return TreePtr();
}

void DatabasePersian::handleThread(queue_ptr pQueue)
{
	if (!pQueue)
	{
		return;
	}

	while (true)
	{
		try {
			Poco::Data::MySQL::Connector::registerConnector();

			// create a session
			database_ptr pDatabase(new database_ptr::element_type(Poco::Data::MySQL::Connector::KEY
				, "host=127.0.0.1;user=root;password=root;db=sakila;compress=true;auto-reconnect=true;protocol=tcp"));
			int count = 0;
			(*pDatabase) << "select count(*) from city;", into(count), now;
			std::cout <<"database:"<< boost::this_thread::get_id() << count << std::endl;
			while (true)
			{
				auto fun = pQueue->pop();
				if (!fun)
				{
					continue;
				}
				fun(pDatabase);
			}
		}
		catch (boost::thread_interrupted&) {
			break;
		}catch (Poco::Data::DataException& e) {
			auto c = e.displayText();
			std::cout << e.displayText() << std::endl;
		}
		catch (std::exception& e) {
			auto c = e.what();
			std::cout << e.what() << std::endl;
		}
	}
}
