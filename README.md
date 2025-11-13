Part 1: Parallel Optimization and False Sharing Diagnosis


In first part daignosing why the supposedly "fully parallel" program was performing poorly (∼51.5 ms total time). Our analysis revealed that the performance bottleneck was False Sharing due to the memory layout of the shared output structure

Initial Performance and Profiling:  


(base) deepaksolanki@vgladmin-HP-Z620-Workstation:~/PLACEMENT/HPCA_ASSINGMENT/part1 1$ make run
./main
Allocating & Initializing Memory
Starting Threads for Running
TheadIdx: 5 completed, time was 41.973508473.
TheadIdx: 1 completed, time was 46.843366250.
TheadIdx: 3 completed, time was 49.90118160.
TheadIdx: 2 completed, time was 50.609778203.
TheadIdx: 0 completed, time was 50.646568748.
TheadIdx: 4 completed, time was 51.489711378.
Final Sensor Readings:
Temperature: 68
Humidity: 98
Pressure: 58
Light: 57
CO2: 24
AQI: 72


--------------------------------------- profiling of thread where lots of time is going on ---------------------------------------

command :- perf record -g ./main 

Samples: 923K of event 'cycles:P', Event count (approx.): 360430793001

  Children      Self  Command  Shared Object         Symbol
  +   99.88%     0.00%  main     libstdc++.so.6.0.30   [.] 0x00007ba4932dc253   
  +   99.87%    97.44%  main     main                  [.] performant_thread_run  
  +   2.32%     0.36%  main     main                  [.] unsigned long std::uniform_int_distribution<unsigned long>::operator()           <std::mersenne_twister_engine<unsigned long, 32ul, 624
  +    1.96%     0.64%  main     main                  [.] unsigned long std::uniform_int_distribution<unsigned long>::operator()<std::mersenne_twister_engine<unsigned long, 32ul, 624
  +    1.16%     0.13%  main     main                  [.] unsigned int std::uniform_int_distribution<unsigned long>::_S_nd<unsigned long, std::mersenne_twister_engine<unsigned long,
  +    1.04%     0.90%  main     main                  [.] std::mersenne_twister_engine<unsigned long, 32ul, 624ul, 397ul, 31ul, 2567483615ul, 11ul, 4294967295ul, 7ul, 2636928640ul, 1

       0.41%     0.07%  main     main                  [.] std::uniform_int_distribution<unsigned long>::param_type::a() const  

       0.13%     0.13%  main     main                  [.] std::mersenne_twister_engine<unsigned long, 32ul, 624ul, 397ul, 31ul, 2567483615ul, 11ul, 4294967295ul, 7ul, 2636928640ul, 1   

       0.10%     0.09%  main     main                  [.] std::uniform_int_distribution<unsigned long>::param_type::b() const      
       
        0.09%     0.09%  main     libc.so.6             [.] __random  

        0.05%     0.05%  main     [unknown]             [k] 0xffffffffc17ecc37    



=====================  Problem ========================================================

Cache line contention :  is a performance issue that occurs in multi- core systems when multiple  processors or thread  compete for access the same cache line .causing delays and performance degradation 


Solution of this problem :  -------------------------------------------

  Data Alignment: Aligning data structures so that frequently accessed independent variables reside on different cache lines can reduce false sharing

  Padding: Explicitly padding data structures can ensure that independent data elements are separated onto different cache lines, preventing false sharing 
  

  ------------------------ After padding performance gain (speedup gained) --------------------------------------


(base) deepaksolanki@vgladmin-HP-Z620-Workstation:~/PLACEMENT/HPCA_ASSINGMENT/part1 1$ make run
./main
Allocating & Initializing Memory
Starting Threads for Running
TheadIdx: 5 completed, time was 10.471572546.
TheadIdx: 3 completed, time was 10.487321192.
TheadIdx: 2 completed, time was 10.511314475.
TheadIdx: 0 completed, time was 10.515815004.
TheadIdx: 4 completed, time was 10.526833225.
TheadIdx: 1 completed, time was 10.548088034.
Final Sensor Readings:
Temperature: 72
Humidity: 70
Pressure: 70
Light: 25
CO2: 84
AQI: 48
(base) d



speedp gain:   Speedup=  Time before padding/ Time after padding

                Speedup= 51.66ms/ 10.55 ms = 4.90 ms 

                optimization resulted in an approximate 4.9× speedup over the initial, broken parallel implementation.