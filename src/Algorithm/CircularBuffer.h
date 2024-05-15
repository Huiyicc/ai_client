//
// Created by 19254 on 24-5-12.
//

#ifndef AUDIO_CLIENT_CIRCULARBUFFER_H
#define AUDIO_CLIENT_CIRCULARBUFFER_H

#include <vector>
#include <stdexcept>
#include <numeric>

namespace AC::Algorithm {

/**
 * @brief 一个基于向量的循环缓冲区类，用于存储类型为T的数据。
 */
template<typename T>
class CircularBuffer {
public:
  /**
 * @brief 构造函数，初始化循环缓冲区。
 * @param size 缓冲区的大小，即能够存储的数据的最大数量。
 */
  explicit CircularBuffer(size_t size) : data(size), head(0), tail(0), count(0) {}

  // 添加元素到队列尾部，如果队列已满，则覆盖最旧的元素
  void push(const T &value) {
    data[tail] = value;
    tail = (tail + 1) % data.size();

    // 先移动头部指针（如果队列已满），再增加计数
    if (count == data.size()) {
      head = (head + 1) % data.size(); // 移动头部指针，实际上覆盖了最旧的数据
    } else {
      ++count;
    }
  }

  /**
 * @brief 获取队列当前的大小。
 * @return 队列中元素的数量。
 */
  [[nodiscard]] size_t size() const {
    return count;
  }

  /**
 * @brief 获取缓冲区的总容量。
 * @return 缓冲区能够存储的最大元素数量。
 */
  [[nodiscard]] size_t capacity() const {
    return data.size();
  }

  /**
 * @brief 获取队列头部的元素（最近添加的元素），不改变队列状态。
 * @return 队列头部的元素。
 * @throws std::out_of_range 如果队列为空，则抛出异常。
 */
  T front() const {
    if (empty()) {
      throw std::out_of_range("empty queue");
    }
    return data[head];
  }

  /**
   * @brief 获取缓冲区中所有元素的副本。
   * @return 包含缓冲区所有元素的向量。
   */
  std::vector<T> get() const {
    return data;
  }

  /**
   * @brief 获取缓冲区中所有元素的副本，并按照添加顺序排序。
   * @return 包含缓冲区所有元素的向量，按照添加顺序排序。
   */
  [[nodiscard]] std::vector<T> getOrdered() {
    std::vector<T> result(data.size());
    result.clear();
    result.insert(result.begin(), data.begin() + head, data.end());
    result.insert(result.end(), data.begin(), data.begin() + tail);
    return std::move(result);
  }

  /**
 * @brief 检查队列是否为空。
 * @return 如果队列为空，则返回true，否则返回false。
 */
  // 检查队列是否为空
  [[nodiscard]] bool empty() const {
    return count == 0;
  }

  /**
   * @brief 获取队列头部的索引。
   * @return 队列头部的索引。
   */
  [[nodiscard]]size_t getHead() const {
    return head;
  }

  /**
   * 是否已满
   * @return 如果队列已满，则返回true，否则返回false。
   */
  [[nodiscard]] bool isFull() const {
    return count >= data.size();
  }
//
//  /**
//   * 对所有元素求均值
//   * @return
//   */
//  [[nodiscard]] float getAverageEnergy() const {
//    if (data.empty()) {
//      return 0.0f; // 或者根据需求处理空vector的情况，比如抛出异常
//    }
//    float sum = std::accumulate(data.begin(), data.end(), 0.0f);
//    return sum / static_cast<float>(data.size());
//  }

private:
  std::vector<T> data; // 存储数据的容器
  size_t head;         // 指向队列头部的索引
  size_t tail;         // 指向队列下一个插入位置的索引
  size_t count;        // 队列中实际元素的数量
};


} // AC

#endif //AUDIO_CLIENT_CIRCULARBUFFER_H
