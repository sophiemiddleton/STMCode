#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <condition_variable>
#include <queue>
#include <future>
#include <atomic>
#include <iostream>

using namespace std;

template<class T>
struct opt {
  T* get() { return static_cast<T*>( static_cast<void*>( & data ) ); };
  T const* get() const { return static_cast<T*>( static_cast<void*>( & data ) ); };

  T& operator*() & { return *get(); }
  T&& operator*() && { return std::move(*get()); }
  T const& operator*() const & { return *get(); }
  T const&& operator*() const&& { return std::move(*get()); }

  explicit operator bool() const { return engaged; }
  bool has_value() const { return (bool)*this; }
  template< class U >
  T value_or( U&& default_value ) const& {
    if (*this) return **this;
    return std::forward<U>(default_value);
  }
  template< class U >
  T value_or( U&& default_value ) && {
    if (*this) return std::move(**this);
    return std::forward<U>(default_value);
  }

  opt(T const& t) {
    emplace(t);
  }
  opt(T&& t) {
    emplace(std::move(t));
  }
  opt() = default;
  opt(opt const& o) {
    if (o) {
      emplace( *o );
    }
  }
  opt(opt && o) {
    if (o) {
      emplace( std::move(*o) );
    }
  }
  opt& operator=(opt const& o) & {
    if (!o) {
      reset();
    } else if (*this) {
      **this = *o;
    } else {
      emplace( *o );
    }
    return *this;
  }
  opt& operator=(opt && o) & {
    if (!o) {
      reset();
    } else if (*this) {
      **this = std::move(*o);
    } else {
      emplace( std::move(*o) );
    }
    return *this;
  }
  template<class...Args>
  T& emplace(Args&&...args) {
    if (*this) reset();
    ::new( static_cast<void*>(&data) ) T(std::forward<Args>(args)...);
    engaged = true;
    return **this;
  }
  void reset() {
    if (*this) {
      get()->~T();
      engaged = false;
    }
  }
  ~opt() { reset(); }
private:
  using storage = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
  bool engaged = false;
  storage data;
};

template<class T>
struct threadsafe_queue {
  opt<T> pop() {
   
    auto l = lock();
    cv.wait( l, [&]{
      return abort || !data.empty();
    });
    if (abort) return {};
    T retval = std::move(data.front());
    data.pop();
    return retval;
  }
  void push( T in ) {
   
    auto l = lock();
    data.push( std::move(in) );
    cv.notify_one();
  }
  void abort_queue() {
    auto l = lock();
    abort = true;
    cv.notify_all();
  }
private:
  mutable std::mutex m;
  std::condition_variable cv;
  std::queue<T> data;
  bool abort = false;

  std::unique_lock<std::mutex> lock() const {
    return std::unique_lock<std::mutex>(m);
  }
};

struct counting_barrier {
public:
  explicit counting_barrier( std::size_t n ):count(n) {}
  void operator--() {
    --count;
    if (count <= 0)
    {
       std::unique_lock<std::mutex> l(m);
       cv.notify_all();
    }
  }
  void wait() {
    std::unique_lock<std::mutex> l(m);
    cv.wait( l, [&]{ return count <= 0; } );
  }
private:
  std::mutex m;
  std::condition_variable cv;
  std::atomic<std::ptrdiff_t> count{0};
};
 
struct thread_pool {
  template<class F, class R=typename std::decay< typename std::result_of< F&() >::type>::type>
  auto add_task( F&& f )
  -> std::future< R >
  {
    
     std::packaged_task<R()> task( std::forward<F>(f) );
     auto retval = task.get_future();
     tasks.push( std::packaged_task<void()>(std::move(task)) );
     return retval;
  }

  void start_thread( std::size_t N=1 )
  {
   
    if (shutdown) return;
    for (std::size_t i = 0; i < N; ++i)
    {
      threads.emplace_back( [this]{
        while (true)
        {
          if(shutdown) return;
          auto task = tasks.pop();
          if (!task)
            return;
          (*task)();
        }
      } );
    }
  }
  void cleanup() {
   
    shutdown = true;
    tasks.abort_queue();
    for (auto&& t:threads)
      t.join();
    threads.clear();
  }
  ~thread_pool() {
    cleanup();
  }

  thread_pool():thread_pool( std::thread::hardware_concurrency() ) {}
  explicit thread_pool( std::size_t N ) {
    start_thread(N);
  }
private:
  threadsafe_queue<std::packaged_task<void()>> tasks;
  std::vector<std::thread> threads;
  std::atomic<bool> shutdown{false};
};
