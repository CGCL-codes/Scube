#ifndef _DegDetectorSlot_H
#define _DegDetectorSlot_H
#include <map>
#include <set>
#include <random>
#include <memory.h>
#include <stdlib.h>
#include "DegDetector.h"

// with exponential-decay probability = para ^ -(1 << (l0p - 1) / phi)
// slot + linkedlist

// struct slot                     // the struct slot is derived from class DegDetector
// {
//     key_type slot_key;          // the least bit of slot_key is a flag of pointer
//     slot_value_type value;      // the high 24-bit is used for bit vector, and the low 8-bit is used for storing addresses
// };

#define para 1.08

using namespace std;

class DegDetectorSlot: public DegDetector
{
private:
    const uint32_t slot_num;
    slot* out_slots = NULL;
    slot* in_slots = NULL;

    set<uint32_t> extend_s_index;    // store the index of the slot which has the extend linkedlist
    set<uint32_t> extend_d_index;    // store the index of the slot which has the extend linkedlist

public:
    DegDetectorSlot(uint32_t matrix_width, uint32_t matrix_depth, uint32_t k_width, uint32_t k_depth, uint32_t slotNum);
    ~DegDetectorSlot();
    bool insert(key_type s, key_type d, hash_type hash_s, hash_type hash_d);
    uint32_t degQuery(key_type n, int type);
    addr_type addrQuery(key_type n, int type);   // type = 0 for out_addr, 1 for in_addr
    bool extendAddr(key_type key, addr_type n, int type);
    void printUsage() 
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

private:
    slot* recoverPointer(slot s);
    bool addSlot(uint32_t addr, int type, key_type key, slot_value_type slot_value);
    bool updateSlotLinkedList(slot* linked_slot, key_type v, hash_type hashValue, int type);
    addr_type visitLinkedSlot(slot* linked_slot, key_type key);
    bool visitLinkedSlot(slot* linked_slot, key_type key, addr_type n);
    bool decayStrategy(uint32_t addr, int type, key_type key, hash_type hashValue);
    bool insertSlot(key_type key, hash_type hashValue, int type);

    int left0Pos(hash_type hashValue);
    int left1Pos(hash_type hashValue);
    int right1Pos(hash_type hashValue);
    int hammingWeight(hash_type hashValue);
};

DegDetectorSlot::DegDetectorSlot(uint32_t matrix_width, uint32_t matrix_depth, uint32_t k_width, uint32_t k_depth, uint32_t slotNum): 
DegDetector(matrix_width, matrix_depth, k_width, k_depth), slot_num(slotNum)
{
#if defined(DEBUG)
    cout << "FINAL VERSION!!!!" << endl;
    cout << "DegDetectorSlot::DegDetectorSlot(matrix_width: " <<  matrix_width << ", matrix_depth: " << matrix_depth << ", k_width: " << k_width << ", k_depth: " << k_depth << ", slot_num: " << slotNum << ")" << endl;
#endif
    // this->out_slots = new slot[slotNum * SLOTROOM];
    // this->in_slots = new slot[slotNum * SLOTROOM];
    posix_memalign((void**)&(this->out_slots), 64, sizeof(slot) * slotNum * SLOTROOM);		// 64-byte alignment of the requested space
    posix_memalign((void**)&(this->in_slots), 64, sizeof(slot) * slotNum * SLOTROOM);		// 64-byte alignment of the requested space
    memset(this->out_slots, 0, sizeof(slot) * slotNum * SLOTROOM);
    memset(this->in_slots, 0, sizeof(slot) * slotNum * SLOTROOM);
}

DegDetectorSlot::~DegDetectorSlot()
{
#if defined(DEBUG)
    cout << "DegDetectorSlot::~DegDetectorSlot()" << endl;
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
    delete[] this->out_slots;
    delete[] this->in_slots;
}

DegDetectorSlot::slot* DegDetectorSlot::recoverPointer(slot s) 
{
    return (slot *) ((((uint64_t)s.slot_key) << 32) | s.value);
}

// linkedlist is extended from the tail.
// parameter addr is the index of the array
// Step 1: create a new slot and set 0 to its content
// Step 2: find the tail slot of the linkedlist
// Step 3: set the pointer flag bit to 1 of tail_slot[SLOTROOM - 2]
// Step 4: copy the content tail_slot[SLOTROOM - 2] to new_slot[0]
// Step 5: store the new_slot pointer to tail_slot[SLOTROOM - 1]
// Step 6: insert the tuple (key, slot_value) to new_slot[1]
bool DegDetectorSlot::addSlot(uint32_t addr, int type, key_type key, slot_value_type slot_value)
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
        
        // store the pointer of new_slot to rec[SLOTROOM - 1]
        rec[SLOTROOM - 1].slot_key = ((uint64_t) new_slot >> 32);       
        rec[SLOTROOM - 1].value = ((uint64_t) new_slot & 0xffffffff);
    }
    else   // first extend
    {    
        slots[addr * SLOTROOM + SLOTROOM - 2].slot_key |= 1;
        new_slot[0].slot_key = slots[addr * SLOTROOM + SLOTROOM - 1].slot_key;
        new_slot[0].value = slots[addr * SLOTROOM + SLOTROOM - 1].value;
        new_slot[1].slot_key = (key << 1);
        new_slot[1].value = slot_value;

        // store the pointer of new_slot to slots[SLOTROOM - 1]
        slots[addr * SLOTROOM + SLOTROOM - 1].slot_key = ((uint64_t) new_slot >> 32);       
        slots[addr * SLOTROOM + SLOTROOM - 1].value = ((uint64_t) new_slot & 0xffffffff);
    }
    return true;
}

// visit the linkedlist which the head pointer is para (slot* head)
// if the linkedlist contains the key, then update the value and return true;
// or else return false.
bool DegDetectorSlot::updateSlotLinkedList(slot* head, key_type key, hash_type hashValue, int type)
{
    uint16_t k1 = (type == 0) ? k_width : k_depth;
    uint32_t len = (type == 0) ? matrix_width : matrix_depth;
    double theta = len * ROOM * 0.8;
    slot* ptr = head;
    bool flag = true;
    while (flag)
    {
        for (int i = 0; i < SLOTROOM; i++)
        {
            if (i == SLOTROOM - 1)
            {
                if ((ptr[SLOTROOM - 2].slot_key & 1) == 1)
                {
                    ptr = recoverPointer(ptr[SLOTROOM - 1]);
                    break;
                }
                else
                    flag = false;
            }
            // if the fingerprint is the same
            if ((ptr[i].slot_key >> 1) == key)
            {
                // "or" operation
                ptr[i].value |= (hashValue << 8);
                int k = left0Pos(ptr[i].value >> 8);
                
                if (k >= k1)
                {
                    // addr_type ad = (1 << (k1 - k + 1)) + 1;
                    // addr_type ad = k1 - (k - 2);
                    addr_type ad = (addr_type) max (ceil((double)((1 << k) / phi) / theta), 2.0);
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
                ptr[i].value = (hashValue << 8 | 2);
                return true;
            }
        }
    }
    return false;
}

addr_type DegDetectorSlot::visitLinkedSlot(slot* linked_slot, key_type key)       // get address
{
    slot* ptr = linked_slot;
    bool flag = true;
    while (flag)
    {
        for (int i = 0; i < SLOTROOM; i++)
        {
            if (i == SLOTROOM - 1)
            {
                if ((ptr[SLOTROOM - 2].slot_key & 1) == 1)
                {
                    ptr = recoverPointer(ptr[SLOTROOM - 1]);
                    break;
                }
                else
                    flag = false;
            }
            // if the fingerprint is the same
            if ((ptr[i].slot_key >> 1) == key) 
            {
                // get the addr info
                return (addr_type)(ptr[i].value & 0xff);
            }
        }
    }
    return 2;
}

bool DegDetectorSlot::visitLinkedSlot(slot* linked_slot, key_type key, addr_type n)           // extend addr
{
    slot* ptr = linked_slot;
    bool flag = true;
    while (flag)
    {
        for (int i = 0; i < SLOTROOM; i++)
        {
            if (i == SLOTROOM - 1)
            {
                if ((ptr[SLOTROOM - 2].slot_key & 1) == 1)
                {
                    ptr = recoverPointer(ptr[SLOTROOM - 1]);
                    break;
                }
                else
                    flag = false;
            }
            // if the fingerprint is the same
            if ((ptr[i].slot_key >> 1) == key) 
            {
                // extend addrs to n
                ptr[i].value &= 0xffffff00;
                ptr[i].value |= n;
                return true;
            }
        }
    }
    return false;
}


// onle decay the low-degree ndoe
// decay indicates that there does not exist the key in responding slots, because we have transverse the slot when inserting
// in addition, performing an extend operation means that the previous nodes in the slot are all high-degree nodes, so we  only need to decay the tail slot
// get the tail slot, and decay all the rooms in the slot
// if the slot is full, performing an extend operation
bool DegDetectorSlot::decayStrategy(uint32_t addr, int type, key_type key, hash_type hashValue)
{
    slot* slots = (type == 0) ? out_slots : in_slots;
    slot* decay_slot = NULL;
    if ((slots[addr * SLOTROOM + SLOTROOM - 2].slot_key & 1) == 1)      // the slot has a linked list，we get the tail slot
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
    else        // the slot has no linked list
    {
        decay_slot = &(slots[addr * SLOTROOM]);
    }
    
    // we kick out the minimum element in the slot according to the hash value by probability
    // if the nodes in the decay_slot are all high-degree nodes, extend it
    int l0p = INT32_MAX;
    int index = -1;
    for (int i = 0; i < SLOTROOM; i++) 
    {
        int l = left0Pos(decay_slot[i].value >> 8);
        if (((decay_slot[i].value & 0xff) == 2) && (l0p > l)) 
        {
            l0p = l;
            index = i;
        }
    }
    if (index == -1)    // if the nodes in the decay_slot are all high-degree nodes, extend it
    {
        if (!addSlot(addr, type, key, hashValue))
        {
            cout << "addSlot() error!" << endl;
            return false;
        }
    }
    else
    {
        if (l0p > 0)    // only decay the low-degree node
        {
            // if ((decay_slot[index].value & 0xff) == 2) {   
            double num = (double)(1 << (l0p - 1)) / phi;
            if (!(rand() % int(pow(para, num))))
            {
                // change the right most 1 of hash_value to 0
                hash_type mask = 1 << (l0p - 1);
                decay_slot[index].value ^= (mask << 8);
                // if the hash_value reduces to 0, kick out
                // if (decay_slot[index].value[index] == 0) {
                if (((decay_slot[index].value >> 8) & 1) == 0)
                {
                    decay_slot[index].slot_key = (key << 1);
                    decay_slot[index].value = ((hashValue << 8) | 2);   // set the initial address to 2
                }
            }
            // }
        }
        else if (l0p == 0)   // "kick-out"
        {
            decay_slot[index].slot_key = (key << 1);
            decay_slot[index].value = ((hashValue << 8) | 2);   // set the initial address to 2
        }
    }
    return true;
}

// update slot
bool DegDetectorSlot::insertSlot(key_type key, hash_type hashValue, int type)
{
    slot* slots = (type == 0) ? out_slots : in_slots;
    uint32_t idx = key % slot_num;
    uint16_t k1 = (type == 0) ? k_width : k_depth;
    uint32_t len = (type == 0) ? matrix_width : matrix_depth;
    double theta = len * ROOM * 0.8;

    // look up if the node is existing or the slot is empty
    bool inserted = false;
    for (int i = 0; i < SLOTROOM; i++) 
    {
        if (!inserted) 
        {
            if ((i == SLOTROOM - 1) && ((slots[idx * SLOTROOM + SLOTROOM - 2].slot_key & 1) == 1))     // the content of the last room in the slot is a pointer
            {
                // recover the pointer
                slot* head = recoverPointer(slots[idx * SLOTROOM + SLOTROOM - 1]);
                inserted = updateSlotLinkedList(head, key, hashValue, type);
            }
            else 
            {
                // if the fingerprint is the same
                if ((slots[idx * SLOTROOM + i].slot_key >> 1) == key) 
                {
                    // "or" operation
                    slots[idx * SLOTROOM + i].value |= (hashValue << 8);
                    int k = left0Pos((slots[idx * SLOTROOM + i].value >> 8));
                    
                   
                    if (k >= k1) {
                        // addr_type ad = (1 << (k - k1 + 1)) + 1;
                        // addr_type ad = k - (k1 - 2);
                        addr_type ad = (addr_type) max (ceil((double)((1 << k) / phi) / theta), 2.0);
                        if ((slots[idx * SLOTROOM + i].value & 0xff) < ad) 
                        {
                            slots[idx * SLOTROOM + i].value &= 0xffffff00;
                            slots[idx * SLOTROOM + i].value |= ad;
                        }
                    }
                    inserted = true;
                }
                // if the slot is empty
                else if ((slots[idx * SLOTROOM + i].slot_key == 0) && (slots[idx * SLOTROOM + i].value == 0)) 
                {
                    slots[idx * SLOTROOM + i].slot_key = (key << 1);
                    slots[idx * SLOTROOM + i].value = ((hashValue << 8) | 2);   // set the initial address to 2
                    inserted = true;
                }
            }
        }
        if (inserted)
            break;
    }

    // we kick out the minimum element in the slot according to the hash value by probability
    if (!inserted) 
    {
        inserted = decayStrategy(idx, type, key, hashValue);
    }
    return inserted;
}

// insert elements to the degree detector
bool DegDetectorSlot::insert(key_type s, key_type d, hash_type hash_s, hash_type hash_d)
{
    uint32_t sAddr = s % slot_num;
    uint32_t dAddr = d % slot_num;

    int sLeft1Pos = left1Pos(hash_s);
    int dLeft1Pos = left1Pos(hash_d);
    hash_type sHashValue = (sLeft1Pos == -1) ? 0 : (1 << sLeft1Pos);
    hash_type dHashValue = (dLeft1Pos == -1) ? 0 : (1 << dLeft1Pos);

    bool out_ins = insertSlot(s, dHashValue, 0);
    bool in_ins = insertSlot(d, sHashValue, 1);

    return (out_ins && in_ins);
}

// return the estimate degree of the node
uint32_t DegDetectorSlot::degQuery(key_type key, int type) 
{
    addr_type addrs = this->addrQuery(key, type);
    if (addrs > 2) 
        return (1 << addrs) / phi;
    uint32_t res = 0;
    uint32_t slot_index = key % slot_num;
    slot* slots = (type == 0) ? (this->out_slots) : (this->in_slots);
    for (int i = 0; i < SLOTROOM; i++) 
    {
        if ((i == SLOTROOM - 1) && ((slots[slot_index * SLOTROOM + SLOTROOM - 2].slot_key & 1) == 1))    // the content of the last room in the slot is a pointer
        {  
            // recover the pointer
            slot* ptr = recoverPointer(slots[slot_index * SLOTROOM + SLOTROOM - 1]);
            slot* ptr1 = ptr;
            while (true) 
            {
                for (int j = 0; j < SLOTROOM; j++) 
                {
                    if ((j == SLOTROOM - 1) && (ptr[SLOTROOM - 2].slot_key & 1) == 1) 
                    {
                        ptr1 = recoverPointer(ptr[SLOTROOM - 1]);
                    }
                    else 
                    {
                        // if the fingerprint is the same
                        if ((ptr[j].slot_key >> 1) == key) 
                        {
                            // return the estimate value according to the hashValue
                            hash_type hashValue = (ptr[j].value >> 8);
                            int l0p = left0Pos(hashValue);
                            res += ((1 << l0p) / phi);
                            return res;
                        }
                    }
                }
                if (ptr1 != ptr)
                    ptr = ptr1;
                else
                    break;
            }
        }
        else 
        {
            if ((slots[slot_index * SLOTROOM + i].slot_key >> 1) == key) 
            {
                // return the estimate value according to the hashValue
                hash_type hashValue = (slots[slot_index * SLOTROOM + i].value >> 8);
                int l0p = left0Pos(hashValue);
                res += ((1 << l0p) / phi);
                return res;
            }
        }
        
    }
    return 0;
}

addr_type DegDetectorSlot::addrQuery(key_type key, int type) 
{
    // 初始给所有的点都设为2轮，0，1
    addr_type res_ad = 2;
    slot* slots = (type == 0) ? out_slots : in_slots;
    uint32_t slot_index = key % slot_num;
    for (int i = 0; i < SLOTROOM; i++)         // query the addr in slots
    {
        if ((i == SLOTROOM - 1) && ((slots[slot_index * SLOTROOM + SLOTROOM - 2].slot_key & 1) == 1))    // 说明最后一个ROOM存储的是指针
        {  
            // 恢复指针
            slot* rec = recoverPointer(slots[slot_index * SLOTROOM + SLOTROOM - 1]);
            addr_type temp = visitLinkedSlot(rec, key);
            if (temp > res_ad) 
            {
                return temp;
            }
                // res_ad = temp;
        }
        else 
        {
            if ((slots[slot_index * SLOTROOM + i].slot_key >> 1) == key) 
            {
                // res_ad = (slots[slot_index * SLOTROOM + i].value & 0xff);
                return (slots[slot_index * SLOTROOM + i].value & 0xff);
            }
        }
    }
    return res_ad;
}

bool DegDetectorSlot::extendAddr(key_type key, addr_type n, int type) 
{
    // cout << "key = " << key << ", n = " << n << endl;
    bool res = false;
    if (n < 2)
    {
        cout << "DegDetectorSlot::extendAddr error!" << endl;
        return false;
    }
    // check the slots, and find out if the slots contains the key
    slot* slots = (type == 0) ? out_slots : in_slots;
    uint32_t slot_index = key % slot_num;
    for (int i = 0; i < SLOTROOM; i++)
    {
        if ((i == SLOTROOM - 1) && ((slots[slot_index * SLOTROOM + SLOTROOM - 2].slot_key & 1) == 1))     // the content of the last room in the slot is a pointer
        {
            // recover the pointer
            slot* rec = recoverPointer(slots[slot_index * SLOTROOM + i]);
            bool temp = visitLinkedSlot(rec, key, n);
            if (temp) 
                return true;
        }
        else 
        {
            // if the key is the same
            if ((slots[slot_index * SLOTROOM + i].slot_key >> 1) == key) 
            {
                slots[slot_index * SLOTROOM + i].value &= 0xffffff00;
                slots[slot_index * SLOTROOM + i].value |= n;
                return true;
            }
            // look up if there exists empty slot
            if ((slots[slot_index * SLOTROOM + i].slot_key == 0) && (slots[slot_index * SLOTROOM + i].value == 0)) 
            {
                slots[slot_index * SLOTROOM + i].slot_key = (key << 1);
                // slots[slot_index * SLOTROOM + i].value = ((dHashValue << 8) | n);   // the address field is set to n
                slots[slot_index * SLOTROOM + i].value = n;   // the address field is set to n
                return true;
            }
        }
    }
    for (int i = 0; i < SLOTROOM; i++)
    {
        if ((i == SLOTROOM - 1) && ((slots[slot_index * SLOTROOM + SLOTROOM - 2].slot_key & 1) == 1))     // the content of the last room in the slot is a pointer
        {
            // recover the pointer
            slot* ptr = recoverPointer(slots[slot_index * SLOTROOM + i]);
            slot* ptr1 = ptr;
            while (true) {
                for (int i = 0; i < SLOTROOM; i++) 
                {
                    if ((i == SLOTROOM - 1) && (ptr[SLOTROOM - 2].slot_key & 1) == 1) 
                    {
                        ptr1 = recoverPointer(ptr[SLOTROOM - 1]);
                    }
                    // else 
                    // {
                    //     cout << "Room " << i << " addr = " << (slots[slot_index * SLOTROOM + i].value & 0xff) << endl;
                    // }
                }
                if (ptr1 != ptr)
                    ptr = ptr1;
                else 
                    break;
            }
        }
        // else 
        // {
        //     cout << "Room " << i << " addr = " << (slots[slot_index * SLOTROOM + i].value & 0xff) << endl;
        // }
    }

    // there is no empty rooms in the slot, performing an extend operation
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
int DegDetectorSlot::left0Pos(hash_type hashValue) 
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
int DegDetectorSlot::left1Pos(hash_type hashValue) 
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
int DegDetectorSlot::right1Pos(hash_type hashValue) 
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
int DegDetectorSlot::hammingWeight(hash_type hashValue) 
{
    int count = 0;
    while (hashValue != 0) 
    {
        count++;
        hashValue = hashValue & (hashValue - 1);
    }
    return count;
}

#endif // _DegDetectorSlot_H