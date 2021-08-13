#ifndef __HQ_BASE_LIB_STRING_UTILITY_H__

#include <string>
#include <algorithm>
#include "Utility.h"

namespace hq{
namespace string_utility {

baselib_declspec std::string toLower(const std::string& str);
baselib_declspec std::string toUpper(const std::string& str);
baselib_declspec std::string trim(const std::string& s);

baselib_declspec bool isAddressHttp(const std::string& address);
baselib_declspec bool isAddressHttps(const std::string& address);
baselib_declspec std::string replaceUrlHost(const std::string& url, const std::string& new_host);

}
}

#endif ///< #ifndef __HQ_BASE_LIB_STRING_UTILITY_H__
