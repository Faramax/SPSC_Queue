#include <cstdint>
#include <cstddef>
#include <SPSCQueue.h>

int main()
{
   static constexpr size_t queue_len = 64;
   SPSCQueue<uint32_t, queue_len> queue;
   return 1;
}