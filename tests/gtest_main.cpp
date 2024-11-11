#include <gtest/gtest.h>

#include "IndexPool.h"
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
   RingBuffer<int> rb{3};
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

TEST(RingBuffer, MultiThreaded) {
   ASSERT_TRUE(RingBufferMultiThreadTest(10, 10'000'000));
}

TEST(IndexPool, Basic) {
   IndexPool pool{ 3 };
   ASSERT_EQ(pool.Available(), 3);
   ASSERT_EQ(pool.Allocate(), 0);
   ASSERT_EQ(pool.Available(), 2);
   ASSERT_EQ(pool.Allocate(), 1);
   ASSERT_EQ(pool.Available(), 1);

   pool.Free(0);
   ASSERT_EQ(pool.Allocate(), 0);
   pool.Free(1);
   ASSERT_EQ(pool.Allocate(), 1);
   ASSERT_EQ(pool.Allocate(), 2);
   pool.Free(0);
   pool.Free(2);
   pool.Free(1);
   ASSERT_EQ(pool.Available(), 3);
}

#include <memory>
#include <iostream>
#include <vector>

template<typename T>
class SimpleAllocator {
public:
   using value_type = T;

   SimpleAllocator() = default;
   // SimpleAllocator(const SimpleAllocator&) = default;
   // SimpleAllocator(SimpleAllocator&&) = default;
   //
   // SimpleAllocator& operator=(const SimpleAllocator&) = default;
   // SimpleAllocator& operator=(SimpleAllocator&&) = default;

   template<typename U>
   constexpr SimpleAllocator(const SimpleAllocator<U>&) noexcept {}

   T* allocate(std::size_t n) {
      std::cout << "Allocating " << n * sizeof(T) << " bytes" << std::endl;
      if (n > std::allocator_traits<SimpleAllocator>::max_size(*this)) {
         throw std::bad_alloc();
      }
      return static_cast<T*>(::operator new(n * sizeof(T)));
   }

   void deallocate(T* p, std::size_t n) noexcept {
      std::cout << "Deallocating memory" << std::endl;
      ::operator delete(p);
   }

   // template<typename U, typename... Args>
   // void construct(U* p, Args&&... args) {
   //    std::cout << "Constructing element" << std::endl;
   //    new(p) U(std::forward<Args>(args)...);
   // }
   //
   // template<typename U>
   // void destroy(U* p) noexcept {
   //    std::cout << "Destroying element" << std::endl;
   //    p->~U();
   // }
   //
   // friend bool operator==(const SimpleAllocator&, const SimpleAllocator&) { return true; }
   // friend bool operator!=(const SimpleAllocator&, const SimpleAllocator&) { return false; }
};

template<typename T>
class PoolAllocator {
public:
   using value_type = T;

   // PoolAllocator() = default;
   PoolAllocator() {
      std::cout << "Allocator create for " << sizeof(T) << " bytes" << std::endl;
   }
   ~PoolAllocator() {
      std::cout << "Allocator destroyed for " << sizeof(T) << " bytes" << std::endl;
   }

   PoolAllocator(const PoolAllocator&) = default;
   PoolAllocator(PoolAllocator&&) = default;
   
   PoolAllocator& operator=(const PoolAllocator&) = default;
   PoolAllocator& operator=(PoolAllocator&&) = default;

   template<typename U>
   constexpr PoolAllocator(const PoolAllocator<U>&) noexcept {}

   T* allocate(std::size_t n) {
      std::cout << "Allocating " << n * sizeof(T) << " bytes" << std::endl;

      assert(n == 1 && "PoolAllocatorWrapper can only allocate one element at a time");
      if (m_freeList.empty()) {
         allocateMemory(m_blockElementCount);
      }
      
      void* ptr = m_freeList.back();
      m_freeList.pop_back();
      return static_cast<T*>(ptr);
   }

   void deallocate(T* p, std::size_t n) noexcept {
      std::cout << "Deallocating memory" << std::endl;

      assert(n == 1 && "PoolAllocatorWrapper can only deallocate one element at a time");
      m_freeList.push_back(p);
   }

private:
   std::vector<std::unique_ptr<uint8_t[]>> m_blocks;
   std::size_t m_blockElementCount = 64;
   std::vector<void*> m_freeList;

   void allocateMemory(std::size_t elementCount) {
      constexpr std::size_t m_elementSize = sizeof(T);

      auto newBlock = std::make_unique<uint8_t[]>(m_elementSize * elementCount);
      uint8_t* rawBlock = newBlock.get();
      m_blocks.push_back(std::move(newBlock));
      for (std::size_t i = 0; i < elementCount; ++i) {
         void* ptr = rawBlock + i * m_elementSize;
         m_freeList.push_back(ptr);
      }
   }
};

TEST(Allocator, Basic) {
   // std::map<int, int, std::less<>, SimpleAllocator<std::pair<const int, int>>> map;
   std::map<int, int, std::less<>, PoolAllocator<std::pair<const int, int>>> map;
   map[0] = 42;
   map[1] = 137;
   map[2] = 228;
   map[3] = 31415;

   ASSERT_EQ(map[0], 42);
   ASSERT_EQ(map[1], 137);

   map.clear();
}

int main(int argc, char** argv) {
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
