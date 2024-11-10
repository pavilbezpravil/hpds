#pragma once
#include <map>

class OrderBook {
public:
   using OrderId = int;
   using OrderPrice = int;
   using OrderQuantity = int;

   struct Order {
      OrderId id;
      OrderPrice price;
      OrderQuantity quantity;
   };

   OrderId AddSellOrder(OrderPrice price, OrderQuantity quantity) {
      return AddOrder(price, quantity, false);
   }

   OrderId AddBuyOrder(OrderPrice price, OrderQuantity quantity) {
      return AddOrder(price, quantity, true);
   }

   OrderId AddOrder(OrderPrice price, OrderQuantity quantity, bool isBuy) {
      OrderId id = GetNextOrderId();
      orderToPriceMap[id] = std::make_pair(price, isBuy);

      if (isBuy) {
         buyOrdersMap[price][id] = quantity;
      } else {
         sellOrdersMap[price][id] = quantity;
      }
      return id;
   }

   struct MatchOrderResult {
      int executeOrders = 0;
      OrderPrice totalOrdersPrice = 0;

      auto operator<=>(const MatchOrderResult&) const = default;
   };

   MatchOrderResult MatchOrders() {
      MatchOrderResult result{};

      while (!sellOrdersMap.empty() && !buyOrdersMap.empty()) {
         auto sellLowerIt = sellOrdersMap.begin();
         auto buyHighestIt = buyOrdersMap.begin();

         if (buyHighestIt->first < sellLowerIt->first) {
            break;
         }

         auto& sellOrders = sellLowerIt->second;
         auto& buyOrders = buyHighestIt->second;

         auto sellOrderIt = sellOrders.begin();
         auto buyOrderIt = buyOrders.begin();

         while (sellOrderIt != sellOrders.end() && buyOrderIt != buyOrders.end()) {
            if (sellOrderIt->second <= buyOrderIt->second) {
               // note: if equal buy must be deleted to and it will on next iteration. I suppose it be faster because less branches
               buyOrderIt->second -= sellOrderIt->second;
               result.totalOrdersPrice += sellOrderIt->second;
               sellOrderIt = sellOrders.erase(sellOrderIt);
            } else {
               sellOrderIt->second -= buyOrderIt->second;
               result.totalOrdersPrice += sellOrderIt->second;
               buyOrderIt = buyOrders.erase(buyOrderIt);
            }
            ++result.executeOrders;
         }

         // we dont must not keep empty orders
         if (buyOrderIt->second == 0) {
            buyOrderIt = buyOrders.erase(buyOrderIt);
            ++result.executeOrders;
         }

         if (sellOrderIt == sellOrders.end()) {
            sellOrdersMap.erase(sellLowerIt);
         }
         if (buyOrderIt == buyOrders.end()) {
            buyOrdersMap.erase(buyHighestIt);
         }
      }

      return result;
   }

private:
   std::map<OrderPrice, std::map<OrderId, OrderQuantity>> sellOrdersMap;
   std::map<OrderPrice, std::map<OrderId, OrderQuantity>, std::greater<>> buyOrdersMap;
   std::map<OrderId, std::pair<OrderPrice, bool>> orderToPriceMap;

   OrderId nextOrderId = 0;

   OrderId GetNextOrderId() {
      return nextOrderId++;
   }
};
