#ifndef _Scube_H
#define _Scube_H
#include <iostream>
#include <vector>
#include <string>
#include "../headers/params.h"

#include <sys/time.h>

using namespace std;

struct basket
{
    fp_type fp_src[ROOM];
    fp_type fp_dst[ROOM];
    w_type weight[ROOM];
    uint16_t ext[ROOM];
};

struct node
{
    key_type key;
    w_type weight;
};

class findx 
{
public:
	findx(const key_type va) { value = va; }
	bool operator()(const vector<node>::value_type &nod) 
    {
		if (nod.key == value)
			return true;
		else
			return false;
	}
private:
	key_type value;
};

class Scube
{
protected:    
    const uint32_t width, depth;
    const uint16_t fingerprint_length;
public:
    Scube(uint32_t width, uint32_t depth, uint16_t fingerprint_length):
    width(width), depth(depth), fingerprint_length(fingerprint_length)
    {
        #if defined(DEBUG)
        cout << "Scube::Scube(width: " << width << ", depth: " << depth << ", fplen: " << fingerprint_length << ")" << endl;
        #endif
    }
    virtual ~Scube() 
    {
        #if defined(DEBUG)
        cout << "Scube::~Scube()" << endl;
        #endif
    }
    virtual bool insert(string s, string d, w_type w) = 0;
    virtual w_type edgeWeightQuery(string s, string d) = 0;
    virtual uint32_t nodeWeightQuery(string s, int type) = 0;
    virtual bool reachabilityQuery(string s, string d) {
        return false;
    }virtual bool reachabilityQuery(string s, string d, int& points, int& lines) {
        return false;
    }
    virtual void printUsageInfo() = 0;
    virtual void printExtendAddrInfo()
    {
        cout << "Scube::printExtendAddrInfo()" << endl;
    }

    virtual bool insert(string s, string d, w_type w, double& kick_time, double& detector_ins_time) {
        kick_time = 0;
        detector_ins_time = 0;
        return false;
    }
};

#endif // _Scube_H