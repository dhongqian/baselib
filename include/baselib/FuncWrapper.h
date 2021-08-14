#ifndef __HQ_BASE_LIB_FUNC_WRAPPER_H_
#define __HQ_BASE_LIB_FUNC_WRAPPER_H_

#include <memory>
#include <functional>
#include <tuple>

#ifdef __GNUC__
#define __CDECL__ __attribute__ ((__cdecl__))
#else
#define __CDECL__ __cdecl
#endif

namespace hq
{
  template <typename F, typename Tuple, bool Done, int Total, int... N>
  struct Execute
  {
    static void run(F& f, Tuple && t)
    {
      Execute<F, Tuple, Total == 1 + sizeof...(N), Total, N..., sizeof...(N)>::run(f, std::forward<Tuple>(t));
    }
  };

  template <typename F, typename Tuple, int Total, int... N>
  struct Execute<F, Tuple, true, Total, N...>
  {
    static void run(F& f, Tuple && t)
    {
      f(std::get<N>(std::forward<Tuple>(t))...);
    }
  };

  template <typename F, typename Tuple>
  void FuncExecuteRun(F& f, Tuple && t)
  {
    typedef typename std::decay<Tuple>::type ttype;
    Execute<F, Tuple, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>::run(f, std::forward<Tuple>(t));
  }

  class TaskBase
  {
  public:
    ~TaskBase(){};
    virtual void Run()=0;
  };
  template <typename FUNC>
  class Task;

  template <typename R, typename... Args>
  class Task<R(__CDECL__*)(Args...)> :public TaskBase
  {
  public:
    explicit Task(R(__CDECL__*func)(Args...), Args... args)
      : fp_(std::bind(func, args...))
      , params_(args...)
    {

    }
    virtual void Run()
    {
      FuncExecuteRun(fp_, params_);
    } 
  private:
    std::function<R(__CDECL__*&)(Args...)> fp_;
    std::tuple<Args...> params_;
  };

#ifdef WIN32
#if !(defined _WIN64) && !(defined _UWP_ARM_)
  template <typename R,typename... Args>
  class Task<R (__stdcall*)(Args...)>:public TaskBase
  {
  public:
    explicit Task(R (__stdcall*func)(Args...), Args... args)
      : fp_(std::bind(func, args...))
      , params_(args...)
    {

    }
    virtual void Run()
    {
      FuncExecuteRun(fp_, params_);
    }
  private:
    std::function<R(__stdcall*&)(Args...)> fp_;
    std::tuple<Args...> params_;
  };
#endif
#endif//WIN32

  template <typename R,typename C, typename ... Args>
  class Task<R (C::*)(Args...)>:public TaskBase
  {
  public:
    explicit Task(R (C::*func)(Args...),C *objP, Args... args) 
      : fp_(std::bind(func, objP, args...))
      , params_(args...)
    {		
    }
    explicit Task(R(C::*func)(Args...), std::shared_ptr<C>objP, Args... args) 
      : sp(objP) 
      , p(objP.get())
      , fp_(std::bind(func, p, args...))
      , params_(args...)
    {
    }
    virtual void Run()
    {
      FuncExecuteRun(fp_, params_);
    }
  private:
    std::shared_ptr<C> sp;
    C* p;
    std::function<R(Args...)> fp_;
    std::tuple<Args...> params_;
  };

  typedef std::shared_ptr<TaskBase> Closure;
  template <typename FUNC, typename ... Args>
  Closure BindFun(FUNC func, Args ... args)
  {
      return Closure(new Task<FUNC>(func, args...));
    }
    
    template <typename FUNC,typename CLASSP, typename ... Args>
    Closure Bind(FUNC func,CLASSP cp, Args ... args)
    {
        return Closure(new Task<FUNC>(func,cp,args...));
    }  

    template <typename FUNC, typename CLASS, typename ... Args>
    Closure Bind(FUNC func, std::shared_ptr<CLASS> cp, Args ... args)
    {
      return Closure(new Task<FUNC>(func, cp, args...));
    }
}

#endif ///< __HQ_BASE_LIB_FUNC_WRAPPER_H_
