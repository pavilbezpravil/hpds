#include <gtest/gtest.h>

#include "OrderBook.h"

TEST(OrderBook, Basic) {
   OrderBook ob;

   ASSERT_EQ(ob.AddSellOrder(11, 1), 0);
   ASSERT_EQ(ob.AddSellOrder(12, 1), 1);

   ASSERT_EQ(ob.AddBuyOrder(9, 1), 2);
   ASSERT_EQ(ob.AddBuyOrder(8, 1), 3);
   ASSERT_EQ(ob.MatchOrders(), OrderBook::MatchOrderResult{});

   ob.AddSellOrder(10, 1);
   ASSERT_EQ(ob.MatchOrders(), OrderBook::MatchOrderResult{});
   ob.AddBuyOrder(10, 1);
   ASSERT_EQ(ob.MatchOrders(), OrderBook::MatchOrderResult(2, 1));

   ob.AddSellOrder(10, 2);
   ASSERT_EQ(ob.MatchOrders(), OrderBook::MatchOrderResult{});
   ob.AddBuyOrder(10, 3);
   ASSERT_EQ(ob.MatchOrders(), OrderBook::MatchOrderResult(1, 2));
}
