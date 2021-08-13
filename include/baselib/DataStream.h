#ifndef _HQ_BASE_LIB_DATA_STREAM_H_
#define _HQ_BASE_LIB_DATA_STREAM_H_

#ifdef WIN32
#include <windows.h>
#else
#endif

#include <cstdio>
#include <cassert>
#include <string>
#ifdef OS_LINUX_QIYI
#include <string.h>
#endif
#include <vector>
#include <list>

#ifndef  __ALIGN_4__
#ifdef __APPLE__
#define __ALIGN_4__
#else
#ifdef ANDROID
#define __ALIGN_4__
#endif //ANDROID
#endif //__APPLE__
#endif //__ALIGN_4__

//#define writestring write_string
//#define readstring read_string

namespace hq {

class DataStream
{
    public :
    DataStream(unsigned char * szBuf,int isize)
    {
        if((NULL ==szBuf)||(0 == isize))
            m_GoodBit = false;
        else
        {
            m_GoodBit = true;
            m_isize = isize;
            buffer = szBuf;
            current = buffer;
        }
    }
    DataStream(char * szBuf,int isize)
    {
        if((NULL ==szBuf)||(0 == isize))
            m_GoodBit = false;
        else
        {
            m_GoodBit = true;
            m_isize = isize;
            buffer = (unsigned char*)szBuf;
            current = buffer;
        }
    }
    ~DataStream()
    {
    }

    void clear()
    {
        current = buffer;
        if(current)
            current[0]=0;
    }

    //此函数拷贝字符串
    bool strcpy(const char * szStr)
    {
        if(current&&szStr)
        {
            int ilen = (int)strlen((const char*)szStr);
            if((m_isize-(current - buffer)) < (ilen +2))
                return false;
            memcpy(current,szStr,ilen+1);
            current += ilen;
            return true;
        }
        return false;
    }
    
    unsigned char * getcurrentpos()
    {
        return current;
    }
    bool move(int ilen)//当前指针向后移动ilen
    {
        
        assert((current + ilen) <= (buffer + m_isize));
        if(m_GoodBit && (current + ilen) <= (buffer + m_isize))
        {
            current += ilen;
        }else{
            m_GoodBit	= false;
        }
        
        return m_GoodBit;
        
    }
    void reset()
    {
        current = buffer;
    }
    void push()
    {
        m_stack.push_back(current);
    }
    void pop()
    {
        current = *m_stack.rbegin();
        m_stack.pop_back();
    }
    unsigned char readbyte()
    {
        assert((current + 1) <= (buffer + m_isize));
        if(m_GoodBit && (current + 1) <= (buffer + m_isize))
        {
            current ++;
            return *(current-1);
        }
        m_GoodBit = false;
        //return (unsigned char)-1;
        return 0;// PGP, 2010-7-13   15:54	返回0更合适
    }
    void writebyte(unsigned char btValue)
    {
        assert((current + 1) <= (buffer + m_isize));
        if(m_GoodBit && (current + 1) <= (buffer + m_isize))
        {
            *current = btValue;
            current ++;
        }
        else
            m_GoodBit = false;
        
    }
    unsigned short readword()
    {
        assert((current + 2) <= (buffer + m_isize));
        if(m_GoodBit && (current + 2) <= (buffer + m_isize))
        {
#ifndef __ALIGN_4__
#ifndef WORDS_BIGENDIAN
            current +=2;
            return *((unsigned short*)(current-2));
#else
            /* FIXME: assume all data read out is in little endian */
            current += 2;
            return bswap_16((unsigned short)*((unsigned short*)(current - 2)));
#endif
#else // __ALIGN_4__
            unsigned short tmp_var = 0;
            unsigned char  *p_tmp = (unsigned char*)&tmp_var;
#ifndef WORDS_BIGENDIAN
            p_tmp[0] = *current;
            p_tmp[1] = *(current + 1);
            current += 2;
#else
            p_tmp[1] = *current;
            p_tmp[0] = *(current + 1);
            current += 2;
#endif
            return tmp_var;
#endif
        }
        
        m_GoodBit = false;
        //return (unsigned short)-1;
        return 0;// PGP, 2010-7-13   15:54	返回0更合适
    }
#ifdef WORDS_BIGENDIAN
#ifndef __ALIGN_4__
    /* no swaping version */
    unsigned short readword_ns()
    {
#ifndef __LINUX__
        assert((current + 2) <= (buffer + m_isize));
#endif
        if(m_GoodBit &&(current + 2) <= (buffer + m_isize))
        {
            current +=2;
            return *((unsigned short*)(current-2));
        }
        
        m_GoodBit = false;
        return (unsigned short)-1;
    }
#else // __ALIGN_4__
    /* no swaping version */
    unsigned short readword_ns()
    {
#ifndef __LINUX__
        assert((current + 2) <= (buffer + m_isize));
#endif
        if(m_GoodBit &&(current + 2) <= (buffer + m_isize))
        {
            unsigned short tmp_var = 0;
            unsigned char *p_tmp = (unsigned char*)&tmp_var;
            p_tmp[0] = *current;
            p_tmp[1] = *(current + 1);
            current += 2;
            return tmp_var;
        }
        
        m_GoodBit = false;
        return (unsigned short)-1;
    }
#endif
#endif // WORDS_BIGENDIAN
    void writeword(unsigned short wValue)
    {
#ifndef __LINUX__
        assert((current + 2) <= (buffer + m_isize));
#endif
        if(m_GoodBit &&(current + 2) <= (buffer + m_isize))
        {
#ifndef __ALIGN_4__
#ifndef WORDS_BIGENDIAN
            *((unsigned short*)current) = wValue;
#else
            /* FIXME: assume all data should be stored in little endian */
            *((unsigned short*)current) = bswap_16((unsigned short)wValue);
#endif
#else // __ALIGN_4__
#ifndef WORDS_BIGENDIAN
            unsigned char *p_tmp = (unsigned char*)&wValue;
            *current = p_tmp[0];
            *(current + 1) = p_tmp[1];
#else
            unsigned char *p_tmp = (unsigned char*)&wValue;
            *current = p_tmp[1];
            *(current + 1) = p_tmp[0];
#endif
#endif
            current +=2;
        }
        else
            m_GoodBit = false;
    }
    
#ifdef WORDS_BIGENDIAN
#ifndef __ALIGN_4__
    /* no swaping version */
    void writeword_ns(unsigned short wValue)
    {
#ifndef __LINUX__
        assert((current + 2) <= (buffer + m_isize));
#endif
        if(m_GoodBit &&(current + 2) <= (buffer + m_isize))
        {
            *((unsigned short*)current) = wValue;
            current +=2;
        }
        else
            m_GoodBit = false;
    }
#else //__ALIGN_4__
    void writeword_ns(unsigned short wValue)
    {
#ifndef __LINUX__
        assert((current + 2) <= (buffer + m_isize));
#endif
        if(m_GoodBit &&(current + 2) <= (buffer + m_isize))
        {
            unsigned char        *p_tmp = (unsigned char*)&wValue;
            *current = p_tmp[0];
            *(current + 1) = p_tmp[1];
            current +=2;
        }
        else
            m_GoodBit = false;
    }
#endif
#endif //WORDS_BIGENDIAN
    
    float readfloat()
    {
#ifndef __LINUX__
        assert((current + sizeof(float)) <= (buffer + m_isize));
#endif
        if(m_GoodBit &&(current + sizeof(float)) <= (buffer + m_isize))
        {
#ifndef __ALIGN_4__
            current +=sizeof(float);
#ifndef WORDS_BIGENDIAN
            return *((float*)(current-sizeof(float)));
#else
            int midval=bswap_32(*(int*)(current-sizeof(float)));
            return *((float*)&midval);
            /* FIXME: assume sizeof(float) = 4 */
            //		return bswap_float(*((float*)(current-sizeof(float))));
#endif
#else // __ALIGN_4__
            float tmp_var = 0.0f;
            unsigned char  *p_tmp = (unsigned char*)&tmp_var;
            int i_tmp;
#ifndef WORDS_BIGENDIAN
            for (i_tmp = 0; i_tmp < sizeof(float); i_tmp++)
                p_tmp[i_tmp] = *(current + i_tmp);
#else
            for (i_tmp = 0; i_tmp < sizeof(float); i_tmp++)
                p_tmp[i_tmp] = *(current + sizeof(float) - i_tmp -1);
#endif
            current +=sizeof(float);
            return tmp_var;
#endif // __ALIGN_4__
        }
        m_GoodBit = false;
        return 0;
        
    }
#ifdef WORDS_BIGENDIAN
#ifndef __ALIGN_4__
    /* no swaping version */
    float readfloat_ns()
    {
#ifndef __LINUX__
        assert((current + sizeof(float)) <= (buffer + m_isize));
#endif
        if(m_GoodBit && (current + sizeof(float)) <= (buffer + m_isize))
        {
            current +=sizeof(float);
            return *((float*)(current-sizeof(float)));
        }
        m_GoodBit = false;
        return 0;
        
    }
#else // __ALIGN_4__
    float readfloat_ns()
    {
#ifndef __LINUX__
        assert((current + sizeof(float)) <= (buffer + m_isize));
#endif
        if(m_GoodBit &&(current + sizeof(float)) <= (buffer + m_isize))
        {
            float tmp_var = 0.0f;
            unsigned char  *p_tmp = (unsigned char*)&tmp_var;
            int i_tmp;
            for (i_tmp = 0; i_tmp < sizeof(float); i_tmp++)
                p_tmp[i_tmp] = *(current + i_tmp);
            current +=sizeof(float);
            return tmp_var;
            
        }
        m_GoodBit = false;
        return 0;
        
    }
#endif
#endif //WORDS_BIGENDIAN
    
    void writefloat(float fValue)
    {
        assert((current + sizeof(float)) <= (buffer + m_isize));
        if((current + sizeof(float)) <= (buffer + m_isize))
        {
#ifndef __ALIGN_4__
#ifndef WORDS_BIGENDIAN
            *((float*)current) = fValue;
#else
            *current = bswap_32(*(int*)&fValue);
            
            /* FIXME: assume sizeof(float) = 4 */
            //			*((float*)current) = bswap_float(fValue);
#endif
#else  // __ALIGN_4__
            unsigned char *p_tmp = (unsigned char*)&fValue;
            int i_tmp = 0;
            
#ifndef WORDS_BIGENDIAN
            for (i_tmp = 0; i_tmp < sizeof(float); i_tmp++)
                *(current + i_tmp) = p_tmp[i_tmp];
#else
            for (i_tmp = 0; i_tmp < sizeof(float); i_tmp++)
                *(current + i_tmp) = p_tmp[sizeof(float) - i_tmp - 1];
#endif
#endif
            current +=sizeof(float);
        }
        else
            m_GoodBit = false;
    }
#ifdef WORDS_BIGENDIAN
#ifndef __ALIGN_4__
    void writefloat_ns(float fValue)
    {
#ifndef __LINUX__
        assert((current + sizeof(float)) <= (buffer + m_isize));
#endif
        if((current + sizeof(float)) <= (buffer + m_isize))
        {
            *((float*)current) = fValue;
            current +=sizeof(float);
        }
        else
            m_GoodBit = false;
    }
#else // __ALIGN_4__
    void writefloat_ns(float fValue)
    {
#ifndef __LINUX__
        assert((current + sizeof(float)) <= (buffer + m_isize));
#endif
        if((current + sizeof(float)) <= (buffer + m_isize))
        {
            unsigned char *p_tmp = (unsigned char*)&fValue;
            int i_tmp = 0;
            
            for (i_tmp = 0; i_tmp < sizeof(float); i_tmp++)
                *(current + i_tmp) = p_tmp[i_tmp];
            current +=sizeof(float);
        }
        else
            m_GoodBit = false;
    }
#endif
#endif // WORDS_BIGENDIAN
    
    int readint()
    {
        assert((current + sizeof(int)) <= (buffer + m_isize));
        if((current + sizeof(int)) <= (buffer + m_isize))
        {
#ifndef __ALIGN_4__
            current +=sizeof(int);
#ifndef WORDS_BIGENDIAN
            return *((int*)(current-sizeof(int)));
#else
            return bswap_32(*((int*)(current-sizeof(int))));
#endif //WORDS_BIGENDIAN
#else //__ALIGN_4__
            unsigned int tmp_var;
            unsigned char *p_tmp = (unsigned char*)&tmp_var;
#ifndef WORDS_BIGENDIAN
            p_tmp[0] = *current;
            p_tmp[1] = *(current + 1);
            p_tmp[2] = *(current + 2);
            p_tmp[3] = *(current + 3);
#else
            p_tmp[3] = *current;
            p_tmp[2] = *(current + 1);
            p_tmp[1] = *(current + 2);
            p_tmp[0] = *(current + 3);
#endif //WORDS_BIGENDIAN
            //current +=4;
            current +=sizeof(int);
            return tmp_var;
#endif //__ALIGN_4__
        }
        m_GoodBit = false;
        return 0;
        
    }
    void writeint(int iValue)
    {
        assert((current + sizeof(int)) <= (buffer + m_isize));
        if((current + sizeof(int)) <= (buffer + m_isize))
        {
#ifndef __ALIGN_4__
#ifndef WORDS_BIGENDIAN
            *((int*)current) = iValue;
#else
            *((int*)current) = bswap_32(iValue);
#endif
#else
#ifndef WORDS_BIGENDIAN
            unsigned char *p_tmp = (unsigned char*)&iValue;
            
            *current = p_tmp[0];
            *(current + 1) = p_tmp[1];
            *(current + 2) = p_tmp[2];
            *(current + 3) = p_tmp[3];
#else
            unsigned char *p_tmp = (unsigned char*)&iValue;
            
            *current = p_tmp[3];
            *(current + 1) = p_tmp[2];
            *(current + 2) = p_tmp[1];
            *(current + 3) = p_tmp[0];
#endif
#endif
            current +=sizeof(int);
        }
        else
            m_GoodBit = false;
    }
    unsigned int readdword()
    {
#ifndef __LINUX__
        assert((current + 4) <= (buffer + m_isize));
#endif
        if(m_GoodBit &&(current + 4) <= (buffer + m_isize))
        {
#ifndef __ALIGN_4__
            current +=4;
#ifndef WORDS_BIGENDIAN
            return *((unsigned int*)(current-4));
#else
            return bswap_32(*((unsigned int*)(current-4)));
#endif
#else
            unsigned int tmp_var;
            unsigned char *p_tmp = (unsigned char*)&tmp_var;
#ifndef WORDS_BIGENDIAN
            p_tmp[0] = *current;
            p_tmp[1] = *(current + 1);
            p_tmp[2] = *(current + 2);
            p_tmp[3] = *(current + 3);
#else
            p_tmp[3] = *current;
            p_tmp[2] = *(current + 1);
            p_tmp[1] = *(current + 2);
            p_tmp[0] = *(current + 3);
#endif
            current +=4;
            return tmp_var;
#endif
        }
        m_GoodBit = false;
        return 0;
    }
#ifdef WORDS_BIGENDIAN
#ifndef __ALIGN_4__
    unsigned int readdword_ns()
    {
#ifndef __LINUX__
        assert((current + 4) <= (buffer + m_isize));
#endif
        if(m_GoodBit && (current + 4) <= (buffer + m_isize))
        {
            current +=4;
            return *((unsigned int*)(current-4));
        }
        m_GoodBit = false;
        return 0;
    }
#else  // __ALIGN_4__
    unsigned int readdword_ns()
    {
#ifndef __LINUX__
        assert((current + 4) <= (buffer + m_isize));
#endif
        if((current + 4) <= (buffer + m_isize))
        {
            unsigned int tmp_var;
            unsigned char *p_tmp = (unsigned char*)&tmp_var;
            p_tmp[0] = *current;
            p_tmp[1] = *(current + 1);
            p_tmp[2] = *(current + 2);
            p_tmp[3] = *(current + 3);
            current += 4;
            return tmp_var;
        }
        m_GoodBit = false;
        return 0;
    }
#endif
#endif //WORDS_BIGENDIAN
    
    void writedword(unsigned int dwValue)
    {
#ifndef __LINUX__
        assert((current + 4) <= (buffer + m_isize));
#endif
        if((current + 4) <= (buffer + m_isize))
        {
#ifndef __ALIGN_4__
#ifndef WORDS_BIGENDIAN
            *((unsigned int*)current) = dwValue;
#else
            *((unsigned int*)current) = bswap_32(dwValue);
#endif
#else  // __ALIGN_4__
#ifndef WORDS_BIGENDIAN
            unsigned char *p_tmp = (unsigned char*)&dwValue;
            
            *current = p_tmp[0];
            *(current + 1) = p_tmp[1];
            *(current + 2) = p_tmp[2];
            *(current + 3) = p_tmp[3];
#else
            unsigned char *p_tmp = (unsigned char*)&dwValue;
            
            *current = p_tmp[3];
            *(current + 1) = p_tmp[2];
            *(current + 2) = p_tmp[1];
            *(current + 3) = p_tmp[0];
#endif
#endif
            current +=4;
        }
        else
            m_GoodBit = false;
    }
#ifdef WORDS_BIGENDIAN
#ifndef __ALIGN_4__
    void writedword_ns(unsigned int dwValue)
    {
#ifndef __LINUX__
        assert((current + 4) <= (buffer + m_isize));
#endif
        if((current + 4) <= (buffer + m_isize))
        {
            *((unsigned int*)current) = dwValue;
            current +=4;
        }
        else
            m_GoodBit = false;
    }
#else //__ALIGN_4__
    void writedword_ns(unsigned int dwValue)
    {
#ifndef __LINUX__
        assert((current + 4) <= (buffer + m_isize));
#endif
        if((current + 4) <= (buffer + m_isize))
        {
            unsigned char *p_tmp = (unsigned char*)&dwValue;
            
            *current = p_tmp[0];
            *(current + 1) = p_tmp[1];
            *(current + 2) = p_tmp[2];
            *(current + 3) = p_tmp[3];
            current +=4;
        }
        else
            m_GoodBit = false;
    }
    
#endif
#endif //WORDS_BIGENDIAN
    
    long long readint64()
    {
        assert((current + 8) <= (buffer + m_isize));
        if(m_GoodBit && (current + 8) <= (buffer + m_isize))
        {
#ifndef __ALIGN_4__
            current +=8;
#ifndef WORDS_BIGENDIAN
            return *((long long*)(current-8));
#else
            return bswap_64(*((long long*)(current-8)));
#endif
#else // __ALIGN_4__
            long long tmp_var = 0ll;
            unsigned char *p_tmp = (unsigned char*)&tmp_var;
#ifndef WORDS_BIGENDIAN
            /*
             p_tmp[0] = *current;
             p_tmp[1] = *(current + 1);
             p_tmp[2] = *(current + 2);
             p_tmp[3] = *(current + 3);
             p_tmp[4] = *(current + 4);
             p_tmp[5] = *(current + 5);
             p_tmp[6] = *(current + 6);
             p_tmp[7] = *(current + 7);
             */
            for (int i = 0; i < 8; i++) {
                p_tmp[i] = *(current + i);
            }
#else
            /*
             p_tmp[0] = *(current + 7);
             p_tmp[1] = *(current + 6);
             p_tmp[2] = *(current + 5);
             p_tmp[3] = *(current + 4);
             p_tmp[4] = *(current + 3);
             p_tmp[5] = *(current + 2);
             p_tmp[6] = *(current + 1);
             p_tmp[7] = *(current);
             */
            for (int i = 0; i < 8; i++) {
                p_tmp[i] = *(current + 7 - i);
            }
#endif
            current +=8;
            return tmp_var;
#endif
        }
        
        m_GoodBit = false;
        //return (long long)-1;
        return 0;// PGP, 2010-7-13   15:54	返回0更合适
    }
#ifdef WORDS_BIGENDIAN
#ifndef __ALIGN_4__
    long long readint64_ns()
    {
#ifndef __LINUX__
        assert((current + 8) <= (buffer + m_isize));
#endif
        if(m_GoodBit &&(current + 8) <= (buffer + m_isize))
        {
            current +=8;
            return *((long long*)(current-8));
        }
        
        m_GoodBit = false;
        //return (long long)-1;
        return 0;// PGP, 2010-7-13   15:54	返回0更合适
    }
#else // __ALIGN_4__
    long long readint64_ns()
    {
#ifndef __LINUX__
        assert((current + 8) <= (buffer + m_isize));
#endif
        if(m_GoodBit &&(current + 8) <= (buffer + m_isize))
        {
            long long tmp_var = 0ll;
            unsigned char *p_tmp = (unsigned char*)&tmp_var;
            /*
             p_tmp[0] = *current;
             p_tmp[1] = *(current + 1);
             p_tmp[2] = *(current + 2);
             p_tmp[3] = *(current + 3);
             p_tmp[4] = *(current + 4);
             p_tmp[5] = *(current + 5);
             p_tmp[6] = *(current + 6);
             p_tmp[7] = *(current + 7);
             */
            for (int i = 0; i < 8; i++) {
                p_tmp[i] = *(current + i);
            }
            current += 8;
            return tmp_var;
        }
        
        m_GoodBit = false;
        //return (long long)-1;
        return 0;// PGP, 2010-7-13   15:54	返回0更合适
    }
#endif
#endif // WORDS_BIGENDIAN
    
    void writeint64(long long iValue)
    {
        assert((current + 8) <= (buffer + m_isize));
        if((current + 8) <= (buffer + m_isize))
        {
#ifndef __ALIGN_4__
#ifndef WORDS_BIGENDIAN
            *((long long*)current) = iValue;
#else
            *((long long*)current) = bswap_64(iValue);
#endif
#else // __ALIGN_4__
            unsigned char *p_tmp = (unsigned char*)&iValue;
#ifndef WORDS_BIGENDIAN
            /*
             *(current) = p_tmp[0];
             *(current + 1) = p_tmp[1];
             *(current + 2) = p_tmp[2];
             *(current + 3) = p_tmp[3];
             *(current + 4) = p_tmp[4];
             *(current + 5) = p_tmp[5];
             *(current + 6) = p_tmp[6];
             *(current + 7) = p_tmp[7];
             */
            for (int i = 0; i < 8; i++) {
                *(current + i) = p_tmp[i];
            }
#else
            /*
             *(current) = p_tmp[7];
             *(current + 1) = p_tmp[6];
             *(current + 2) = p_tmp[5];
             *(current + 3) = p_tmp[4];
             *(current + 4) = p_tmp[3];
             *(current + 5) = p_tmp[2];
             *(current + 6) = p_tmp[1];
             *(current + 7) = p_tmp[0];
             */
            for (int i = 0; i < 8; i++) {
                *(current + i) = p_tmp[7 - i];
            }
#endif
#endif
            current +=8;
        }
        else
            m_GoodBit = false;
    }
#ifdef WORDS_BIGENDIAN
#ifndef __ALIGN_4__
    void writeint64_ns(long long iValue)
    {
#ifndef __LINUX__
        assert((current + 8) <= (buffer + m_isize));
#endif
        if((current + 8) <= (buffer + m_isize))
        {
            *((long long*)current) = iValue;
            current +=8;
        }
        else
            m_GoodBit = false;
    }
#else // __ALIGN_4__
    void writeint64_ns(long long iValue)
    {
#ifndef __LINUX__
        assert((current + 8) <= (buffer + m_isize));
#endif
        if((current + 8) <= (buffer + m_isize))
        {
            unsigned char *p_tmp = (unsigned char*)&iValue;
            /*
             *(current) = p_tmp[0];
             *(current + 1) = p_tmp[1];
             *(current + 2) = p_tmp[2];
             *(current + 3) = p_tmp[3];
             *(current + 4) = p_tmp[4];
             *(current + 5) = p_tmp[5];
             *(current + 6) = p_tmp[6];
             *(current + 7) = p_tmp[7];
             */
            for (int i = 0; i < 8; i++) {
                *(current + i) = p_tmp[i];
            }
            current +=8;
        }
        else
            m_GoodBit = false;
    }
#endif
#endif // WORDS_BIGENDIAN
    
    unsigned char * readdata(unsigned int dwLen)
    {
        assert((current + dwLen) <= (buffer + m_isize));
        if(m_GoodBit && (current + dwLen) <= (buffer + m_isize))
        {
            current +=dwLen;
            return (unsigned char*)(current-dwLen);
        }
        
        m_GoodBit = false;
        return NULL;
    }
    //增加安全接口,当读内存越界时,返回错误
    //2008.5.5, Add by YP.
    bool readdata(unsigned int dwLen,unsigned char * pbyData)
    {
        if(m_GoodBit && (current + dwLen) <= (buffer + m_isize))
        {
            memcpy(pbyData,current,dwLen);
            current +=dwLen;
            return true;
        }
        m_GoodBit = false; //add by pgp
        return false;
    }
    bool readdata(unsigned char * pbyData,unsigned int dwLen)
    {
        if(m_GoodBit && (current + dwLen) <= (buffer + m_isize))
        {
            memcpy(pbyData,current,dwLen);
            current +=dwLen;
            return true;
        }
        m_GoodBit = false; //add by pgp
        return false;
    }
    void writedata(unsigned char const * pData,unsigned int dwLen)
    {
        assert((current + dwLen) <= (buffer + m_isize));
        if((current + dwLen) <= (buffer + m_isize))
        {
            memcpy(current,pData,dwLen);
            current +=dwLen;
        }
        else
            m_GoodBit = false;
    }
    
    // PGP, 2010-9-10   17:51	读UTF8字符串
    char * read_utf8_string()
    {
        int ilen = 0;
        int buf_left = leavedata();
        bool good = false;
        for(ilen=0; m_GoodBit && ilen<buf_left; ++ilen)
        {
            if(0==current[ilen])
            {
                good	= true;
                break;
            }
        }
        if(!good)
        {
            m_GoodBit	= false;
            return NULL;
        }
        char * szRes = (char*)current;
        if(m_GoodBit && (current + ilen) <= (buffer + m_isize))
        {
            current +=(ilen+1);
            return szRes;
        }
        m_GoodBit = false;
        return NULL;
    }
    wchar_t* read_wstring()
    {
        int ilen = 0;
        int buf_left = leavedata()/sizeof(wchar_t);
        bool good = false;
        for(ilen=0; m_GoodBit && ilen<buf_left; ++ilen)
        {
            if(0==((wchar_t*)current)[ilen])
            {
                good	= true;
                break;
            }
        }
        if(!good)
        {
            m_GoodBit	= false;
            return NULL;
        }
        wchar_t * szRes = (wchar_t*)current;
        if(m_GoodBit && (current + ilen*sizeof(wchar_t)+2) <= (buffer + m_isize))
        {
            current +=(ilen*sizeof(wchar_t)+2);
            return szRes;
        }
        m_GoodBit = false;
        return NULL;
    }
    char * read_string()
    {
        int ilen = 0;
        int buf_left = leavedata();
        bool good = false;
        for(ilen=0; m_GoodBit && ilen<buf_left; ++ilen)
        {
            if(0==current[ilen])
            {
                good	= true;
                break;
            }
        }
        if(!good)
        {
            m_GoodBit	= false;
            return nullptr;
        }
        char * szRes = (char*)current;
        if(m_GoodBit && (current + ilen+1) <= (buffer + m_isize))
        {
            current +=(ilen+1);
            return szRes;
        }
        m_GoodBit = false;
        return nullptr;
    }
    
    //add by lh 原因：原来的strcpy函数操作时没有将0Copy进去，函数名没有对应
    bool write_string(const char * szStr)
    {
        if(current&&szStr)
        {
            int ilen = (int)strlen(szStr);
            if(ilen>0)
            {
                if((m_isize-(current - buffer)) < (ilen +1))	//原来为(ilen +2)
                    return false;
                //memcpy(current,szStr,ilen+1);
                //current += (ilen+1);				//修改原来的current += ilen
                //return true;
                // PANGUIPIN, 2011-4-18   16:59	拷贝字符串时，不管字符串是否为空，都要拷贝一个0进去
                memcpy(current,szStr,ilen);
                current += (ilen);
            }
        }
        writebyte(0);
        return true;
    }
    bool write_utf8_string(const char * szStr)
    {
        if(current&&szStr)
        {
            int ilen = (int)strlen((const char*)szStr);
            if(ilen>0)
            {
                if((m_isize-(current - buffer)) < (ilen +1))	//原来为(ilen +2)
                    return false;
                // 			memcpy(current,szStr,ilen+1);
                // 			current += (ilen+1);				//修改原来的current += ilen
                // 			return true;
                // PANGUIPIN, 2011-4-18   16:59	拷贝字符串时，不管字符串是否为空，都要拷贝一个0进去
                //否则连续拷贝多个字符串，有些是空的，当读时就会有错误
                memcpy(current,szStr,ilen);
                current += (ilen);
            }
        }
        writebyte(0);
        return true;
    }
    bool write_wstring(const wchar_t * szStr)
    {
        if(current&&szStr)
        {
            int ilen = (int)wcslen(szStr);
            if(ilen>0)
            {
                if((m_isize-(current - buffer)) < (ilen*2+2))
                    return false;
                // 			memcpy(current,szStr,ilen*2+2);
                // 			current += (ilen*2+2);
                // 			return true;
                memcpy(current,szStr,ilen*2);
                current += (ilen*2);
            }
        }
        writeword(0);
        return true;
    }
    
    //标示操作是否成功
    /*! Simon.M.Lu, 2007-11-22   11:10
     *	设置good_bit
     */
    void good_bit(bool flag){m_GoodBit=flag;}
    bool good_bit()
    {
        return m_GoodBit;
    }
    
    //add end
    int size()
    {
        return (int)(current-buffer);
    }
    int leavedata()//缓冲区剩余字节
    {
        return m_isize-size();
    }
    const unsigned char * getbuffer(){return buffer;}
    int getbufferlength(void)const{return m_isize;}
    protected :
    //用来对读写操作检查是否成功,当失败时设置为false,对应的数据返回为0
    bool 	m_GoodBit;
    
    unsigned char* buffer;
    unsigned char* current;
    int m_isize;
    std::list<unsigned char*> m_stack;//堆栈
};


//增加流操作符
//use:
//	DataStream cdrIn(pBuff, len);
//	cdrIn >> dwLen >> ucFlag;
// 	assert(cdrIn.good_bit());		//check

//	DataStream cdOut(pBuff, len);
//	cdOut << dwLen << ucFlag;
// 	assert(cdOut.good_bit());		//check

//输入流
#ifdef WIN32
inline DataStream & operator >> (DataStream &is, DWORD & x)
{
    x = is.readdword();
    return  is;
}
#endif
/*! panguipin, 2012-5-25   14:08
 *	添加unsigned int
 */
inline DataStream & operator >> (DataStream &is, unsigned int & x)
{
    x = is.readdword();
    return  is;
}
inline DataStream & operator >> (DataStream &is, unsigned short & x)
{
    x = is.readword();
    return  is;
}
inline DataStream & operator >> (DataStream &is, unsigned char & x)
{
    x = is.readbyte();
    return  is;
}

inline DataStream & operator >> (DataStream &is, float & x)
{
    x = is.readfloat();
    return  is;
}
inline DataStream & operator >> (DataStream &is, long long & x)
{
    x = is.readint64();
    return  is;
}

inline DataStream & operator >> (DataStream &is, int & x)
{
    x = is.readint();
    return  is;
}

inline DataStream & operator >> (DataStream &is, unsigned long long & x)
{
    x = is.readint64();
    return  is;
}
inline DataStream & operator >> (DataStream &is, std::string & x)
{
    char * pstr = is.read_utf8_string();
    if(pstr)//如果是空指针,赋值给string会崩溃
        x = pstr;
    return  is;
}
inline DataStream & operator >> (DataStream &is, std::wstring & x)
{
    wchar_t * pstr = is.read_wstring();
    if(pstr)//如果是空指针,赋值给string会崩溃
        x = pstr;
    return  is;
}


//输出流
#ifdef WIN32
inline DataStream & operator << (DataStream &os, const DWORD & x)
{
    os.writedword(x);
    return  os;
}
#endif
/*! panguipin, 2012-5-25   14:08
 *	添加unsigned int
 */
inline DataStream & operator << (DataStream &os, const unsigned int & x)
{
    os.writedword(x);
    return  os;
}
inline DataStream & operator << (DataStream &os, const unsigned short & x)
{
    os.writeword(x);
    return  os;
}
inline DataStream & operator << (DataStream &os, const unsigned char & x)
{
    os.writebyte(x );
    return  os;
}

inline DataStream & operator << (DataStream &os, const float & x)
{
    os.writefloat(x);
    return  os;
}
inline DataStream & operator << (DataStream &os, const long long & x)
{
    os.writeint64(x);
    return  os;
}
inline DataStream & operator << (DataStream &os, const int & x)
{
    os.writeint(x);
    return  os;
}

inline DataStream & operator << (DataStream &os, const unsigned long long & x)
{
    os.writeint64(x);
    return  os;
}

inline DataStream & operator << (DataStream &os, const std::string & x)
{
    os.write_utf8_string(x.c_str());
    return  os;
}
inline DataStream & operator << (DataStream &os, const std::wstring & x)
{
    os.write_wstring(x.c_str());
    return  os;
}
//使用
//CInputDataStream dsIn(pBuf, bufLen);
//dsIn >> dwLen1 >> ucFlag1;
// 	assert(dsIn.good_bit());		//check

//数据流操作函数,输入
class CInputDataStream : public DataStream
{
public:
    CInputDataStream(unsigned char * szBuf, int isize)
    :DataStream(szBuf, isize)
    {
    }
    CInputDataStream & operator >> (unsigned int & dwValue)
    {
        dwValue = readdword();
        return *this;
    }
    
    CInputDataStream & operator >> (unsigned short & wValue)
    {
        wValue = readword();
        return *this;
    }
    CInputDataStream & operator >> (unsigned char & ucValue)
    {
        ucValue = readbyte();
        return *this;
    }
    
    CInputDataStream & operator >> (float & fValue)
    {
        fValue = readfloat();
        return *this;
    }
    CInputDataStream & operator >> (long long & i64Value)
    {
        i64Value = readint64();
        return *this;
    }
    CInputDataStream & operator >> (std::string & strValue)
    {
        strValue = read_string();
        return *this;
    }
};

//输出流,增加内存自动分配功能及流操作符重载
//使用
//COutputDataStream dsOut;
//dsOut << (unsigned int)0 << unsigned char('Z') << (unsigned short)wTest;
//assert(dsOut.good_bit());		//check

class COutputDataStream : public DataStream
{
public:
#define CHUNK 1024
    COutputDataStream()
    :DataStream((unsigned char*)NULL, 0),
    pOutBuf(NULL)
    {
        //预分配1024的内存
        pOutBuf = new unsigned char[CHUNK];
        buffer =  pOutBuf;
        current = buffer;	
        m_isize = CHUNK;
    }
    //预分配指定的缓冲区大小
    COutputDataStream(int isize)
    :DataStream((unsigned char*)NULL, isize)
    ,pOutBuf(NULL)
    {
        pOutBuf = new unsigned char[isize];
        buffer =  pOutBuf;
        current = buffer;	
        m_isize = isize;
    }
    
    COutputDataStream(unsigned char * szBuf, int isize);
    //	:DataStream(szBuf, isize),
    //	pOutBuf(NULL)
    //{
    //}
    
    ~COutputDataStream()
    {
        if(NULL != pOutBuf)
        {
            delete [] pOutBuf;
        }
        pOutBuf = NULL;
        buffer = NULL;
        m_isize = 0;
    }
    
    inline void ReAllocMem(size_t nLen = 0)
    {
        //分配内存增倍
        unsigned int dwSize = m_isize * 2;
        size_t nSize = current - buffer;	//原来的消息长度
        m_isize = std::max<unsigned int>(dwSize, static_cast<unsigned int>(nLen+nSize));
        
        bool bDelete =true;
        if(NULL == pOutBuf)
            bDelete = false;
        //重分配内存
        //vctBuff.resize(m_isize);
        pOutBuf = new unsigned char[m_isize];
        
        //原来的数据Copy到的新缓冲区
        memcpy(pOutBuf, buffer, nSize);
        
        //重新设置缓冲位置等
        if(bDelete)
            delete [] buffer;			//删除旧的
        
        buffer =  pOutBuf;
        current = buffer + nSize;
    }
    
    COutputDataStream& operator << ( unsigned int & dwValue )
    {
        if((current + sizeof(dwValue)) > (buffer + m_isize))
        {
            ReAllocMem();
        }
        writedword(dwValue);
        return *this;
    }
    
    void writedata(unsigned char * pData,unsigned int dwLen)
    {
        if((current + dwLen) > (buffer + m_isize))
        {
            ReAllocMem(dwLen);
        }
        memcpy(current,pData,dwLen);		
        current += dwLen;
    }
    
    COutputDataStream& operator << ( unsigned short & wValue )
    {
        if((current + sizeof(unsigned short)) > (buffer + m_isize))
        {
            ReAllocMem();
        }
        writeword(wValue);
        return *this;
    }
    
    COutputDataStream& operator << ( unsigned char & ucValue )
    {
        if((current + sizeof(unsigned char)) > (buffer + m_isize))
        {
            ReAllocMem();
        }
        writebyte(ucValue);
        return *this;
    }
    
    COutputDataStream& operator << ( float & fValue )
    {
        if((current + sizeof(float)) > (buffer + m_isize))
        {
            ReAllocMem();
        }
        writefloat(fValue);
        return *this;
    }
    
    COutputDataStream& operator << ( long long & i64Value )
    {
        if((current + sizeof(long long)) > (buffer + m_isize))
        {
            ReAllocMem();
        }
        writeint64(i64Value);
        return *this;
    }
    
private:
    //vector<char>	vctBuff;
    unsigned char * 	pOutBuf;
    
};


class CNetworkByteOrder
{
public:
    static unsigned short int convert(unsigned short int iValue)
    {
        unsigned short int iData;
        ((unsigned char*)&iData)[0] = ((unsigned char*)&iValue)[1];
        ((unsigned char*)&iData)[1] = ((unsigned char*)&iValue)[0];
        return iData;
    }
    static int convert(int iValue)
    {
        int iData;
        ((unsigned char*)&iData)[0] = ((unsigned char*)&iValue)[3];
        ((unsigned char*)&iData)[1] = ((unsigned char*)&iValue)[2];
        ((unsigned char*)&iData)[2] = ((unsigned char*)&iValue)[1];
        ((unsigned char*)&iData)[3] = ((unsigned char*)&iValue)[0];
        return iData;
    }
    static long long convert(long long iValue)
    {
        long long iData;
        ((unsigned char*)&iData)[0] = ((unsigned char*)&iValue)[7];
        ((unsigned char*)&iData)[1] = ((unsigned char*)&iValue)[6];
        ((unsigned char*)&iData)[2] = ((unsigned char*)&iValue)[5];
        ((unsigned char*)&iData)[3] = ((unsigned char*)&iValue)[4];
        ((unsigned char*)&iData)[4] = ((unsigned char*)&iValue)[3];
        ((unsigned char*)&iData)[5] = ((unsigned char*)&iValue)[2];
        ((unsigned char*)&iData)[6] = ((unsigned char*)&iValue)[1];
        ((unsigned char*)&iData)[7] = ((unsigned char*)&iValue)[0];
        return iData;
    }
    
};

}

#endif ///< _HQ_BASE_LIB_DATA_STREAM_H_