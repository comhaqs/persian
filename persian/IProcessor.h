#ifndef __IPROCESSOR_H__
#define __IPROCESSOR_H__


#include <memory>
#include <functional>
#include <string>
#include <vector>
#include "IContext.h"
#include "PersianDefine.h"

template<typename TData>
class IProcessor {
public:
	typedef std::function<void(std::shared_ptr<IContext<TData> >, std::shared_ptr<TData>)> fun_route;

	virtual bool addListen(const std::string& tag, fun_route fun) = 0;
};







#endif // !__IPROCESSOR_H__

