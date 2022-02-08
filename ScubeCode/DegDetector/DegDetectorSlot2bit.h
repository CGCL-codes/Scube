#ifndef _DegDetectorSlot2bit_H
#define _DegDetectorSlot2bit_H
#include <iostream>
#include <stdlib.h>
#include <malloc.h>
#include <map>
#include <set>
#include <random>
#include <memory.h>
#include "DegDetector.h"

// slot + linkedlist
// just maintains reserved_bits for each key
using namespace std;

class DegDetectorSlot2bit: public DegDetector
{
private:
    const int BITVECBITS = 2;      // slot.value中高BITVECBITS位作为bitvector
    const int UPDATEBITS = 8;      // slot.value中UPDATEBITS位作为update_times
    const int ADDRBITS = 8;        // slot.value中低ADDRBITS位作为addr
    struct slot
    {
        key_type slot_key;          // 最低位作为指针标记
        slot_value_type value;      // 高2bit作为bitvector, 6bit作为update_times, 低8位作为addr
    };
    slot_value_type addr_mask;
    slot_value_type update_mask;
    slot_value_type bitvec_mask;

    const uint32_t slot_num;
    slot* out_slots = NULL;
    slot* in_slots = NULL;
    int ignored_bits;
    int reserved_bits;
    uint32_t hash_mask = 0;
    double alpha = 0.8;             // 根据alpha * width算备选地址个数
    double exp_deg;

    set<uint32_t> extend_s_index;    // 存储有扩展链slot的地址index
    set<uint32_t> extend_d_index;    // 存储有扩展链slot的地址index

public:
    DegDetectorSlot2bit(uint32_t matrix_width, uint32_t matrix_depth, uint32_t slotNum, double exp_deg, int ignored_bits, int reserved_bits = 2, double alpha = 0.8, int update_bits = 8, int addr_bits = 6);
    ~DegDetectorSlot2bit();
    bool insert(key_type s, key_type d, hash_type hash_s, hash_type hash_d);
    uint32_t degQuery(key_type n, int type);
    addr_type addrQuery(key_type n, int type);   // type = 0 for out_addr, 1 for in_addr
    bool extendAddr(key_type key, addr_type n, int type);
    void printUsage();
    
private:
    slot* recoverPointer(slot s);
    bool addSlot(uint32_t addr, int type, key_type key, slot_value_type slot_value);
    bool decayStrategy(uint32_t addr, int type, key_type key, int update_bit);
    bool insertSlot(key_type key, int update_bit, int type);
};

DegDetectorSlot2bit::DegDetectorSlot2bit(uint32_t matrix_width, uint32_t matrix_depth, uint32_t slotNum, double exp_deg, int ignored_bits, int reserved_bits, double alpha, int update_bits, int addr_bits): 
DegDetector(matrix_width, matrix_depth), slot_num(slotNum), exp_deg(exp_deg), ignored_bits(ignored_bits), reserved_bits(reserved_bits), BITVECBITS(reserved_bits), UPDATEBITS(update_bits), ADDRBITS(addr_bits)
{
#if defined(DEBUG)
    cout << "DegDetectorSlot2bit::DegDetectorSlot2bit(matrix_width: " << matrix_width << ", matrix_depth: " << matrix_depth 
         << ", slot_num: " << slotNum << ", exp_deg: " << exp_deg << ", ignored_bits: " << ignored_bits << ", reserved_bits: " << reserved_bits 
         << ", alpha: " << alpha  << ", BITVECBITS: " << BITVECBITS  << ", UPDATEBITS: " << UPDATEBITS << ", ADDRBITS: " << ADDRBITS << ")" << endl;
#endif
    this->hash_mask = ((1 << ignored_bits) - 1);
    this->addr_mask = ((1 << ADDRBITS) - 1);
    this->update_mask = (((1 << UPDATEBITS) - 1) << ADDRBITS);
    this->bitvec_mask = (((1 << BITVECBITS) - 1) << (ADDRBITS + UPDATEBITS));

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

DegDetectorSlot2bit::~DegDetectorSlot2bit()
{
#if defined(DEBUG)
    cout << "DegDetectorSlot2bit::~DegDetectorSlot2bit()" << endl;
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

DegDetectorSlot2bit::slot* DegDetectorSlot2bit::recoverPointer(slot s) 
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
bool DegDetectorSlot2bit::addSlot(uint32_t addr, int type, key_type key, slot_value_type slot_value)
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
bool DegDetectorSlot2bit::decayStrategy(uint32_t addr, int type, key_type key, int update_bit)
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
    int index = -1;
    int min_update = INT32_MAX;
    for (int i = 0; i < SLOTROOM; i++) 
    {        
        if (((decay_slot[i].value & addr_mask) == 2))
        {
            int update_val = ((decay_slot[i].value & update_mask) >> ADDRBITS);
            if (update_val < min_update) 
            {
                min_update = update_val;
                index = i;
            }
        }
    }
    // 所有ROOM里面的点都是高度点，直接进行扩展
    if (index == -1)    
    {
        if (!addSlot(addr, type, key, ((1 << (ADDRBITS + UPDATEBITS + update_bit)) | 2)))
        {
            cout << "addSlot() error!" << endl;
            return false;
        }
    }
    else
    {
        // 对potential high degree点进行衰退
        // 如果slot中的bit vector为0
        if ((decay_slot[index].value & bitvec_mask) == 0)
        {
            // 如果update times也为0，这个时候直接替换即可
            if ((decay_slot[index].value & update_mask) == 0)
            {
                decay_slot[index].slot_key = (key << 1);
                decay_slot[index].value = ((1 << (ADDRBITS + UPDATEBITS + update_bit)) | 2);   // 初始(potential high degree)地址设置为2
            }
            else        // 否则update times--
            {
                decay_slot[index].value -= (1 << ADDRBITS);
            }
        }
        else    // 如果slot中的bit vector不为0
        {
            // 如果bit vector对应的update bit为1，则直接将其置为0
            if (((decay_slot[index].value & (1 << (ADDRBITS + UPDATEBITS + update_bit))) != 0))
            {
                decay_slot[index].value -= (1 << (ADDRBITS + UPDATEBITS + update_bit));
            }
            else    // 将bit vector置为0
            {
                decay_slot[index].value &= (~bitvec_mask);
            }
        }
    }
    return true;
}

// update slot
// 对于高度点不进行衰退，如果某个slot都被高度点占满，
// 则判断SLOTNUM - 2中key的最高位bit是不是1，是1则说明SLOTNUM - 1中存储的是指针（一个指针64bit）
bool DegDetectorSlot2bit::insertSlot(key_type key, int update_bit, int type)
{
    slot* slots = (type == 0) ? out_slots : in_slots;
    uint32_t idx = key % slot_num;
    uint32_t len = (type == 0) ? matrix_width : matrix_depth;
    double theta = len * ROOM * alpha;

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
                // update bit
                ptr[i].value |= (1 << (ADDRBITS + UPDATEBITS + update_bit));
                // check all bit
                bool update = true;
                for (int j = 0; j < reserved_bits; j++) 
                {
                    if ((ptr[i].value & (1 << (ADDRBITS + UPDATEBITS + j))) == 0) 
                    {
                        update = false;
                        break;
                    }
                }
                if (update)
                {
                    // clear the bit vector and add update_times
                    ptr[i].value &= (~bitvec_mask);
                    ptr[i].value += (1 << ADDRBITS);
                    // update addrs
                    int upd_times = ((ptr[i].value & update_mask) >> ADDRBITS);
                    if (upd_times == 0)
                    {
                        cout << "upd_times is out of range! Considering add more bits to UPDATEBITS." << endl;
                    }
                    uint32_t addrs =  (ptr[i].value & addr_mask);
                    if (addrs < 2) 
                    {
                        cout << "error! DegDetectorSlot2bit-line 340" << endl;
                    }
                    addr_type ad = (addr_type) max (ceil((double)((double)(upd_times * exp_deg) / theta)), (double)addrs);
                    if (ad > addr_mask) 
                    {
                        cout << "ad is out of range! Considering add more bits to ADDRBITS." << endl;
                    }
                    if (ad > addrs)
                    {
                        // update addr
                        ptr[i].value &= (~addr_mask);
                        ptr[i].value |= ad;
                    }
                }
                return true;
                
                // 两个连续“1”才是高度点
                // if (k > 1) {
                //     // addr_type ad = (addr_type) max (ceil((double)((1 << (k + ignored_bits)) / phi) / theta), 2.0);
                //     // addr_type ad = (addr_type) max (ceil((double)(((1 << ignored_bits) + (1 << k) - ignored_bits - 1.0 / (1 << ignored_bits)) / phi) / theta), 2.0);
                //     addr_type ad = (addr_type) max (ceil((double)(EXPDEG(ignored_bits, k) / theta)), 2.0);
                //     if ((ptr[i].value & addr_mask) < ad) 
                //     {
                //         ptr[i].value &= 0xffffff00;
                //         ptr[i].value |= ad;
                //     }
                // }
                // return true;
            }
            // if the slot is empty
            else if ((ptr[i].slot_key == 0) && (ptr[i].value == 0)) 
            {
                ptr[i].slot_key = (key << 1);
                ptr[i].value = ((1 << (ADDRBITS + UPDATEBITS + update_bit)) | 2);   // 初始(potential high degree)地址设置为2
                return true;
            }
        }
    }
    

    // 前面的slot中已经满了，而且没有fingerprint匹配的，因此进行decay
    // we kick out the minimum element in the slot according to the hash value by probability
    return addSlot(idx, type, key, ((1 << (ADDRBITS + UPDATEBITS + update_bit)) | 2));
    // return decayStrategy(idx, type, key, update_bit);
}

// insert elements to the degree detector
// 1. 对于hash值中l1p大于等于ignored_bits，插入到slot里面去，否则忽略；
// 2. 对potential high degree进行衰退；
// 3. 在查询的时候判断是否是高度点，判断方法是bit vector中是否存在两个连续的1
bool DegDetectorSlot2bit::insert(key_type s, key_type d, hash_type hash_s, hash_type hash_d)
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
uint32_t DegDetectorSlot2bit::degQuery(key_type key, int type) 
{
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
                // res_ad = (ptr[i].value & addr_mask);
                uint32_t res = (uint32_t)(((ptr[i].value & update_mask) >> ADDRBITS) * exp_deg);
                return res;
            }
        }
    }
    return 0;
}

addr_type DegDetectorSlot2bit::addrQuery(key_type key, int type) 
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
                // res_ad = (ptr[i].value & addr_mask);
                return (addr_type)(ptr[i].value & addr_mask);
            }
        }
    }
    return 2;
}

bool DegDetectorSlot2bit::extendAddr(key_type key, addr_type n, int type) 
{
    // cout << "key = " << key << ", n = " << n << endl;
    bool res = false;
    if (n < 2)
    {
        cout << "DegDetectorSlot2bit::extendAddr error!" << endl;
        return false;
    }
    if (n > addr_mask) 
    {
        cout << "extendAddr(n = " << n << ") is out of range! Considering add more bits to ADDRBITS." << endl;
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
                ptr[i].value &= (~addr_mask);
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

void DegDetectorSlot2bit::printUsage() 
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
                addr_type ad = (ptr1[i].value & addr_mask);
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
                addr_type ad = (ptr2[i].value & addr_mask);
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

    uint32_t in_count = 0, out_count = 0;
    // uint32_t in_pointers = 0, out_pointers = 0;
    for (int i = 0; i < slot_num * SLOTROOM; i++) 
    {
        if (in_slots[i].slot_key != 0)
            in_count++;
        if (out_slots[i].slot_key != 0) 
            out_count++;
    }

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

    cout << "------------------DegDeteSlot----------------------" << endl;
    cout << "in_count = " << in_count << ", in_slot size = " << (slot_num * SLOTROOM)
         << ", in_slot usage = " << (((double)in_count / (double)(slot_num * SLOTROOM)) * 100) << "%" << endl;
    cout << "out_count = " << out_count << ", out_slot size = " << (slot_num * SLOTROOM)
         << ", out_slot usage = " << (((double)out_count / (double)(slot_num * SLOTROOM)) * 100) << "%" << endl;
    cout << "----------------------------------------------------" << endl;
}

#endif // _DegDetectorSlot2bit_H
