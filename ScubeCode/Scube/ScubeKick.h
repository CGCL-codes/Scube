#ifndef _ScubeKick_H
#define _ScubeKick_H
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
#include <sys/time.h>
#include "Scube.h"
#include "../headers/HashFunction.h"
#include "../headers/params.h"
#include "../DegDetector/DegDetectorSlot.h"


#ifdef H0
#define HASH 0
#elif H1
#define HASH 1
#elif H2
#define HASH 2
#elif H3
#define HASH 3
#elif H4
#define HASH 4
#elif H5
#define HASH 5
#else
#define HASH 6
#endif

using namespace std;

class ScubeKick: public Scube
{
private:
    struct mapnode
    {
        addr_type addr;
        fp_type fp;
    };
    int n = 0;
    const uint8_t kick_times = 10;
    const uint32_t slot_num = 1000;
    basket* value = NULL;
    DegDetector* degDetector = NULL;

    uint32_t s_extend = 0, d_extend = 0, max_kick_extend = 0;

public:
    // DegSlot
    ScubeKick(uint32_t width, uint32_t depth, uint8_t fingerprint_length, uint16_t k_width, uint16_t k_depth, uint32_t kick_times, uint32_t slot_num, int degSlothash);
    ~ScubeKick();
    bool insert(string s, string d, w_type w);
    bool insert(string s, string d, w_type w, double& kick_time, double& detector_ins_time);
    w_type edgeWeightQuery(string s, string d);
    uint32_t nodeWeightQuery(string s, int type);
    uint32_t nodeWeightQueryTime(string v, int type, double& matrix_time, double& addr_time);
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

    bool kick_out(key_type& cur_key_s, key_type& cur_key_d, w_type& cur_w, addr_type* cur_addr_row, addr_type* cur_addr_col, int& row_addrs, int& col_addrs);
    bool fp_insert(key_type key_s, key_type key_d, w_type w);
    bool fp_insert(key_type key_s, key_type key_d, w_type w, double& kick_time, double& dector_query_time);
};
ScubeKick::ScubeKick(uint32_t width, uint32_t depth, uint8_t fingerprint_length, uint16_t k_width, uint16_t k_depth, uint32_t kick_times, uint32_t slot_num):
Scube(width, depth, fingerprint_length), kick_times(kick_times) 
{
#if defined(DEBUG)
    cout << "ScubeKick::ScubeKick(width: " <<  width << ", depth: " << depth << ", fplen: " << (uint32_t) fingerprint_length << ", k_width: " << k_width << ", k_depth: " << k_depth << ", kick_times: " << kick_times << ", slot_num: " << slot_num << ")" << endl;
#endif
    s_extend = 0, d_extend = 0, max_kick_extend = 0;
    // this->degDetector = new DegDetectorSlot(width, depth, threshold, slot_num);
    this->degDetector = new DegDetectorSlot(width, depth, k_width, k_depth, slot_num);
    // this->value = new basket[width * depth];
    posix_memalign((void**)&(this->value), 64, sizeof(basket) * width * depth);		// 64-byte alignment of the requested space
    memset(value, 0, sizeof(basket) * width * depth);
}

ScubeKick::~ScubeKick()
{
#if defined(DEBUG)
	cout << "ScubeKick::~ScubeKick()" << endl;
#endif
    delete degDetector;
    delete[] value;
}

/***************private functions***************/
// s is the ID of the source node, d is the ID of the destination node, w is the edge weight.
int ScubeKick::getMinIndex(uint16_t * a, int length) 
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

bool ScubeKick::kick_out(key_type& cur_key_s, key_type& cur_key_d, w_type& cur_w, addr_type* cur_addr_row, addr_type* cur_addr_col, int& row_addrs, int& col_addrs)
{
    addr_type map_idx_s = degDetector->addrQuery(cur_key_s, 0);
    addr_type map_idx_d = degDetector->addrQuery(cur_key_d, 1);

    addr_type mask = (1 << fingerprint_length) - 1;
    fp_type cur_fp_s = cur_key_s & mask;
    fp_type cur_fp_d = cur_key_d & mask;
    addr_type cur_addr_s = cur_key_s >> fingerprint_length;
    addr_type cur_addr_d = cur_key_d >> fingerprint_length;
    uint16_t cur_ext = ((map_idx_s - row_addrs) << 8) | (map_idx_d - col_addrs);
    // uint16_t cur_ext = ((map_idx_s - 2) << 8) | (map_idx_d - 2);

    int max_kick = 1000;
    bool inserted = false;
    uint32_t pre_row = -1, pre_col = -1;
    // key_type ks[1000], kd[1000];
    
    for (int kick_num = 0; !inserted; kick_num++)
    {
        // ks[kick_num] = cur_key_s;
        // kd[kick_num] = cur_key_d;
        if(kick_num > kick_times && kick_num <= max_kick)
        {
            bool brk = false;
            addr_type n1 = degDetector->addrQuery(cur_key_s, 0);
            addr_type n2 = degDetector->addrQuery(cur_key_d, 1);

            if (n1 > 2) 
            { 
                bool ext = degDetector->extendAddr(cur_key_s, n1 + 1, 0);
                if (!ext)
                    cout << "extend address error! " << endl;
                brk = true;
                s_extend++;
            }
            if (n2 > 2) 
            { 
                bool ext = degDetector->extendAddr(cur_key_d, n2 + 1, 1);
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
            addr_type n1 = degDetector->addrQuery(cur_key_s, 0); 
            addr_type n2 = degDetector->addrQuery(cur_key_d, 1);

            bool ext1 = degDetector->extendAddr(cur_key_s, n1 + 1, 0);
            if (!ext1)
                cout << "extend address error!" << endl;
                
            bool ext2 = degDetector->extendAddr(cur_key_d, n2 + 1, 1);
            if (!ext2)
                cout << "extend address error!" << endl;             
            break;
        }
        
        // uint16_t kick_index[ROOM * row_addrs * col_addrs];
        uint16_t* kick_index = new uint16_t[ROOM * row_addrs * col_addrs];
        memset(kick_index, -1, ROOM * row_addrs * col_addrs * sizeof(uint16_t));
        for (int i = 0; i < row_addrs; i++)
        {
            for (int j = 0; j < col_addrs; j++)
            {
                if ((cur_addr_row[i] == pre_row) && (cur_addr_col[j] == pre_col)) 
                    continue;
                uint32_t pos = cur_addr_row[i] * width + cur_addr_col[j];
                for (int k = 0; k < ROOM; k++)
                {
                    // kick_index[4 * i + 2 * j + k] = value[pos].ext[k];
                    kick_index[col_addrs * ROOM * i + ROOM * j + k] = value[pos].ext[k];
                    // key_type keys = recoverAddrSeed(value[pos].fp_src[k], (value[pos].ext[k] >> 8), cur_addr_row[i], this->depth);
                    // key_type keyd = recoverAddrSeed(value[pos].fp_dst[k], (value[pos].ext[k] & 0xff), cur_addr_col[j], this->width);
                }
            }
        }

        uint16_t idx = getMinIndex(kick_index, ROOM * row_addrs * col_addrs);
        if (idx == -1) {
            cout << "Error: ScubeKick:Kick_out()~line 250" << endl;
        }
        delete[] kick_index;
        uint16_t cur_i = (idx & 4) >> 2;
        uint16_t cur_j = (idx & 2) >> 1;
        uint16_t cur_k = (idx & 1);

        pre_row = cur_addr_row[cur_i];
        pre_col = cur_addr_col[cur_j];

        addr_type kick_pos = cur_addr_row[cur_i] * width + cur_addr_col[cur_j];
        fp_type kick_fp_s = value[kick_pos].fp_src[cur_k];
        fp_type kick_fp_d = value[kick_pos].fp_dst[cur_k];
        w_type kick_fp_w = value[kick_pos].weight[cur_k];
        uint16_t kick_ext = value[kick_pos].ext[cur_k];

        value[kick_pos].fp_src[cur_k] = cur_fp_s;
        value[kick_pos].fp_dst[cur_k] = cur_fp_d;
        value[kick_pos].weight[cur_k] = cur_w;
        value[kick_pos].ext[cur_k] = ((cur_ext & 0xff00) + (cur_i << 8)) | ((cur_ext & 0xff) + cur_j);

        cur_fp_s = kick_fp_s;
        cur_fp_d = kick_fp_d;
        cur_w = kick_fp_w;
        // cur_ext = kick_ext;
        uint16_t cur_idx_s = kick_ext >> 8;
        uint16_t cur_idx_d = kick_ext & 0xff;

        cur_addr_s = recoverAddrSeed(cur_fp_s, cur_idx_s, cur_addr_row[cur_i], depth);
        cur_addr_d = recoverAddrSeed(cur_fp_d, cur_idx_d, cur_addr_col[cur_j], width);

        cur_key_s = (cur_addr_s << fingerprint_length) + cur_fp_s;
        cur_key_d = (cur_addr_d << fingerprint_length) + cur_fp_d;

        addr_type cur_map_idx_s = degDetector->addrQuery(cur_key_s, 0);
        addr_type cur_map_idx_d = degDetector->addrQuery(cur_key_d, 1);
        
        // cur_ext = ((cur_map_idx_s - 2) << 8) | (cur_map_idx_d - 2);
        uint16_t addr_no = kick_ext;
////
        // calculate all alternate addresses
        // calculate all the seeds
        uint32_t* seed_s = new uint32_t[cur_map_idx_s];
        uint32_t* seed_d = new uint32_t[cur_map_idx_d];
        seed_s[0] = cur_fp_s;
        seed_d[0] = cur_fp_d;
        for(int i = 1; i < cur_map_idx_s; i++)
            seed_s[i] = (seed_s[i - 1] * MUL + INC) % MOD;
        for(int i = 1; i < cur_map_idx_d; i++)
            seed_d[i] = (seed_d[i - 1] * MUL + INC) % MOD;
    
        row_addrs = (cur_map_idx_s > 2) ? (cur_map_idx_s - ((1 << ((int)ceil(log2(cur_map_idx_s - 1))) - 1) + 1)) : 2;
        col_addrs = (cur_map_idx_d > 2) ? (cur_map_idx_d - ((1 << ((int)ceil(log2(cur_map_idx_d - 1))) - 1) + 1)) : 2;
        if (row_addrs < 2)
            row_addrs = 2;
        if (col_addrs < 2)
            col_addrs = 2;

        cur_ext = ((cur_map_idx_s - row_addrs) << 8) | (cur_map_idx_d - col_addrs);

        if (row_addrs > 255) {
            // delete[] cur_addr_row;
            // cur_addr_row = new addr_type[row_addrs];
            cout << "Address number is bigger than 255!" << endl;
        }
        if (col_addrs > 255) {
            // delete[] cur_addr_col;
            // cur_addr_col = new addr_type[col_addrs];
            cout << "Address number is bigger than 255!" << endl;
        }
        for (int i = 0; i < row_addrs; i++) {
            cur_addr_row[i] = (cur_addr_s + seed_s[cur_map_idx_s - row_addrs + i]) % depth;
        }
        for (int i = 0; i < col_addrs; i++) {
            cur_addr_col[i] = (cur_addr_d + seed_d[cur_map_idx_d - col_addrs + i]) % width;
        }
        // cur_addr_row[0] = (cur_addr_s + seed_s[cur_map_idx_s - 2]) % depth;
        // cur_addr_row[1] = (cur_addr_s + seed_s[cur_map_idx_s - 1]) % depth;
        // cur_addr_col[0] = (cur_addr_d + seed_d[cur_map_idx_d - 2]) % width;
        // cur_addr_col[1] = (cur_addr_d + seed_d[cur_map_idx_d - 1]) % width;


        bool empty = false;
        uint16_t temp_idx = 0;
        int temp_k = -1;
        addr_type temp_pos = 0;
        for (int i = 0; i < cur_map_idx_s; i++)
        {
            addr_type row_addr = (cur_addr_s + seed_s[i]) % depth;
            for (int j = 0; j < cur_map_idx_d; j++)
            {
                addr_type col_addr = (cur_addr_d + seed_d[j]) % width;
                addr_type pos = row_addr * width + col_addr;
                uint16_t idx = (i << 8) |  j;
                for (int k = 0; k < ROOM; ++k)
                {
                    if((value[pos].fp_src[k] == cur_fp_s) && (value[pos].fp_dst[k] == cur_fp_d) && (value[pos].ext[k] == idx)) 
                    {
                        value[pos].weight[k] += cur_w;
                        inserted = true;
                        break;
                    }
                    if(value[pos].fp_src[k] == 0 && !empty) // find an empty room
                    {
                        empty = true;
                        temp_idx = idx;
                        temp_k = k;
                        temp_pos = pos;
                    }
                }
                if(inserted)
                    break;
            }
            if(inserted)
                break;
        }
        if(!inserted && empty) 
        {
            value[temp_pos].ext[temp_k] = temp_idx;
            value[temp_pos].fp_src[temp_k] = cur_fp_s;
            value[temp_pos].fp_dst[temp_k] = cur_fp_d;
            value[temp_pos].weight[temp_k] = cur_w;
            inserted = true;
        }
        delete[] seed_s;
        delete[] seed_d;
    }
    return inserted;
}
bool ScubeKick::fp_insert(key_type key_s, key_type key_d, w_type w)
{
    // query the number of alternative addresses of each node
    addr_type map_idx_s = degDetector->addrQuery(key_s, 0);
    addr_type map_idx_d = degDetector->addrQuery(key_d, 1);

    hash_type mask = (1 << fingerprint_length) - 1;
    fp_type fp_s = key_s & mask;
    fp_type fp_d = key_d & mask;
    addr_type addr_s = key_s >> fingerprint_length;
    addr_type addr_d = key_d >> fingerprint_length;

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

    addr_type* addr_row = new addr_type[map_idx_s];
    addr_type* addr_col = new addr_type[map_idx_d];
    bool inserted = false;
    bool empty = false;
    uint16_t temp_idx = 0;
    int temp_k = -1;
    addr_type temp_pos = 0;
    for (int i = 0; i < map_idx_s; i++)
    {
        addr_row[i] = (addr_s + seed1[i]) % depth;
        for (int j = 0; j < map_idx_d; j++)
        {
            addr_col[j] = (addr_d + seed2[j]) % width;
            addr_type pos = addr_row[i] * width + addr_col[j];
            uint16_t idx = (i << 8) |  j;
            for (int k = 0; k < ROOM; ++k)
            {
                if((value[pos].fp_src[k] == fp_s) && (value[pos].fp_dst[k] == fp_d) && (value[pos].ext[k] == idx)) 
                {
                    value[pos].weight[k] += w;
                    inserted = true;
                    break;
                }
                if(value[pos].fp_src[k] == 0 && !empty) // find an empty room
                {
                    empty = true;
                    temp_idx = idx;
                    temp_k = k;
                    temp_pos = pos;
                }
            }
            if(inserted)
                break;
        }
        if(inserted)
            break;
    }
    if(!inserted && empty) 
    {
        value[temp_pos].ext[temp_k] = temp_idx;
        value[temp_pos].fp_src[temp_k] = fp_s;
        value[temp_pos].fp_dst[temp_k] = fp_d;
        value[temp_pos].weight[temp_k] = w;
        inserted = true;
    }
    if (!inserted)
    {
        // kick-out strategy
        key_type cur_key_s = key_s;
        key_type cur_key_d = key_d;
        w_type cur_w = w;

        // addr_type cur_addr_row[2] = {addr_row[map_idx_s - 2], addr_row[map_idx_s - 1]};
        // addr_type cur_addr_col[2] = {addr_col[map_idx_d - 2], addr_col[map_idx_d - 1]};
        // inserted = kick_out(cur_key_s, cur_key_d, cur_w, cur_addr_row, cur_addr_col);
        addr_type* cur_addr_row = new addr_type[255];
        addr_type* cur_addr_col = new addr_type[255];

        int row_addrs = (map_idx_s > 2) ? (map_idx_s - ((1 << ((int)ceil(log2(map_idx_s - 1))) - 1) + 1)) : 2;
        int col_addrs = (map_idx_d > 2) ? (map_idx_d - ((1 << ((int)ceil(log2(map_idx_d - 1))) - 1) + 1)) : 2;
        if (row_addrs < 2)
            row_addrs = 2;
        if (col_addrs < 2)
            col_addrs = 2;
            
        // addr_type* cur_addr_row = new addr_type[row_addrs];
        // addr_type* cur_addr_col = new addr_type[col_addrs];
        if (row_addrs > 255) {
            cout << "Address number is bigger than 255!" << endl;
            // delete[] cur_addr_row;
            // cur_addr_row = new addr_type[row_addrs];
        }
        if (col_addrs > 255) {
            // delete[] cur_addr_col;
            // cur_addr_col = new addr_type[col_addrs];
            cout << "Address number is bigger than 255!" << endl;
        }

        for (int i = 0; i < row_addrs; i++) {
            cur_addr_row[i] = addr_row[map_idx_s - row_addrs + i];
        }
        for (int i = 0; i < col_addrs; i++) {
            cur_addr_col[i] = addr_col[map_idx_d - col_addrs + i];
        }
        inserted = kick_out(cur_key_s, cur_key_d, cur_w, cur_addr_row, cur_addr_col, row_addrs, col_addrs);
        delete[] cur_addr_row;
        delete[] cur_addr_col;
        if (!inserted)
        {
            // continually insert
            bool inserted = fp_insert(cur_key_s, cur_key_d, cur_w);
            if (!inserted)
                cout << "You got a BIG problem!" << endl;
        }
    }
    delete[] seed1;
    delete[] seed2;
    delete[] addr_row;
    delete[] addr_col;
    return true;
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

    // insert to the degDetector
    degDetector->insert(key_s, key_d, hash_s, hash_d);
    // insert to the hash matrix
    bool inserted = fp_insert(key_s, key_d, w);
    if (!inserted)
        cout << "You got a problem!" << endl;
    return inserted;
}

bool ScubeKick::insert(string s, string d, w_type w, double& kick_time, double& detector_ins_time) {
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

    timeval tp1, tp2;
    gettimeofday( &tp1, NULL);
    // insert to the degDetector
    degDetector->insert(key_s, key_d, hash_s, hash_d);
    // insert to the hash matrix
    gettimeofday( &tp2, NULL);
    detector_ins_time = (tp2.tv_sec - tp1.tv_sec) * 1000000 +  (tp2.tv_usec - tp1.tv_usec);

    bool inserted = fp_insert(key_s, key_d, w, kick_time, detector_ins_time);
    if (!inserted)
        cout << "You got a problem!" << endl;
    return inserted;
}
bool ScubeKick::fp_insert(key_type key_s, key_type key_d, w_type w, double& kick_time, double& dector_query_time)
{
    // query the number of alternative addresses of each node
    addr_type map_idx_s = degDetector->addrQuery(key_s, 0);
    addr_type map_idx_d = degDetector->addrQuery(key_d, 1);

    hash_type mask = (1 << fingerprint_length) - 1;
    fp_type fp_s = key_s & mask;
    fp_type fp_d = key_d & mask;
    addr_type addr_s = key_s >> fingerprint_length;
    addr_type addr_d = key_d >> fingerprint_length;

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

    addr_type* addr_row = new addr_type[map_idx_s];
    addr_type* addr_col = new addr_type[map_idx_d];
    bool inserted = false;
    bool empty = false;
    uint16_t temp_idx = 0;
    int temp_k = -1;
    addr_type temp_pos = 0;
    for (int i = 0; i < map_idx_s; i++)
    {
        addr_row[i] = (addr_s + seed1[i]) % depth;
        for (int j = 0; j < map_idx_d; j++)
        {
            addr_col[j] = (addr_d + seed2[j]) % width;
            addr_type pos = addr_row[i] * width + addr_col[j];
            uint16_t idx = (i << 8) |  j;
            for (int k = 0; k < ROOM; ++k)
            {
                if((value[pos].fp_src[k] == fp_s) && (value[pos].fp_dst[k] == fp_d) && (value[pos].ext[k] == idx)) 
                {
                    value[pos].weight[k] += w;
                    inserted = true;
                    break;
                }
                if(value[pos].fp_src[k] == 0 && !empty) // find an empty address
                {
                    empty = true;
                    temp_idx = idx;
                    temp_k = k;
                    temp_pos = pos;
                }
            }
            if(inserted)
                break;
        }
        if(inserted)
            break;
    }
    if(!inserted && empty) 
    {
        value[temp_pos].ext[temp_k] = temp_idx;
        value[temp_pos].fp_src[temp_k] = fp_s;
        value[temp_pos].fp_dst[temp_k] = fp_d;
        value[temp_pos].weight[temp_k] = w;
        inserted = true;
    }
    
    if (!inserted)
    {
        timeval tp3, tp4;
        gettimeofday( &tp3, NULL);
        // kick-out strategy
        key_type cur_key_s = key_s;
        key_type cur_key_d = key_d;
        w_type cur_w = w;

        // addr_type cur_addr_row[2] = {addr_row[map_idx_s - 2], addr_row[map_idx_s - 1]};
        // addr_type cur_addr_col[2] = {addr_col[map_idx_d - 2], addr_col[map_idx_d - 1]};
        // inserted = kick_out(cur_key_s, cur_key_d, cur_w, cur_addr_row, cur_addr_col);
        addr_type* cur_addr_row = new addr_type[255];
        addr_type* cur_addr_col = new addr_type[255];

        int row_addrs = (map_idx_s > 2) ? (map_idx_s - ((1 << ((int)ceil(log2(map_idx_s - 1))) - 1) + 1)) : 2;
        int col_addrs = (map_idx_d > 2) ? (map_idx_d - ((1 << ((int)ceil(log2(map_idx_d - 1))) - 1) + 1)) : 2;
        if (row_addrs < 2)
            row_addrs = 2;
        if (col_addrs < 2)
            col_addrs = 2;
            
        // addr_type* cur_addr_row = new addr_type[row_addrs];
        // addr_type* cur_addr_col = new addr_type[col_addrs];
        if (row_addrs > 255) {
            cout << "Address number is bigger than 255!" << endl;
            // delete[] cur_addr_row;
            // cur_addr_row = new addr_type[row_addrs];
        }
        if (col_addrs > 255) {
            // delete[] cur_addr_col;
            // cur_addr_col = new addr_type[col_addrs];
            cout << "Address number is bigger than 255!" << endl;
        }

        for (int i = 0; i < row_addrs; i++) {
            cur_addr_row[i] = addr_row[map_idx_s - row_addrs + i];
        }
        for (int i = 0; i < col_addrs; i++) {
            cur_addr_col[i] = addr_col[map_idx_d - col_addrs + i];
        }
        inserted = kick_out(cur_key_s, cur_key_d, cur_w, cur_addr_row, cur_addr_col, row_addrs, col_addrs);
        delete[] cur_addr_row;
        delete[] cur_addr_col;
        
        if (!inserted)
        {
            // continually insert
            bool inserted = fp_insert(cur_key_s, cur_key_d, cur_w);
            if (!inserted)
                cout << "You got a BIG problem!" << endl;
        }
        gettimeofday( &tp4, NULL);
        kick_time = (tp4.tv_sec - tp3.tv_sec) * 1000000 +  (tp4.tv_usec - tp3.tv_usec);
    }
    delete[] seed1;
    delete[] seed2;
    delete[] addr_row;
    delete[] addr_col;
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

    // query the address number of the current node
    addr_type map_idx_s = degDetector->addrQuery(key_s, 0);
    addr_type map_idx_d = degDetector->addrQuery(key_d, 1);
    
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

    // query
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
uint32_t ScubeKick::nodeWeightQuery(string v, int type) // s1 is the ID of the queried node, function for node query.
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
        // query the address number of the current node
        addr_type map_idx_s = degDetector->addrQuery(key_v, 0);
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
        // query the address number of the current node
        addr_type map_idx_d = degDetector->addrQuery(key_v, 1);
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
uint32_t ScubeKick::nodeWeightQueryTime(string v, int type, double& matrix_time, double& addr_time) // s1 is the ID of the queried node, function for node query.
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
        // query the address number of the current node
        gettimeofday( &ts1, NULL);
        addr_type map_idx_s = degDetector->addrQuery(key_v, 0);
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
        // query the address number of the current node
        gettimeofday( &ts1, NULL);
        addr_type map_idx_d = degDetector->addrQuery(key_v, 1);
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

bool ScubeKick::reachabilityQuery(string s, string d)  // s1 is the ID of the source node, s2 is the ID of the destination node, return whether reachable.
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

bool ScubeKick::reachabilityQuery(string s, string d, int& points, int& lines)  // s1 is the ID of the source node, s2 is the ID of the destination node, return whether reachable.
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

#endif // _ScubeKick_H
