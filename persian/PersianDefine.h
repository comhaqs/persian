#ifndef __PERSIAN_DEFINE_H__
#define __PERSIAN_DEFINE_H__

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/function.hpp>
#include <functional>
#include <memory>
#include <vector>


typedef std::shared_ptr<boost::asio::io_service> ServicePtr;
typedef std::shared_ptr<boost::asio::ip::tcp::socket> SocketPtr;
typedef std::shared_ptr<std::vector<unsigned char> > FramePtr;
typedef std::shared_ptr<boost::property_tree::ptree> TreePtr;

//////////////////////////////////////
class IPlugin {
public:
	virtual bool beforeStart() { return true; };
	virtual bool start() { return true; };
	virtual bool afterStart() { return true; };
	virtual void beforeStop() {};
	virtual void stop() {};
	virtual void afterStop() {};
};
typedef std::shared_ptr<IPlugin> IPluginPtr;


////////////////////////////////////
class IServerNet : public IPlugin {
public:
	typedef std::function<void(FramePtr, unsigned int, SocketPtr)> callback;

	virtual void setPort(const int port) = 0;
	virtual void setCallback(callback fun) = 0;
};
typedef std::shared_ptr<IServerNet> IServerNetPtr;


//////////////////////////////////////
class RequestPersian {
public:
	FramePtr pFrame;
	std::string Route;
	TreePtr pData;
	unsigned int Seq;
};
typedef std::shared_ptr<RequestPersian> RequestPersianPtr;

typedef RequestPersian ResponsePersian;
typedef std::shared_ptr<ResponsePersian> ResponsePersianPtr;
/////////////////////////////////////////
class IProtocol {
public:
	virtual bool encode(RequestPersianPtr pRequest) = 0;
	virtual bool decode(ResponsePersianPtr pResponse) = 0;
};
typedef std::shared_ptr<IProtocol> IProtocolPtr;

///////////////////////////////////////////
template<typename TReturn, typename TDatabase, typename TQueue>
auto queryDatabase(boost::asio::yield_context yield, std::function<std::shared_ptr<TReturn>(std::shared_ptr<TDatabase>)> fun, std::shared_ptr<TQueue> pQueue) {
	using db_ptr = std::shared_ptr<TDatabase>;
	using completion_type = boost::asio::async_completion<boost::asio::yield_context, void(std::shared_ptr<TReturn>)>;
	completion_type completion{ yield };
	auto handler = completion.completion_handler;
	std::function<void(db_ptr)> proxy = std::bind([handler, fun](db_ptr d) {
		auto result = fun(d);
		using boost::asio::asio_handler_invoke;
		asio_handler_invoke(std::bind(handler, result), &handler);
	}, std::placeholders::_1);
	pQueue->push(proxy);
	return completion.result.get();
};


/////////////////////////////////////////////


#endif // !__PERSIAN_DEFINE_H__
