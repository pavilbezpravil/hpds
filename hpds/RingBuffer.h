#pragma once
#include <vector>
#include <optional>

// todo: use proper memory model
template<typename T>
class RingBuffer {
public:
   RingBuffer(int size) : capacity(size + 1) {
      buffer.resize(capacity);
   }

   bool Push(const T& data) {
      int nextTail = Increment(tail);
      if (nextTail == head) {
         return false;
      }

      buffer[tail] = data;
      tail = nextTail;

      return true;

   }

   std::optional<T> Pop() {
      if (head == tail) {
         return {};
      }

      T ret = std::move(buffer[head]);
      head = Increment(head);

      return ret;
   }

   T PopWait() {
      while (true) {
         if (auto data = Pop()) {
            return data.value();
         }
      }
   }

   bool WasEmpty() const {
      return tail == head;
   }

   bool WasFull() const {
      return Increment(tail) == head;
   }

private:
   std::vector<T> buffer;
   int capacity;
   int head = 0;
   int tail = 0;

   int Increment(int val) const {
      return (val + 1) % capacity; // note: if capacity known in compile time % will be faster!
   }
};
