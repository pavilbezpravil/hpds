#include <gtest/gtest.h>

#include "Queue.h"
#include "RingBuffer.h"

TEST(Queue, Basic)
{
   Queue q;

   q.Enqueue(0);
   q.Enqueue(1);

   ASSERT_EQ(q.Dequeue(), 0);
   ASSERT_EQ(q.Dequeue(), 1);
   ASSERT_FALSE(q.Dequeue().has_value());
   ASSERT_TRUE(q.Empty());

   q.Enqueue(2);
   q.Enqueue(3);
   ASSERT_EQ(q.Dequeue(), 2);
   ASSERT_EQ(q.Dequeue(), 3);
   ASSERT_FALSE(q.Dequeue().has_value());
   ASSERT_TRUE(q.Empty());
}

TEST(RingBuffer, SingleThread) {
   RingBuffer<int> rb{2};
   ASSERT_TRUE(rb.WasEmpty());
   ASSERT_FALSE(rb.WasFull());

   ASSERT_TRUE(rb.Push(0));
   ASSERT_TRUE(rb.Push(1));
   ASSERT_FALSE(rb.Push(2));
   ASSERT_TRUE(rb.WasFull());

   ASSERT_EQ(rb.Pop(), 0);
   ASSERT_EQ(rb.Pop(), 1);
   ASSERT_FALSE(rb.Pop().has_value());
}

TEST(RingBuffer, MT) {
   RingBuffer<int> rb{ 2 };

   int count = 10'000'000;

   std::thread writer{
   [&]
   {
      int data = 0;
      while (data < count) {
         while (!rb.Push(data));
         ++data;
      }
   }};

   int nextExpected = 0;

   std::thread reader{
   [&]
   {
      while (nextExpected < count) {
         int data = rb.PopWait();

         if (data != nextExpected) {
            break;
         }
         ++nextExpected;
      }
   }};

   writer.join();
   reader.join();

   ASSERT_EQ(nextExpected, count);
}

int main(int argc, char** argv) {
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
