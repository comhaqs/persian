#ifndef __DATABASE_PERSIAN_H__
#define __DATABASE_PERSIAN_H__


#include "PersianDefine.h"
#include "IDatabase.h"
#include "QueueCondition.h"
#include <boost/thread.hpp>

class DatabasePersian : public IDatabase, public IPlugin
{
public:
	typedef std::shared_ptr<QueueCondition<std::function<void (database_ptr)>> > queue_ptr;


	virtual bool start();
	virtual void stop();
	virtual void afterStop();
	virtual data_ptr query(boost::asio::yield_context yield, fun_type fun);
	virtual void exec(boost::asio::yield_context yield, const std::string sql);

protected:
	static data_ptr funSql(database_ptr pSession, const std::string sql);
	static void handleThread(queue_ptr pQueue);

	queue_ptr m_pQueue;
	boost::thread m_Thread;
};
typedef std::shared_ptr<DatabasePersian> DatabasePersianPtr;




#endif


