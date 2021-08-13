#include "baselib/Utility.h"
#include <vector>

namespace hq{
namespace utility {

#if 0
std::string get_module_path(const char* module_name) {
    std::string str;
#ifdef WIN32
    wchar_t value[MAX_PATH + 1];
    DWORD len = ::GetModuleFileName(NULL, (LPWCH)value, MAX_PATH);
    if(MAX_PATH == len) {
        // may be truncated
    }
    value[len] = '\0';
    return std::string(value);
#else
    char sLine[1024] = {0};
    void* pSymbol = (void*)"";
    FILE* fp;
    char* pPath;

    fp = fopen("/proc/self/maps", "r");
    if(fp != NULL) {
        while(!feof(fp)) {
            unsigned long start, end;

            if(!fgets(sLine, sizeof(sLine), fp))
                continue;
            if(!strstr(sLine, " r-xp ") || !strchr(sLine, '/'))
                continue;

            sscanf(sLine, "%lx-%lx ", &start, &end);
            if(pSymbol >= (void*)start && pSymbol < (void*)end) {
                char* tmp;
                //    size_t len;

                /* Extract the filename; it is always an absolute path */
                pPath = strchr(sLine, '/');

                /* Get rid of the newline */
                tmp = strstr(pPath, module_name);//'/n');
                if(tmp) *tmp = 0;
                p = pPath;
            }
        }
        fclose(fp);
    }
#endif
    return std::string();
}
#endif

void Utf8ToGBK(const std::string& strUtf8, std::string& strGBK) {
#ifdef WIN32
    int wchar_len = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), (int)strUtf8.length(), NULL, 0);
    if(0 == wchar_len)
        return;

    wchar_t* pwcBuf = new wchar_t[wchar_len + 1];
    if(!pwcBuf) {
        delete[] pwcBuf;
        return;
    }
    memset(pwcBuf, 0, (wchar_len + 1) * sizeof(wchar_t));

    MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), (int)strUtf8.length(), pwcBuf, wchar_len);

    int ansi_len = WideCharToMultiByte(CP_ACP, 0, pwcBuf, wchar_len, NULL, 0, NULL, NULL);
    if(0 == wchar_len) {
        delete[] pwcBuf;
        return;
    }

    char* pcBuf = new char[ansi_len + 1];
    if(!pcBuf) {
        delete[] pwcBuf;
        return;
    }
    memset(pcBuf, 0, ansi_len + 1);

    WideCharToMultiByte(CP_ACP, 0, pwcBuf, wchar_len, pcBuf, ansi_len, NULL, NULL);
    strGBK = pcBuf;

    delete[] pwcBuf;
    delete[] pcBuf;
#else
    if(strUtf8.empty()) {
        return;
    }

    iconv_t cv = iconv_open("GBK", "UTF-8");
    if(cv == (iconv_t)-1) {
        return;
    }

    size_t utf8_length = strUtf8.length();
    char* utf8_string = const_cast<char*>(strUtf8.c_str());
    size_t gbk_length = utf8_length + 1;
    char* gbk_string = new char[gbk_length];
    char* gbk_org_string = gbk_string;
    memset(gbk_string, 0, gbk_length);

    iconv(cv, &utf8_string, &utf8_length, &gbk_string, &gbk_length);

    iconv_close(cv);

    strGBK = gbk_org_string;
    delete[]gbk_org_string;
#endif
}
std::string Utf8ToGBK(const std::string& strUtf8) {
#ifdef WIN32
    std::string strGBK;
    int wchar_len = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), (int)strUtf8.length(), NULL, 0);
    if(0 == wchar_len)
        return strGBK;

    wchar_t* pwcBuf = new wchar_t[wchar_len + 1];
    if(!pwcBuf)
        return strGBK;
    memset(pwcBuf, 0, (wchar_len + 1) * sizeof(wchar_t));

    MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), (int)strUtf8.length(), pwcBuf, wchar_len);

    int ansi_len = WideCharToMultiByte(CP_ACP, 0, pwcBuf, wchar_len, NULL, 0, NULL, NULL);
    if(0 == wchar_len) {
        delete[] pwcBuf;
        return strGBK;
    }

    char* pcBuf = new char[ansi_len + 1];
    if(!pcBuf) {
        delete[] pwcBuf;
        return strGBK;
    }
    memset(pcBuf, 0, ansi_len + 1);

    WideCharToMultiByte(CP_ACP, 0, pwcBuf, wchar_len, pcBuf, ansi_len, NULL, NULL);
    strGBK = pcBuf;

    delete[] pwcBuf;
    delete[] pcBuf;
    return strGBK;
#else
    if(strUtf8.empty()) {
        return "";
    }

    iconv_t cv = iconv_open("GBK", "UTF-8");
    if(cv == (iconv_t)-1) {
        return "";
    }

    size_t iUtf8Length = strUtf8.length();
    char* szUtf8 = const_cast<char*>(strUtf8.c_str());
    size_t iGBKLength = iUtf8Length + 1;
    char* szGBK = new char[iGBKLength];
    char* szGBK_Org = szGBK;
    memset(szGBK, 0, iGBKLength);

    iconv(cv, &szUtf8, &iUtf8Length, &szGBK, &iGBKLength);

    iconv_close(cv);

    std::string strGBK = szGBK_Org;
    delete[]szGBK_Org;

    return strGBK;
#endif
}
void GBKToUtf8(const std::string& strGBK, std::string& strUtf8) {
#ifdef WIN32
    int wchar_len = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), strGBK.length(), NULL, 0);
    if(0 == wchar_len)
        return;

    wchar_t* pwcBuf = new wchar_t[wchar_len + 1];
    if(!pwcBuf)
        return;
    memset(pwcBuf, 0, (wchar_len + 1) * sizeof(wchar_t));

    MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), strGBK.length(), pwcBuf, wchar_len);

    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, pwcBuf, wchar_len, NULL, 0, NULL, NULL);
    if(0 == utf8_len) {
        delete[] pwcBuf;
        return;
    }

    char* pcBuf = new char[utf8_len + 1];
    if(!pcBuf) {
        delete[] pwcBuf;
        return;
    }

    memset(pcBuf, 0, utf8_len + 1);

    WideCharToMultiByte(CP_UTF8, 0, pwcBuf, wchar_len, pcBuf, utf8_len, NULL, NULL);

    strUtf8 = pcBuf;
    delete[] pcBuf;
    delete[] pwcBuf;
#else
    if(strGBK.empty()) {
        return;
    }

    iconv_t cv = iconv_open("UTF-8", "GBK");
    if(cv == (iconv_t)-1) {
        return;
    }

    size_t gbk_length = strGBK.length();
    char* gbk_string = const_cast<char*>(strGBK.c_str());

    size_t utf8_length = (gbk_length + 1) * 3 / 2;
    char* utf8_string = new char[utf8_length];
    char* utf8_org_string = utf8_string;
    memset(utf8_string, 0, utf8_length);

    iconv(cv, &gbk_string, &gbk_length, &utf8_string, &utf8_length);

    iconv_close(cv);

    strUtf8 = utf8_org_string;
    delete[]utf8_org_string;
#endif
}

std::string GBKToUtf8(const std::string& strGBK) {
#ifdef WIN32
    std::string strUtf8;
    int wchar_len = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), strGBK.length(), NULL, 0);
    if(0 == wchar_len)
        return strUtf8;

    wchar_t* pwcBuf = new wchar_t[wchar_len + 1];
    if(!pwcBuf)
        return strUtf8;
    memset(pwcBuf, 0, (wchar_len + 1) * sizeof(wchar_t));

    MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), strGBK.length(), pwcBuf, wchar_len);

    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, pwcBuf, wchar_len, NULL, 0, NULL, NULL);
    if(0 == utf8_len) {
        delete[] pwcBuf;
        return strUtf8;
    }

    char* pcBuf = new char[utf8_len + 1];
    if(!pcBuf) {
        delete[] pwcBuf;
        return strUtf8;
    }

    memset(pcBuf, 0, utf8_len + 1);

    WideCharToMultiByte(CP_UTF8, 0, pwcBuf, wchar_len, pcBuf, utf8_len, NULL, NULL);

    strUtf8 = pcBuf;
    delete[] pcBuf;
    delete[] pwcBuf;

    return strUtf8;
#else
    if(strGBK.empty()) {
        return "";
    }

    iconv_t cv = iconv_open("UTF-8", "GBK");
    if(cv == (iconv_t)-1) {
        return "";
    }

    size_t iGBKLength = strGBK.length();
    char* szGBK = const_cast<char*>(strGBK.c_str());

    size_t iUtf8Length = (iGBKLength + 1) * 3 / 2;
    char* szUtf8 = new char[iUtf8Length];
    char* szUtf8_org = szUtf8;
    memset(szUtf8, 0, iUtf8Length);

    iconv(cv, &szGBK, &iGBKLength, &szUtf8, &iUtf8Length);

    iconv_close(cv);

    std::string strUtf8 = szUtf8_org;
    delete[]szUtf8_org;

    return strUtf8;
#endif
}

std::string UnicodeToUtf8(const wchar_t* wszUnicode, INT32 i32Len) {
    std::string strUtf8;
#ifdef WIN32
    int iUtf8Len = WideCharToMultiByte(CP_UTF8, 0, wszUnicode, i32Len, NULL, 0, NULL, NULL);
    if(0 == iUtf8Len)
        return strUtf8;

    char* pBuf = new char[iUtf8Len + 1];
    memset(pBuf, 0, iUtf8Len + 1);

    WideCharToMultiByte(CP_UTF8, 0, wszUnicode, i32Len, pBuf, iUtf8Len, NULL, NULL);

    strUtf8 = pBuf;
    delete[] pBuf;
#else
    if(!wszUnicode) {
        return "";
    }
    iconv_t cv = iconv_open("UTF-8", "UCS-4LE");
    if(cv == (iconv_t)-1) {
        return "";
    }

    size_t iUnicodeLength = i32Len * sizeof(wchar_t);
    char* szUnicode = const_cast<char*>(reinterpret_cast<const char*>(wszUnicode));

    size_t iUtf8Length = iUnicodeLength + 1;
    char* szUtf8 = new char[iUtf8Length];
    memset(szUtf8, 0, iUtf8Length);

    iconv(cv, &szUnicode, &iUnicodeLength, &szUtf8, &iUtf8Length);

    iconv_close(cv);

    strUtf8 = szUtf8;
    delete[]szUtf8;

#endif
    return strUtf8;
}

std::wstring Utf8ToUnicode(const char* szUtf8, INT32 i32Len) {
    std::wstring wstrUnicode;
#ifdef WIN32
    int i32Unicode = MultiByteToWideChar(CP_UTF8, 0, szUtf8, i32Len, NULL, 0);
    if(0 == i32Unicode)
        return wstrUnicode;

    wchar_t* pBuf = new wchar_t[i32Unicode + 1];
    memset(pBuf, 0, (i32Unicode + 1) * sizeof(wchar_t));

    MultiByteToWideChar(CP_UTF8, 0, szUtf8, i32Len, pBuf, i32Unicode);

    wstrUnicode = pBuf;
    delete[] pBuf;
    //Utf82Unicode(strUCS,utf8Str);
#else

    if(!szUtf8) {
        return L"";
    }
    iconv_t cv = iconv_open("UCS-4LE", "UTF-8");
    if(cv == (iconv_t)-1) {
        return L"";
    }

    size_t iUtf8Length = i32Len;
    char* szUtf8_string = const_cast<char*>(szUtf8);

    size_t iUnicodeLength = (iUtf8Length + 1) * sizeof(wchar_t);
    char* szUnicode = new char[iUnicodeLength];
    memset(szUnicode, 0, iUnicodeLength);

    iconv(cv, &szUtf8_string, &iUtf8Length, &szUnicode, &iUnicodeLength);

    iconv_close(cv);

    wstrUnicode = reinterpret_cast<wchar_t*>(szUnicode);
    delete[]szUnicode;
#endif
    return wstrUnicode;
}

std::string UnicodeToAnsi(const wchar_t* wszUnicode, INT32 i32Len, UINT32 u32CodePage) {
    std::string strAnsi;
#ifdef WIN32
    int iAansiLen = WideCharToMultiByte(u32CodePage, 0, wszUnicode, i32Len, NULL, 0, NULL, NULL);
    if(0 == iAansiLen)
        return strAnsi;;

    char* pBuf = new char[iAansiLen + 1];
    memset(pBuf, 0, iAansiLen + 1);

    WideCharToMultiByte(u32CodePage, 0, wszUnicode, i32Len, pBuf, iAansiLen, NULL, NULL);

    strAnsi = pBuf;
    delete[] pBuf;
#else

    if(!wszUnicode) {
        return "";
    }
    iconv_t cv = iconv_open("GBK", "UCS-4LE");
    if(cv == (iconv_t)-1) {
        return "";
    }

    size_t iUnicodeLength = (i32Len + 1) * sizeof(wchar_t);
    char* szUnicode = const_cast<char*>(reinterpret_cast<const char*>(wszUnicode));

    size_t iGBKLength = iUnicodeLength / 2 + 1;
    char* szGBK = new char[iGBKLength];
    memset(szGBK, 0, iGBKLength);

    iconv(cv, &szUnicode, &iUnicodeLength, &szGBK, &iGBKLength);

    iconv_close(cv);

    strAnsi = szGBK;
    delete[]szGBK;
#endif
    return strAnsi;
}

std::string UnicodeToAnsi(const std::wstring& wstrUnicode) {
    std::string strAnsi;
#ifdef WIN32
    int iAnsiLen = WideCharToMultiByte(CP_ACP, 0, wstrUnicode.c_str(), wstrUnicode.length(), NULL, 0, NULL, NULL);
    if(0 == iAnsiLen)
        return strAnsi;;

    char* pBuf = new char[iAnsiLen + 1];
    memset(pBuf, 0, iAnsiLen + 1);

    WideCharToMultiByte(CP_ACP, 0, wstrUnicode.c_str(), wstrUnicode.length(), pBuf, iAnsiLen, NULL, NULL);

    strAnsi = pBuf;
    delete[] pBuf;
#else

    if(wstrUnicode.empty()) {
        return "";
    }
    iconv_t cv = iconv_open("GBK", "UCS-4LE");
    if(cv == (iconv_t)-1) {
        return "";
    }

    size_t iUnicodeLength = (wstrUnicode.length() + 1) * sizeof(wchar_t);
    char* szUnicode = const_cast<char*>(reinterpret_cast<const char*>(wstrUnicode.c_str()));

    size_t iGBKLength = iUnicodeLength / 2 + 1;
    char* szGBK = new char[iGBKLength];
    memset(szGBK, 0, iGBKLength);

    iconv(cv, &szUnicode, &iUnicodeLength, &szGBK, &iGBKLength);

    iconv_close(cv);

    strAnsi = szGBK;
    delete[]szGBK;
#endif
    return strAnsi;
}
std::wstring AnsiToUnicode(const std::string& strAnsi) {
    std::wstring strUCS;
#ifdef WIN32
    int iUnicodeLen = MultiByteToWideChar(CP_ACP, 0, strAnsi.c_str(), strAnsi.length(), NULL, 0);
    if(0 == iUnicodeLen)
        return strUCS;

    wchar_t* pBuf = new wchar_t[iUnicodeLen + 1];
    memset(pBuf, 0, (iUnicodeLen + 1) * sizeof(wchar_t));

    MultiByteToWideChar(CP_ACP, 0, strAnsi.c_str(), strAnsi.length(), pBuf, iUnicodeLen);

    strUCS = pBuf;
    delete[] pBuf;
#else
    if(strAnsi.empty()) {
        return L"";
    }
    iconv_t cv = iconv_open("UCS-4LE", "GBK");
    if(cv == (iconv_t)-1) {
        return L"";
    }

    size_t iGBKLength = strAnsi.length();
    char* szGBK = const_cast<char*>(strAnsi.c_str());

    size_t iUnicodeLength = (iGBKLength + 1) * sizeof(wchar_t);
    char* szUnicode = new char[iUnicodeLength];
    memset(szUnicode, 0, iUnicodeLength);

    iconv(cv, &szGBK, &iGBKLength, &szUnicode, &iUnicodeLength);

    iconv_close(cv);

    strUCS = reinterpret_cast<wchar_t*>(szUnicode);
    delete[]szUnicode;
#endif
    return strUCS;
}
std::wstring AnsiToUnicode(const char* szAnsi, INT32 i32Len, UINT32 u32CodePage) {
    std::wstring wstrUnicode;
#ifdef WIN32
    int iUnicodeLength = MultiByteToWideChar(u32CodePage, 0, szAnsi, i32Len, NULL, 0);
    if(0 == iUnicodeLength)
        return wstrUnicode;

    wchar_t* pBuf = new wchar_t[iUnicodeLength + 1];
    memset(pBuf, 0, (iUnicodeLength + 1) * sizeof(wchar_t));

    MultiByteToWideChar(u32CodePage, 0, szAnsi, i32Len, pBuf, iUnicodeLength);

    wstrUnicode = pBuf;
    delete[] pBuf;
#else

    if(!szAnsi) {
        return L"";
    }
    iconv_t cv = iconv_open("UCS-4LE", "GBK");
    if(cv == (iconv_t)-1) {
        return L"";
    }

    size_t iGBKLength = i32Len;
    char* szGBK = const_cast<char*>(szAnsi);

    size_t iUnicodeLength = (iGBKLength + 1) * sizeof(wchar_t);
    char* szUnicode = new char[iUnicodeLength];
    memset(szUnicode, 0, iUnicodeLength);

    iconv(cv, &szGBK, &iGBKLength, &szUnicode, &iUnicodeLength);

    iconv_close(cv);

    wstrUnicode = reinterpret_cast<wchar_t*>(szUnicode);
    delete[]szUnicode;
#endif
    return wstrUnicode;
}

#if 0
std::string GB2312ToUtf8(const std::string& strGBK) {
#ifdef WIN32
    std::wstring wstrUnicode;
    int iUnicodeLength = MultiByteToWideChar(CP_GB2312, 0, strGBK.c_str(), strGBK.length(), NULL, 0);
    if(0 == iUnicodeLength)
        return std::string();

    wchar_t* pBuf = new wchar_t[iUnicodeLength + 1];
    memset(pBuf, 0, (iUnicodeLength + 1) * sizeof(wchar_t));

    MultiByteToWideChar(CP_GBK, 0, strGBK.c_str(), strGBK.length(), pBuf, iUnicodeLength);

    wstrUnicode = pBuf;
    delete[] pBuf;

    return  UnicodeToUtf8(wstrUnicode.c_str(), (int)wstrUnicode.length());
#else
    return  strGBK;
#endif
}
#endif

baselib_declspec std::string getCurrentDate() {
#ifdef WIN32
    SYSTEMTIME wtm;

    GetLocalTime(&wtm);
    char timebuf[1024] = {0};
    sprintf_s(timebuf, sizeof(timebuf), "%04d-%02d-%02d %02d:%02d:%02d.%03d", wtm.wYear, wtm.wMonth, wtm.wDay, wtm.wHour, wtm.wMinute, wtm.wSecond, wtm.wMilliseconds);
    std::string strTime = timebuf;
    return strTime;

#else

    struct timeval    tv;
    struct tm         timenow;

    gettimeofday(&tv, 0);
    localtime_r(&tv.tv_sec, &timenow);

    char timebuf[1024] = {0};
    os_sprintf_s(timebuf, sizeof(timebuf), "%04d-%02d-%02d %02d:%02d:%02d.%03d", timenow.tm_year + 1900, timenow.tm_mon + 1, timenow.tm_mday, timenow.tm_hour, timenow.tm_min, timenow.tm_sec, int(tv.tv_usec / 1000));

    std::string strTime = timebuf;
    return strTime;
#endif
}

baselib_declspec void logInit(const std::string& log_config) {
    log4cplus::PropertyConfigurator::doConfigure(log_config.c_str());
}

}
}
