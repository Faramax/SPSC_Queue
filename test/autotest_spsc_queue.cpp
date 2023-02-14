#define BOOST_TEST_MODULE fb test
#include <boost/test/unit_test.hpp>

#include <random>
#include <thread>

#include <3rd-party/spsc_queue/SPSCQueue.h>

BOOST_AUTO_TEST_CASE(build)
{
   SPSCQueue<uint8_t, 64> queue;
}

BOOST_AUTO_TEST_CASE(push_pop_string)
{
   char orig_str[] = "My perfect_string";
   static constexpr size_t str_size = sizeof(orig_str);

   SPSCQueue<char, 64> queue;
   for(char& c : orig_str)
   {
      decltype(auto) v = queue.alloc();
      if (!v) break;
      *v = c;
      queue.push();
   }

   char got_str[str_size];
   for(size_t i = 0; i < str_size; ++i)
   {
      decltype(auto) v = queue.front();
      if (!v) break;
      got_str[i] = *v;
      queue.pop();
   }

   BOOST_CHECK_EQUAL(orig_str, got_str);
}

BOOST_AUTO_TEST_CASE(overflow)
{
   static constexpr size_t queue_size = 64;
   SPSCQueue<char, queue_size> queue;
   size_t i = 0;
   for(; i < queue_size - 1; ++i)
   {
      decltype(auto) v = queue.alloc();
      if (!v) break;
      *v = 5;
      queue.push();
   }
   BOOST_CHECK_EQUAL(i, queue_size - 1);
   decltype(auto) v = queue.alloc();
   BOOST_CHECK_NE(v, nullptr);
   queue.push();
   v = queue.alloc();
   BOOST_CHECK_EQUAL(v, nullptr);
}

BOOST_AUTO_TEST_CASE(underflow)
{
   static constexpr size_t queue_size = 64;
   SPSCQueue<char, queue_size> queue;
   size_t i;
   for(i = 0; i < queue_size; ++i)
   {
      decltype(auto) v = queue.alloc();
      if (!v) break;
      *v = 5;
      queue.push();
   }
   BOOST_CHECK_EQUAL(i, queue_size);
   for(i = 0; i < queue_size - 1; ++i)
   {
      decltype(auto) v = queue.front();
      if (!v) break;
      queue.pop();
   }
   BOOST_CHECK_EQUAL(i, queue_size - 1);
   decltype(auto) v = queue.front();
   BOOST_CHECK_NE(v, nullptr);
   queue.pop();
   v = queue.front();
   BOOST_CHECK_EQUAL(v, nullptr);
}

BOOST_AUTO_TEST_CASE(concurrent_threads)
{
   static constexpr size_t queue_size = 1024;
   static constexpr size_t check_len = 32 * 1024 * 1024;
   SPSCQueue<char, queue_size> queue;

   std::thread sender_thread([&]()
      {
         std::minstd_rand rand_engine(check_len);
         for (size_t i = 0; i < check_len; ++i) {
            decltype(auto) p_val = queue.alloc();
            while(p_val == nullptr);
               p_val = queue.alloc();
            *p_val = rand_engine();
            queue.push();
         }
      }
   );

   std::thread receiver_thread([&]()
      {
         std::minstd_rand rand_engine(check_len);
         for (size_t i = 0; i < check_len; ++i) {
            decltype(auto) v = queue.front();
            while(v == nullptr);
               v = queue.front();
            BOOST_CHECK_EQUAL(*v, (char)rand_engine());
            queue.pop();
         }
      }
   );
   sender_thread.join();
   receiver_thread.join();
}