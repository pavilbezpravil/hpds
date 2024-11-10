#pragma once
#include <map>

class OrderBook {
public:
   using OrderId = int;
   using Price = int;
   using Quantity = int;

   struct LevelInfo {
      Price price;
      Quantity quantity;
   };

   struct TradeResult {
      int canceledOrders = 0;
      Price volume = 0;

      auto operator<=>(const TradeResult&) const = default;
   };

   struct OrderResult {
      OrderId id;
      TradeResult tradeResult;

      auto operator<=>(const OrderResult&) const = default;
   };

   OrderResult AddSellOrder(Price price, Quantity quantity) {
      return AddOrder(price, quantity, false);
   }

   OrderResult AddBuyOrder(Price price, Quantity quantity) {
      return AddOrder(price, quantity, true);
   }

   OrderResult AddOrder(Price price, Quantity quantity, bool isBuy) {
      OrderId id = GetNextOrderId();
      orderToPriceMap[id] = std::make_pair(price, isBuy);

      if (isBuy) {
         buyOrdersMap[price].map[id] = quantity;
         buyOrdersMap[price].quantity += quantity;
      } else {
         sellOrdersMap[price].map[id] = quantity;
         sellOrdersMap[price].quantity += quantity;
      }
      return OrderResult{ id, MatchOrders() };
   }

   int TotalOrders() const {
      return (int)orderToPriceMap.size();
   }

private:
   struct OrdersMapLevelInfo {
      std::map<OrderId, Quantity> map;
      Quantity quantity;
   };

   std::map<Price, OrdersMapLevelInfo> sellOrdersMap;
   std::map<Price, OrdersMapLevelInfo, std::greater<>> buyOrdersMap;
   std::map<OrderId, std::pair<Price, bool>> orderToPriceMap;

   TradeResult MatchOrders() {
      TradeResult result{};

      auto sellLowerIt = sellOrdersMap.begin();
      auto buyHighestIt = buyOrdersMap.begin();

      while (sellLowerIt != sellOrdersMap.end() && buyHighestIt != buyOrdersMap.end()) {
         if (buyHighestIt->first < sellLowerIt->first) {
            break;
         }

         auto& sellOrders = sellLowerIt->second.map;
         auto& buyOrders = buyHighestIt->second.map;

         Quantity& sellQuantities = sellLowerIt->second.quantity;
         Quantity& buyQuantities = buyHighestIt->second.quantity;

         auto sellOrderIt = sellOrders.begin();
         auto buyOrderIt = buyOrders.begin();

         while (sellOrderIt != sellOrders.end() && buyOrderIt != buyOrders.end()) {
            Quantity quantity = std::min(sellOrderIt->second, buyOrderIt->second);

            result.volume += quantity;

            sellOrderIt->second -= quantity;
            buyOrderIt->second -= quantity;

            sellQuantities -= quantity;
            buyQuantities -= quantity;

            if (sellOrderIt->second == 0) {
               orderToPriceMap.erase(sellOrderIt->first);
               sellOrderIt = sellOrders.erase(sellOrderIt);
               ++result.canceledOrders;
            }
            if (buyOrderIt->second == 0) {
               orderToPriceMap.erase(buyOrderIt->first);
               buyOrderIt = buyOrders.erase(buyOrderIt);
               ++result.canceledOrders;
            }
         }

         if (sellOrderIt == sellOrders.end()) {
            sellLowerIt = sellOrdersMap.erase(sellLowerIt);
         }
         if (buyOrderIt == buyOrders.end()) {
            buyHighestIt = buyOrdersMap.erase(buyHighestIt);
         }
      }

      return result;
   }

   OrderId nextOrderId = 0;

   OrderId GetNextOrderId() {
      return nextOrderId++;
   }
};
