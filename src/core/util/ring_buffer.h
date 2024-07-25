#ifndef GRPC_SRC_CORE_UTIL_RING_BUFFER_H_
#define GRPC_SRC_CORE_UTIL_RING_BUFFER_H_

#include <grpc/support/port_platform.h>

#include <cstddef>
#include <iterator>
#include <vector>

#include "absl/types/optional.h"

namespace grpc_core {

template <typename T, int capacity>
class RingBuffer {
 private:
  struct Node {
    T data;
    Node* next;
  };

 public:
  class RingBufferIterator {
   public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = const T;
    using pointer = void;
    using reference = void;
    using difference_type = std::ptrdiff_t;

    RingBufferIterator& operator++() {
      if (head_ == tail_) {
        head_ = tail_ = nullptr;
        buffer_ = nullptr;
      } else {
        head_ = head_->next;
      }
      return *this;
    }

    RingBufferIterator operator++(int) {
      RingBufferIterator tmp(*this);
      operator++();
      return tmp;
    }

    bool operator==(const RingBufferIterator& rhs) const {
      return (buffer_ == rhs.buffer_ && head_ == rhs.head_ &&
              tail_ == rhs.tail_);
    }

    bool operator!=(const RingBufferIterator& rhs) const {
      return !operator==(rhs);
    }

    T operator*() { return head_->data; }

    RingBufferIterator() : buffer_(nullptr), head_(nullptr), tail_(nullptr) {};
    RingBufferIterator(const RingBufferIterator& other) = default;
    RingBufferIterator(const RingBuffer<T, capacity>* buffer)
        : buffer_(buffer), head_(buffer->head_), tail_(buffer->tail_) {
      if (!head_ || !tail_) {
        buffer_ = nullptr;
      } else if (head_ == tail_) {
      }
    }

   private:
    friend class RingBuffer<T, capacity>;
    const RingBuffer<T, capacity>* buffer_;
    RingBuffer<T, capacity>::Node* head_;
    RingBuffer<T, capacity>::Node* tail_;
  };

  RingBuffer() : data_(capacity) {
    for (int i = 0; i < data_.size(); ++i) {
      data_[i].next = &data_[(i + 1) % capacity];
    }
  }

  void Append(T data) {
    if (!tail_) {
      head_ = tail_ = &data_[0];
      tail_->data = std::move(data);
      return;
    }
    if (tail_->next == head_) {
      // Buffer is full. Advance head to make room.
      head_ = head_->next;
    }
    tail_ = tail_->next;
    tail_->data = std::move(data);
  }

  // Returns the data of the first element in the buffer and removes it from
  // the buffer. If the buffer is empty, returns absl::nullopt.
  absl::optional<T> PopIfNotEmpty() {
    if (!head_ || !tail_) return absl::nullopt;
    T data = std::move(head_->data);
    if (head_ == tail_) {
      head_ = tail_ = nullptr;
    } else {
      head_ = head_->next;
    }
    return data;
  }

  void Clear() { head_ = tail_ = nullptr; }

  RingBufferIterator begin() const { return RingBufferIterator(this); }

  RingBufferIterator end() const { return RingBufferIterator(); }

 private:
  friend class RingBufferIterator;
  std::vector<Node> data_;
  Node* head_ = nullptr;
  Node* tail_ = nullptr;
};

}  // namespace grpc_core

#endif  // GRPC_SRC_CORE_UTIL_RING_BUFFER_H_
