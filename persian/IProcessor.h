#ifndef __IPROCESSOR_H__
#define __IPROCESSOR_H__


#include <memory>
#include <functional>
#include <string>
#include <vector>
#include "IContext.h"
#include "PersianDefine.h"

class IProcessor {
public:
	typedef std::function<void(IContextPtr, TreePtr)> callback;

	virtual bool addListen(const std::wstring& tag, callback fun) = 0;
};
typedef std::shared_ptr<IProcessor> IProcessorPtr;







#endif // !__IPROCESSOR_H__

