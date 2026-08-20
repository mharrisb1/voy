/*
 * markings:managed
 *
 * File: buffer.cpp
 * Copyright (c) 2026 Michael Harris
 * SPDX-License-Identifier: MIT
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 *
 * markings:managed
 */

#include <voy/buffer.hpp>

namespace voy::buffer {

RingBuffer::RingBuffer(size_t capacity) : capacity_(capacity) {
  buffer_.reserve(capacity_);
}

void RingBuffer::push(std::string_view chunk) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (buffer_.size() < capacity_) {
    buffer_.push_back(std::string(chunk));
  } else {
    buffer_[head_] = std::string(chunk);
    head_          = (head_ + 1) % capacity_;
  }
}

std::vector<std::string> RingBuffer::drain() {
  std::lock_guard<std::mutex> lock(mutex_);

  std::vector<std::string> result;

  size_t current_size = buffer_.size();
  result.reserve(current_size);

  for (size_t i = 0; i < current_size; ++i) {
    result.push_back(std::move(buffer_[(head_ + i) % capacity_]));
  }

  buffer_.clear();
  head_ = 0;

  return result;
}

}  // namespace voy::buffer
