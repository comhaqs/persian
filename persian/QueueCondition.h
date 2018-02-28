#ifndef __QUEUE_CONDITION_H__
#define __QUEUE_CONDITION_H__

#include <memory>
#include <condition_variable>
#include <mutex>
#include <list>
#include <chrono>

template<typename T>
class QueueCondition {
public:
	typedef std::shared_ptr<QueueCondition<T> > c_ptr;
	QueueCondition<T>():m_bStop(false) {
	};

	virtual T pop() {
		std::unique_lock <std::mutex> lock(m_Mutex);
		while (!m_bStop && m_Queue.empty()) {
			m_Condition.wait(lock);
		}
		if (m_bStop)
		{
			return T();
		}
		auto data = m_Queue.front();
		m_Queue.pop_front();
		return data;
	};

	virtual T popBySeconds(long time) {
		std::unique_lock <std::mutex> lock(m_Mutex);
		if (!m_bStop && m_Queue.empty()) {
			m_Condition.wait_for(lock, std::chrono::seconds(time));
		}
		if (m_bStop || m_Queue.empty()) {
			return T();
		}
		auto data = m_Queue.front();
		m_Queue.pop_front();
		return data;
	};

	virtual void push(T data) {
		if (m_bStop)
		{
			return;
		}
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			m_Queue.push_back(data);
			lock.unlock();
		}
		m_Condition.notify_one();
	};

	virtual void stop() {
		m_bStop = true;
	};

	virtual bool isStop() {
		return m_bStop;
	};

protected:
	std::list<T> m_Queue;
	std::mutex m_Mutex;
	std::condition_variable m_Condition;
	std::atomic_bool m_bStop;
};







#endif // !__QUEUE_CONDITION_H__

