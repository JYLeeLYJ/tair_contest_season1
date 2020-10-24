#ifndef KVFILE_INCLUDE_H
#define KVFILE_INCLUDE_H

#include <array>
#include <utility>
#include <vector>
#include <cstdio>

#include "db.hpp"
#include "utils.hpp"

struct head_info{
    char key[KEY_SIZE];
    uint32_t index_block_id;
    uint32_t value_len;
};

struct block_index
: std::array<uint32_t , 4>{};

struct value_block
: std::array<char , 128>{};

struct meta_info 
: std::array<char , 1_KB>{};

template<
    std::size_t n_key_head, 
    std::size_t n_index_block,
    std::size_t n_value_block>
class kv_file_info{
    void * pbase;
public :
    head_info * key_heads;
    block_index * block_indices;
    value_block * value_blocks;
    meta_info * meta;

public:

    kv_file_info() = default;

    explicit kv_file_info(void * base , size_t sz) noexcept
    : pbase(base) {
        constexpr auto key_sz = sizeof(head_info) * n_key_head  , value_sz = sizeof(value_block) * n_value_block;
        constexpr auto index_sz = sizeof(block_index) * n_index_block;

        base = (char *)base + 1_KB;
        sz-= 1_KB;
        key_heads = reinterpret_cast<head_info*>(base) ; //reinterpret_cast<head_info *>(align(sizeof(head_info) , key_sz , base , sz));
        base = (char *)base + key_sz , sz -= key_sz;
        block_indices = reinterpret_cast<block_index *>(align(sizeof(block_index) , index_sz , base , sz));
        base = (char *)base + index_sz , sz -= index_sz;
        value_blocks = reinterpret_cast<value_block *>(align(256, value_sz, base,sz));
        if(!key_heads || !value_sz || !block_indices) perror("align failed.") , exit(0);
    }

    void * base() const{
        return pbase;
    }
};

static_assert(sizeof(head_info) == 24  && sizeof(value_block) == 128 && sizeof(block_index) == 16, "");

#endif