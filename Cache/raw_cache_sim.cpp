#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <list>
using std::cout, std::cin, std::string, std::vector;

struct trace_data
{
    bool rw;
    long long address;
    int data = -1;
    int instr_ct;
};

vector<trace_data> readTraces(string &traceFile)
{
    std::ifstream inTrace(traceFile);
    string traceLine;
    vector<trace_data> traces;
    while (getline(inTrace, traceLine))
    {
        trace_data trace;
        std::stringstream ss(traceLine);
        string token;
        getline(ss, token, ' ');
        trace.rw = (bool)(stoi(token));
        getline(ss, token, ' ');
        trace.address = stoll(token, 0, 16);
        getline(ss, token, ' ');
        trace.instr_ct = stoi(token);
        // if (trace.rw)
        // {
        //     getline(ss, token, ' ');
        //     trace.data = stoi(token, 0, 16);
        // }
        traces.push_back(trace);
    }
    inTrace.close();
    return traces;
}

class CPUreq
{
public:
    bool read_write;
    long long tag;
    long long index;
    long long offset;
    int data;
    int instr_ct;
    CPUreq(trace_data &trace, int offset_size, long long offset_mask, int index_size, long long index_mask)
    {
        long long addr = trace.address;
        addr >>= 2; // byte address to word address
        read_write = trace.rw;
        offset = addr & offset_mask;
        index = (addr >> offset_size) & index_mask;
        tag = addr >> (index_size + offset_size);
        data = trace.data;
        instr_ct = trace.instr_ct;
    }
    void getReqData()
    {
        cout << read_write << " " << tag << " " << index << " " << offset << " ";
        if (read_write)
            cout << data;
        cout << "\n";
    }
};

class cache_block
{
public:
    bool valid_bit, dirty_bit;
    long long tag;
    vector<int> data_block;
    cache_block(int block_size)
    {
        valid_bit = 0;
        dirty_bit = 0;
        tag = -1;
        data_block.assign(block_size, 0);
    }
};

class lru
{
    std::list<int> q;
    std::unordered_map<int, std::list<int>::iterator> lookup_table;

public:
    void removeExistingElement(int ind)
    {
        if (lookup_table.find(ind) != lookup_table.end())
        {
            q.erase(lookup_table[ind]);
            lookup_table.erase(ind);
        }
    }

    void addMRUelement(int ind)
    {
        q.push_front(ind);
        lookup_table[ind] = q.begin();
    }

    int removeLRUelement()
    {
        int evict_ind = q.back();
        q.pop_back();
        lookup_table.erase(evict_ind);
        return evict_ind;
    }

    void printData()
    {
        for (auto it = q.begin(); it != q.end(); it++)
            cout << *it << " ";
        cout << "\n";
    }
};

class Cache
{
public:
    vector<vector<cache_block>> cache_array;
    vector<lru> lru_array;
    int cache_size;
    int block_size;
    int offset_size;
    long long offset_mask;
    int block_num;
    int set_num;
    int ways;
    int index_size;
    long long index_mask;

    Cache(int cache_sz, int ways, int block_sz)
    {
        this->ways = ways;
        cache_size = cache_sz * 256; // KB -> no of words
        block_size = block_sz / 4;   // bytes -> no of words
        offset_size = std::__lg(block_size);
        offset_mask = (1 << offset_size) - 1;
        block_num = cache_size / block_size;
        set_num = block_num / ways;
        index_size = std::__lg(set_num);
        index_mask = (1 << index_size) - 1;
        cache_array.assign(set_num, vector<cache_block>(ways, cache_block(block_size)));
        lru_array.resize(set_num);
    }

    void hitUpdate(CPUreq &newRequest, int ind)
    {
        lru &lru_queue = lru_array[newRequest.index];
        lru_queue.removeExistingElement(ind);
        lru_queue.addMRUelement(ind);
    }

    int insert(CPUreq &newRequest)
    {
        vector<cache_block> &set = cache_array[newRequest.index];
        lru &lru_queue = lru_array[newRequest.index];
        for (int i = 0; i < ways; i++)
        {
            if (!set[i].valid_bit)
            {
                lru_queue.addMRUelement(i);
                return i;
            }
        }
        return -1;
    }

    int replace(CPUreq &newRequest)
    {
        lru &lru_queue = lru_array[newRequest.index];
        int evict_ind = lru_queue.removeLRUelement();
        lru_queue.addMRUelement(evict_ind);
        return evict_ind;
    }

    void evict(cache_block &evict_block, CPUreq &newRequest, int &dirty_evict, int &cycles)
    {
        if (evict_block.valid_bit && evict_block.dirty_bit)
            dirty_evict++, cycles += 2; // memory write
    }
};

vector<int> readParams(string &paramsFile)
{
    string paramLine;
    std::ifstream inParams(paramsFile);
    vector<int> params;
    while (getline(inParams, paramLine))
    {
        vector<string> tokens;
        std::stringstream ss(paramLine);
        string token;
        while (getline(ss, token, ':'))
            tokens.push_back(token);

        params.push_back(stoi(tokens[1]));
    }
    inParams.close();
    return params;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "Incorrect Format\n";
        return 0;
    }
    string traceFile = argv[1];
    string paramsFile = "params.txt";

    // Cache initialization
    vector<int> params = readParams(paramsFile);
    int cache_size = params[0];
    int ways = params[1];
    int block_size = params[2];
    int miss_pen = params[3];
    string rep_policy = "LRU (Least Recently Used)";
    string write_policy = "Write Back and Write Allocate";
    Cache cache(cache_size, ways, block_size);
    int hits = 0, misses = 0, reads = 0, writes = 0, dirty_evict = 0;
    int cycles = 0, instr_count = 0;

    // Reading Traces
    vector<trace_data> traces = readTraces(traceFile);

    // Simulation
    for (auto &trace : traces)
    {
        CPUreq newRequest(trace, cache.offset_size, cache.offset_mask, cache.index_size, cache.index_mask);
        int CPUresp;
        bool miss = true;
        vector<cache_block> &set = cache.cache_array[newRequest.index];
        instr_count += newRequest.instr_ct;
        cycles += newRequest.instr_ct;
        for (int i = 0; i < cache.ways; i++)
        {
            cache_block &block = set[i];
            if (block.tag == newRequest.tag) // hit
            {
                hits++;
                miss = false;
                cache.hitUpdate(newRequest, i);
                if (!newRequest.read_write) // read
                {
                    // CPUresp = block.data_block[newRequest.offset];
                    reads++;
                }
                else // write
                {
                    // block.data_block[newRequest.offset] = newRequest.data;
                    block.valid_bit = 1;
                    block.dirty_bit = 1;
                    writes++;
                }
                break;
            }
        }
        if (miss) // miss
        {
            misses++;
            cycles += miss_pen; // miss penalty (memory access + read)
            cache_block newBlock(cache.block_size);
            newBlock.tag = newRequest.tag;
            int insertion_ind = cache.insert(newRequest);
            if (insertion_ind == -1)
            {
                int evict_ind = cache.replace(newRequest);
                cache_block evict_block = set[evict_ind];
                cache.evict(evict_block, newRequest, dirty_evict, cycles);
                insertion_ind = evict_ind;
            }
            if (!newRequest.read_write) // read
            {
                // CPUresp = newBlock.data_block[newRequest.offset];
                newBlock.valid_bit = 1;
                newBlock.dirty_bit = 0;
                set[insertion_ind] = newBlock;
                reads++;
            }
            else // write
            {
                // newBlock.data_block[newRequest.offset] = newRequest.data;
                newBlock.valid_bit = 1;
                newBlock.dirty_bit = 1;
                set[insertion_ind] = newBlock;
                writes++;
            }
        }
    }

    // Printing results
    cout << "Cache Size : " << cache_size << " KB\n";
    cout << "Associativity : " << ways << "\n";
    cout << "Block Size : " << block_size << " bytes\n";
    cout << "Replacement policy : " << rep_policy << "\n";
    cout << "Write policy : " << write_policy << "\n";
    cout << "Trace Name : " << traceFile << "\n";
    cout << "No of instructions : " << instr_count << "\n";
    cout << "No of memory accesses : " << reads + writes << "\n";
    cout << "No of loads : " << reads << "\n";
    cout << "No of stores : " << writes << "\n";
    cout << "Execution time : " << cycles << " cycles\n";
    cout << "No of cache hits : " << hits << "\n";
    cout << "No of cache misses : " << misses << "\n";
    cout << "No of dirty evictions : " << dirty_evict << "\n";
    cout << "Miss penalty (memory access + operation) : " << miss_pen << " cycles\n";
    cout << "------------------------------------------------\n";
    double hit_rate = (double)hits / (hits + misses);
    cout << "Cache Hit Rate = " << hit_rate << "\n";
    double cpi = (double)cycles / instr_count;
    cout << "Memory CPI =  " << cpi - 1 << "\n";
    cout << "Total CPI = " << cpi << "\n";
    double amat = ((double)dirty_evict / (hits + misses)) * 2 + (1 - hit_rate) * miss_pen + 1;
    cout << "AMAT = " << amat << " cycles\n";
    return 0;
}