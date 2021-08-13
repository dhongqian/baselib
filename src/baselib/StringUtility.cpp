/*
 * @Author: your name
 * @Date: 2021-05-12 16:40:13
 * @LastEditTime: 2021-05-14 18:32:39
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \base_lib\src\string_utility.cpp
 */
#include "baselib/StringUtility.h"
#include <vector>

namespace hq{
namespace string_utility {


std::string toLower(const std::string& str) {
    std::string lower(str);
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    return lower;
}

std::string toUpper(const std::string& str) {
    std::string upper(str);
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    return upper;
}

bool isntspace(const char& ch) {
    return !isspace(ch);
}

std::string ltrim(const std::string& s) {
    std::string::const_iterator iter = std::find_if(s.begin(), s.end(), isntspace);
    return std::string(iter, s.end());
}

std::string rtrim(const std::string& s) {
    std::string::const_iterator iter = std::find_if(s.rbegin(), s.rend(), isntspace).base();
    return std::string(s.begin(), iter);
}

std::string trim(const std::string& s) {
    std::string::const_iterator iter1 = std::find_if(s.begin(), s.end(), isntspace);
    std::string::const_iterator iter2 = std::find_if(s.rbegin(), s.rend(), isntspace).base();

    return iter1 < iter2 ? std::string(iter1, iter2) : std::string("");
}

bool isAddressHttp(const std::string& address) {
    if(address.empty()) return false;

    std::string::size_type start_pos = address.find("http://");
    return (std::string::npos != start_pos);
}

bool isAddressHttps(const std::string& address) {
    if(address.empty()) return false;

    std::string::size_type start_pos = address.find("https://");
    return (std::string::npos != start_pos);
}

std::string replaceUrlHost(const std::string& url, const std::string& new_host) {
    if(url.empty() || new_host.empty()) return "";

    bool is_https = false;
    std::string::size_type start_pos = 0;
    if(isAddressHttp(url)) {
        start_pos += 7;
    }
    else if(isAddressHttps(url)) {
        start_pos += 8;
        is_https = true;
    }
    else {
        //    url has no http:// or https://
    }

    std::string::size_type end_pos = url.find('/', start_pos);
    std::string new_address = url;
    new_address.replace(start_pos, end_pos - start_pos, new_host);

    return new_address;
}

int getNext(std::string patter_str, std::vector<int> &next_vec) {
    next_vec.resize(patter_str.size());
    next_vec[0] = -1;
    size_t j = 0, k=-1;

    while (j < patter_str.size() - 1) {
       if (k == -1 || patter_str.at(j) == patter_str.at(k)) {
           if (patter_str.at(++j) == patter_str.at(++k)) { // 当两个字符相等时要跳过
              next_vec[j] = next_vec[k];
           } else {
              next_vec[j] = k;
           }
       } else {
           k = next_vec[k];
       }
    }

    return 0;
}

}
}
