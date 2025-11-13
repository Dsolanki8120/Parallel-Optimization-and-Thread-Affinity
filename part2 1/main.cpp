#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <sys/mman.h>
#include <map>
#include <iostream>
#include <stdlib.h>
#include "work.h"
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <fstream> 


struct ThreadInfo {
    pid_t tid;
    pthread_t handle;
    cpu_set_t cpuset;

    std::chrono::high_resolution_clock::time_point start_time;
    double runtime;
};

static std::map<uint32_t, ThreadInfo> thread_map; // thread_map stores ThreadInfo objects keyed by threadIdx


// implement the functions here.
void start_monitoring_for_thread(uint32_t threadIdx, pid_t tid, pthread_t handle) {
    ThreadInfo info;
    info.tid = tid;
    info.handle = handle;

    CPU_ZERO(&info.cpuset);
    int s = pthread_getaffinity_np(handle, sizeof(cpu_set_t), &info.cpuset);
    if (s != 0)
        std::cerr << "pthread_getaffinity_np: " << strerror(s) << "\n";

    info.start_time = std::chrono::high_resolution_clock::now();

    // Give the thread a little time to start real work
    usleep(200000); // 0.2 seconds

    // Run perf for 5 seconds regardless of thread lifetime
    std::ostringstream cmd;
    cmd << "nohup perf stat -e cycles,instructions,cache-misses,context-switches "
        << "-p " << tid
        << " -o perf_thread_" << threadIdx << ".log -- sleep 5 >/dev/null 2>&1 &";

    int ret = system(cmd.str().c_str());
    if (ret != 0)
        std::cerr << "Failed to launch perf for thread " << threadIdx << std::endl;
    else
        std::cout << "Perf started for threadIdx: " << threadIdx
                  << " (tid=" << tid << ")\n";

    thread_map[threadIdx] = info;
}


void stop_monitoring_for_thread(uint32_t threadIdx) {
    auto it = thread_map.find(threadIdx);
    if (it != thread_map.end()) {
        sleep(10  );
        auto end_time = std::chrono::high_resolution_clock::now();
        it->second.runtime =
            std::chrono::duration<double>(end_time - it->second.start_time).count();

        std::cout << "Stop Monitoring threadIdx: " << threadIdx
                  << " TID=" << it->second.tid << " "
                  << "Runtime: " << it->second.runtime << " sec\n";

        thread_map.erase(it);
    }
}

int32_t get_thread_affinity(uint32_t threadIdx) {
    static std::map<uint32_t, int> mapping;
    if(mapping.empty()) {
        // Read mapping from Python output file
        std::ifstream fin("thread_affinity.txt");
        int t, core;
        while(fin >> t >> core) {
            mapping[t] = core;
        }
    }
    if(mapping.find(threadIdx) != mapping.end())
        return mapping[threadIdx];
    return -1;
}



// int32_t get_thread_affinity(uint32_t threadIdx) {
//     auto it = thread_map.find(threadIdx);
//     if (it == thread_map.end()) return -1;

//     for (int j = 0; j < CPU_SETSIZE; j++) {
//         if (CPU_ISSET(j, &it->second.cpuset)) {
//             return j;
//         }
//     }
//     return -1;
// }


int main(int argc, char **argv)
{
    // DO NOT MODIFY THE FOLLOWING BLOCK.
    // =================================================
    if (argc != 2)
    {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    int sr_no = atoi(argv[1]);
    int num_threads = work_init(sr_no);
    // =================================================
    // Modify below this
  
    std::cout << "The number of threads: " << num_threads << std::endl;

    // Modify above this
    // DO NOT MODIFY THE FOLLOWING BLOCK.
    // =================================================
    work_start_monitoring();
    // =================================================
    // Modify below this

    // (Optional) Add your own analysis / logging here

    // Modify above this
    // DO NOT MODIFY THE FOLLOWING BLOCK.
    work_run();
    // =================================================
    // Modify below this

    return 0;
}