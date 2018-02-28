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
class IServerNet {
public:
	virtual void setPort(const int port) = 0;
};
typedef std::shared_ptr<IServerNet> IServerNetPtr;


//////////////////////////////////////
template<typename TData>
class RequestBase {
public:
	FramePtr pFrame;
	std::string Route;
	std::shared_ptr<TData> pData;
	unsigned int Seq;
};
typedef RequestBase<TreePtr::element_type> RequestPersian;
typedef std::shared_ptr<RequestBase<TreePtr::element_type> > RequestPersianPtr;

template<typename TData>
class ResponseBase : public RequestBase<TData> {
public:
};
typedef ResponseBase<TreePtr::element_type> ResponsePersian;
typedef std::shared_ptr<ResponsePersian> ResponsePersianPtr;
/////////////////////////////////////////
template<typename TData>
class IProtocol {
public:
	virtual bool encode(std::shared_ptr<RequestBase<TData> > pRequest) = 0;
	virtual bool decode(std::shared_ptr<ResponseBase<TData> > pResponse) = 0;
};

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
