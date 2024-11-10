#include <gtest/gtest.h>

#include "OrderBook.h"

TEST(OrderBook, Basic) {
   OrderBook ob;

   ASSERT_EQ(ob.AddSellOrder(11, 1).id, 0);
   ASSERT_EQ(ob.AddSellOrder(12, 1).id, 1);

   ASSERT_EQ(ob.AddBuyOrder(9, 1).id, 2);
   ASSERT_EQ(ob.AddBuyOrder(8, 1).id, 3);

   ASSERT_EQ(ob.AddSellOrder(10, 1).tradeResult, OrderBook::TradeResult{});
   ASSERT_EQ(ob.AddBuyOrder(10, 1).tradeResult, OrderBook::TradeResult(2, 1));

   ASSERT_EQ(ob.AddSellOrder(10, 2).tradeResult, OrderBook::TradeResult{});
   ASSERT_EQ(ob.AddBuyOrder(10, 3).tradeResult, OrderBook::TradeResult(1, 2));
}

TEST(OrderBook, RichBuyer) {
   OrderBook ob;

   ob.AddSellOrder(15, 5);
   ob.AddSellOrder(14, 4);
   ob.AddSellOrder(13, 3);
   ob.AddSellOrder(12, 2);
   ob.AddSellOrder(11, 1);

   ASSERT_EQ(ob.AddBuyOrder(15, 15).tradeResult, OrderBook::TradeResult( 6, 15 ));
   ASSERT_EQ(ob.TotalOrders(), 0);
}

TEST(OrderBook, RichSeller) {
   OrderBook ob;

   ob.AddBuyOrder(15, 5);
   ob.AddBuyOrder(14, 4);
   ob.AddBuyOrder(13, 3);
   ob.AddBuyOrder(12, 2);
   ob.AddBuyOrder(11, 1);

   ASSERT_EQ(ob.AddSellOrder(0, 15).tradeResult, OrderBook::TradeResult(6, 15));
   ASSERT_EQ(ob.TotalOrders(), 0);
}

TEST(OrderBook, Buy) {
   OrderBook ob;

   ob.AddSellOrder(15, 5);
   ob.AddSellOrder(14, 4);
   ob.AddSellOrder(13, 3);
   ob.AddSellOrder(12, 2);
   ob.AddSellOrder(11, 1);

   ASSERT_EQ(ob.AddBuyOrder(15, 4).tradeResult, OrderBook::TradeResult(3, 4));
   ASSERT_EQ(ob.TotalOrders(), 3);
}

TEST(OrderBook, Sell) {
   OrderBook ob;

   ob.AddBuyOrder(15, 5);
   ob.AddBuyOrder(14, 4);
   ob.AddBuyOrder(13, 3);
   ob.AddBuyOrder(12, 2);
   ob.AddBuyOrder(11, 1);

   ASSERT_EQ(ob.AddSellOrder(10, 13).tradeResult, OrderBook::TradeResult(4, 13));
   ASSERT_EQ(ob.TotalOrders(), 2);
}
