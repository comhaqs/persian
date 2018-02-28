#ifndef __DATABASE_PERSIAN_H__
#define __DATABASE_PERSIAN_H__


#include "PersianDefine.h"
#include "IDatabase.h"
#include "QueueCondition.h"
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/DataException.h>

template<typename TData>
class DatabasePersian : public IDatabase<TData>, public IPlugin
{
public:
	typedef std::shared_ptr<TData> data_ptr;
	typedef std::shared_ptr<QueueCondition<std::function<void (database_ptr)>> > queue_ptr;

	virtual bool start() {
		m_pQueue = queue_ptr(new queue_ptr::element_type);
		m_Thread = boost::thread(std::bind(DatabasePersian::handleThread, m_pQueue));
		return IPlugin::start();
	};

	virtual void stop() {
		m_Thread.interrupt();
	};

	virtual void afterStop() {
		m_Thread.timed_join(boost::posix_time::seconds(2));
	};

	virtual data_ptr query(boost::asio::yield_context yield, fun_type fun) {
		return queryDatabase(yield, fun, m_pQueue);
	};

	virtual void exec(boost::asio::yield_context yield, const std::string sql) {
		queryDatabase<data_ptr::element_type, database_ptr::element_type, queue_ptr::element_type>(
			yield, std::bind(DatabasePersian::funSql, std::placeholders::_1, sql), m_pQueue);
	};

protected:
	static data_ptr funSql(database_ptr pSession, const std::string sql) {
		if (!pSession)
		{
			return data_ptr();
		}
		(*pSession) << sql;
		return data_ptr();
	};

	static void handleThread(queue_ptr pQueue) {
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
				std::cout << "database:" << boost::this_thread::get_id() << count << std::endl;
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
			}
			catch (Poco::Data::DataException& e) {
				auto c = e.displayText();
				std::cout << e.displayText() << std::endl;
			}
			catch (std::exception& e) {
				auto c = e.what();
				std::cout << e.what() << std::endl;
			}
		}
	};

	queue_ptr m_pQueue;
	boost::thread m_Thread;
};





#endif


