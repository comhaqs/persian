#ifndef __TEST_COROUTINE_H__
#define __TEST_COROUTINE_H__

#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/coroutine2/all.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <string>
#include<vector>


class testCoroutine
{
public:
	typedef boost::shared_ptr<boost::asio::io_service> service_ptr;
	typedef boost::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor_ptr;
	typedef boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;
	typedef boost::coroutines2::asymmetric_coroutine<void> coroutine_type;
	typedef boost::shared_ptr<std::vector<unsigned char> > frame_ptr;
	typedef boost::shared_ptr<coroutine_type::push_type> push_ptr;
	typedef boost::shared_ptr<coroutine_type::pull_type> pull_ptr;

	virtual bool start();
	std::map<int, std::function<void(int)> > m_MapCallback;
protected:
	virtual void createServer(service_ptr pService);
	virtual void createClient(service_ptr pService);

	template<typename Handler>
	void wait(boost::asio::io_service &io, Handler & handler, int index) {
		typename boost::asio::handler_type<Handler, void(int)>::type handler_(std::forward<Handler>(handler));
		boost::asio::async_result<decltype(handler_)> result(handler_);
		m_MapCallback.insert(std::make_pair(index, handler_));
		return result.get();
	};

	template<typename T>
	static void test(T param, int tag) {
		(*param)(tag);
	}
	/*
	int wait(boost::asio::io_service& service, boost::asio::yield_context yield, int index) {
		
		typedef boost::asio::handler_type<decltype(yield), void(int)>::type ResultSetter;

		auto result_setter(std::make_shared<ResultSetter>(yield));
		boost::asio::async_result<ResultSetter> result_getter(*result_setter);
		std::function<void(int)> fun = std::bind(testCoroutine::test<std::shared_ptr<ResultSetter> >, result_setter, std::placeholders::_1);
		m_MapCallback.insert(std::make_pair(index, fun));
		return result_getter.get();
	};
	*/

	template <typename Signature, typename CompletionToken>
	auto wait3(CompletionToken token, int index) {
		using completion_type = boost::asio::async_completion<CompletionToken, Signature>;
		completion_type completion{ token };
		auto handler = completion.completion_handler;
		std::function<void(int)> fun = std::bind([handler](int v) {
			using boost::asio::asio_handler_invoke;
			asio_handler_invoke(std::bind(handler, v), &handler);
		}, std::placeholders::_1);
		m_MapCallback.insert(std::make_pair(index, fun));
		return completion.result.get();
	};

	acceptor_ptr m_pAcceptor;
	socket_ptr m_pSocket;
	service_ptr m_pService;

	
};
typedef boost::shared_ptr<testCoroutine> testCoroutinePtr;


#endif


