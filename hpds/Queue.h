#pragma once
#include <optional>

class Queue {
public:
   void Enqueue(int data) {
      Node* node = new Node; // optimize
      node->data = data;

      if (tail) {
         tail->next = node;
      }

      tail = node;

      if (!head) {
         head = node;
      }
   }

   std::optional<int> Dequeue() {
      if (!head) {
         return {};
      }

      Node* node = head;
      head = head->next;

      if (!head) {
         tail = nullptr;
      }

      int ret = node->data;
      delete node;
      return ret;
   }

   bool Empty() const { return !head; }

private:
   struct Node {
      Node* next = nullptr;
      int data = 0;
   };

   Node* head = nullptr;
   Node* tail = nullptr;
};
