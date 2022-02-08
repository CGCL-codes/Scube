#ifndef _DegDetectorNewSlot_H
#define _DegDetectorNewSlot_H
#include <iostream>
#include <stdlib.h>
#include <malloc.h>
#include <map>
#include <set>
#include <random>
#include <memory.h>
#include "DegDetector.h"

// ignore the low-n bits of the hash value of each nodes

// #define EXPDEG(n, k)  ((1 / phi) * ((1 << ((n) + (k)))))
#define EXPDEG(n, k)  ((1 / phi) * (((1 / 3) * ((1 << (n - 1)) - (1 / (1 << (n - 1))))) + (1 << (n + k - 1)) + (1 / (1 << (n)))))        // new formula
// 1/3*(2^(n-1) - 1/2^(n-1))+2^(n+k-1)+2^(-n)
// #define EXPDEG(n, k)  ((1 / phi) * ((1 << (n)) + (1 << (k)) - (n) - (1 / (1 << (n)))))

using namespace std;
// #define para 1.08

// hash value = xxxx1(k 0s) --> potential high degree nodes
// && hash value = xxx10(k 0s) --> high degree nodes, store it in the slot

// with exponential-decay probability = 1/2    // para ^ -(1 << (l0p - 1) / phi)
// slot + linkedlist

class DegDetectorNewSlot: public DegDetector
{
private:
    struct slot
    {
        key_type slot_key;          // 最低位作为指针标记
        slot_value_type value;      // 高24bit作为bitvector, 低8位作为addr
    };
    const uint32_t slot_num;
    const double phi = 0.77351;
    const uint16_t k_width = 0;               // k阈值，l0p的位置
    const uint16_t k_depth = 0;               // k阈值，l0p的位置
    const double n_width = 0;
    const double n_depth = 0;
    slot* out_slots = NULL;
    slot* in_slots = NULL;
    int ignore_bits;
    double alpha = 0.8;             // 根据alpha*width算备选地址个数

    set<uint32_t> extend_s_index;    // 存储有扩展链slot的地址index
    set<uint32_t> extend_d_index;    // 存储有扩展链slot的地址index

public:
    DegDetectorNewSlot(uint32_t matrix_width, uint32_t matrix_depth, uint32_t k_width, uint32_t k_depth, uint32_t slotNum, int ignore_bits, double alpha = 0.8);
    ~DegDetectorNewSlot();
    bool insert(key_type s, key_type d, hash_type hash_s, hash_type hash_d);
    // uint32_t degQuery(key_type n, int type);
    addr_type addrQuery(key_type n, int type);   // type = 0 for out_addr, 1 for in_addr
    bool extendAddr(key_type key, addr_type n, int type);
    void printUsage();
    
private:
    slot* recoverPointer(slot s);
    bool addSlot(uint32_t addr, int type, key_type key, slot_value_type slot_value);
    bool decayStrategy(uint32_t addr, int type, key_type key, hash_type hashValue);
    bool insertSlot(key_type key, hash_type hashValue, int type);

    int left0Pos(hash_type hashValue);
    int left1Pos(hash_type hashValue);
    int right1Pos(hash_type hashValue);
    int hammingWeight(hash_type hashValue);
};

DegDetectorNewSlot::DegDetectorNewSlot(uint32_t matrix_width, uint32_t matrix_depth, uint32_t k_width, uint32_t k_depth, uint32_t slotNum, int ignore_bits, double alpha): 
DegDetector(matrix_width, matrix_depth), k_width(k_width), k_depth(k_depth), n_width((1 << k_width) / phi), n_depth((1 << k_depth) / phi), slot_num(slotNum), ignore_bits(ignore_bits)
{
#if defined(DEBUG)
    cout << "DegDetectorNewSlot-new-formula::DegDetectorNewSlot(matrix_width: " << matrix_width << ", matrix_depth: " << matrix_depth 
         << ", k_width: " << k_width << ", k_depth: " << k_depth << ", slot_num: " << slotNum << ", ignore_bits: " << ignore_bits 
         << ", alpha: " << alpha << ")" << endl;
    cout << "k_width = " << k_width << ", n_width = " << n_width << endl;
    cout << "k_depth = " << k_depth << ", n_depth = " << n_depth << endl;
#endif
    // this->out_slots = new slot[slotNum * SLOTROOM];
    // this->in_slots = new slot[slotNum * SLOTROOM];
    // posix_memalign((void**)&(this->out_slots), 64, sizeof(slot) * slotNum * SLOTROOM);		// 64-byte alignment of the requested space
    // posix_memalign((void**)&(this->in_slots), 64, sizeof(slot) * slotNum * SLOTROOM);		// 64-byte alignment of the requested space
    // this->out_slots = (slot *) aligned_alloc(64, sizeof(slot) * slotNum * SLOTROOM);		    // 64-byte alignment of the requested space
    // this->in_slots = (slot *) aligned_alloc(64, sizeof(slot) * slotNum * SLOTROOM);		    // 64-byte alignment of the requested space
    this->out_slots = (slot *) memalign(64, sizeof(slot) * slotNum * SLOTROOM);		            // 64-byte alignment of the requested space
    this->in_slots = (slot *) memalign(64, sizeof(slot) * slotNum * SLOTROOM);		            // 64-byte alignment of the requested space
    memset(this->out_slots, 0, sizeof(slot) * slotNum * SLOTROOM);
    memset(this->in_slots, 0, sizeof(slot) * slotNum * SLOTROOM);
}

DegDetectorNewSlot::~DegDetectorNewSlot()
{
#if defined(DEBUG)
    cout << "DegDetectorNewSlot::~DegDetectorNewSlot()" << endl;
#endif
    for (set<uint32_t>::iterator iter = extend_s_index.begin(); iter != extend_s_index.end(); iter++) {
        uint32_t index = *iter;
        slot* head = recoverPointer(out_slots[index * SLOTROOM + SLOTROOM - 1]);
        cout << "index = " << index << ", head = ";
        slot* ptr = head;
        while (ptr != NULL) 
        {
            cout << ptr;
            if ((ptr[SLOTROOM - 2].slot_key & 1) == 1) 
            {
                slot* ptr1 = recoverPointer(ptr[SLOTROOM - 1]);
                delete[] ptr;
                ptr = ptr1;
                cout << " -> ";
            }
            else 
            {
                delete[] ptr;
                ptr = NULL;
                cout << endl;
            }
        }
    }
    for (set<uint32_t>::iterator iter = extend_d_index.begin(); iter != extend_d_index.end(); iter++)
    {
        uint32_t index = *iter;
        
        slot* head = recoverPointer(in_slots[index * SLOTROOM + SLOTROOM - 1]);
        cout << "index = " << index << ", head = ";
        slot* ptr = head;
        while (ptr != NULL)
        {
            cout << ptr;
            if ((ptr[SLOTROOM - 2].slot_key & 1) == 1)
            {
                slot* ptr1 = recoverPointer(ptr[SLOTROOM - 1]);
                delete[] ptr;
                ptr = ptr1;
                cout << " -> ";
            }
            else
            {
                delete[] ptr;
                ptr = NULL;
                cout << endl;
            }
        }
    }
    // delete[] this->out_slots;
    // delete[] this->in_slots;
    free(this->out_slots);
    free(this->in_slots);
}

DegDetectorNewSlot::slot* DegDetectorNewSlot::recoverPointer(slot s) 
{
    return (slot *) ((((uint64_t)s.slot_key) << 32) | s.value);
}

// "链表"尾部扩展, addr为数组index
// 1. new一个slot, 并清空
// 2. 找到链表尾部的slot
// 3. 将尾部的slot第SLOTNUM-2个ROOM的key的最低位设置为1
// 4. 将尾部的slot的第SLOTNUM-1个ROOM的值复制到new slot的第1个ROOM中
// 5. 将尾部的slot的第SLOTNUM-1个ROOM设置为new slot的指针
// 6. 将待存储的点存储到new slot的第二个ROOM中
bool DegDetectorNewSlot::addSlot(uint32_t addr, int type, key_type key, slot_value_type slot_value)
{
    if (type == 0)
        this->extend_s_index.insert(addr);
    else
        this->extend_d_index.insert(addr);
    
    slot* slots = (type == 0) ? out_slots : in_slots;
    slot* new_slot = new slot[SLOTROOM];
    memset(new_slot, 0, sizeof(slot) * SLOTROOM);
    // cout << "addr = " << addr << ", new_slot = " << new_slot << ", type = " << type << endl;
    
    if (new_slot == NULL)
        return false;
    
    if ((slots[addr * SLOTROOM + SLOTROOM - 2].slot_key & 1) == 1)      // 不是第一次扩展
    { 
        slot* rec = recoverPointer(slots[addr * SLOTROOM + SLOTROOM - 1]);
        while (true) 
        {
            if ((rec[SLOTROOM - 2].slot_key & 1) == 1)
                rec = recoverPointer(rec[SLOTROOM - 1]);
            else 
                break;
        }
        rec[SLOTROOM - 2].slot_key |= 1;
        new_slot[0].slot_key = rec[SLOTROOM - 1].slot_key;
        new_slot[0].value = rec[SLOTROOM - 1].value;
        new_slot[1].slot_key = (key << 1);
        new_slot[1].value = slot_value;
        // 将数组中的第SLOTNUM - 1个单元的两个字段设置为指针
        rec[SLOTROOM - 1].slot_key = ((uint64_t) new_slot >> 32);       
        rec[SLOTROOM - 1].value = ((uint64_t) new_slot & 0xffffffff);
    }
    else   // 第一次扩展
    {    
        slots[addr * SLOTROOM + SLOTROOM - 2].slot_key |= 1;
        new_slot[0].slot_key = slots[addr * SLOTROOM + SLOTROOM - 1].slot_key;
        new_slot[0].value = slots[addr * SLOTROOM + SLOTROOM - 1].value;
        new_slot[1].slot_key = (key << 1);
        new_slot[1].value = slot_value;
        // 将数组中的第SLOTNUM - 1个单元的两个字段设置为指针
        slots[addr * SLOTROOM + SLOTROOM - 1].slot_key = ((uint64_t) new_slot >> 32);       
        slots[addr * SLOTROOM + SLOTROOM - 1].value = ((uint64_t) new_slot & 0xffffffff);
    }
    return true;
}

// 只对potential high degree进行衰退（衰退说明该key不存在与对应slot(链)中，因为在插入的时候已经遍历过一次slot(链)，另外执行扩展说明前面的节点都是高度点，此时只需对尾slot进行衰退）
// 有链的话直接拿到尾slot，对尾slot中的room进行衰退
// 如果尾slot都被高度点占满，此时扩展节点
bool DegDetectorNewSlot::decayStrategy(uint32_t addr, int type, key_type key, hash_type hashValue)
{
    slot* slots = (type == 0) ? out_slots : in_slots;
    slot* decay_slot = NULL;
    if ((slots[addr * SLOTROOM + SLOTROOM - 2].slot_key & 1) == 1)      // slot有链，直接拿到尾slot
    {
        slot* rec = recoverPointer(slots[addr * SLOTROOM + SLOTROOM - 1]);
        while (true) 
        {
            if ((rec[SLOTROOM - 2].slot_key & 1) == 1)
                rec = recoverPointer(rec[SLOTROOM - 1]);
            else 
                break;
        }
        decay_slot = rec;
    }
    else        // slot无链
    {
        decay_slot = &(slots[addr * SLOTROOM]);
    }
    
    // we kick out the minimum element in the slot according to the hash value by probability
    // decay_slot中全部为高度点时，直接进行扩展
    int l0p = INT32_MAX;
    int index = -1;
    for (int i = 0; i < SLOTROOM; i++) 
    {
        int l = left0Pos(decay_slot[i].value >> 8);
        if (((decay_slot[i].value & 0xff) == 2) && (l < 2) && (l0p > l))
        {
            l0p = l;
            index = i;
        }
    }
    if (index == -1)    // 所有ROOM里面的点都是高度点，直接进行扩展
    {
        if (!addSlot(addr, type, key, hashValue))
        {
            cout << "addSlot() error!" << endl;
            return false;
        }
    }
    else
    {
        if (l0p == 1)    // 只对potential high degree点进行衰退，衰退概率为0.5，因为插入的点也是potential high degree，地位等同
        {
            // if ((decay_slot[index].value & 0xff) == 2) {   
            // double num = (double)(1 << (l0p + delta_k - 1)) / phi;
            // if (!(rand() % int(pow(para, num))))
            srand((unsigned)time(NULL));
            if (rand() % 2)
            {
                // kick-out
                decay_slot[index].slot_key = (key << 1);
                decay_slot[index].value = ((hashValue << 8) | 2);   // 初始地址设置为2

                // // change the right most 1 of hash_value to 0
                // hash_type mask = 1 << (l0p - 1);
                // decay_slot[index].value ^= (mask << 8);
                // // if the hash_value reduces to 0, kick out
                // // if (decay_slot[index].value[index] == 0) {
                // if (((decay_slot[index].value >> 8) & 1) == 0)
                // {
                //     decay_slot[index].slot_key = (key << 1);
                //     decay_slot[index].value = ((hashValue << 8) | 2);   // 初始地址设置为2
                // }
            }
        }
        else if (l0p == 0)   // 直接替换
        {
            decay_slot[index].slot_key = (key << 1);
            decay_slot[index].value = ((hashValue << 8) | 2);   // 初始地址设置为2
        }
    }
    return true;
}

// update slot
// 对于高度点不进行衰退，如果某个slot都被高度点占满，
// 则判断SLOTNUM - 2中key的最高位bit是不是1，是1则说明SLOTNUM - 1中存储的是指针（一个指针64bit）
bool DegDetectorNewSlot::insertSlot(key_type key, hash_type hashValue, int type)
{
    slot* slots = (type == 0) ? out_slots : in_slots;
    uint32_t idx = key % slot_num;
    uint32_t len = (type == 0) ? matrix_width : matrix_depth;
    double theta = len * ROOM * 0.8;

    // look up if the node is existing or the slot is empty
    slot* ptr = &slots[idx * SLOTROOM];
    bool flag = true;
    while (flag)
    {
        for (int i = 0; i < SLOTROOM; i++) 
        {
            if (i == SLOTROOM - 1)
            {
                if ((ptr[SLOTROOM - 2].slot_key & 1) == 1)              // 说明最后一个ROOM存储的是指针
                {
                    // 恢复指针
                    ptr = recoverPointer(ptr[SLOTROOM - 1]);
                    break;
                }
                else
                {
                    flag = false;
                }
            }
            // if the fingerprint is the same
            if ((ptr[i].slot_key >> 1) == key) 
            {
                // "or" operation
                ptr[i].value |= (hashValue << 8);
                int k = left0Pos((ptr[i].value >> 8));
                
                // 两个连续“1”才是高度点
                if (k > 1) 
                {
                    // addr_type ad = (addr_type) max (ceil((double)((1 << (k + ignore_bits)) / phi) / theta), 2.0);
                    // addr_type ad = (addr_type) max (ceil((double)(((1 << ignore_bits) + (1 << k) - ignore_bits - 1.0 / (1 << ignore_bits)) / phi) / theta), 2.0);
                    addr_type ad = (addr_type) max (ceil((double)(EXPDEG(ignore_bits, k) / theta)), 2.0);
                    if ((ptr[i].value & 0xff) < ad) 
                    {
                        ptr[i].value &= 0xffffff00;
                        ptr[i].value |= ad;
                    }
                }
                return true;
            }
            // if the slot is empty
            else if ((ptr[i].slot_key == 0) && (ptr[i].value == 0)) 
            {
                ptr[i].slot_key = (key << 1);
                ptr[i].value = ((hashValue << 8) | 2);   // 初始(potential high degree)地址设置为2
                return true;
            }
        }
    }
    

    // 前面的slot中已经满了，而且没有fingerprint匹配的，因此进行decay
    // we kick out the minimum element in the slot according to the hash value by probability
    return decayStrategy(idx, type, key, hashValue);
}

// insert elements to the degree detector
// 1. 对于hash值中l1p大于等于ignore_bits，插入到slot里面去，否则忽略；
// 2. 对potential high degree进行衰退；
// 3. 在查询的时候判断是否是高度点，判断方法是bit vector中是否存在两个连续的1
bool DegDetectorNewSlot::insert(key_type s, key_type d, hash_type hash_s, hash_type hash_d)
{
    bool res = true;
    int sLeft1Pos = left1Pos(hash_s);
    if (sLeft1Pos >= ignore_bits) 
    {
        hash_type sHashValue = 1 << (sLeft1Pos - ignore_bits);
        res = res && insertSlot(d, sHashValue, 1);
    }

    int dLeft1Pos = left1Pos(hash_d);
    if (dLeft1Pos >= ignore_bits) 
    {
        hash_type dHashValue = 1 << (dLeft1Pos - ignore_bits);
        res = res && insertSlot(s, dHashValue, 0);
    }

    return res;
}

// return the estimate degree of the node
// uint32_t DegDetectorNewSlot::degQuery(key_type key, int type) 
// {
//     uint16_t delta_k = (type == 0) ? k_width : k_depth;
//     addr_type addrs = this->addrQuery(key, type);
//     if (addrs > 2) 
//         return (1 << addrs) / phi;
//     uint32_t res = 0;
//     uint32_t slot_index = key % slot_num;
//     slot* slots = (type == 0) ? (this->out_slots) : (this->in_slots);
//     for (int i = 0; i < SLOTROOM; i++) 
//     {
//         if ((i == SLOTROOM - 1) && ((slots[slot_index * SLOTROOM + SLOTROOM - 2].slot_key & 1) == 1))    // 说明最后一个ROOM存储的是指针
//         {  
//             // 恢复指针
//             slot* ptr = recoverPointer(slots[slot_index * SLOTROOM + SLOTROOM - 1]);
//             slot* ptr1 = ptr;
//             while (true) 
//             {
//                 for (int j = 0; j < SLOTROOM; j++) 
//                 {
//                     if ((j == SLOTROOM - 1) && (ptr[SLOTROOM - 2].slot_key & 1) == 1) 
//                     {
//                         ptr1 = recoverPointer(ptr[SLOTROOM - 1]);
//                     }
//                     else 
//                     {
//                         // if the fingerprint is the same
//                         if ((ptr[j].slot_key >> 1) == key) 
//                         {
//                             // return the estimate value according to the hashValue
//                             hash_type hashValue = (ptr[j].value >> 8);
//                             int l0p = left0Pos(hashValue);
//                             res += ((1 << (l0p + delta_k)) / phi);
//                             return res;
//                         }
//                     }
//                 }
//                 if (ptr1 != ptr)
//                     ptr = ptr1;
//                 else
//                     break;
//             }
//         }
//         else 
//         {
//             if ((slots[slot_index * SLOTROOM + i].slot_key >> 1) == key) 
//             {
//                 // return the estimate value according to the hashValue
//                 hash_type hashValue = (slots[slot_index * SLOTROOM + i].value >> 8);
//                 int l0p = left0Pos(hashValue);
//                 res += ((1 << l0p) / phi);
//                 return res;
//             }
//         }
//     }
//     return 0;
// }

addr_type DegDetectorNewSlot::addrQuery(key_type key, int type) 
{
    // 初始给所有的点都设为2轮，0，1
    // addr_type res_ad = 2;
    slot* slots = (type == 0) ? out_slots : in_slots;
    uint32_t slot_index = key % slot_num;

    slot* ptr = &slots[slot_index * SLOTROOM];
    bool flag = true;
    while (flag)
    {
        for (int i = 0; i < SLOTROOM; i++)         // query the addr in slots
        {
            if (i == SLOTROOM - 1)
            {
                if ((ptr[SLOTROOM - 2].slot_key & 1) == 1)              // 说明最后一个ROOM存储的是指针
                {
                    // 恢复指针
                    ptr = recoverPointer(ptr[SLOTROOM - 1]);
                    break;
                }
                else
                {
                    flag = false;
                }
            }
            // if the fingerprint is the same
            if ((ptr[i].slot_key >> 1) == key) 
            {
                // get the addr info
                // res_ad = (ptr[i].value & 0xff);
                return (addr_type)(ptr[i].value & 0xff);
            }
        }
    }
    return 2;
}

bool DegDetectorNewSlot::extendAddr(key_type key, addr_type n, int type) 
{
    // cout << "key = " << key << ", n = " << n << endl;
    bool res = false;
    if (n < 2)
    {
        cout << "DegDetectorNewSlot::extendAddr error!" << endl;
        return false;
    }
    // 首先查找slots中是否有key
    slot* slots = (type == 0) ? out_slots : in_slots;
    uint32_t slot_index = key % slot_num;

    slot* ptr = &slots[slot_index * SLOTROOM];
    bool flag = true;
    while (flag)
    {
        for (int i = 0; i < SLOTROOM; i++)
        {
            if (i == SLOTROOM - 1)
            {
                if ((ptr[SLOTROOM - 2].slot_key & 1) == 1)              // 说明最后一个ROOM存储的是指针
                {
                    // 恢复指针
                    ptr = recoverPointer(ptr[SLOTROOM - 1]);
                    break;
                }
                else
                {
                    flag = false;
                }
            }
            // if the key is the same
            if ((ptr[i].slot_key >> 1) == key) 
            {
                // extend addrs to n
                ptr[i].value &= 0xffffff00;
                ptr[i].value |= n;
                return true;
            }
            // look up if there exists empty slot
            if ((ptr[i].slot_key == 0) && (ptr[i].value == 0)) 
            {
                ptr[i].slot_key = (key << 1);
                // ptr[i].value = ((dHashValue << 8) | n);   // 地址设置为n
                ptr[i].value = n;   // 地址设置为n
                return true;
            }
        }
    }

    // 没有空的room，扩展
    if (!addSlot(slot_index, type, key, n))
    {
        cout << "addSlot() error!" << endl;
        return false;
    }
    // else 
    // {
    //     cout << "extendAddr-addSlot(" << slot_index << ", " << type << ", " << key << ", " << n << ")" << endl;
    // }
    return true;
}

// return the position of the left most 0-bit in hashValue, ranks start at 0.
// return 32 while hashValue = 0xffffffff
int DegDetectorNewSlot::left0Pos(hash_type hashValue) 
{
    int i = 0;
    while (hashValue != 0) 
    {
        if ((hashValue & 1) == 0)
            return i;
        else 
        {
            i++;
            hashValue >>= 1;
        }
    }
    return i;
}

// return the position of the left most 1-bit in hashValue, ranks start at 0.
// return -1 while hashValue = 0
int DegDetectorNewSlot::left1Pos(hash_type hashValue) 
{
    int i = 0;
    while (hashValue != 0) 
    {
        if ((hashValue & 1) != 0)
            return i;
        else 
        {
            i++;
            hashValue >>= 1;
        }
    }
    return -1;
}

// return the position of the right most 1-bit in hashValue, ranks start at 0.
// return -1 while hashValue = 0
int DegDetectorNewSlot::right1Pos(hash_type hashValue) 
{
    int i = 31;
    while (i >= 0) 
    {
        hash_type mask = 1 << i;
        if ((hashValue & mask) != 0)
            return i;
        else
            i--;
    }
    return i;
}

// count the number of 1
int DegDetectorNewSlot::hammingWeight(hash_type hashValue) 
{
    int count = 0;
    while (hashValue != 0) 
    {
        count++;
        hashValue = hashValue & (hashValue - 1);
    }
    return count;
}

void DegDetectorNewSlot::printUsage() 
{
    map<addr_type, uint32_t> out_ad, in_ad;
    for(int idx = 0; idx < slot_num; idx++) {
        slot* ptr1 = &out_slots[idx * SLOTROOM];
        bool flag = true;
        while (flag)
        {
            for (int i = 0; i < SLOTROOM; i++)
            {
                if (i == SLOTROOM - 1)
                {
                    if ((ptr1[SLOTROOM - 2].slot_key & 1) == 1)
                    {
                        ptr1 = recoverPointer(ptr1[SLOTROOM - 1]);
                        break;
                    }
                    else
                        flag = false;
                }
                addr_type ad = (ptr1[i].value & 0xff);
                map<addr_type, uint32_t>::iterator iter = out_ad.find(ad);
                if (iter != out_ad.end())
                {
                    out_ad[ad]++;
                }
                else
                {
                    out_ad[ad] = 1;
                }
            }
        }

        slot* ptr2 = &in_slots[idx * SLOTROOM];
        flag = true;
        while (flag)
        {
            for (int i = 0; i < SLOTROOM; i++)
            {
                if (i == SLOTROOM - 1)
                {
                    if ((ptr2[SLOTROOM - 2].slot_key & 1) == 1)
                    {
                        ptr2 = recoverPointer(ptr2[SLOTROOM - 1]);
                        break;
                    }
                    else
                        flag = false;
                }
                addr_type ad = (ptr2[i].value & 0xff);
                map<addr_type, uint32_t>::iterator iter = in_ad.find(ad);
                if (iter != in_ad.end())
                {
                    in_ad[ad]++;
                }
                else
                {
                    in_ad[ad] = 1;
                }
            }
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

    // uint32_t in_count = 0, in_pointers = 0;
    // uint32_t out_count = 0, out_pointers = 0;;
    // for (int i = 0; i < slot_num; i++) 
    // {
    //     if (in_slots[i].slot_key != 0)
    //         in_count++;
    //     if (out_slots[i].slot_key != 0) 
    //         out_count++;
    // }

    // cout << "set<uint32_t> extend_s_index: " << endl;
    // for (set<uint32_t>::iterator iter = extend_s_index.begin(); iter != extend_s_index.end(); iter++)
    // {
    //     cout << *iter << ", ";
    // }
    // cout << endl;
    // cout << "set<uint32_t> extend_d_index: " << endl;
    // for (set<uint32_t>::iterator iter = extend_d_index.begin(); iter != extend_d_index.end(); iter++)
    // {
    //     cout << *iter << ", ";
    // }
    // cout << endl;

    // cout << "------------------DegDeteSlot----------------------" << endl;
    // cout << "in_count = " << in_count << ", in_slot size = " << slot_num
    //      << ", in_slot usage = " << (((double)in_count / (double)slot_num) * 100) << "%" << endl;
    // cout << "out_count = " << out_count << ", out_slot size = " << slot_num
    //      << ", out_slot usage = " << (((double)out_count / (double)slot_num) * 100) << "%" << endl;
    // cout << "----------------------------------------------------" << endl;
}

#endif // _DegDetectorNewSlot_H
