#define BOOST_TEST_MODULE fb test
#include <boost/test/unit_test.hpp>

#include <random>
#include <thread>

#include <language_extentions.hpp>

#include <3rd-party/spsc_queue/SPSCQueue.h>
#include <3rd-party/spsc_queue/SPSCVarQueue.h>

BOOST_AUTO_TEST_SUITE(SPSCQueueSuite)
   BOOST_AUTO_TEST_CASE(build)
   {
      SPSCQueue<uint8_t, 64> queue;
   }

   BOOST_AUTO_TEST_CASE(push_pop_string)
   {
      char orig_str[] = "My perfect_string";
      sce size_t str_size = sizeof(orig_str);

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
      sce size_t queue_size = 64;
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
      sce size_t queue_size = 64;
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
      sce size_t queue_size = 1024;
      sce size_t check_len = 32 * 1024;
      SPSCQueue<char, queue_size> queue;

      std::thread sender_thread([&]()
         {
            std::minstd_rand rand_engine(check_len);
            for (size_t i = 0; i < check_len; ++i) {
               decltype(auto) p_val = queue.alloc();
               while(p_val == nullptr)
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
               while(v == nullptr)
                  v = queue.front();
               BOOST_CHECK_EQUAL(*v, (char)rand_engine());
               queue.pop();
            }
         }
      );
      sender_thread.join();
      receiver_thread.join();
   }
BOOST_AUTO_TEST_SUITE_END() // SPSCQueueSuite



BOOST_AUTO_TEST_SUITE(SPSCVarQueueSuite)
   BOOST_AUTO_TEST_CASE(build)
   {
      SPSCVarQueue<64> queue;
   }

   BOOST_AUTO_TEST_CASE(push_pop_string)
   {
      char orig_str[] = "My perfect_string";
      sce size_t str_size = sizeof(orig_str);

      typedef SPSCVarQueue<1024 * 8> MsgQ;
      MsgQ queue;

      {
         MsgQ::MsgHeader* header = queue.alloc(str_size);
         BOOST_REQUIRE_NE(header, nullptr);
         char *msg_begin = (char*) (header + 1);
         strncpy(msg_begin, orig_str, str_size);
         queue.push();
      }
      char got_str[str_size];
      {
         MsgQ::MsgHeader* header = queue.front();
         BOOST_REQUIRE_NE(header, nullptr);
         char* msg_begin = (char*) (header + 1);
         strncpy(got_str, msg_begin, str_size);
         queue.pop();
      }
      BOOST_CHECK_EQUAL(orig_str, got_str);
   }

   BOOST_AUTO_TEST_CASE(overflow)
   {
      sce size_t queue_size  = 512;
      using MsgQ             = SPSCVarQueue<queue_size>;
      sce size_t block_size  = sizeof(MsgQ::Block);
      MsgQ  queue;
      sce size_t str_size    = queue_size - block_size - 2*block_size;   // 1-for 2nd message; 2-for VarQueue headers; rest - for str - 1st message
      {
         MsgQ::MsgHeader* header = queue.alloc(str_size);
         BOOST_CHECK_NE(header, nullptr);
         queue.push();
      }

      {
         MsgQ::MsgHeader* header = queue.alloc(str_size);
         BOOST_CHECK_EQUAL(header, nullptr);
      }

      {
         MsgQ::MsgHeader* header = queue.alloc(block_size);
         BOOST_CHECK_NE(header, nullptr);
         queue.push();
      }
   }

   BOOST_AUTO_TEST_CASE(underflow)
   {
      sce size_t queue_size  = 512;
      using MsgQ             = SPSCVarQueue<queue_size>;
      sce size_t block_size  = sizeof(MsgQ::Block);
      MsgQ  queue;
      sce size_t str_size    = queue_size/2 - block_size;   // for 2 messages and their headers

      for(size_t i = 0; i < 2; ++i)
      {
         MsgQ::MsgHeader* header = queue.alloc(str_size);
         BOOST_REQUIRE_NE(header, nullptr);
         queue.push();
      }

      {
         MsgQ::MsgHeader *header = queue.front();
         BOOST_REQUIRE_NE(header, nullptr);
         queue.pop();
      }
      {
         MsgQ::MsgHeader *header = queue.front();
         BOOST_REQUIRE_NE(header, nullptr);
         queue.pop();
      }
      {
         MsgQ::MsgHeader* header = queue.front();
         BOOST_CHECK_EQUAL(header, nullptr);
      }
   }

   struct concurrent_fixture {
      sce size_t block_size = sizeof(Block);
      sce size_t queue_size = 1024 * block_size;
      sce size_t check_len = 32 * queue_size;
      using      queueT    = SPSCVarQueue<queue_size>;
      queueT&    queue;

      concurrent_fixture(queueT& q)
         : queue(q)
      {}

      std::thread make_sender_thread() {
         return std::thread([&]() {
                               std::minstd_rand message_engine(check_len);
                               std::minstd_rand length_engine(check_len);
                               sce size_t max_message_size = 256;
                               for (size_t i = 0; i < check_len;) {
                                  auto const message_size = length_engine() % max_message_size;
                                  decltype(auto) p_val = queue.alloc(message_size);
                                  while (p_val == nullptr)
                                     p_val = queue.alloc(message_size);
                                  char *msg_begin = (char *) (p_val + 1);
                                  for (size_t j = 0; j < message_size; ++j) {
                                     *msg_begin = static_cast<char>(message_engine());
                                     ++msg_begin;
                                  }
                                  queue.push();
                                  i += message_size;
                               }
                            }
         );
      }
      std::thread make_receiver_thread()
      {
         return std::thread([&]()
            {
               std::minstd_rand message_engine(check_len);
               for (size_t i = 0; i < check_len;) {
                  MsgHeader *p_val = queue.front();
                  while (p_val == nullptr)
                     p_val = queue.front();
                  char *msg_begin = (char *) (p_val + 1);
                  auto const message_size = p_val->size - sizeof(MsgHeader);
                  for (size_t j = 0; j < message_size; ++j) {
                     BOOST_REQUIRE_EQUAL(*msg_begin, static_cast<char>(message_engine()));
                     ++msg_begin;
                  }
                  queue.pop();
                  i += message_size;
               }
            }
         );
      }
   };

   BOOST_AUTO_TEST_CASE(concurrent_threads)
   {
      concurrent_fixture::queueT queue;
      concurrent_fixture f{queue};
      std::thread sender_thread = f.make_sender_thread();
      std::thread receiver_thread = f.make_receiver_thread();
      sender_thread.join();
      receiver_thread.join();
   }

   BOOST_AUTO_TEST_CASE(on_memory_concurrent_threads)
   {
      using                queueT = concurrent_fixture::queueT;
      size_t               memory_size = sizeof(queueT);
      void*                memory_blob = malloc(memory_size);
      queueT*              queue = new(memory_blob) queueT;
      concurrent_fixture   f{*queue};
      std::thread          sender_thread = f.make_sender_thread();
      std::thread          receiver_thread = f.make_receiver_thread();
      sender_thread.join();
      receiver_thread.join();
   }
BOOST_AUTO_TEST_SUITE_END() // SPSCVarQueueSuite