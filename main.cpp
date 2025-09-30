#include <thread>
#include <vector>
#include <iostream>
#include "performant_thread.h"

// Struct representing sensor readings
struct SensorReadings {
    int8_t temperature;
    char padding1[63];
    int8_t humidity;
    char padding2[63];
    int8_t pressure;
    char padding3[63];
    int8_t light;
    char padding4[63];
    int8_t co2;
    char padding5[63];  
    int8_t aqi;
};

struct thread_info
{
  void (*thread_func)(uint32_t, uint8_t *, int8_t *) = nullptr;
  uint32_t threadIdx = -1;
  uint8_t *data_addr = nullptr;
  int8_t *data = nullptr;
};

std::vector<thread_info> _threads;

SensorReadings readings;

int work_init()
{
  std::cout << "Allocating & Initializing Memory" << std::endl;
  std::vector<thread_info> threads;

  int thread_count = 6;
  for (int i = 0; i < thread_count; i++)
  {
    thread_info thread_info;
    /*
    This ensures that when the thread is launched later in work_run(), it will execute the parallel work defined in performant_thread_run.
    */
    thread_info.thread_func = performant_thread_run;
    /*
    This index is used by the parallel function (performant_thread_run) to partition or differentiate its work from other threads.
    */
    thread_info.threadIdx = i;

    // actual code used collected raw data but we replaced it with random
    // data for the sake of privacy and simplicity.
    uint8_t *thread_data = (uint8_t *)malloc(TOTAL_SIZE);     // This creates a dedicated 1 MiB block of input data for the current thread to work on.
    madvise(thread_data, TOTAL_SIZE, MADV_HUGEPAGE); //  Using huge pages is a performance optimization. 
                                                     // It reduces the number of entries needed in the Translation Lookaside Buffer (TLB), 
                                                     // which is a cache for page table entries. Fewer TLB misses mean faster address translation and thus faster memory 
                                                     // access for the threads. 
                                                     // This reflects a key focus on low-level performance tuning in the assignment.
    for (long unsigned int j = 0; j < TOTAL_SIZE; j++)
    {
      thread_data[j] = rand() % 256;
      /*
      This line is executed repeatedly inside a loop to fill the entire 1 MiB block of memory dedicated to the current thread with randomized,
      simulated weather data. This is done to create a realistic workload for the parallel processing step.
      */
    }
    thread_info.data_addr = thread_data;
    /*
    This action stores the base address of the input data that the thread is supposed to process. When the thread is launched later, 
    this address will be passed as the second argument to the performant_thread_run function to tell it where to find its unique input data.
    */
    switch(i) {
      case 0:
      thread_info.data = &readings.temperature; // Thread 0 is configured to write its result to the temperature field.
      break;
      case 1:
      thread_info.data = &readings.humidity; // Thread 1 is configured to write its result to the humidity field.
      break;
      case 2:
      thread_info.data = &readings.pressure; // Thread 2 is configured to write its result to the pressure field. 
      break;
      case 3:
      thread_info.data = &readings.light; // Thread 3 is configured to write its result to the light field. 
      break;
      case 4:
      thread_info.data = &readings.co2; // Thread 4 is configured to write its result to the co2 field.
      break;
      case 5:
      thread_info.data = &readings.aqi; // Thread 5 is configured to write its result to the aqi field.
      break;
      default:
        perror("Error, no need of this thread");  
    }
    _threads.push_back(thread_info); // Store the configured thread_info in the global _threads vector for later use in work_run().
  }

  return _threads.size();  // Return the total number of threads initialized.
}

/* Work run Function is the thread managment function. it's  primary purpose is to create ,start, and wait for thread completetion of all worker threads tha
that execute the parallel workload
*/
void work_run()
{
  std::cout << "Starting Threads for Running" << std::endl;
  std::vector<std::thread> threads;

  for (long unsigned int i = 0; i < _threads.size(); i++)
  {
    /*  create 
    threads.push_back(...)= It adds the newly created std::thread object (everything inside the parentheses) 
                             to the end of the vector, allowing the main program to track and manage the thread later 
    
    std::thread(...)=  _threads[i].thread_func:This is the function (or callable object/pointer) that the new thread will execute. It is the entry point for the thread's parallel workload. 
                                              This function will likely call the external library's performant_thread_run() function.
                      _threads[i].threadIdx: This is the first argument passed to thread_func. 
                                            It likely represents the unique index (ID) of the thread, 
                                            often used to partition the workload or identify the thread in logs.
                      _threads[i].data_addr: This is the second argument passed to thread_func. Based on the function declaration,
                                              this is a base memory address (uint8_t*) pointing to the overall dataset or a shared memory region.
                      _threads[i].data: This is the third argument passed to thread_func. This is likely a pointer (int8_t*) to the specific, 
                                        partitioned segment of data that only this thread will work on.
*/
    threads.push_back(std::thread(_threads[i].thread_func, _threads[i].threadIdx, _threads[i].data_addr, _threads[i].data));
  }

  for (long unsigned int i = 0; i < threads.size(); i++)
  {
    /*
      threads[i].join():  This is the crucial synchronization call. The join() method is called on each thread object in the threads vector.
                          It blocks the calling thread (the main program) until the thread represented by threads[i] has completed its execution.
                          This ensures that the main program waits for all worker threads to finish before proceeding, 
                          which is essential for data integrity and proper program termination.
    */
    threads[i].join();
  }
  return;
}

int main(void)
{

  work_init();
  work_run();

  std::cout << "Final Sensor Readings:\n";
  std::cout << "Temperature: " << static_cast<int>(readings.temperature) << "\n";
  std::cout << "Humidity: " << static_cast<int>(readings.humidity) << "\n";
  std::cout << "Pressure: " << static_cast<int>(readings.pressure) << "\n";
  std::cout << "Light: " << static_cast<int>(readings.light) << "\n";
  std::cout << "CO2: " << static_cast<int>(readings.co2) << "\n";
  std::cout << "AQI: " << static_cast<int>(readings.aqi) << "\n";
}
