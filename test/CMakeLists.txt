add_executable(mt_spsc_q
   spsc_q.cpp)
target_link_libraries(mt_spsc_q
   spsc_queue)
IF(MCU)
   target_link_libraries(mt_spsc_q
      mcu_libc)
ENDIF()

IF(X86)
   add_executable(mt_spsc_multhread_q
      cpupin.h
      multhread_q.cc
      rdtsc.h)
   target_link_libraries(mt_spsc_multhread_q pthread rt)

   add_executable(mt_spsc_multhread_varq
      cpupin.h
      multhread_varq.cc
      rdtsc.h)
   target_link_libraries(mt_spsc_multhread_varq pthread rt)

   add_executable(mt_spsc_shmq_recv
      cpupin.h
      rdtsc.h
      shmmap.h
      shmq_msg.h
      shmq_recv.cc)
   target_link_libraries(mt_spsc_shmq_recv pthread rt)

   add_executable(mt_spsc_shmq_send
      cpupin.h
      rdtsc.h
      shmmap.h
      shmq_msg.h
      shmq_send.cc)
   target_link_libraries(mt_spsc_shmq_send pthread rt)
ENDIF()