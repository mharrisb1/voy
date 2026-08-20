/*
 * markings:managed
 *
 * File: buffer.hpp
 * Copyright (c) 2026 Michael Harris
 * SPDX-License-Identifier: MIT
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 *
 * markings:managed
 */

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

  RingBuffer(RingBuffer&& other) noexcept
      : capacity_(other.capacity_), head_(other.head_), size_(other.size_) {
    std::lock_guard<std::mutex> lock(other.mutex_);
    buffer_ = std::move(other.buffer_);
  }

  RingBuffer& operator=(RingBuffer&& other) noexcept {
    if (this != &other) {
      std::lock(mutex_, other.mutex_);
      std::lock_guard<std::mutex> lock1(mutex_, std::adopt_lock);
      std::lock_guard<std::mutex> lock2(other.mutex_, std::adopt_lock);
      buffer_   = std::move(other.buffer_);
      capacity_ = other.capacity_;
      head_     = other.head_;
      size_     = other.size_;
    }
    return *this;
  }

 private:
  mutable std::mutex       mutex_;
  std::vector<std::string> buffer_;
  size_t                   capacity_;
  size_t                   head_{0};
  size_t                   size_{0};
};

}  // namespace voy::buffer
