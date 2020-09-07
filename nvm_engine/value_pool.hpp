#ifndef VALUE_POOL_INCLUDE_H
#define VALUE_POOL_INCLUDE_H

#include <sys/mman.h>
#include <cstdio>
#include <cstdlib>
#include <utility>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "include/utils.hpp"
#include "include/logger.hpp"

template<size_t N >
class memory_pool:disable_copy{
public:
    explicit memory_pool(const std::string file){

        Logger::instance().sync_log("file = "+file);
        
        //fopen
        _fp = fopen(file.data() , "wb");
        if(_fp == NULL){
            Logger::instance().sync_log("failed to fopen");
            perror("failed to fopen");
            exit(1);
        }

        //open
        _fd = open(file.data(), O_RDWR);
        if (_fd < 0) {
            Logger::instance().sync_log("failed to open the file");
            perror("failed to open the file");
            exit(1);
        }
        
        //mmap
        _base = (char*)mmap(NULL, N, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);

        if(_base == MAP_FAILED || _base == NULL) {
            Logger::instance().sync_log("failed to mmap.");
            perror("mmap failed");
            exit(1);
        }

        Logger::instance().sync_log("mmap base addr = " + std::to_string(uint64_t(_base)));
        Logger::instance().sync_log("_fp = " + std::to_string(uint64_t(_fp)));
    }

    ~memory_pool() noexcept{
        if(!_base)munmap(_base , N);
        if(!(_fd < 0)) close(_fd);
        if(!_fp) fclose(_fp);
    }

    memory_pool(memory_pool && mem) noexcept
    :_base(mem._base){
        mem._base = nullptr;
    }

    template<int n>
    int foo(){
        return N;
    }

    void append_memory(size_t sz){
        static char data[1024]{};

        if(_endoff.fetch_add(sz) + sz > N){
            Logger::instance().sync_log("Out of presist memory N = " + std::to_string(N) + " , _endoff = " + std::to_string(_endoff) );
            perror("Out of presist memory");
            exit(1);
        }
        fwrite(data , sizeof(char) , sz , _fp);
        fflush(_fp);
    }
    
    memory_pool & operator= (memory_pool && mem) noexcept{
        _base = mem._base; 
        mem._base = nullptr;
    }

    constexpr size_t length() const{
        return N;
    }

    void * base() const{
        return _base;
    }

protected:
    void *  _base{nullptr};
    std::atomic_uint32_t  _endoff{0};
    int     _fd{-1};
    FILE *  _fp{nullptr};
};

struct Value{

    static constexpr size_t value_size = 80;

    char data[value_size]{};
    
    std::string to_string() const {
        return std::string(&data[0] , value_size);
    }

    Value & operator = (const Slice & val) noexcept{
        memset(data , 0 , value_size);
        memcpy(data , val.data() , std::min(val.size(), value_size));
        return * this;
    }
};

template<size_t N , bool pre_allocate = false>
class value_pool:disable_copy{
public:
    static constexpr size_t VALUE_SIZE = Value::value_size; 
    static constexpr size_t KEY_SIZE = 16;
    static constexpr size_t MEM_SIZE = VALUE_SIZE * N;
public:
    explicit value_pool(const std::string & file ) 
    :pmem(file){
        if (pre_allocate)
            pre_allocated_trunks();
    }

    //out of range will lead to undefined behavior
    Value & operator[](size_t i) const noexcept{
        return value(i);        
    }

    Value & value(size_t i) const noexcept{
        return data()[i];
    }

    bool set_value(size_t i , const Slice & str) noexcept{
        data()[i] = str;
        return true;
    }

    size_t append_new_value(const Slice & str ) {
        if(!pre_allocate)
            pmem.append_memory(VALUE_SIZE);
        
        auto index = seq ++;
        set_value(index , str);
        return index;
    }

private:

    void pre_allocated_trunks(){
        constexpr int n_trunks = MEM_SIZE / 1024 ;
        for ( int i = 0 ; i < n_trunks ; ++ i){
            pmem.append_memory(1024);
        }
    }

    Value * data() const noexcept{
        return static_cast<Value*>(pmem.base());
    }

private:
    memory_pool< MEM_SIZE > pmem;
    std::atomic<size_t> seq{0};

};

#endif