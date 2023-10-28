# CSN-221 Course Project - Cache

This project was made as part of course CSN-221 : Computer Architecture and Microprocessors in Autumn Semester 2023-24.

This is a configurable data cache simulator.

Associativity can be modified to configure a *N-way set-associative, direct-mapped or fully-associative cache*.

The cache simulator uses **LRU (least recently used)** replacement policy.

It uses **Write Back and Write Allocate policies** for handling stores.

It also evaluates statistics like hit rate, average memory access time, no of loads, no of stores etc.

## Contents of Cache Folder :

- Configurable data cache simulator `cache_sim.cpp` written in C++ which also simulates main memory for integrating with processor simulator.

- Another simulator `raw_cache_sim.cpp` written in C++ which does not handle data. It is meant for evaluating statistics for large memory traces which are beyond the scope of first simulator. 

## Cache Simulator Description and Usage Guidelines :

The cache simulator will take as arguments one .txt file containing your memory traces.

**Commands:**<br>
After compiling `cache_sim.cpp` or `raw_cache_sim.cpp`, run:
```shell
./<file_name>.exe <trace_file>.txt
```

Some sample traces taken from links in References have been provided in `small_traces` and `large_traces`.
 - `small_traces` : Contains memory traces which can be tested on either simulator. Also contains a C++ script for generating new random traces.
 - `large_traces` : Contains large memory traces which MUST be tested only on `raw_cache_sim.cpp`. Also contains two C++ scripts used for processing traces which were not in right format. The traces in the folder are already processed and can be used directly.

You can also add you own traces in .txt format.

**Guidelines:**
1. Cache configuration parameters must be specified only in `params.txt` file present in Cache folder.

2. Parameters needed to be specified :
   - Cache Size (in KB and power of 2)
   - Associativity (power of 2)
   - Block Size (in bytes and power of 2)
   - Miss Penalty (in no of cycles)
  
3. The memory is BYTE-ADDRESSABLE.

4. All loads and stores happen in terms of a single 32-bit word.

5. Trace Format:
   - For `cache_sim.cpp`:<br>
      `LS ADDRESS DATAWORD`<br>
      where LS is a 0 for a load and 1 for a store, ADDRESS is an 8-character hexadecimal number, and DATAWORD is the 8-character hexadecimal data provided only in case of stores. There is a single space between each field.
   - For `raw_cache_sim.cpp`:<br>
      `LS ADDRESS IC`<br>
      where LS is a 0 for a load and 1 for a store, ADDRESS is an 8-character hexadecimal number, and IC is the number of instructions (in base 10) executed between the previous memory access and this one (including the load or store instruction itself). There is a single space between each field. The instruction count information will be used to calculate execution time in cycles.
      
6. Remember to change the size of main_memory array in `cache_sim.cpp` according to the addresses of your trace. (Current size of 1 MB works fine with all traces in `small_traces` folder)

### Calculations : 

  Hit Rate = hits / (hits + misses)
  Miss Rate = 1 - Hit Rate

  Cache access and operation time = 1 cycle (common to hits & misses)
  Miss Penalty = Main memory access and operation time (for misses) = User specified
  Extra penalty for memory writes for dirty evictions = 2 cycles

  AMAT = Hit Rate * Hit Time + Miss Rate * Miss Time
  = Hit Rate * 1 + Miss Rate * (1 + miss_pen) + (dirty_evict * 2)/(hits+misses)
  = 1 + miss_rate * miss_pen + (dirty_evict * 2) / (hits + misses)

  Assuming CPI = 1 for non-load store instructions:
  Overall CPI = total_cycles / total_instr_ct
  Memory CPI = Overall CPI - 1

### Calculations :

- **Hit Rate**: $$\frac{{\text{{hits}}}}{{\text{{hits}} + \text{{misses}}}}$$
- **Miss Rate**: 1 - hit_rate
- **Cache access and operation time**: 1 cycle (common to hits & misses)
- **Miss Penalty**: Main memory access and operation time (for misses) = User specified
- **Extra penalty for memory writes for dirty evictions**: 2 cycles

The Average Memory Access Time (AMAT) is calculated as follows:

$$\text{{AMAT}} = \text{{hit\_rate}} \times \text{{hit\_time}} + \text{{miss\_rate}} \times \text{{miss\_time}}$$
$$\text{{AMAT}} = \text{{hit\_rate}} \times 1 + \text{{miss\_rate}} \times (1 + \text{{miss\_pen}}) + \frac{{\text{{dirty\_evict}} \times 2}}{{\text{{hits}} + \text{{misses}}}}$$
$$\text{{AMAT}} = 1 + \text{{miss\_rate}} \times \text{{miss\_pen}} + \frac{{\text{{dirty\_evict}} \times 2}}{{\text{{hits}} + \text{{misses}}}}$$

Assuming CPI = 1 for non-load store instructions :

- **Overall CPI**: $$\frac{{\text{{total\_cycles}}}}{{\text{{total\_instr\_ct}}}}$$
- **Memory CPI**: Overall CPI - 1

### Do have a look at `state_table.xlsx`, it tabulates the states of cache blocks and their transitions and the few design choices made to simplify the design.

### Comparative Study :

### References :
1. https://occs.oberlin.edu/~ctaylor/classes/210SP13/cache.html
2. https://www.cs.utexas.edu/users/mckinley/352/homework/project.html
3. http://www.cs.uccs.edu/~xzhou/teaching/CS4520/Projects/Cache/Cache_Simulator.htm

*Though adding all references used is not possible yet the ones which have been most helpful while making this project have been highlighted. A lot of inputs and ideas have been taken from these references.*
<br><br>
*Feel free to report bugs or make pull requests.*
## Cheers !!!