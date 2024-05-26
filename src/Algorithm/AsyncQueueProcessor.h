//
// Created by 19254 on 24-5-25.
//

#ifndef AUDIO_CLIENT_ASYNCQUEUEPROCESSOR_H
#define AUDIO_CLIENT_ASYNCQUEUEPROCESSOR_H

#include <boost/lockfree/queue.hpp>
#include <memory>
#include <thread>
#include <mutex>

namespace AC::Algorithm {

/**
 * @brief 异步队列处理器
 */
template<typename TYPE>
class AsyncQueueProcessor {
public:
  typedef std::function<void(AsyncQueueProcessor<TYPE> *, TYPE &)> AsyncQueueProcessCallback;

private:
  std::shared_ptr<boost::lockfree::queue<TYPE, boost::lockfree::fixed_sized<false>>> m_queue = nullptr;
  std::thread m_thread;
  std::mutex m_queue_mutex;
  std::condition_variable m_data_available_cv;
  AsyncQueueProcessCallback m_process;
  bool m_running = false;

public:
  /**
   * @brief 构造函数
   * @param size 队列初始大小
   */
  explicit AsyncQueueProcessor(size_t size);

  /**
   * @brief 构造函数
   * @param size 队列大小
   * @param processCallback 处理回调
   */
  explicit AsyncQueueProcessor(size_t size, const AsyncQueueProcessCallback &processCallback);

  /**
   * @brief 析构函数
   */
  ~AsyncQueueProcessor();

  /**
   * @brief 启动队列
   */
  void Start();

  /**
   * @brief 启动队列
   * @param processCallback 处理回调
   */
  void Start(const AsyncQueueProcessCallback &processCallback);

  /**
   * @brief 停止队列
   */
  void Stop();

  /**
   * @brief 投递数据
   * @param data 数据
   */
  void Push(TYPE &data);

  /**
   * @brief 是否为空
   */
  bool Empty();

  /**
   * @brief 获取队列大小
   */
  size_t Size();

  /**
   * @brief 清空队列
   */
  void Clear();

  /**
   * @brief 设置处理回调
   * @param processCallback 处理回调
   */
  void SetProcessCallback(const AsyncQueueProcessCallback &processCallback);

  /**
   * @brief 设置处理回调
   * @param processCallback 处理回调
   */
  void SetProcessCallback(const AsyncQueueProcessCallback &&processCallback) ;

  /**
   * @brief 是否正在运行
   */
  bool IsRunning();

};

} // AC::Algorithm

#endif //AUDIO_CLIENT_ASYNCQUEUEPROCESSOR_H
