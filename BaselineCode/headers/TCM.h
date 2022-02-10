#include <math.h>
#include <string>
#include <iostream>
#include <memory.h>
#include <queue>
#include "hashfunctions.h"
using namespace std;

class TCM {
private:
    const int width;
    const int depth;
    const int hashnum;
    uint32_t **value;
public:
	TCM(int width, int depth, int hashnum);
    ~TCM() {
		for (int i = 0; i < hashnum; ++i) {
            delete [] value[i];
        }
        delete [] value;
    }
	void insert(string src, string dst, uint32_t weight);
	uint32_t edgeWeightQuery(string src, string dst) const;
	uint32_t nodeWeightQuery(string v, int type) const;
	bool reachabilityQuery(string s1, string s2);
	void countBuffer() {}
};
TCM::TCM(int width, int depth, int hashnum): width(width), depth(depth), hashnum(hashnum) {
	this->value = new uint32_t*[hashnum];
	for (int i = 0; i < hashnum; ++i) {
        value[i] = new uint32_t[width * depth];
        memset(value[i], 0, sizeof(uint32_t) * width * depth);
    }
}
void TCM::insert(string src, string dst, uint32_t weight) {
	for (int i = 0; i < hashnum; i++) {
		uint32_t hash1 = ((*hfunc[i])((unsigned char*)(src.c_str()), src.length()));
		uint32_t hash2 = ((*hfunc[i])((unsigned char*)(dst.c_str()), dst.length()));

		hash1 = hash1 % depth;
		hash2 = hash2 % width;
		value[i][hash1 * width + hash2] += weight;
	}
}
uint32_t TCM::edgeWeightQuery(string src, string dst) const {
	uint32_t min = UINT32_MAX;
    for(int i = 0; i < hashnum; i++) {
    	uint32_t hash1 = ((*hfunc[i])((unsigned char*)(src.c_str()), src.length()));
		uint32_t hash2 = ((*hfunc[i])((unsigned char*)(dst.c_str()), dst.length()));

		hash1 = hash1 % depth;
		hash2 = hash2 % width;
    	uint32_t v = value[i][hash1 * width + hash2];
    	if(v < min)
    		min = v;
	}
    return min;
}
uint32_t TCM::nodeWeightQuery(string v, int type) const {
	uint32_t min = UINT32_MAX;
	if (type == 0) {
		for (int i = 0; i < hashnum; i++) {
			uint32_t hash = ((*hfunc[i])((unsigned char*)(v.c_str()), v.length())) % depth;
			uint32_t sum = 0;
			for (int j = 0; j < width; j++) {
				sum += value[i][hash * width + j];
			}
			if (sum < min)
				min = sum;
			if (min == 0)
				break;
		}
	}
	else {
		for (int i = 0; i < hashnum; i++) {
			uint32_t hash = ((*hfunc[i])((unsigned char*)(v.c_str()), v.length())) % width;
			int sum = 0;
			for (int j = 0; j < depth; j++) {
				sum += value[i][j * width + hash];			
			}
			if (sum < min)
				min = sum;
			if (min == 0)
				break;
		}
	}
	return min;
}

// const unsigned char*v1, const unsigned char*v2, int len1,int  len2
bool TCM::reachabilityQuery(string s1, string s2) 
{
	for(int i = 0; i < hashnum; i++) {
		bool found = false;
		bool checked[width];
		for(int j = 0; j < width; j++)
			checked[j] = false;
		unsigned int src = ((*hfunc[i])((unsigned char*)(s1.c_str()), s1.length())) % depth;
		unsigned int dest = ((*hfunc[i])((unsigned char*)(s2.c_str()), s2.length())) % width;
		queue<unsigned int> q;
		unsigned int v1 = src;
		unsigned int v2 = dest;
		int hash = v1 * width + v2;
		if(value[i][hash] > 0)
			continue;
		else {
			q.push(v1);
			checked[v1] = true; 
			while(!q.empty()) {
				v1 = q.front();
				if(value[i][v1 * width + dest] > 0) {
					//	cout<<"found"<<endl;
					found = true;
					break;
				}
				for(int j = 0; j < width; j++) {
					hash = v1 * width + j;
					if((value[i][hash] > 0) && (checked[j] == false)) {
						q.push(j);
						//	cout<<"in!";
						checked[j] = true;
					}
				}
				q.pop();
			}
			if(!found) {
				return false;
			}
		}
	}
	return true;
}