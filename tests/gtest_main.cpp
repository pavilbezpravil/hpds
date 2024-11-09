#include <gtest/gtest.h>
#include "Queue.h"

TEST(Queue, Basic) {
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

int main(int argc, char** argv) {
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
