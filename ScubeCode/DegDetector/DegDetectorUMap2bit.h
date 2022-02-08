#ifndef _DegDetectorUMap2bit_H
#define _DegDetectorUMap2bit_H
#include <iostream>
#include <stdlib.h>
#include <malloc.h>
#include <map>
#include <set>
#include <random>
#include <memory.h>
#include <unordered_map>
#include "DegDetector.h"

// hash table
// just maintains reserved_bits for each key
using namespace std;

typedef uint32_t value_type;            // 存储bit vec、update times、addr的字段

class DegDetectorUMap2bit: public DegDetector
{
private:
    const int BITVECBITS = 2;      // value_type中高BITVECBITS位作为bitvector
    const int UPDATEBITS = 8;      // value_type中UPDATEBITS位作为update_times
    const int ADDRBITS = 8;        // value_type中低ADDRBITS位作为addr
    
    hash_type hash_mask;
    value_type addr_mask;
    value_type update_mask;
    value_type bitvec_mask;

    int ignored_bits;
    int reserved_bits;
    double alpha = 0.8;             // 根据alpha * width算备选地址个数
    double exp_deg;

    unordered_map<key_type, value_type> out_bit_vec;
    unordered_map<key_type, value_type> in_bit_vec;
public:
    DegDetectorUMap2bit(uint32_t matrix_width, uint32_t matrix_depth, double exp_deg, int ignored_bits, int reserved_bits = 2, double alpha = 0.8, int update_bits = 8, int addr_bits = 6);
    ~DegDetectorUMap2bit();
    bool insert(key_type s, key_type d, hash_type hash_s, hash_type hash_d);
    uint32_t degQuery(key_type n, int type);
    addr_type addrQuery(key_type n, int type);   // type = 0 for out_addr, 1 for in_addr
    bool extendAddr(key_type key, addr_type n, int type);
    void printUsage();
    
private:
    bool insertSlot(key_type key, int update_bit, int type);
};

DegDetectorUMap2bit::DegDetectorUMap2bit(uint32_t matrix_width, uint32_t matrix_depth, double exp_deg, int ignored_bits, int reserved_bits, double alpha, int update_bits, int addr_bits): 
DegDetector(matrix_width, matrix_depth), exp_deg(exp_deg), ignored_bits(ignored_bits), reserved_bits(reserved_bits), BITVECBITS(reserved_bits), UPDATEBITS(update_bits), ADDRBITS(addr_bits)
{
#if defined(DEBUG)
    cout << "DegDetectorUMap2bit::DegDetectorUMap2bit(matrix_width: " << matrix_width << ", matrix_depth: " << matrix_depth << ", exp_deg: " << exp_deg 
         << ", ignored_bits: " << ignored_bits << ", reserved_bits: " << reserved_bits << ", alpha: " << alpha  << ", BITVECBITS: " << BITVECBITS  
         << ", UPDATEBITS: " << UPDATEBITS << ", ADDRBITS: " << ADDRBITS << ")" << endl;
#endif
    this->hash_mask = ((1 << ignored_bits) - 1);
    this->addr_mask = ((1 << ADDRBITS) - 1);
    this->update_mask = (((1 << UPDATEBITS) - 1) << ADDRBITS);
    this->bitvec_mask = (((1 << BITVECBITS) - 1) << (ADDRBITS + UPDATEBITS));
}

DegDetectorUMap2bit::~DegDetectorUMap2bit()
{
#if defined(DEBUG)
    cout << "DegDetectorUMap2bit::~DegDetectorUMap2bit()" << endl;
#endif
}

// update slot
bool DegDetectorUMap2bit::insertSlot(key_type key, int update_bit, int type)
{
    unordered_map<key_type, value_type>* bit_vec = (type == 0) ? &out_bit_vec : &in_bit_vec;
    uint32_t len = (type == 0) ? matrix_width : matrix_depth;
    double theta = len * ROOM * alpha;

    // look up if the node is existing
    unordered_map<key_type, value_type>::iterator iter = bit_vec->find(key);
    if (iter != bit_vec->end())
    {
        // update bit
        (iter->second) |= (1 << (ADDRBITS + UPDATEBITS + update_bit));
        // check all bit
        bool update = true;
        for (int j = 0; j < reserved_bits; j++) 
        {
            if (((iter->second) & (1 << (ADDRBITS + UPDATEBITS + j))) == 0) 
            {
                update = false;
                break;
            }
        }
        if (update)
        {
            // clear the bit vector and add update_times
            (iter->second) &= (~bitvec_mask);
            (iter->second) += (value_type)(1 << ADDRBITS);
            // update addrs
            int upd_times = (((iter->second) & update_mask) >> ADDRBITS);
            if (upd_times == 0)
            {
                cout << "upd_times is out of range! Considering add more bits to UPDATEBITS." << endl;
            }
            uint32_t addrs =  ((iter->second) & addr_mask);
            if (addrs < 2) 
            {
                cout << "error! DegDetectorUMap2bit-line 340" << endl;
            }
            addr_type ad = (addr_type) max (ceil((double)((double)(upd_times * exp_deg) / theta)), (double)addrs);
            if (ad > addr_mask) 
            {
                cout << "ad is out of range! Considering add more bits to ADDRBITS." << endl;
            }
            if (ad > addrs)
            {
                // update addr
                (iter->second) &= (~addr_mask);
                (iter->second) |= (value_type)ad;
            }
        }
    }
    else 
    {
        value_type val = ((1 << (ADDRBITS + UPDATEBITS + update_bit)) | 2);     // 初始(potential high degree)地址设置为2
        bit_vec->insert(std::make_pair(key, val));
    }
    return true;
}

// insert elements to the degree detector
// 1. 对于hash值中l1p大于等于ignored_bits，插入到slot里面去，否则忽略；
// 2. 对potential high degree进行衰退；
// 3. 在查询的时候判断是否是高度点，判断方法是bit vector中是否存在两个连续的1
bool DegDetectorUMap2bit::insert(key_type s, key_type d, hash_type hash_s, hash_type hash_d)
{
    bool res = true;
    if ((hash_s & hash_mask) == 0) 
    {
        int update_bit = -1;
        for (int i = 0; i < reserved_bits; i++) 
        {
            if (((1 << (ignored_bits + i)) & hash_s) != 0)
            {
                update_bit = i;
                break;
            }
        }
        if (update_bit != -1)
        {
            res = res && insertSlot(d, update_bit, 1);
        }
        // int update_bit = -1;
        // for (int i = 0; i < reserved_bits - 1; i++) 
        // {
        //     if (((1 << (ignored_bits + i)) & hash_s) != 0)
        //     {
        //         update_bit = i;
        //         break;
        //     }
        // }
        // if (update_bit == -1) 
        // {
        //     if ((hash_s >> (ignored_bits + reserved_bits - 1)) != 0) 
        //     {
        //         update_bit = reserved_bits - 1;
        //     }
        // }
        // if (update_bit != -1)
        // {
        //     res = res && insertSlot(d, update_bit, 1);
        // }
    }

    if ((hash_d & hash_mask) == 0) 
    {
        int update_bit = -1;
        for (int i = 0; i < reserved_bits; i++) 
        {
            if (((1 << (ignored_bits + i)) & hash_d) != 0)
            {
                update_bit = i;
                break;
            }
        }
        if (update_bit != -1)
        {
            res = res && insertSlot(s, update_bit, 0);
        }
        // int update_bit = -1;
        // for (int i = 0; i < reserved_bits - 1; i++) 
        // {
        //     if (((1 << (ignored_bits + i)) & hash_d) != 0)
        //     {
        //         update_bit = i;
        //         break;
        //     }
        // }
        // if (update_bit == -1) 
        // {
        //     if ((hash_d >> (ignored_bits + reserved_bits - 1)) != 0) 
        //     {
        //         update_bit = reserved_bits - 1;
        //     }
        // }
        // if (update_bit != -1)
        // {
        //     res = res && insertSlot(s, update_bit, 0);
        // }
    }

    return res;
}

// return the estimate degree of the node
uint32_t DegDetectorUMap2bit::degQuery(key_type key, int type) 
{
    unordered_map<key_type, value_type>* bit_vec = (type == 0) ? &out_bit_vec : &in_bit_vec;

    // look up if the node is existing
    unordered_map<key_type, value_type>::iterator iter = bit_vec->find(key);
    if (iter != bit_vec->end())
    {
        uint32_t res = (uint32_t)((((iter->second) & update_mask) >> ADDRBITS) * exp_deg);
        return res;
    }
    return 0;
}

addr_type DegDetectorUMap2bit::addrQuery(key_type key, int type) 
{
    // 初始给所有的点都设为2轮，0，1
    // addr_type res_ad = 2;
    unordered_map<key_type, value_type>* bit_vec = (type == 0) ? &out_bit_vec : &in_bit_vec;

    // look up if the node is existing
    unordered_map<key_type, value_type>::iterator iter = bit_vec->find(key);
    if (iter != bit_vec->end())
    {
        return (addr_type)((iter->second) & addr_mask);
    }
    return 2;
}

bool DegDetectorUMap2bit::extendAddr(key_type key, addr_type n, int type) 
{
    // cout << "key = " << key << ", n = " << n << endl;
    bool res = false;
    if (n < 2)
    {
        cout << "DegDetectorUMap2bit::extendAddr error!" << endl;
        return false;
    }
    if (n > addr_mask) 
    {
        cout << "extendAddr(n = " << n << ") is out of range! Considering add more bits to ADDRBITS." << endl;
    }
    // 首先查找slots中是否有key
    unordered_map<key_type, value_type>* bit_vec = (type == 0) ? &out_bit_vec : &in_bit_vec;

    // look up if the node is existing
    unordered_map<key_type, value_type>::iterator iter = bit_vec->find(key);
    if (iter != bit_vec->end())
    {
        // extend addrs to n
        (iter->second) &= (~addr_mask);
        (iter->second) |= (value_type)n;
        return true;
    }
    else 
    {
        bit_vec->insert(std::make_pair(key, (value_type)n));        // 地址设置为n
    }
    return 2;
}

void DegDetectorUMap2bit::printUsage() 
{
    map<addr_type, uint32_t> out_ad, in_ad;
    for (unordered_map<key_type, value_type>::iterator iter = out_bit_vec.begin(); iter != out_bit_vec.end(); iter++)
    {
        addr_type ad = ((iter->second) & addr_mask);
        map<addr_type, uint32_t>::iterator iter1 = out_ad.find(ad);
        if (iter1 != out_ad.end())
        {
            out_ad[ad]++;
        }
        else
        {
            out_ad[ad] = 1;
        }
    }
    for (unordered_map<key_type, value_type>::iterator iter = in_bit_vec.begin(); iter != in_bit_vec.end(); iter++)
    {
        addr_type ad = ((iter->second) & addr_mask);
        map<addr_type, uint32_t>::iterator iter1 = in_ad.find(ad);
        if (iter1 != in_ad.end())
        {
            in_ad[ad]++;
        }
        else
        {
            in_ad[ad] = 1;
        }
    }
    cout << "out addrs" << endl;
    for (map<addr_type, uint32_t>::iterator iter = out_ad.begin(); iter != out_ad.end(); iter++) {
        cout << "addr-" << iter->first << ": " << iter->second << endl;
    }
    cout << endl << "in addrs" << endl;
    for (map<addr_type, uint32_t>::iterator iter = in_ad.begin(); iter != in_ad.end(); iter++) {
        cout << "addr-" << iter->first << ": " << iter->second << endl;
    }
}

#endif // _DegDetectorUMap2bit_H
