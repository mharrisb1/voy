#pragma once

#include <mutex>
#include <string>
#include <string_view>
#include <vector>

namespace voy::buffer {

class RingBuffer {
 public:
  explicit RingBuffer(size_t capacity);

  void                                   push(std::string_view chunk);
  [[nodiscard]] std::vector<std::string> drain();

 private:
  std::mutex               mutex_;
  std::vector<std::string> buffer_;
  size_t                   capacity_;
  size_t                   head_{0};
};

}  // namespace voy::buffer
