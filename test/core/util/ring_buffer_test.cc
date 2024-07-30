//
//
// Copyright 2015 gRPC authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//

#include <grpc/support/port_platform.h>

#include "gtest/gtest.h"
#include "src/core/util/ring_buffer.h"

namespace grpc_core {

constexpr int kBufferCapacity = 1000;

TEST(RingBufferTest, BufferAppendPopTest) {
  RingBuffer<int, kBufferCapacity> buffer;
  EXPECT_FALSE(buffer.PopIfNotEmpty().has_value());

  for (int i = 0; i < 1.5 * kBufferCapacity; ++i) {
    buffer.Append(i);
  }
  // Pop half of the elements. Elements in [0.5 * kBufferCapacity,
  // kBufferCapacity) are popped.
  int j = 0.5 * kBufferCapacity;
  for (int i = 0; i < 0.5 * kBufferCapacity; ++i) {
    EXPECT_EQ(buffer.PopIfNotEmpty(), j++);
  }
  EXPECT_EQ(j, kBufferCapacity);
  // Iterate over the remaining elements.
  for (auto it = buffer.begin(); it != buffer.end(); ++it) {
    EXPECT_EQ(*it, j++);
  }
  // Elements in [kBufferCapacity, 1.5 * kBufferCapacity) should be present.
  EXPECT_EQ(j, 1.5 * kBufferCapacity);

  // Append some more elements. The buffer should now have elements in
  // [kBufferCapacity, 2 * kBufferCapacity).
  for (int i = 0; i < 0.5 * kBufferCapacity; ++i) {
    buffer.Append(j++);
  }
  // Pop all the elements.
  j = kBufferCapacity;
  while (true) {
    auto ret = buffer.PopIfNotEmpty();
    if (!ret.has_value()) break;
    EXPECT_EQ(*ret, j++);
  }
  EXPECT_EQ(j, 2 * kBufferCapacity);
}

TEST(RingBufferTest, BufferAppendIterateTest) {
  RingBuffer<int, kBufferCapacity> buffer;
  for (int i = 0; i < 5 * kBufferCapacity; ++i) {
    buffer.Append(i);
    int j = std::max(0, i + 1 - kBufferCapacity);
    // If i >= kBufferCapacity, check that the buffer contains only the last
    // kBufferCapacity elements [i + 1 - kBufferCapacity, i]. Otherwise check
    // that the buffer contains all elements from 0 to i.
    for (auto it = buffer.begin(); it != buffer.end(); ++it) {
      EXPECT_EQ(*it, j++);
    }
    // Check that j was incremented at each step which implies that all the
    // required elements were present in the buffer.
    EXPECT_EQ(j, i + 1);
  }
  buffer.Clear();
  EXPECT_EQ(buffer.begin(), buffer.end());
}

}  // namespace grpc_core

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
