#ifndef _DegDetector_H
#define _DegDetector_H
#include <iostream>
#include "../headers/params.h"

using namespace std;

class DegDetector
{
protected:
    struct slot 
    {
        key_type slot_key;
        slot_value_type value;
    };
public:
    // const double threshold = 0.8;            // the threshold of row/column
    const double phi = 0.77351;
    const uint16_t k_width = 0;                 // the width threshold of width (position of least 0 bit of bit vector)
    const uint16_t k_depth = 0;                 // the width threshold of depth (position of least 0 bit of bit vector)
    const double n_width = 0;
    const double n_depth = 0;
    const uint32_t matrix_width = 0;            // the width of the matrix
    const uint32_t matrix_depth = 0;            // the depth of the matrix
    DegDetector() 
    { 
        cout << "DegDetector::DegDetector()" << endl; 
    }
    DegDetector(uint32_t width, uint32_t depth): matrix_width(width), matrix_depth(depth) 
    {
        #if defined(DEBUG)
        cout << "DegDetector::DegDetector(width: " <<  width << ", depth: " << depth << ")" << endl;
        #endif
    }
    DegDetector(uint32_t width, uint32_t depth, uint16_t k_width, uint16_t k_depth): matrix_width(width), matrix_depth(depth), k_width(k_width), k_depth(k_depth), n_width((1 << k_width) / phi), n_depth((1 << k_depth) / phi)
    {
        #if defined(DEBUG)
        cout << "DegDetector::DegDetector(width: " <<  width << ", depth: " << depth << ", k_width: " << k_width << ", k_depth: " << k_depth << ")" << endl;
        #endif
        cout << "k_width = " << k_width << ", n_width = " << n_width << endl;
        cout << "k_depth = " << k_depth << ", n_depth = " << n_depth << endl;
    }
    virtual ~DegDetector() 
    {
        #if defined(DEBUG)
        cout << "DegDetector::~DegDetector()" << endl;
        #endif
    }
    // the insert process of each tuple (s, d)
    virtual bool insert(key_type s, key_type d, hash_type hash_s, hash_type hash_d) = 0;
    // return the estimation in/out degree of the node n
    virtual uint32_t degQuery(key_type key, int type) = 0;
    // return the row/column addr numbers of the node n
    virtual addr_type addrQuery(key_type key, int type) = 0;   // type = 0 for out_addr, 1 for in_addr
    virtual bool extendAddr(key_type key, addr_type n, int type) = 0;
    virtual void printUsage() = 0;
};

#endif // _DegDetector_H