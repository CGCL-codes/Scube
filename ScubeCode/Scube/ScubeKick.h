#ifndef _SCUBEKICK_H
#define _SCUBEKICK_H
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <set>
#include <map>
#include <cmath>
#include <stdlib.h>
#include <bitset>
#include <memory.h>
#include <malloc.h>
#include <sys/time.h>
#include "Scube.h"
#include "../headers/HashFunction.h"
#include "../headers/params.h"
#include "../DegDetector/DegDetectorNewSlot.h"
#include "../DegDetector/DegDetectorSlot.h"
#include "../DegDetector/DegDetectorSlot2bit.h"
#include "../DegDetector/DegDetectorUMap2bit.h"

using namespace std;

// !!!seed要用32bit，16bit不行!!!
// 避免两个元素之间的循环踢 + multiple hash

#ifdef H1
#define HASH 1
#elif H2
#define HASH 2
#elif H3
#define HASH 3
#elif H4
#define HASH 4
#elif H5
#define HASH 5
#elif H6
#define HASH 6
#else
#define HASH 0
#endif

class ScubeKick: public Scube
{
private:
    struct mapnode
    {
        addr_type addr;
        fp_type fp;
    };
    int n = 0;
    int writeflag = 0;
    const uint8_t kick_times = 10;
    const uint32_t slot_num = 1000;
    basket* value = NULL;
    DegDetector* degDetector = NULL;
    uint32_t s_extend = 0, d_extend = 0, max_kick_extend = 0;
    bool slot2bit = false;

public:
    // DegSlot
    ScubeKick(uint32_t width, uint32_t depth, uint8_t fingerprint_length, uint16_t k_width, uint16_t k_depth, uint32_t kick_times, uint32_t slot_num, int writeflag);
    // DegNewSlot
    ScubeKick(uint32_t width, uint32_t depth, uint8_t fingerprint_length, uint16_t k_width, uint16_t k_depth, uint32_t kick_times, uint32_t slot_num, int writeflag, int ignore_bits, double alpha);
    // DegSlot2bit
    ScubeKick(uint32_t width, uint32_t depth, uint8_t fingerprint_length, uint32_t kick_times, uint32_t slot_num, int writeflag, double exp_deg, int ignore_bits, int reserved_bits, double alpha);
    // DegUMap2bit
    ScubeKick(uint32_t width, uint32_t depth, uint8_t fingerprint_length, uint32_t kick_times, int writeflag, double exp_deg, int ignore_bits, int reserved_bits, double alpha);
    ~ScubeKick();
    bool insert(string s, string d, w_type w);
    bool insert(string s, string d, w_type w, double& kick_time, double& detector_ins_time);
    w_type edgeWeightQuery(string s, string d);
    uint32_t nodeWeightQuery(string s, int type);
    uint32_t nodeWeightQuery(string v, int type, double& matrix_time, double& addr_time);
    bool reachabilityQuery(string s, string d);
    bool reachabilityQuery(string s, string d, int& points, int& lines);
    void printUsageInfo();
    void printExtendAddrInfo() 
    {
        cout << "s_extend = " << s_extend << endl;
        cout << "d_extend = " << d_extend << endl;
        cout << "max_kick_extend = " << max_kick_extend << endl;
    }

private:
    int getMinIndex(uint16_t * a, int length);
    uint32_t recoverAddrSeed(fp_type fp, uint16_t idx, addr_type addr, uint32_t mod);

    int insert_matrix(addr_type& addr_s, fp_type& fp_s, addr_type& addr_d, fp_type& fp_d, w_type& w, addr_type& idx_s, addr_type& idx_d, addr_type* addr_rows, addr_type* addr_cols);
    bool kick_out(addr_type& addr_s, fp_type& fp_s, addr_type& addr_d, fp_type& fp_d, w_type& w, addr_type& idx_s, addr_type& idx_d, addr_type* addr_row, addr_type* addr_col, int& row_addrs, int& col_addrs);
};
// DegSlot
ScubeKick::ScubeKick(uint32_t width, uint32_t depth, uint8_t fingerprint_length, uint16_t k_width, uint16_t k_depth, uint32_t kick_times, uint32_t slot_num, int writeflag):
Scube(width, depth, fingerprint_length), kick_times(kick_times) 
{
#if defined(DEBUG)
    cout << "ScubeKick::ScubeKick(width: " <<  width << ", depth: " << depth << ", fplen: " << (uint32_t) fingerprint_length << ", k_width: " << k_width << ", k_depth: " << k_depth << ", kick_times: " << kick_times << ", slot_num: " << slot_num << ", FLAG: " << writeflag << ")" << endl;
#endif
    s_extend = 0, d_extend = 0, max_kick_extend = 0;
    this->degDetector = new DegDetectorSlot(width, depth, k_width, k_depth, slot_num);
    if (writeflag > 0)
        writeflag = 1;
    // this->value = new basket[width * depth];
    // posix_memalign((void**)&(this->value), 64, sizeof(basket) * width * depth);		// 64-byte alignment of the requested space
    // this->value = (basket *) aligned_alloc(64, sizeof(basket) * width * depth);	    // 64-byte alignment of the requested space
    this->value = (basket *) memalign(64, sizeof(basket) * width * depth);	            // 64-byte alignment of the requested space
    memset(value, 0, sizeof(basket) * width * depth);
}

//DegNewSlot
ScubeKick::ScubeKick(uint32_t width, uint32_t depth, uint8_t fingerprint_length, uint16_t k_width, uint16_t k_depth, uint32_t kick_times, uint32_t slot_num, int writeflag, int ignore_bits, double alpha):
Scube(width, depth, fingerprint_length), kick_times(kick_times)
{
#if defined(DEBUG)
    cout << "ScubeKick::ScubeKick(width: " <<  width << ", depth: " << depth << ", fplen: " << (uint32_t) fingerprint_length 
         << ", k_width: " << k_width << ", k_depth: " << k_depth << ", kick_times: " << kick_times << ", slot_num: " 
         << slot_num << ", FLAG: " << writeflag << ", ignore_bits: " << ignore_bits << ", alpha: " << alpha << ")" << endl;
#endif
    s_extend = 0, d_extend = 0, max_kick_extend = 0;
    this->degDetector = new DegDetectorNewSlot(width, depth, k_width, k_depth, slot_num, ignore_bits, alpha);
    if (writeflag > 0)
        writeflag = 1;
    // this->value = new basket[width * depth];
    // posix_memalign((void**)&(this->value), 64, sizeof(basket) * width * depth);		// 64-byte alignment of the requested space
    // this->value = (basket *) aligned_alloc(64, sizeof(basket) * width * depth);  	// 64-byte alignment of the requested space
    this->value = (basket *) memalign(64, sizeof(basket) * width * depth);	            // 64-byte alignment of the requested space
    memset(value, 0, sizeof(basket) * width * depth);
}

// DegSlot2bit
ScubeKick::ScubeKick(uint32_t width, uint32_t depth, uint8_t fingerprint_length, uint32_t kick_times, uint32_t slot_num, int writeflag, double exp_deg, int ignore_bits, int reserved_bits, double alpha):
Scube(width, depth, fingerprint_length), kick_times(kick_times)
{
#if defined(DEBUG)
    cout << "ScubeKick::ScubeKick(width: " <<  width << ", depth: " << depth << ", fplen: " << (uint32_t) fingerprint_length 
         << ", kick_times: " << kick_times << ", slot_num: " << slot_num << ", FLAG: " << writeflag 
         << ", exp_deg: " << exp_deg << ", ignore_bits: " << ignore_bits
         << ", reserved_bits: " << reserved_bits << ", alpha: " << alpha << ")" << endl;
#endif
    this->slot2bit = true;
    s_extend = 0, d_extend = 0, max_kick_extend = 0;
    this->degDetector = new DegDetectorSlot2bit(width, depth, slot_num, exp_deg, ignore_bits, reserved_bits, alpha);
    if (writeflag > 0)
        writeflag = 1;
    // this->value = new basket[width * depth];
    // posix_memalign((void**)&(this->value), 64, sizeof(basket) * width * depth);		// 64-byte alignment of the requested space
    // this->value = (basket *) aligned_alloc(64, sizeof(basket) * width * depth);	    // 64-byte alignment of the requested space
    this->value = (basket *) memalign(64, sizeof(basket) * width * depth);	            // 64-byte alignment of the requested space
    memset(value, 0, sizeof(basket) * width * depth);
}

// DegSlotUMap
ScubeKick::ScubeKick(uint32_t width, uint32_t depth, uint8_t fingerprint_length, uint32_t kick_times, int writeflag, double exp_deg, int ignore_bits, int reserved_bits, double alpha):
Scube(width, depth, fingerprint_length), kick_times(kick_times)
{
#if defined(DEBUG)
    cout << "ScubeKick::ScubeKick(width: " <<  width << ", depth: " << depth << ", fplen: " << (uint32_t) fingerprint_length 
         << ", kick_times: " << kick_times << ", FLAG: " << writeflag << ", exp_deg: " << exp_deg 
         << ", ignore_bits: " << ignore_bits << ", reserved_bits: " << reserved_bits << ", alpha: " << alpha << ")" << endl;
#endif
    this->slot2bit = true;
    s_extend = 0, d_extend = 0, max_kick_extend = 0;
    this->degDetector = new DegDetectorUMap2bit(width, depth, exp_deg, ignore_bits, reserved_bits, alpha);
    if (writeflag > 0)
        writeflag = 1;
    // this->value = new basket[width * depth];
    // posix_memalign((void**)&(this->value), 64, sizeof(basket) * width * depth);		// 64-byte alignment of the requested space
    // this->value = (basket *) aligned_alloc(64, sizeof(basket) * width * depth);	    // 64-byte alignment of the requested space
    this->value = (basket *) memalign(64, sizeof(basket) * width * depth);	            // 64-byte alignment of the requested space
    memset(value, 0, sizeof(basket) * width * depth);
}

    
ScubeKick::~ScubeKick()
{
    #if defined(DEBUG)
	cout << "ScubeKick::~ScubeKick()" << endl;
    #endif
    delete degDetector;
    // delete[] value;
    free(this->value);
}

/***************private functions***************/
// return the min value index of array a
int ScubeKick::getMinIndex(uint16_t *a, int length) 
{
    uint16_t min = a[0];
    int index = 0;
    for(int i = 1; i < length; i++)
    {
        if(a[i] < min)
        {
            min = a[i];
            index = i;
        }
    }
    return index;
}
// recover the address seed by the fingerprint and the address index
uint32_t ScubeKick::recoverAddrSeed(fp_type fp, uint16_t idx, addr_type addr, uint32_t mod)
{
    uint32_t res = 0;
    uint32_t shifter = fp;
    for (int v = 0; v < idx; v++)
    {
        shifter = (shifter * MUL + INC) % MOD;
    }
    uint32_t tmp_h = addr;
    while (tmp_h < shifter)
        tmp_h += mod;
    res = tmp_h - shifter;
    return res;
}
// -1: insert failed
// 0: insert succeed and the edge does not exist (update)
// 1: insert succeed and the edge exists
int ScubeKick::insert_matrix(addr_type& addr_s, fp_type& fp_s, addr_type& addr_d, fp_type& fp_d, w_type& w, addr_type& idx_s, addr_type& idx_d, addr_type* addr_rows, addr_type* addr_cols)
{
    //  calculate the seeds
    uint32_t* seed1 = new uint32_t[idx_s];
    uint32_t* seed2 = new uint32_t[idx_d];
    seed1[0] = fp_s;
    for(int i = 1; i < idx_s; i++)
    {
        seed1[i] = (seed1[i - 1] * MUL + INC) % MOD;
    }
    seed2[0] = fp_d;
    for(int i = 1; i < idx_d; i++)
    {
        seed2[i] = (seed2[i - 1] * MUL + INC) % MOD;
    }

    // 判断当前边信息是否已经存在在当前（所有）备选地址的某一个
    // 判断当前（所有）备选地址是否有空地址
    bool empty = false;
    uint16_t temp_idx = 0;
    int temp_k = -1;
    addr_type temp_pos = 0;
    for (int i = 0; i < idx_s; i++)
    {
        addr_rows[i] = (addr_s + seed1[i]) % depth;
        for (int j = 0; j < idx_d; j++)
        {
            addr_cols[j] = (addr_d + seed2[j]) % width;
            addr_type pos = addr_rows[i] * width + addr_cols[j];
            uint16_t idx = (i << 8) |  j;
            for (int k = 0; k < ROOM; ++k)
            {
                if((value[pos].fp_src[k] == fp_s) && (value[pos].fp_dst[k] == fp_d) && (value[pos].ext[k] == idx)) 
                {
                    value[pos].weight[k] += w;
                    delete[] seed1;
                    delete[] seed2;
                    return 1;
                }
                if(value[pos].fp_src[k] == 0 && !empty) //找到空地址
                {
                    empty = true;
                    temp_idx = idx;
                    temp_k = k;
                    temp_pos = pos;
                }
            }
        }
    }
    if(empty) 
    {
        value[temp_pos].ext[temp_k] = temp_idx;
        value[temp_pos].fp_src[temp_k] = fp_s;
        value[temp_pos].fp_dst[temp_k] = fp_d;
        value[temp_pos].weight[temp_k] = w;
        delete[] seed1;
        delete[] seed2;
        return 0;
    }
    delete[] seed1;
    delete[] seed2;
    return -1;
}
// kick-out strategy
bool ScubeKick::kick_out(addr_type& addr_s, fp_type& fp_s, addr_type& addr_d, fp_type& fp_d, w_type& w, addr_type& idx_s, addr_type& idx_d, addr_type* rows, addr_type* cols, int& row_kicks, int& col_kicks)
{
    // 在当前 “最新” 的row_addrs行、col_addrs列踢
    uint16_t ext = ((idx_s - row_kicks) << 8) | (idx_d - col_kicks);    //base ext 需要加上kick_index
    // uint16_t cur_ext = ((map_idx_s - 2) << 8) | (map_idx_d - 2);     //base ext 需要加上kick_index

    int max_kick = 1000;
    bool inserted = false;
    uint32_t pre_row = -1, pre_col = -1;    // 踢的地址
    // key_type ks[1000], kd[1000];
    key_type key_s = ((addr_s << fingerprint_length) + fp_s);
    key_type key_d = ((addr_d << fingerprint_length) + fp_d);

    
    for (int kick_num = 0; !inserted; kick_num++)
    {
        // ks[kick_num] = cur_key_s;
        // kd[kick_num] = cur_key_d;
        // 当踢的次数达到kick_times时，这时每次都判断踢出来的边中是不是含有高度点，若是，则直接扩展地址；否则继续踢
        // 当踢的次数达到max_kick时，这时直接扩展地址
        if(kick_num > kick_times && kick_num <= max_kick)       //判断踢出来的边中含不含高度点
        {
            bool brk = false;
            // 判断cur_key_s、cur_key_d是否是高度点
            addr_type n1 = degDetector->addrQuery(key_s, 0);
            addr_type n2 = degDetector->addrQuery(key_d, 1);

            if (n1 > 2) 
            { 
                // 扩展行地址
                bool ext = degDetector->extendAddr(key_s, n1 + 1, 0);     // 将该“高度”点记录到map中
                if (!ext)
                    cout << "extend address error! " << endl;
                brk = true;
                s_extend++;
            }
            if (n2 > 2) 
            { 
                // 扩展列地址
                bool ext = degDetector->extendAddr(key_d, n2 + 1, 1);     // 将该“高度”点记录到map中
                if (!ext)
                    cout << "extend address error!" << endl;
                brk = true;
                d_extend++;
            }
            if (brk)
                break;
        }
        else if (kick_num > max_kick)
        {
            max_kick_extend++;
            // 达到了max_kick次数，这时候直接将当前顶点视为高度顶点，s, d均扩展地址
            // 扩展地址将当前fp的地址轮数加1即可
            addr_type n1 = degDetector->addrQuery(key_s, 0);   // 从degDetector中获取指纹对应的map_index
            addr_type n2 = degDetector->addrQuery(key_d, 1);   // 从degDetector中获取指纹对应的map_index

            // 扩展行地址
            bool ext1 = degDetector->extendAddr(key_s, n1 + 1, 0);    // 将该“高度”点记录到map中
            if (!ext1)
                cout << "extend address error!" << endl;
                
            // 扩展列地址
            bool ext2 = degDetector->extendAddr(key_d, n2 + 1, 1);    // 将该“高度”点记录到map中
            if (!ext2)
                cout << "extend address error!" << endl;             
            break;
        }
        
        // 找出Roomnum * row_addrs * col_addrs个备选地址中，index最小的那一个
        // uint16_t kick_index[ROOM * row_addrs * col_addrs];
        uint16_t* kick_index = new uint16_t[ROOM * row_kicks * col_kicks];
        memset(kick_index, -1, ROOM * row_kicks * col_kicks * sizeof(uint16_t));
        for (int i = 0; i < row_kicks; i++)
        {
            for (int j = 0; j < col_kicks; j++)
            {
                if ((rows[i] == pre_row) && (cols[j] == pre_col)) 
                    continue;
                uint32_t pos = rows[i] * width + cols[j];
                for (int k = 0; k < ROOM; k++)
                {
                    // kick_index[4 * i + 2 * j + k] = value[pos].ext[k];
                    kick_index[col_kicks * ROOM * i + ROOM * j + k] = value[pos].ext[k];
                    // key_type keys = recoverAddrSeed(value[pos].fp_src[k], (value[pos].ext[k] >> 8), cur_addr_row[i], this->depth);
                    // key_type keyd = recoverAddrSeed(value[pos].fp_dst[k], (value[pos].ext[k] & 0xff), cur_addr_col[j], this->width);
                }
            }
        }

        uint16_t idx = getMinIndex(kick_index, ROOM * row_kicks * col_kicks);
        if (idx == -1) {
            cout << "Error: ScubeKick:Kick_out()~line 250" << endl;
        }
        delete[] kick_index;
        uint16_t cur_i = (idx & 4) >> 2;
        uint16_t cur_j = (idx & 2) >> 1;
        uint16_t cur_k = (idx & 1);

        pre_row = rows[cur_i];
        pre_col = cols[cur_j];

        // 将准备踢出来的item放入kick_字段
        addr_type kick_pos = rows[cur_i] * width + cols[cur_j];
        fp_type kick_fp_s = value[kick_pos].fp_src[cur_k];
        fp_type kick_fp_d = value[kick_pos].fp_dst[cur_k];
        w_type kick_fp_w = value[kick_pos].weight[cur_k];
        uint16_t kick_ext = value[kick_pos].ext[cur_k];

        // 将当前待插入的item插入到矩阵中
        value[kick_pos].fp_src[cur_k] = fp_s;
        value[kick_pos].fp_dst[cur_k] = fp_d;
        value[kick_pos].weight[cur_k] = w;
        value[kick_pos].ext[cur_k] = ((ext & 0xff00) + (cur_i << 8)) | ((ext & 0xff) + cur_j);
////
        // 将踢出来的边信息记录到fp_s、fp_d、w字段，继续寻找bucket以插入
        fp_s = kick_fp_s;
        fp_d = kick_fp_d;
        w = kick_fp_w;

        //根据fp以及轮数还原addr
        addr_s = recoverAddrSeed(kick_fp_s, (kick_ext >> 8), rows[cur_i], depth);
        addr_d = recoverAddrSeed(kick_fp_d, (kick_ext & 0xff), cols[cur_j], width);

        key_s = ((addr_s << fingerprint_length) + fp_s);
        key_d = ((addr_d << fingerprint_length) + fp_d);
        idx_s = degDetector->addrQuery(key_s, 0);   // 从degDetector中获取指纹对应的map_index
        idx_d = degDetector->addrQuery(key_d, 1);   // 从degDetector中获取指纹对应的map_index
        
        addr_type* addr_rows = new addr_type[idx_s];
        addr_type* addr_cols = new addr_type[idx_d];
        // insert to the hash matrix
        int res = insert_matrix(addr_s, fp_s, addr_d, fp_d, w, idx_s, idx_d, addr_rows, addr_cols);
        if (res != -1)
        {   
            inserted = true;
        }
        // 在当前( n - (2^(ceil(log2(n - 1) - 1)) + 1) )行踢，因为正常通过DegDetector扩展的地址序列为2, 3, 5, 9, 17, 33...
        // row_kicks = (idx_s > 2) ? (idx_s - ((1 << ((int)ceil(log2(idx_s - 1))) - 1) + 1)) : 2;
        // col_kicks = (idx_d > 2) ? (idx_d - ((1 << ((int)ceil(log2(idx_d - 1))) - 1) + 1)) : 2;
        // if (row_kicks < 2)
        //     row_kicks = 2;
        // if (col_kicks < 2)
        //     col_kicks = 2;
        // if (row_kicks > 255) {
        //     cout << "Address number is bigger than 255!" << endl;
        // }
        // if (col_kicks > 255) {
        //     cout << "Address number is bigger than 255!" << endl;
        // }
        // 在当前 “最新” 的两行踢
        row_kicks = 2;
        col_kicks = 2;
        
        ext = ((idx_s - row_kicks) << 8) | (idx_d - col_kicks);
        
        for (int i = 0; i < row_kicks; i++) {
            rows[i] = addr_rows[idx_s - row_kicks + i];
        }
        for (int i = 0; i < col_kicks; i++) {
            cols[i] = addr_cols[idx_d - col_kicks + i];
        }
        delete[] addr_rows;
        delete[] addr_cols;
    }
    if (!inserted) 
    {
        addr_type* addr_rows = new addr_type[idx_s];
        addr_type* addr_cols = new addr_type[idx_d];
        // insert to the hash matrix
        int res = insert_matrix(addr_s, fp_s, addr_d, fp_d, w, idx_s, idx_d, addr_rows, addr_cols);
        if (res == -1)
        {
            // 在当前( n - (2^(ceil(log2(n - 1) - 1)) + 1) )行踢，因为正常通过DegDetector扩展的地址序列为2, 3, 5, 9, 17, 33...
            // int row_kicks = (idx_s > 2) ? (idx_s - ((1 << ((int)ceil(log2(idx_s - 1))) - 1) + 1)) : 2;
            // int col_kicks = (idx_d > 2) ? (idx_d - ((1 << ((int)ceil(log2(idx_d - 1))) - 1) + 1)) : 2;
            // if (row_kicks < 2)
            //     row_kicks = 2;
            // if (col_kicks < 2)
            //     col_kicks = 2;
            // if (row_kicks > 255) {
            //     cout << "Address number is bigger than 255!" << endl;
            // }
            // if (col_kicks > 255) {
            //     cout << "Address number is bigger than 255!" << endl;
            // }
            // 在当前row_addrs行、col_addrs列踢
            int row_kicks = 2;
            int col_kicks = 2;
            addr_type* rows = new addr_type[255];
            addr_type* cols = new addr_type[255];
            
            for (int i = 0; i < row_kicks; i++) {
                rows[i] = addr_rows[idx_s - row_kicks + i];
            }
            for (int i = 0; i < col_kicks; i++) {
                cols[i] = addr_cols[idx_d - col_kicks + i];
            }
            inserted = kick_out(addr_s, fp_s, addr_d, fp_d, w, idx_s, idx_d, rows, cols, row_kicks, col_kicks);
            delete[] rows;
            delete[] cols;
        }
        else 
        {
            inserted = true;
        }
        delete[] addr_rows;
        delete[] addr_cols;
    }
    return inserted;
}
/***********************************************/
// s is the ID of the source node, d is the ID of the destination node, w is the edge weight.
bool ScubeKick::insert(string s, string d, w_type w) 
{
    hash_type hash_s = (*hfunc[HASH])((unsigned char*)(s.c_str()), s.length());
    hash_type hash_d = (*hfunc[HASH])((unsigned char*)(d.c_str()), d.length());
    hash_type mask = (1 << fingerprint_length) - 1;

    addr_type addr_s = (hash_s >> fingerprint_length) % depth;
    addr_type addr_d = (hash_d >> fingerprint_length) % width;

    fp_type fp_s = hash_s & mask;
    fp_type fp_d = hash_d & mask;
    if(fp_s == 0) fp_s = 1;
    if(fp_d == 0) fp_d = 1;

    key_type key_s = (addr_s << fingerprint_length) + fp_s;
    key_type key_d = (addr_d << fingerprint_length) + fp_d;

    if (!this->slot2bit) 
    {
        // insert to the degDetector
        if (!degDetector->insert(key_s, key_d, hash_s, hash_d))
        {
            cout << "Failed in inserting (" << s << ", " << d << ") to degree detector!" << endl;
        }
        // query the number of alternative addresses of each node
        addr_type idx_s = degDetector->addrQuery(key_s, 0);
        addr_type idx_d = degDetector->addrQuery(key_d, 1);
        addr_type* addr_rows = new addr_type[idx_s];
        addr_type* addr_cols = new addr_type[idx_d];
        // insert to the hash matrix
        int res = insert_matrix(addr_s, fp_s, addr_d, fp_d, w, idx_s, idx_d, addr_rows, addr_cols);
        bool inserted = false;
        if (res == -1)
        {
            // 直接先开始踢kick_times轮，踢完之后再判断踢出来的点是不是高度点
            // √ 踢只在当前 “最新” 的两行踢 <先判断当前被踢的元素是否已经在矩阵中，是则直接插入，否则在“最新”的两行踢>
            // × 在当前( n - (2^(ceil(log2(n - 1) - 1)) + 1) )行踢，因为正常通过DegDetector扩展的地址序列为2, 3, 5, 9, 17, 33...
            // 在当前 “最新” 的两行踢
            // addr_type rows[2] = {addr_row[idx_s - 2], addr_row[idx_s - 1]};
            // addr_type cols[2] = {addr_col[idx_d - 2], addr_col[idx_d - 1]};
            // int row_addrs = 2;
            // int col_addrs = 2;
            //////////
            // 在当前( n - (2^(ceil(log2(n - 1) - 1)) + 1) )行踢，因为正常通过DegDetector扩展的地址序列为2, 3, 5, 9, 17, 33...
            // int row_kicks = (idx_s > 2) ? (idx_s - ((1 << ((int)ceil(log2(idx_s - 1))) - 1) + 1)) : 2;
            // int col_kicks = (idx_d > 2) ? (idx_d - ((1 << ((int)ceil(log2(idx_d - 1))) - 1) + 1)) : 2;
            // if (row_kicks < 2)
            //     row_kicks = 2;
            // if (col_kicks < 2)
            //     col_kicks = 2;
            // if (row_kicks > 255) {
            //     cout << "Address number is bigger than 255!" << endl;
            // }
            // if (col_kicks > 255) {
            //     cout << "Address number is bigger than 255!" << endl;
            // }
            int row_kicks = 2;
            int col_kicks = 2;
            addr_type* rows = new addr_type[255];
            addr_type* cols = new addr_type[255];
            
            for (int i = 0; i < row_kicks; i++) {
                rows[i] = addr_rows[idx_s - row_kicks + i];
            }
            for (int i = 0; i < col_kicks; i++) {
                cols[i] = addr_cols[idx_d - col_kicks + i];
            }
            inserted = kick_out(addr_s, fp_s, addr_d, fp_d, w, idx_s, idx_d, rows, cols, row_kicks, col_kicks);
            delete[] rows;
            delete[] cols;
            if (!inserted)
            {
                cout << "Failed in inserting edge(" << s << ", " << d << ", " << w << ")!" << endl;
            }
        }
        else
        {
            inserted = true;
        }
        delete[] addr_rows;
        delete[] addr_cols;
        return inserted;
    }
    else
    {
        // query the number of alternative addresses of each node
        addr_type idx_s = degDetector->addrQuery(key_s, 0);
        addr_type idx_d = degDetector->addrQuery(key_d, 1);
        addr_type* addr_rows = new addr_type[idx_s];
        addr_type* addr_cols = new addr_type[idx_d];
        // insert to the hash matrix
        int res = insert_matrix(addr_s, fp_s, addr_d, fp_d, w, idx_s, idx_d, addr_rows, addr_cols);
        bool inserted = false;
        if (res == 0 || res == -1) 
        {
            // update the degDetector
            if (!degDetector->insert(key_s, key_d, hash_s, hash_d))
            {
                cout << "Failed in inserting (" << s << ", " << d << ") to degree detector!" << endl;
            }
        }
        if (res == -1)
        {
            // 在当前( n - (2^(ceil(log2(n - 1) - 1)) + 1) )行踢，因为正常通过DegDetector扩展的地址序列为2, 3, 5, 9, 17, 33...
            // int row_kicks = (idx_s > 2) ? (idx_s - ((1 << ((int)ceil(log2(idx_s - 1))) - 1) + 1)) : 2;
            // int col_kicks = (idx_d > 2) ? (idx_d - ((1 << ((int)ceil(log2(idx_d - 1))) - 1) + 1)) : 2;
            // if (row_kicks < 2)
            //     row_kicks = 2;
            // if (col_kicks < 2)
            //     col_kicks = 2;
            // if (row_kicks > 255) {
            //     cout << "Address number is bigger than 255!" << endl;
            // }
            // if (col_kicks > 255) {
            //     cout << "Address number is bigger than 255!" << endl;
            // }
            // 在当前row_kicks行、col_kicks列踢
            int row_kicks = 2;
            int col_kicks = 2;
            addr_type* rows = new addr_type[255];
            addr_type* cols = new addr_type[255];
            
            for (int i = 0; i < row_kicks; i++) {
                rows[i] = addr_rows[idx_s - row_kicks + i];
            }
            for (int i = 0; i < col_kicks; i++) {
                cols[i] = addr_cols[idx_d - col_kicks + i];
            }
            inserted = kick_out(addr_s, fp_s, addr_d, fp_d, w, idx_s, idx_d, rows, cols, row_kicks, col_kicks);
            delete[] rows;
            delete[] cols;
            if (!inserted)
            {
                cout << "Failed in inserting edge(" << s << ", " << d << ", " << w << ")!" << endl;
            }
        }
        else
        {
            inserted = true;
        }
        delete[] addr_rows;
        delete[] addr_cols;
        return inserted;
    }
    return true;
}
// s is the ID of the source node, d is the ID of the destination node, w is the edge weight.
bool ScubeKick::insert(string s, string d, w_type w, double& kick_time, double& detector_ins_time) 
{
    kick_time = 0;
    detector_ins_time = 0;

    hash_type hash_s = (*hfunc[HASH])((unsigned char*)(s.c_str()), s.length());
    hash_type hash_d = (*hfunc[HASH])((unsigned char*)(d.c_str()), d.length());
    hash_type mask = (1 << fingerprint_length) - 1;

    addr_type addr_s = (hash_s >> fingerprint_length) % depth;
    addr_type addr_d = (hash_d >> fingerprint_length) % width;

    fp_type fp_s = hash_s & mask;
    fp_type fp_d = hash_d & mask;
    if(fp_s == 0) fp_s = 1;
    if(fp_d == 0) fp_d = 1;

    key_type key_s = (addr_s << fingerprint_length) + fp_s;
    key_type key_d = (addr_d << fingerprint_length) + fp_d;

    if (!this->slot2bit) 
    {
        timeval tp1, tp2;
        gettimeofday( &tp1, NULL);
        // insert to the degDetector
        if (!degDetector->insert(key_s, key_d, hash_s, hash_d))
        {
            cout << "Failed in inserting (" << s << ", " << d << ") to degree detector!" << endl;
        }
        gettimeofday( &tp2, NULL);
        detector_ins_time = (tp2.tv_sec - tp1.tv_sec) * 1000000 +  (tp2.tv_usec - tp1.tv_usec);

        // query the number of alternative addresses of each node
        addr_type idx_s = degDetector->addrQuery(key_s, 0);
        addr_type idx_d = degDetector->addrQuery(key_d, 1);
        addr_type* addr_rows = new addr_type[idx_s];
        addr_type* addr_cols = new addr_type[idx_d];
        // insert to the hash matrix
        int res = insert_matrix(addr_s, fp_s, addr_d, fp_d, w, idx_s, idx_d, addr_rows, addr_cols);
        bool inserted = false;
        if (res == -1)
        {
            // 直接先开始踢kick_times轮，踢完之后再判断踢出来的点是不是高度点
            // √ 踢只在当前 “最新” 的两行踢 <先判断当前被踢的元素是否已经在矩阵中，是则直接插入，否则在“最新”的两行踢>
            // × 在当前( n - (2^(ceil(log2(n - 1) - 1)) + 1) )行踢，因为正常通过DegDetector扩展的地址序列为2, 3, 5, 9, 17, 33...
            // 在当前 “最新” 的两行踢
            // addr_type rows[2] = {addr_row[idx_s - 2], addr_row[idx_s - 1]};
            // addr_type cols[2] = {addr_col[idx_d - 2], addr_col[idx_d - 1]};
            // int row_kicks = 2;
            // int col_kicks = 2;
            //////////
            // 在当前( n - (2^(ceil(log2(n - 1) - 1)) + 1) )行踢，因为正常通过DegDetector扩展的地址序列为2, 3, 5, 9, 17, 33...
            timeval tp3, tp4;
            gettimeofday( &tp3, NULL);
            // int row_kicks = (idx_s > 2) ? (idx_s - ((1 << ((int)ceil(log2(idx_s - 1))) - 1) + 1)) : 2;
            // int col_kicks = (idx_d > 2) ? (idx_d - ((1 << ((int)ceil(log2(idx_d - 1))) - 1) + 1)) : 2;
            // if (row_kicks < 2)
            //     row_kicks = 2;
            // if (col_kicks < 2)
            //     col_kicks = 2;
            // if (row_kicks > 255) {
            //     cout << "Address number is bigger than 255!" << endl;
            // }
            // if (col_kicks > 255) {
            //     cout << "Address number is bigger than 255!" << endl;
            // }
            int row_kicks = 2;
            int col_kicks = 2;
            addr_type* rows = new addr_type[255];
            addr_type* cols = new addr_type[255];
            
            for (int i = 0; i < row_kicks; i++) {
                rows[i] = addr_rows[idx_s - row_kicks + i];
            }
            for (int i = 0; i < col_kicks; i++) {
                cols[i] = addr_cols[idx_d - col_kicks + i];
            }
            inserted = kick_out(addr_s, fp_s, addr_d, fp_d, w, idx_s, idx_d, rows, cols, row_kicks, col_kicks);
            delete[] rows;
            delete[] cols;
            if (!inserted)
            {
                cout << "Failed in inserting edge(" << s << ", " << d << ", " << w << ")!" << endl;
            }
            gettimeofday( &tp4, NULL);
            kick_time = (tp4.tv_sec - tp3.tv_sec) * 1000000 +  (tp4.tv_usec - tp3.tv_usec);
        }
        else
        {
            inserted = true;
        }
        delete[] addr_rows;
        delete[] addr_cols;
        return inserted;
    }
    else
    {
        // query the number of alternative addresses of each node
        addr_type idx_s = degDetector->addrQuery(key_s, 0);
        addr_type idx_d = degDetector->addrQuery(key_d, 1);
        addr_type* addr_rows = new addr_type[idx_s];
        addr_type* addr_cols = new addr_type[idx_d];
        // insert to the hash matrix
        int res = insert_matrix(addr_s, fp_s, addr_d, fp_d, w, idx_s, idx_d, addr_rows, addr_cols);
        bool inserted = false;
        if (res == 0 || res == -1) 
        {
            timeval tp1, tp2;
            gettimeofday( &tp1, NULL);
            // update the degDetector
            if (!degDetector->insert(key_s, key_d, hash_s, hash_d))
            {
                cout << "Failed in inserting (" << s << ", " << d << ") to degree detector!" << endl;
            }
            gettimeofday( &tp2, NULL);
            detector_ins_time = (tp2.tv_sec - tp1.tv_sec) * 1000000 +  (tp2.tv_usec - tp1.tv_usec);
        }
        if (res == -1)
        {
            timeval tp3, tp4;
            gettimeofday( &tp3, NULL);
            // 在当前( n - (2^(ceil(log2(n - 1) - 1)) + 1) )行踢，因为正常通过DegDetector扩展的地址序列为2, 3, 5, 9, 17, 33...
            // int row_kicks = (idx_s > 2) ? (idx_s - ((1 << ((int)ceil(log2(idx_s - 1))) - 1) + 1)) : 2;
            // int col_kicks = (idx_d > 2) ? (idx_d - ((1 << ((int)ceil(log2(idx_d - 1))) - 1) + 1)) : 2;
            // if (row_kicks < 2)
            //     row_kicks = 2;
            // if (col_kicks < 2)
            //     col_kicks = 2;
            // if (row_kicks > 255) {
            //     cout << "Address number is bigger than 255!" << endl;
            // }
            // if (col_kicks > 255) {
            //     cout << "Address number is bigger than 255!" << endl;
            // }
            // 在当前row_kicks行、col_kicks列踢
            int row_kicks = 2;
            int col_kicks = 2;
            addr_type* rows = new addr_type[255];
            addr_type* cols = new addr_type[255];
            
            for (int i = 0; i < row_kicks; i++) {
                rows[i] = addr_rows[idx_s - row_kicks + i];
            }
            for (int i = 0; i < col_kicks; i++) {
                cols[i] = addr_cols[idx_d - col_kicks + i];
            }
            inserted = kick_out(addr_s, fp_s, addr_d, fp_d, w, idx_s, idx_d, rows, cols, row_kicks, col_kicks);
            delete[] rows;
            delete[] cols;
            if (!inserted)
            {
                cout << "Failed in inserting edge(" << s << ", " << d << ", " << w << ")!" << endl;
            }
            gettimeofday( &tp4, NULL);
            kick_time = (tp4.tv_sec - tp3.tv_sec) * 1000000 +  (tp4.tv_usec - tp3.tv_usec);
        }
        else
        {
            inserted = true;
        }
        delete[] addr_rows;
        delete[] addr_cols;
        return inserted;
    }
    return true;
}

w_type ScubeKick::edgeWeightQuery(string s, string d) // s1 is the ID of the source node, s2 is the ID of the destination node, return the weight of the edge
{
    hash_type hash_s = (*hfunc[HASH])((unsigned char*)(s.c_str()), s.length());
    hash_type hash_d = (*hfunc[HASH])((unsigned char*)(d.c_str()), d.length());
    hash_type mask = (1 << fingerprint_length) - 1;
    addr_type addr_s = (hash_s >> fingerprint_length) % depth;
    addr_type addr_d = (hash_d >> fingerprint_length) % width;

    fp_type fp_s = hash_s & mask;
    fp_type fp_d = hash_d & mask;
    if(fp_s == 0) fp_s = 1;
    if(fp_d == 0) fp_d = 1;

    key_type key_s = (addr_s << fingerprint_length) + fp_s;
    key_type key_d = (addr_d << fingerprint_length) + fp_d;

    // 查询当前顶点的地址数
    addr_type map_idx_s = degDetector->addrQuery(key_s, 0);   // 从degDetector中获取指纹对应的map_index
    addr_type map_idx_d = degDetector->addrQuery(key_d, 1);   // 从degDetector中获取指纹对应的map_index
    
    //  calculate the seeds
    uint32_t* seed1 = new uint32_t[map_idx_s];
    uint32_t* seed2 = new uint32_t[map_idx_d];
    seed1[0] = fp_s;
    for(int i = 1; i < map_idx_s; i++)
    {
        seed1[i] = (seed1[i - 1] * MUL + INC) % MOD;
    }
    seed2[0] = fp_d;
    for(int i = 1; i < map_idx_d; i++)
    {
        seed2[i] = (seed2[i - 1] * MUL + INC) % MOD;
    }

    //查询
    w_type res = 0;
    int hit = 0;
    for (int i = 0; i < map_idx_s; ++i)
    {
        uint32_t addr_row = (addr_s + seed1[i]) % depth;
        for (int j = 0; j < map_idx_d; ++j)
        {
            uint32_t addr_col = (addr_d + seed2[j]) % width;
            uint32_t pos = addr_row * width + addr_col;
            uint16_t idx = (i << 8) |  j;
            for (int k = 0; k < ROOM; ++k)
            {
                if((value[pos].fp_src[k] == fp_s) && (value[pos].fp_dst[k] == fp_d) && (value[pos].ext[k] == idx)) 
                {
                    // delete[] seed1;
                    // delete[] seed2;
                    // return value[pos].weight[k];
                    hit++;
                    res += value[pos].weight[k];
                }
            }
        }
    }
    delete[] seed1;
    delete[] seed2;
    // if (hit == 0 || hit > 1)
    //     cout << "hit = " << hit << endl;
    return res;
}
/* type 0 is for successor query, type 1 is for precusor query */
uint32_t ScubeKick::nodeWeightQuery(string v, int type)
{
    uint32_t sum_weight = 0;
    hash_type hash_v = (*hfunc[HASH])((unsigned char*)(v.c_str()), v.length());
    hash_type mask = (1 << fingerprint_length) - 1;
    fp_type fp_v = hash_v & mask;
    if(fp_v == 0) fp_v = 1;

    if(type == 0)
    {
        addr_type addr_v = (hash_v >> fingerprint_length) % depth;
        key_type key_v = (addr_v << fingerprint_length) + fp_v;
        // 查询当前顶点的地址数
        addr_type map_idx_s = degDetector->addrQuery(key_v, 0);   // 从degDetector中获取指纹对应的map_index
        //  calculate the seeds
        uint32_t* seed1 = new uint32_t[map_idx_s];
        seed1[0] = fp_v;
        for(int i = 1; i < map_idx_s; i++)
        {
            seed1[i] = (seed1[i - 1] * MUL + INC) % MOD;
        }
        
        for (int i = 0; i < map_idx_s; i++)
        {
            addr_type addr_row = ((addr_v + seed1[i]) % depth);
            for (int j = 0; j < width; j++)
            {
                addr_type pos = addr_row * width + j;
                for (int k = 0; k < ROOM; k++)
                {
                    if((value[pos].fp_src[k] == fp_v) && ((value[pos].ext[k] >> 8) == i)) {
                        sum_weight += value[pos].weight[k];
                    }
                }
            }
        }
        delete[] seed1;
    }
    else if (type == 1)
    {
        addr_type addr_v = (hash_v >> fingerprint_length) % width;
        key_type key_v = (addr_v << fingerprint_length) + fp_v;
        // 查询当前顶点的地址数
        addr_type map_idx_d = degDetector->addrQuery(key_v, 1);   // 从degDetector中获取指纹对应的map_index
        //  calculate the seeds
        uint32_t* seed2 = new uint32_t[map_idx_d];
        seed2[0] = fp_v;
        for(int i = 1; i < map_idx_d; i++)
        {
            seed2[i] = (seed2[i - 1] * MUL + INC) % MOD;
        }
        
        for (int i = 0; i < depth; i++)
        {
            for (int j = 0; j < map_idx_d; ++j)
            {
                addr_type addr_col = (addr_v + seed2[j]) % width;
                addr_type pos = i * width + addr_col;
                for (int k = 0; k < ROOM; k++)
                {
                    if((value[pos].fp_dst[k] == fp_v) && ((value[pos].ext[k] & 0xff) == j)) {
                        sum_weight += value[pos].weight[k];
                    }
                }
            }
        }
        delete[] seed2;
    }
    return sum_weight;
}

/* type 0 is for successor query, type 1 is for precusor query */
// v is the ID of the queried node, function for node query.
uint32_t ScubeKick::nodeWeightQuery(string v, int type, double& matrix_time, double& addr_time) 
{
    matrix_time = 0;
    addr_time = 0;

    timeval ts1, te1, ts2, te2;

    uint32_t sum_weight = 0;
    hash_type hash_v = (*hfunc[HASH])((unsigned char*)(v.c_str()), v.length());
    hash_type mask = (1 << fingerprint_length) - 1;
    fp_type fp_v = hash_v & mask;
    if(fp_v == 0) fp_v = 1;

    if(type == 0)
    {
        addr_type addr_v = (hash_v >> fingerprint_length) % depth;
        key_type key_v = (addr_v << fingerprint_length) + fp_v;
        // 查询当前顶点的地址数
        gettimeofday( &ts1, NULL);
        addr_type map_idx_s = degDetector->addrQuery(key_v, 0);   // 从degDetector中获取指纹对应的map_index
        gettimeofday( &te1, NULL);
        addr_time = (te1.tv_sec - ts1.tv_sec) * 1000000 +  (te1.tv_usec - ts1.tv_usec);
        //  calculate the seeds
        uint32_t* seed1 = new uint32_t[map_idx_s];
        seed1[0] = fp_v;
        for(int i = 1; i < map_idx_s; i++)
        {
            seed1[i] = (seed1[i - 1] * MUL + INC) % MOD;
        }
        gettimeofday( &ts2, NULL);
        for (int i = 0; i < map_idx_s; i++)
        {
            addr_type addr_row = ((addr_v + seed1[i]) % depth);
            for (int j = 0; j < width; j++)
            {
                addr_type pos = addr_row * width + j;
                for (int k = 0; k < ROOM; k++)
                {
                    if((value[pos].fp_src[k] == fp_v) && ((value[pos].ext[k] >> 8) == i)) {
                        sum_weight += value[pos].weight[k];
                    }
                }
            }
        }
        gettimeofday( &te2, NULL);
        matrix_time = (te2.tv_sec - ts2.tv_sec) * 1000000 +  (te2.tv_usec - ts2.tv_usec);
        delete[] seed1;
    }
    else if (type == 1)
    {
        addr_type addr_v = (hash_v >> fingerprint_length) % width;
        key_type key_v = (addr_v << fingerprint_length) + fp_v;
        // 查询当前顶点的地址数
        gettimeofday( &ts1, NULL);
        addr_type map_idx_d = degDetector->addrQuery(key_v, 1);   // 从degDetector中获取指纹对应的map_index
        gettimeofday( &te1, NULL);
        addr_time = (te1.tv_sec - ts1.tv_sec) * 1000000 +  (te1.tv_usec - ts1.tv_usec);
        //  calculate the seeds
        uint32_t* seed2 = new uint32_t[map_idx_d];
        seed2[0] = fp_v;
        for(int i = 1; i < map_idx_d; i++)
        {
            seed2[i] = (seed2[i - 1] * MUL + INC) % MOD;
        }
        gettimeofday( &ts2, NULL);

        for (int i = 0; i < map_idx_d; i++)
        {
            addr_type addr_col = (addr_v + seed2[i]) % width;
            for (int j = 0; j < depth; ++j)
            {
                addr_type pos = j * width + addr_col;
                for (int k = 0; k < ROOM; k++)
                {
                    if((value[pos].fp_dst[k] == fp_v) && ((value[pos].ext[k] & 0xff) == i)) {
                        sum_weight += value[pos].weight[k];
                    }
                }
            }
        }
        
        // for (int i = 0; i < depth; i++)
        // {
        //     for (int j = 0; j < map_idx_d; ++j)
        //     {
        //         addr_type addr_col = (addr_v + seed2[j]) % width;
        //         addr_type pos = i * width + addr_col;
        //         for (int k = 0; k < ROOM; k++)
        //         {
        //             if((value[pos].fp_dst[k] == fp_v) && ((value[pos].ext[k] & 0xff) == j)) {
        //                 sum_weight += value[pos].weight[k];
        //             }
        //         }
        //     }
        // }
        gettimeofday( &te2, NULL);
        matrix_time = (te2.tv_sec - ts2.tv_sec) * 1000000 +  (te2.tv_usec - ts2.tv_usec);
        delete[] seed2;
    }
    return sum_weight;
}

// s is the ID of the source node, d is the ID of the destination node, return whether reachable.
bool ScubeKick::reachabilityQuery(string s, string d)  
{
    hash_type hash_s = (*hfunc[HASH])((unsigned char*)(s.c_str()), s.length());
    hash_type hash_d = (*hfunc[HASH])((unsigned char*)(d.c_str()), d.length());
    hash_type mask = (1 << fingerprint_length) - 1;
    
    addr_type addr_s = (hash_s >> fingerprint_length) % depth;
    addr_type addr_d = (hash_d >> fingerprint_length) % width;

    fp_type fp_s = hash_s & mask;
    fp_type fp_d = hash_d & mask;
    if(fp_s == 0) fp_s = 1;
    if(fp_d == 0) fp_d = 1;

    key_type key_s = (addr_s << fingerprint_length) + fp_s;
    key_type key_d = (addr_d << fingerprint_length) + fp_d;
    
	int pos;
	map<key_type, bool> checked;
	queue<mapnode> q;
	mapnode e;
	e.addr = addr_s;
	e.fp = fp_s;
	q.push(e);
	checked[key_s] = true;
	map<unsigned int, bool>::iterator IT;

    addr_type temp_addr;
    fp_type temp_fp;
    addr_type ad_1, ad_2;

    // ad_2 = degDetector->addrQuery(key_d, 1);
    // int* tmp2 = new int[ad_2];
    // tmp2[0] = fp_d;
    // for (int i = 0; i < ad_2; i++) 
    // {
    //     tmp2[i] = (tmp2[i - 1] * MUL + INC) % MOD;
    // }

	while (!q.empty())
	{
		e = q.front();
		temp_addr = e.addr;
		temp_fp = e.fp;
        key_type temp_key = (temp_addr << fingerprint_length) + temp_fp;
        ad_1 = degDetector->addrQuery(temp_key, 0);
        ad_2 = degDetector->addrQuery(key_d, 1);

		int* tmp1 = new int[ad_1];
		int* tmp2 = new int[ad_2];
        tmp1[0] = temp_fp;
		tmp2[0] = fp_d;
		
		for (int i = 1; i < ad_1; i++)
		{
			tmp1[i] = (tmp1[i - 1] * MUL + INC) % MOD;
		}
        for (int i = 1; i < ad_2; i++) 
        {
            tmp2[i] = (tmp2[i - 1] * MUL + INC) % MOD;
        }
		for (int i1 = 0; i1 < ad_1; i1++)
		{
			int p1 = (temp_addr + tmp1[i1]) % depth;
			for (int i2 = 0; i2 < ad_2; i2++)
			{
				int p2 = (addr_d + tmp2[i2]) % width;
				int pos = p1 * width + p2;
                uint16_t idx = (i1 << 8) | i2;
				for (int i3 = 0; i3 < ROOM; i3++)
				{
					if ((value[pos].ext[i3] == idx) && (value[pos].fp_src[i3] == temp_fp) && (value[pos].fp_dst[i3] == fp_d))
					{
						delete []tmp1;
						delete []tmp2;
						return true;
					}
				}
			}
		}
// find in matrix
		for (int i1 = 0; i1 < ad_1; i1++)
		{
			int p1 = (temp_addr + tmp1[i1]) % depth;
			for (int i2 = 0; i2 < width; i2++)
			{
				int pos = p1 * width + i2;
				for (int i3 = 0; i3 < ROOM; i3++)
				{
                    if (value[pos].fp_src[i3] == temp_fp && ((value[pos].ext[i3] >> 8) == i1))
					{
						uint32_t tmp_g = value[pos].fp_dst[i3];
						int tmp_s = value[pos].ext[i3] & 0xff;
			
						uint32_t shifter = tmp_g;
						for (int v = 0; v < tmp_s; v++)
							shifter = (shifter * MUL + INC) % MOD;
						uint32_t tmp_h = i2;
						while (tmp_h < shifter)
                            tmp_h += width;				/////////
						tmp_h -= shifter;

						key_type val = (tmp_h << fingerprint_length) + tmp_g;
					
						IT = checked.find(val);
						if (IT == checked.end())
						{
							mapnode tmp_e;
							tmp_e.addr = tmp_h;
							tmp_e.fp = tmp_g;
							q.push(tmp_e);
							checked[val] = true;
						}
					}
				
				}
			}
		}
		delete[] tmp1;
		delete[] tmp2;
		q.pop();
	}
	return false;
}

// s is the ID of the source node, d is the ID of the destination node, return whether reachable.
bool ScubeKick::reachabilityQuery(string s, string d, int& points, int& lines)  
{
    hash_type hash_s = (*hfunc[HASH])((unsigned char*)(s.c_str()), s.length());
    hash_type hash_d = (*hfunc[HASH])((unsigned char*)(d.c_str()), d.length());
    hash_type mask = (1 << fingerprint_length) - 1;
    
    addr_type addr_s = (hash_s >> fingerprint_length) % depth;
    addr_type addr_d = (hash_d >> fingerprint_length) % width;

    fp_type fp_s = hash_s & mask;
    fp_type fp_d = hash_d & mask;
    if(fp_s == 0) fp_s = 1;
    if(fp_d == 0) fp_d = 1;

    key_type key_s = (addr_s << fingerprint_length) + fp_s;
    key_type key_d = (addr_d << fingerprint_length) + fp_d;
    
	int pos;
	map<key_type, bool> checked;
	queue<mapnode> q;
	mapnode e;
	e.addr = addr_s;
	e.fp = fp_s;
	q.push(e);
	checked[key_s] = true;
	map<key_type, bool>::iterator IT;

    addr_type ad_1;
    addr_type temp_addr;
    fp_type temp_fp;
    // addr_type ad_1, ad_2;

    // ad_2 = degDetector->addrQuery(key_d, 1);
    // int* tmp2 = new int[ad_2];
    // tmp2[0] = fp_d;
    // for (int i = 1; i < ad_2; i++) 
    // {
    //     tmp2[i] = (tmp2[i - 1] * MUL + INC) % MOD;
    // }

    points = 0;
    lines = 0;
	while (!q.empty())
	{
		e = q.front();
        points++;       //////////////////
		temp_addr = e.addr;
		temp_fp = e.fp;
        key_type temp_key = (temp_addr << fingerprint_length) + temp_fp;
        ad_1 = degDetector->addrQuery(temp_key, 0);
        lines += ad_1;          //////////////////
        // ad_2 = degDetector->addrQuery(key_d, 1);

		int* tmp1 = new int[ad_1];
		// int* tmp2 = new int[ad_2];
        tmp1[0] = temp_fp;
		// tmp2[0] = fp_d;
		
		for (int i = 1; i < ad_1; i++)
		{
			tmp1[i] = (tmp1[i - 1] * MUL + INC) % MOD;
		}
        // for (int i = 1; i < ad_2; i++) 
        // {
        //     tmp2[i] = (tmp2[i - 1] * MUL + INC) % MOD;
        // }
		// for (int i1 = 0; i1 < ad_1; i1++)
		// {
		// 	int p1 = (temp_addr + tmp1[i1]) % depth;
		// 	for (int i2 = 0; i2 < ad_2; i2++)
		// 	{
		// 		int p2 = (addr_d + tmp2[i2]) % width;
		// 		int pos = p1 * width + p2;
        //         uint16_t idx = (i1 << 8) | i2;
		// 		for (int i3 = 0; i3 < ROOM; i3++)
		// 		{
		// 			if ((value[pos].ext[i3] == idx) && (value[pos].fp_src[i3] == temp_fp) && (value[pos].fp_dst[i3] == fp_d))
		// 			{
		// 				delete []tmp1;
		// 				delete []tmp2;
		// 				return true;
		// 			}
		// 		}
		// 	}
		// }
// find in matrix
		for (int i1 = 0; i1 < ad_1; i1++)
		{
			int p1 = (temp_addr + tmp1[i1]) % depth;
			for (int i2 = 0; i2 < width; i2++)
			{
				int pos = p1 * width + i2;
				for (int i3 = 0; i3 < ROOM; i3++)
				{
                    if (value[pos].fp_src[i3] == temp_fp && ((value[pos].ext[i3] >> 8) == i1))
					{
						// uint32_t tmp_g = value[pos].fp_dst[i3];
                        fp_type tmp_g = value[pos].fp_dst[i3];
						int tmp_s = value[pos].ext[i3] & 0xff;
			
						uint32_t shifter = tmp_g;
						for (int v = 0; v < tmp_s; v++)
							shifter = (shifter * MUL + INC) % MOD;
						uint32_t tmp_h = i2;
						while (tmp_h < shifter)
                            tmp_h += width;                 /////////
							// tmp_h += depth;				/////////
						tmp_h -= shifter;

						key_type val = (tmp_h << fingerprint_length) + tmp_g;
                        if (val == key_d) 
                        {
                            delete []tmp1;
                            // delete []tmp2;
                            return true;
                        }
					
						IT = checked.find(val);
						if (IT == checked.end())
						{
							mapnode tmp_e;
							tmp_e.addr = tmp_h;
							tmp_e.fp = tmp_g;
							q.push(tmp_e);
							checked[val] = true;
						}
					}
				
				}
			}
		}
		delete[] tmp1;
        // delete[] tmp2;
		q.pop();
	}
    // delete[] tmp2;
    // cout << "points = " << points << ", lines = " << lines << endl;     //////////////////
    // getchar();          //////////////////
	return false;
}

void ScubeKick::printUsageInfo() 
{
    // print the deg detect info
    cout << "ScubeKick::printUsageInfo()" << endl;
    degDetector->printUsage();
    // print the matrix usage
    cout << "-----------------------Matrix-----------------------" << endl;
    uint32_t room_count = 0;
	uint32_t bucket_count = 0;
	for (uint32_t i = 0; i < width * depth; i++) 
    {
		if ((value[i].fp_src[0] != 0) && (value[i].weight[0] != 0)) 
        {
			bucket_count++;
		}
		for (uint8_t j = 0; j < ROOM; j++) 
        {
			if ((value[i].fp_src[j] != 0) && (value[i].weight[j] != 0)) 
            {
				room_count++;
			}
		}
	}
    cout << "Scube room_count = " << room_count << ", total room = " << (width * depth * ROOM) << ", space usage is " << 
			(double)room_count / (double)(width * depth * ROOM) * 100 << "%" << endl;
	cout << "Scube bucket_count = " << bucket_count << ", total bucket = " << (width * depth) << ", space usage is " << 
			(double)bucket_count / (double)(width * depth) * 100 << "%" << endl;
    cout << "----------------------------------------------------" << endl;
}

#endif // _SCUBEKICK_H
