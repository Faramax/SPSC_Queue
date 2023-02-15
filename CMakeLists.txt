add_library(spsc_queue INTERFACE
   SPSCQueue.h
   SPSCQueueOPT.h
   SPSCVarQueue.h
   SPSCVarQueueOPT.h
   SPSCOnMemoryQueue.h)
target_include_directories(spsc_queue INTERFACE .)
add_subdirectory( test )