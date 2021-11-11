#include<iostream>
#include<string>
#include<vector>
#include<queue>
#include<set>
#include<map>
#include<cmath>
#include<stdlib.h>
#include<bitset>
#include<memory.h>
#include "HashFunction.h"
using namespace std;
#define prime 739
#define bigger_p 1048576
#define timer 5
#define M 80000

#define Roomnum 2 // This is the parameter to controll the maximum number of rooms in a bucket. 

struct basket {
	uint16_t src[Roomnum];
	uint16_t dst[Roomnum];
	int16_t  weight[Roomnum];
	uint32_t idx;
};
struct mapnode {
	unsigned int h;
	unsigned short g;
};
struct linknode {
	unsigned int key;
	short weight;
	linknode* next;
};

class GSS {
private:
	int w;
	int r;
	int p;
	int s;
	int f;	
	basket* value;
public:
	vector<linknode*> buffer;
	map<unsigned int, int> index;
	int n;
	GSS(int width, int range, int p_num, int size,int f_num);
	~GSS() {
		delete[] value;
		cleanupBuffer();
	}
	void insert(string s1, string s2,int weight);
	void cleanupBuffer();
	int edgeWeightQuery(string s1, string s2);  		// s1 is the ID of the source node, s2 is the ID of the destination node, return the weight of the edge
	int nodeWeightQuery(string s1, int type); 			//src_type = 0 dst_type = 1
	void countBuffer();
};

void GSS::countBuffer() {
	 // print the matrix usage
    uint32_t room_count = 0;
	uint32_t bucket_count = 0;
	for (uint32_t i = 0; i < (this->w * this->w); i++) {
		if ((this->value[i].src[0] != 0) && (this->value[i].weight[0] != 0)) {
			bucket_count++;
		}
		for (uint32_t j = 0; j < Roomnum; j++) {
			if ((this->value[i].src[j] != 0) && (this->value[i].weight[j] != 0)) {
				room_count++;
			}
		}
	}
	cout << "---------------------------------------" << endl;
	cout << "GSS room_count = " << room_count << ", total room = " << (this->w * this->w * Roomnum) << ", space usage is " << 
			(double)room_count / (double)(this->w * this->w * Roomnum) * 100 << "%" << endl;
	cout << "GSS bucket_count = " << bucket_count << ", total bucket = " << (this->w * this->w) << ", space usage is " << 
			(double)bucket_count / (double)(this->w * this->w) * 100 << "%" << endl;

	cout << "First dim of buffer = " << this->buffer.size() << endl;
	uint32_t max_second_dim_len = 0;
	uint32_t total_second_dim_len = 0;
	
	for (vector<linknode*>::iterator iter = buffer.begin(); iter != buffer.end(); iter++) {
		linknode* ptr = *iter;
		uint len = 0;
		while (ptr != NULL) {
			len++;
			linknode* tmp = ptr->next;
			ptr = tmp;
		}
		if (len > max_second_dim_len) {
			max_second_dim_len = len;
		}
		total_second_dim_len += len;
	}
	cout << "Second dim (total) of buffer = " << total_second_dim_len << endl;
	cout << "Max second dim len = " << max_second_dim_len << endl;
	return;
}

GSS::GSS(int width, int range, int p_num, int size,int f_num) {
	w = width;	/* the side length of matrix */
	r = range; 	/* r x r mapped baskets */
	p = p_num; 	/* candidate buckets */
	s = size; 	/* multiple rooms */
	f = f_num; 	/* finger print length */
	n = 0;
	value = new basket[w * w];
	memset(value, 0, sizeof(basket) * w * w);
}
void GSS::cleanupBuffer() {
	vector<linknode*>::iterator IT = buffer.begin();
	linknode* e, *tmp;
	for (; IT != buffer.end(); ++IT) {
		e = *IT;
		while (e != NULL) {
			tmp = e->next;
			delete e;
			e = tmp;
		}
	}
}
void GSS::insert(string s1, string s2, int weight) {		
	unsigned int hash1 = (*hfunc[0])((unsigned char*)(s1.c_str()), s1.length());
	unsigned int hash2 = (*hfunc[0])((unsigned char*)(s2.c_str()), s2.length());
	unsigned int tmp = pow(2, f) - 1;
	unsigned short g1 = hash1 & tmp;
	if(g1 == 0)  g1 += 1;
	unsigned int h1 = (hash1 >> f) % w;
	unsigned short g2 = hash2 & tmp;
	if(g2==0)  g2 += 1;
	unsigned int h2 = (hash2 >> f) % w;
	
	unsigned int k1 = (h1 << f) + g1;
	unsigned int k2 = (h2 << f) + g2;

	int* tmp1 = new int[r];
	int* tmp2 = new int[r];
	tmp1[0] = g1;
	tmp2[0] = g2;
	for(int i = 1; i < r; i++) {
		tmp1[i] = (tmp1[i - 1] * timer + prime) % bigger_p;
		tmp2[i] = (tmp2[i - 1] * timer + prime) % bigger_p;
	}
	bool inserted = false;
	long key = g1 + g2;
	for(int i = 0; i < p; i++) {
		key = (key * timer + prime) % bigger_p;
		int index = key % (r * r);
		int index1 = index / r;
		int index2 = index % r; 
		int p1 = (h1 + tmp1[index1]) % w;
		int p2 = (h2 + tmp2[index2]) % w;
		int pos = p1 * w + p2;
		for (int j = 0; j < s; j++) {
			if ( ( ((value[pos].idx>>(j<<3))&((1<<8)-1)) == (index1|(index2<<4)) ) && (value[pos].src[j]== g1) && (value[pos].dst[j] == g2) ) {
				value[pos].weight[j] += weight;
				inserted = true;
				break;
			}
			if (value[pos].src[j] == 0) {
				value[pos].idx |= ((index1 | (index2 << 4)) << (j<<3));
				value[pos].src[j] = g1;
				value[pos].dst[j] = g2;
				value[pos].weight[j] = weight;
				inserted = true;
				break;
			}
		}
		if(inserted)
			break;
	} 
	if(!inserted) {
		map<unsigned int, int>::iterator it = index.find(k1);
		if(it!=index.end()) {
			int tag = it->second;
			linknode* node = buffer[tag];
			while(true) {
				if (node->key == k2) {   
					node->weight += weight;
					break;
				}
				if(node->next==NULL) {
					linknode* ins = new linknode;
					ins->key = k2;
					ins->weight = weight;
					ins->next = NULL;
					node->next = ins;
					break;
				}
				node = node->next;
			}
		}
		else {
			index[k1] = n;
			n++;
			linknode* node = new linknode;
			node->key = k1;
			node->weight = 0;
			if (k1 != k2) {  //k1==k2 means loop
				linknode* ins = new linknode;
				ins->key = k2;
				ins->weight = weight;
				ins->next = NULL;
				node->next = ins;
			}
			else { 
				node->weight += weight;
				node->next = NULL;
			}
			buffer.push_back(node); 
		}	
	}
	delete [] tmp1;
	delete [] tmp2;
	return;
}
int GSS::edgeWeightQuery(string s1, string s2) {
	unsigned int hash1 = (*hfunc[0])((unsigned char*)(s1.c_str()), s1.length());
	unsigned int hash2 = (*hfunc[0])((unsigned char*)(s2.c_str()), s2.length());
	int tmp = pow(2, f) - 1;
	unsigned short g1 = hash1 & tmp;
	if (g1 == 0) g1 += 1;
	unsigned int h1 = (hash1 >> f) % w;
	unsigned short g2 = hash2 & tmp;
	if (g2 == 0) g2 += 1;
	unsigned int h2 = (hash2 >> f) % w;
	int* tmp1 = new int[r];
	int* tmp2 = new int[r];
	tmp1[0] = g1;
	tmp2[0] = g2;
	for (int i = 1; i<r; i++) {
		tmp1[i] = (tmp1[i - 1] * timer + prime) % bigger_p;
		tmp2[i] = (tmp2[i - 1] * timer + prime) % bigger_p;
	}
	long key = g1 + g2;

	for (int i = 0; i<p; i++) {
		key = (key * timer + prime) % bigger_p;
		int index = key % (r * r);
		int index1 = index / r;
		int index2 = index % r;
		int p1 = (h1 + tmp1[index1]) % w;
		int p2 = (h2 + tmp2[index2]) % w;
		int pos = p1 * w + p2;
		for (int j = 0; j<s; j++) {
			if ((((value[pos].idx >> (j << 3))&((1 << 8) - 1)) == (index1 | (index2 << 4))) && (value[pos].src[j] == g1) && (value[pos].dst[j] == g2)) {
				delete []tmp1;
				delete []tmp2;
				return value[pos].weight[j];
			}
		}
		
	}
	unsigned int k1 = (h1 << f) + g1;
	unsigned int k2 = (h2 << f) + g2;
	map<unsigned int, int>::iterator it = index.find(k1);
	if (it != index.end()) {
		int tag = it->second;
		linknode* node = buffer[tag];
		while (node!=NULL) {
			if (node->key == k2) {
				delete []tmp1;
				delete []tmp2;
				return node->weight;
			}
			node = node->next;
		}
	}
	delete []tmp1;
	delete []tmp2;
	return 0;
}
int GSS::nodeWeightQuery(string s1, int type) {
	int weight = 0;
	unsigned int hash1 = (*hfunc[0])((unsigned char*)(s1.c_str()), s1.length());
	int tmp = pow(2, f) - 1;
	unsigned short g1 = hash1 & tmp;
	if (g1 == 0) g1 += 1;
	unsigned int h1 = (hash1 >> f) % w;
	int* tmp1 = new int[r];
	tmp1[0] = g1;
	for (int i = 1; i < r; i++) {
		tmp1[i] = (tmp1[i - 1] * timer + prime) % bigger_p;
	}
	for (int i = 0; i < r; i++) {
		int p1 = (h1 + tmp1[i]) % w;
		for (int k = 0; k < w; k++) {
			if (type == 0) {	/*successor query*/
				int pos = p1 * w + k;
				for (int j = 0; j < s; ++j) {
					if (type == 0 && (((value[pos].idx >> ((j << 3)))&((1 << 4) - 1)) == i) && (value[pos].src[j] == g1)) {
						weight += value[pos].weight[j];
					}
				}
			}
			else if (type == 1)	{/*precursor query*/
				int pos = p1 + k * w;
				for (int j = 0; j < s; ++j) {
					if (type == 1 && (((value[pos].idx >> ((j << 3) + 4))&((1 << 4) - 1)) == i) && (value[pos].dst[j] == g1)) {
						weight += value[pos].weight[j];
					}
				}
			}
		}
	}
	if (type == 0) {
		unsigned int k1 = (h1 << f) + g1;
		map<unsigned int, int>::iterator it = index.find(k1);
		if (it != index.end()) {
			int tag = it->second;
			linknode* node = buffer[tag];
			node = node->next;
			while (node != NULL) {
				weight += node->weight;
				node = node->next;
			}
		}
	}
	else if (type == 1) {
		unsigned int k1 = (h1 << f) + g1;
		for (map<unsigned int, int>::iterator it = index.begin(); it != index.end(); ++it) {
			int tag = it->second;
			linknode* node = buffer[tag];
			node = node->next;
			while (node != NULL) {
				if(node->key == k1)
					weight += node->weight;
				node = node->next;
			}
		}
	}
	delete []tmp1;
	return weight;
}
