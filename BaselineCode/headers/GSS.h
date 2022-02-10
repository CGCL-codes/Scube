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
#include "hashfunctions.h"
using namespace std;
#define prime 739
#define bigger_p 1048576
#define timer 5
#define M 80000

#ifdef ROOM1
#define Roomnum 1	// This is the parameter to controll the maximum number of rooms in a bucket. 
#else
#define Roomnum 2 	// This is the parameter to controll the maximum number of rooms in a bucket. 
#endif

// struct basket
// {
// 	unsigned short src[Roomnum];
// 	unsigned short dst[Roomnum];
// 	short  weight[Roomnum];
// 	unsigned int idx;
// };
// struct mapnode
// {
// 	unsigned int h;
// 	unsigned short g;
// };
// struct linknode
// {
// 	unsigned int key;
// 	short weight;
// 	linknode* next;
// };

struct basket {
	uint16_t src[Roomnum];
	uint16_t dst[Roomnum];
	uint32_t  weight[Roomnum];
	uint32_t idx;
	// uint16_t idx[Roomnum];
};

struct mapnode
{
	unsigned int h;
	unsigned short g;
};

struct linknode {
	uint32_t key;
	uint32_t weight;
	linknode* next;
};

struct edge {
	uint32_t s;
	uint32_t d;
	edge(uint32_t s, uint32_t d): s(s), d(d) {}
	bool operator<(const edge &b) const
	{
		bool res = false;
		if (this->s < b.s)	
			res = true;
		else if (this->s > b.s)	
			res = false;
		else if (this->s == b.s) {
			if (this->d < b.d)	
				res = true;
			else
				res = false;
			// else if (strcmp(this->d.c_str(), b.d.c_str()) >= 0)	res = false;
		}
		// if (strcmp(this->s.c_str(), b.s.c_str()) < 0)	
		// 	res = true;
		// else if (strcmp(this->s.c_str(), b.s.c_str()) > 0)	
		// 	res = false;
		// else if (strcmp(this->s.c_str(), b.s.c_str()) == 0) {
		// 	if (strcmp(this->d.c_str(), b.d.c_str()) < 0)	
		// 		res = true;
		// 	else
		// 		res = false;
		// 	// else if (strcmp(this->d.c_str(), b.d.c_str()) >= 0)	res = false;
		// }
		return res;
	}
};

class GSS {
private:
	int width;						/* the width of matrix */
	int depth;						/* the depth of matrix */
	int r;							/* r × r mapped baskets */
	int p;							/* candidate buckets */
	int slot_num;					/* multiple rooms */
	int fingerprint_length;			/* finger print length */
	basket* value;
public:
	// set<edge> buffer_edges;
	vector<linknode*> buffer;
	map<unsigned int, int> index;
	int n;
	GSS(int width, int depth, int range, int candidate, int slot_num,int fingerprint_length);
	~GSS() {
		delete[] value;
		cleanupBuffer();
	}
	void insert(string s1, string s2,int weight);
	void cleanupBuffer();
	int edgeWeightQuery(string s1, string s2);  		// s1 is the ID of the source node, s2 is the ID of the destination node, return the weight of the edge
	int nodeWeightQuery(string s1, int type); 			//src_type = 0 dst_type = 1
	bool reachabilityQuery(string s1, string s2);
	bool reachabilityQuery(string s1, string s2, int& points, int& lines);
	void countBuffer();
	int nodeWeightQueryTime(string s1, int type, double& matrix_time, double& buffer_time);

	int edgeWeightQueryTime(string s1, string s2, double& matrix_time, double& buffer_time, double& hash_time, double& addr_time);  		// s1 is the ID of the source node, s2 is the ID of the destination node, return the weight of the edge
	int nodeWeightQueryTime(string s1, int type, double& matrix_time, double& buffer_time, double& hash_time, double& addr_time); 			//src_type = 0 dst_type = 1
};

void GSS::countBuffer() {
	// print the number of buffer edges
	// cout << "buffer_edges = " << buffer_edges.size() << endl;
	// print the matrix usage
    uint32_t room_count = 0;
	uint32_t bucket_count = 0;
	for (uint32_t i = 0; i < (this->width * this->depth); i++) {
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
	cout << "GSS room_count = " << room_count << ", total room = " << (this->width * this->depth * Roomnum) << ", space usage is " << 
			(double)room_count / (double)(this->width * this->depth * Roomnum) * 100 << "%" << endl;
	cout << "GSS bucket_count = " << bucket_count << ", total bucket = " << (this->width * this->depth) << ", space usage is " << 
			(double)bucket_count / (double)(this->width * this->depth) * 100 << "%" << endl;

	cout << "First dim of buffer = " << this->buffer.size() << endl;
	uint32_t max_second_dim_len = 0;
	uint32_t total_second_dim_len = 0;
	
	for (vector<linknode*>::iterator iter = buffer.begin(); iter != buffer.end(); iter++) {
		linknode* ptr = *iter;
		uint32_t len = 0;
		while (ptr != NULL) {
			len++;
			ptr = ptr->next;
		}
		if (len > max_second_dim_len) {
			max_second_dim_len = len;
		}
		total_second_dim_len += len;
	}
	cout << "Second dim (total) of buffer = " << total_second_dim_len << endl;
	cout << "Max second dim len = " << max_second_dim_len << endl;
	cout << "rate = " << (double)(total_second_dim_len) / (double)(this->width * this->depth * Roomnum) << endl;
	return;
}

GSS::GSS(int width, int depth, int range, int candidate, int slot_num,int fingerprint_length):
width(width), depth(depth), r(range), p(candidate), slot_num(slot_num), fingerprint_length(fingerprint_length) {
	n = 0;
	value = new basket[width * depth];
	memset(value, 0, sizeof(basket) * width * depth);
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
void GSS::insert(string s1, string s2, int weight)
{
	unsigned int hash1 = (*hfunc[0])((unsigned char*)(s1.c_str()), s1.length());
	unsigned int hash2 = (*hfunc[0])((unsigned char*)(s2.c_str()), s2.length());
	unsigned int tmp = pow(2, fingerprint_length) - 1;
	unsigned short g1 = hash1 & tmp;
	if(g1 == 0)  g1 += 1;
	unsigned int h1 = (hash1 >> fingerprint_length) % depth;
	unsigned short g2 = hash2 & tmp;
	if(g2==0)  g2 += 1;
	unsigned int h2 = (hash2 >> fingerprint_length) % width;
	
	unsigned int k1 = (h1 << fingerprint_length) + g1;
	unsigned int k2 = (h2 << fingerprint_length) + g2;

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
		int p1 = (h1 + tmp1[index1]) % depth;
		int p2 = (h2 + tmp2[index2]) % width;
		int pos = p1 * width + p2;
		for (int j = 0; j < slot_num; j++) {
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
			////////
			// if ((value[pos].idx[j] == (index1 | (index2 << 8))) && (value[pos].src[j]== g1) && (value[pos].dst[j] == g2)) {
			// 	value[pos].weight[j] += weight;
			// 	inserted = true;
			// 	break;
			// }
			// if (value[pos].src[j] == 0) {
			// 	value[pos].idx[j] = (index1 | (index2 << 8));
			// 	value[pos].src[j] = g1;
			// 	value[pos].dst[j] = g2;
			// 	value[pos].weight[j] = weight;
			// 	inserted = true;
			// 	break;
			// }
		}
		if(inserted)
			break;
	} 
	if(!inserted) {
		// buffer_edges.insert(edge(k1, k2));
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
	int tmp = pow(2, fingerprint_length) - 1;
	unsigned short g1 = hash1 & tmp;
	if (g1 == 0) g1 += 1;
	unsigned int h1 = (hash1 >> fingerprint_length) % depth;
	unsigned short g2 = hash2 & tmp;
	if (g2 == 0) g2 += 1;
	unsigned int h2 = (hash2 >> fingerprint_length) % width;
	int* tmp1 = new int[r];
	int* tmp2 = new int[r];
	tmp1[0] = g1;
	tmp2[0] = g2;
	for (int i = 1; i<r; i++) {
		tmp1[i] = (tmp1[i - 1] * timer + prime) % bigger_p;
		tmp2[i] = (tmp2[i - 1] * timer + prime) % bigger_p;
	}
	long key = g1 + g2;

	for (int i = 0; i < p; i++) {
		key = (key * timer + prime) % bigger_p;
		int index = key % (r * r);
		int index1 = index / r;
		int index2 = index % r;
		int p1 = (h1 + tmp1[index1]) % depth;
		int p2 = (h2 + tmp2[index2]) % width;
		int pos = p1 * width + p2;
		for (int j = 0; j < slot_num; j++) {
			if ((((value[pos].idx >> (j << 3))&((1 << 8) - 1)) == (index1 | (index2 << 4))) && (value[pos].src[j] == g1) && (value[pos].dst[j] == g2)) {
				delete []tmp1;
				delete []tmp2;
				return value[pos].weight[j];
			}
			// if ((value[pos].idx[j] == (index1 | (index2 << 8))) && (value[pos].src[j] == g1) && (value[pos].dst[j] == g2)) {
			// 	delete []tmp1;
			// 	delete []tmp2;
			// 	return value[pos].weight[j];
			// }
		}		
	}
	unsigned int k1 = (h1 << fingerprint_length) + g1;
	unsigned int k2 = (h2 << fingerprint_length) + g2;
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
int GSS::nodeWeightQuery(string s1, int type)
{
	int weight = 0;
	unsigned int hash1 = (*hfunc[0])((unsigned char*)(s1.c_str()), s1.length());
	int tmp = pow(2, fingerprint_length) - 1;
	unsigned short g1 = hash1 & tmp;
	if (g1 == 0) g1 += 1;

	int* tmp1 = new int[r];
	tmp1[0] = g1;
	for (int i = 1; i < r; i++) {
		tmp1[i] = (tmp1[i - 1] * timer + prime) % bigger_p;
	}
	if (type == 0) {  		/*successor query*/
		unsigned int h1 = (hash1 >> fingerprint_length) % depth;
		for (int i = 0; i < r; i++) {
			int p1 = (h1 + tmp1[i]) % depth;
			for (int k = 0; k < width; k++) {
				int pos = p1 * width + k;
				for (int j = 0; j < slot_num; ++j) {
					if ((((value[pos].idx >> ((j << 3)))&((1 << 4) - 1)) == i) && (value[pos].src[j] == g1)) {
						weight += value[pos].weight[j];
					}
					// if (((value[pos].idx[j] & 0xff) == i) && (value[pos].src[j] == g1)) {
					// 	weight += value[pos].weight[j];
					// }
				}
			}
		}
		unsigned int k1 = (h1 << fingerprint_length) + g1;
		map<unsigned int, int>::iterator it = index.find(k1);
		if (it != index.end()) {
			int tag = it->second;
			linknode* node = buffer[tag];
			// node = node->next;
			while (node != NULL) {
				weight += node->weight;
				node = node->next;
			}
		}
	}
	else if (type == 1) {	/*precursor query*/
		unsigned int h1 = (hash1 >> fingerprint_length) % width;
		for (int i = 0; i < r; i++) {
			int p1 = (h1 + tmp1[i]) % width;
			for (int k = 0; k < depth; k++) {
				int pos = k * width + p1;
				for (int j = 0; j < slot_num; ++j) {
					if ((((value[pos].idx >> ((j << 3) + 4))&((1 << 4) - 1)) == i) && (value[pos].dst[j] == g1)) {
						weight += value[pos].weight[j];
					}
					// if (((value[pos].idx[j] >> 8) == i) && (value[pos].dst[j] == g1)) {
					// 	weight += value[pos].weight[j];
					// }
				}
			}
		}
		unsigned int k1 = (h1 << fingerprint_length) + g1;
		for (map<unsigned int, int>::iterator it = index.begin(); it != index.end(); ++it) {
			int tag = it->second;
			linknode* node = buffer[tag];
			// node = node->next;
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

int GSS::nodeWeightQueryTime(string s1, int type, double& matrix_time, double& buffer_time) {
	matrix_time= 0;
	buffer_time = 0;

	timeval ts1, te1, ts2, te2;

	int weight = 0;
	unsigned int hash1 = (*hfunc[0])((unsigned char*)(s1.c_str()), s1.length());
	int tmp = pow(2, fingerprint_length) - 1;
	unsigned short g1 = hash1 & tmp;
	if (g1 == 0) g1 += 1;

	int* tmp1 = new int[r];
	tmp1[0] = g1;
	for (int i = 1; i < r; i++) {
		tmp1[i] = (tmp1[i - 1] * timer + prime) % bigger_p;
	}
	if (type == 0) {  		/*successor query*/
		unsigned int h1 = (hash1 >> fingerprint_length) % depth;
		gettimeofday( &ts1, NULL);
		for (int i = 0; i < r; i++) {
			int p1 = (h1 + tmp1[i]) % depth;
			for (int k = 0; k < width; k++) {
				int pos = p1 * width + k;
				for (int j = 0; j < slot_num; ++j) {
					if ((((value[pos].idx >> ((j << 3)))&((1 << 4) - 1)) == i) && (value[pos].src[j] == g1)) {
						weight += value[pos].weight[j];
					}
					// if (((value[pos].idx[j] & 0xff) == i) && (value[pos].src[j] == g1)) {
					// 	weight += value[pos].weight[j];
					// }
				}
			}
		}
		gettimeofday( &te1, NULL);
        matrix_time = (te1.tv_sec - ts1.tv_sec) * 1000000 +  (te1.tv_usec - ts1.tv_usec);
		gettimeofday( &ts2, NULL);
		unsigned int k1 = (h1 << fingerprint_length) + g1;
		map<unsigned int, int>::iterator it = index.find(k1);
		if (it != index.end()) {
			int tag = it->second;
			linknode* node = buffer[tag];
			// node = node->next;
			while (node != NULL) {
				weight += node->weight;
				node = node->next;
			}
		}
		gettimeofday( &te2, NULL);
        buffer_time = (te2.tv_sec - ts2.tv_sec) * 1000000 +  (te2.tv_usec - ts2.tv_usec);
	}
	else if (type == 1) {	/*precursor query*/
		unsigned int h1 = (hash1 >> fingerprint_length) % width;
		gettimeofday( &ts1, NULL);
		for (int i = 0; i < r; i++) {
			int p1 = (h1 + tmp1[i]) % width;
			for (int k = 0; k < depth; k++) {
				int pos = k * width + p1;
				for (int j = 0; j < slot_num; ++j) {
					if ((((value[pos].idx >> ((j << 3) + 4))&((1 << 4) - 1)) == i) && (value[pos].dst[j] == g1)) {
						weight += value[pos].weight[j];
					}
					// if (((value[pos].idx[j] >> 8) == i) && (value[pos].dst[j] == g1)) {
					// 	weight += value[pos].weight[j];
					// }
				}
			}
		}
		gettimeofday( &te1, NULL);
        matrix_time = (te1.tv_sec - ts1.tv_sec) * 1000000 +  (te1.tv_usec - ts1.tv_usec);
		gettimeofday( &ts2, NULL);
		unsigned int k1 = (h1 << fingerprint_length) + g1;
		for (map<unsigned int, int>::iterator it = index.begin(); it != index.end(); ++it) {
			int tag = it->second;
			linknode* node = buffer[tag];
			// node = node->next;
			while (node != NULL) {
				if(node->key == k1)
					weight += node->weight;
				node = node->next;
			}
		}
		gettimeofday( &te2, NULL);
        buffer_time = (te2.tv_sec - ts2.tv_sec) * 1000000 +  (te2.tv_usec - ts2.tv_usec);
	}
	delete []tmp1;
	return weight;
}



bool GSS::reachabilityQuery(string s1, string s2) {  // s1 is the ID of the source node, s2 is the ID of the destination node, return whether reachable.
	unsigned int hash1 = (*hfunc[0])((unsigned char*)(s1.c_str()), s1.length());
	unsigned int hash2 = (*hfunc[0])((unsigned char*)(s2.c_str()), s2.length());
	int tmp = pow(2, fingerprint_length) - 1;
	unsigned short g1 = hash1 & tmp;
	if (g1 == 0) g1 += 1;
	unsigned int h1 = (hash1 >> fingerprint_length) % depth;
	unsigned short g2 = hash2 & tmp;
	if (g2 == 0) g2 += 1;
	unsigned int h2 = (hash2 >> fingerprint_length) % width;
	int pos;
	map<unsigned int, bool> checked;
	queue<mapnode> q;
	mapnode e;
	e.h = h1;
	e.g = g1;
	q.push(e);
	checked[((h1 << fingerprint_length) + g1)] = true;
	map<unsigned int, bool>::iterator	IT;

	while (!q.empty())
	{
		e = q.front();
		h1 = e.h;
		g1 = e.g;
		int* tmp1 = new int[r];
		int* tmp2 = new int[r];
		tmp2[0] = g2;
		tmp1[0] = g1;
		for (int i = 1; i<r; i++)
		{
			tmp1[i] = (tmp1[i - 1] * timer + prime) % bigger_p;
			tmp2[i] = (tmp2[i - 1] * timer + prime) % bigger_p;
		}
		for (int i1 = 0; i1<r; i1++)
		{
			int p1 = (h1 + tmp1[i1]) % depth;
			for (int i2 = 0; i2<r; i2++)
			{
				int p2 = (h2 + tmp2[i2]) % width;
				int pos = p1*width + p2;
				for (int i3 = 0; i3<slot_num; i3++)
				{
					if ((((value[pos].idx >> (i3 << 3))&((1 << 8) - 1)) == (i1 | (i2 << 4))) && (value[pos].src[i3] == g1) && (value[pos].dst[i3] == g2))
					{
						delete []tmp1;
						delete []tmp2;
						return true;
					}
				}
			}
		}
		
//	 find in buffer
		unsigned int k1 = (h1 << fingerprint_length) + g1;
		map<unsigned int, int>::iterator it = index.find(k1);
		if (it != index.end())
		{
			int tag = it->second;
			linknode* node = buffer[tag];
			while (node != NULL)
			{
				if (node->key != k1)
				{
					unsigned int val = node->key;
					unsigned int temp_h = (val) >> fingerprint_length;
					unsigned int tmp = pow(2, fingerprint_length);
					unsigned short temp_g = (val%tmp);
					if ((temp_h == h2) && (temp_g == g2))
					{
						delete []tmp1;
						delete []tmp2;
						return true;
					}
		
					IT = checked.find(val);
					if (IT == checked.end())
					{
						mapnode temp_e;
						temp_e.h = temp_h;
						temp_e.g = temp_g;
						q.push(temp_e);
						checked[val] = true;;
					}
				}
				node = node->next;
			}
		}
// find in matrix
		for (int i1 = 0; i1<r; i1++)
		{
			int p1 = (h1 + tmp1[i1]) % depth;
			for (int i2 = 0; i2<width; i2++)
			{
				int pos = p1*width + i2;
				for (int i3 = 0; i3<slot_num; i3++)
				{
					if (value[pos].src[i3] == g1 && (((value[pos].idx >> (i3 << 3))&((1 << 4) - 1)) == i1))
					{
						int tmp_g = value[pos].dst[i3];
						int tmp_s = ((value[pos].idx >> ((i3 << 3) + 4))&((1 << 4) - 1));
			
						int shifter = tmp_g;
						for (int v = 0; v < tmp_s; v++)
							shifter = (shifter*timer + prime) % bigger_p;
						int tmp_h = i2;
						while (tmp_h < shifter)
							tmp_h += width;
						tmp_h -= shifter;

						unsigned int val = (tmp_h << fingerprint_length) + tmp_g;
					
						IT = checked.find(val);
						if (IT == checked.end())
						{
					
							mapnode tmp_e;
							tmp_e.h = tmp_h;
							tmp_e.g = tmp_g;
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
bool GSS::reachabilityQuery(string s1, string s2, int& points, int& lines) {  // s1 is the ID of the source node, s2 is the ID of the destination node, return whether reachable.
	unsigned int hash1 = (*hfunc[0])((unsigned char*)(s1.c_str()), s1.length());
	unsigned int hash2 = (*hfunc[0])((unsigned char*)(s2.c_str()), s2.length());
	int tmp = pow(2, fingerprint_length) - 1;
	unsigned short g1 = hash1 & tmp;
	if (g1 == 0) g1 += 1;
	unsigned int h1 = (hash1 >> fingerprint_length) % depth;
	unsigned short g2 = hash2 & tmp;
	if (g2 == 0) g2 += 1;
	unsigned int h2 = (hash2 >> fingerprint_length) % width;
	int pos;
	map<unsigned int, bool> checked;
	queue<mapnode> q;
	mapnode e;
	e.h = h1;
	e.g = g1;
	q.push(e);
	checked[((h1 << fingerprint_length) + g1)] = true;
	map<unsigned int, bool>::iterator	IT;

	points = 0;
    lines = 0;
	while (!q.empty())
	{
		e = q.front();
		points++;
		h1 = e.h;
		g1 = e.g;
		int* tmp1 = new int[r];
		lines += r;
		int* tmp2 = new int[r];
		tmp2[0] = g2;
		tmp1[0] = g1;
		for (int i = 1; i<r; i++)
		{
			tmp1[i] = (tmp1[i - 1] * timer + prime) % bigger_p;
			tmp2[i] = (tmp2[i - 1] * timer + prime) % bigger_p;
		}
		for (int i1 = 0; i1<r; i1++)
		{
			int p1 = (h1 + tmp1[i1]) % depth;
			for (int i2 = 0; i2<r; i2++)
			{
				int p2 = (h2 + tmp2[i2]) % width;
				int pos = p1*width + p2;
				for (int i3 = 0; i3<slot_num; i3++)
				{
					if ((((value[pos].idx >> (i3 << 3))&((1 << 8) - 1)) == (i1 | (i2 << 4))) && (value[pos].src[i3] == g1) && (value[pos].dst[i3] == g2))
					{
						delete []tmp1;
						delete []tmp2;
						return true;
					}
				}
			}
		}
		
//	 find in buffer
		unsigned int k1 = (h1 << fingerprint_length) + g1;
		map<unsigned int, int>::iterator it = index.find(k1);
		if (it != index.end())
		{
			int tag = it->second;
			linknode* node = buffer[tag];
			while (node != NULL)
			{
				if (node->key != k1)
				{
					unsigned int val = node->key;
					unsigned int temp_h = (val) >> fingerprint_length;
					unsigned int tmp = pow(2, fingerprint_length);
					unsigned short temp_g = (val%tmp);
					if ((temp_h == h2) && (temp_g == g2))
					{
						delete []tmp1;
						delete []tmp2;
						return true;
					}
		
					IT = checked.find(val);
					if (IT == checked.end())
					{
						mapnode temp_e;
						temp_e.h = temp_h;
						temp_e.g = temp_g;
						q.push(temp_e);
						checked[val] = true;;
					}
				}
				node = node->next;
			}
		}
// find in matrix
		for (int i1 = 0; i1<r; i1++)
		{
			int p1 = (h1 + tmp1[i1]) % depth;
			for (int i2 = 0; i2<width; i2++)
			{
				int pos = p1*width + i2;
				for (int i3 = 0; i3<slot_num; i3++)
				{
					if (value[pos].src[i3] == g1 && (((value[pos].idx >> (i3 << 3))&((1 << 4) - 1)) == i1))
					{
						int tmp_g = value[pos].dst[i3];
						int tmp_s = ((value[pos].idx >> ((i3 << 3) + 4))&((1 << 4) - 1));
			
						int shifter = tmp_g;
						for (int v = 0; v < tmp_s; v++)
							shifter = (shifter*timer + prime) % bigger_p;
						int tmp_h = i2;
						while (tmp_h < shifter)
							tmp_h += width;				/////////
						tmp_h -= shifter;

						unsigned int val = (tmp_h << fingerprint_length) + tmp_g;
					
						IT = checked.find(val);
						if (IT == checked.end())
						{
					
							mapnode tmp_e;
							tmp_e.h = tmp_h;
							tmp_e.g = tmp_g;
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

int GSS::edgeWeightQueryTime(string s1, string s2, double& matrix_time, double& buffer_time, double& hash_time, double& addr_time) {
	matrix_time = 0;
	buffer_time = 0;
	hash_time = 0;
	addr_time = 0;

	timeval ts1, te1, ts2, te2, ta1, ta2, tm1, tm2, tb1, tb2;
	gettimeofday( &ts1, NULL);
	unsigned int hash1 = (*hfunc[0])((unsigned char*)(s1.c_str()), s1.length());
	unsigned int hash2 = (*hfunc[0])((unsigned char*)(s2.c_str()), s2.length());
	gettimeofday( &te1, NULL);
	double delta_t1 = (te1.tv_sec - ts1.tv_sec) * 1000000 +  (te1.tv_usec - ts1.tv_usec);
	hash_time += delta_t1;


	gettimeofday( &ts2, NULL);
	int tmp = pow(2, fingerprint_length) - 1;
	unsigned short g1 = hash1 & tmp;
	if (g1 == 0) g1 += 1;
	unsigned int h1 = (hash1 >> fingerprint_length) % depth;
	unsigned short g2 = hash2 & tmp;
	if (g2 == 0) g2 += 1;
	unsigned int h2 = (hash2 >> fingerprint_length) % width;
	int* tmp1 = new int[r];
	int* tmp2 = new int[r];
	tmp1[0] = g1;
	tmp2[0] = g2;
	for (int i = 1; i<r; i++) {
		tmp1[i] = (tmp1[i - 1] * timer + prime) % bigger_p;
		tmp2[i] = (tmp2[i - 1] * timer + prime) % bigger_p;
	}
	long key = g1 + g2;
	gettimeofday( &te2, NULL);
	double delta_t2 = (te2.tv_sec - ts2.tv_sec) * 1000000 +  (te2.tv_usec - ts2.tv_usec);
	addr_time += delta_t2;

	int val_m = -1;
	for (int i = 0; i < p; i++) {
		gettimeofday( &ta1, NULL);
		key = (key * timer + prime) % bigger_p;
		int index = key % (r * r);
		int index1 = index / r;
		int index2 = index % r;
		int p1 = (h1 + tmp1[index1]) % depth;
		int p2 = (h2 + tmp2[index2]) % width;
		int pos = p1 * width + p2;
		gettimeofday( &ta2, NULL);
		double delta_t3 = (ta2.tv_sec - ta1.tv_sec) * 1000000 +  (ta2.tv_usec - ta1.tv_usec);
		addr_time += delta_t3;
		for (int j = 0; j < slot_num; j++) {
			gettimeofday( &tm1, NULL);
			if ((((value[pos].idx >> (j << 3))&((1 << 8) - 1)) == (index1 | (index2 << 4))) && (value[pos].src[j] == g1) && (value[pos].dst[j] == g2)) {
				val_m = value[pos].weight[j];
			}
			gettimeofday( &tm2, NULL);
			double delta_t4 = (tm2.tv_sec - tm1.tv_sec) * 1000000 +  (tm2.tv_usec - tm1.tv_usec);
			matrix_time += delta_t4;
			if (val_m != 0) {
				delete []tmp1;
				delete []tmp2;
				return val_m;
			}
			// if ((value[pos].idx[j] == (index1 | (index2 << 8))) && (value[pos].src[j] == g1) && (value[pos].dst[j] == g2)) {
			// 	delete []tmp1;
			// 	delete []tmp2;
			// 	return value[pos].weight[j];
			// }
		}		
	}
	int val_b = 0;
	bool find_b = false;
	gettimeofday( &tb1, NULL);
	unsigned int k1 = (h1 << fingerprint_length) + g1;
	unsigned int k2 = (h2 << fingerprint_length) + g2;
	map<unsigned int, int>::iterator it = index.find(k1);
	if (it != index.end()) {
		int tag = it->second;
		linknode* node = buffer[tag];
		while (node!=NULL) {
			if (node->key == k2) {
				val_b = node->weight;
				find_b = true;
				break;
			}
			node = node->next;
		}
	}
	gettimeofday( &tb2, NULL);
	double delta_t5 = (tb2.tv_sec - tb1.tv_sec) * 1000000 +  (tb2.tv_usec - tb1.tv_usec);
	buffer_time += delta_t5;
	delete []tmp1;
	delete []tmp2;
	if (find_b)
		return val_b;
	return 0;
}
int GSS::nodeWeightQueryTime(string s1, int type, double& matrix_time, double& buffer_time, double& hash_time, double& addr_time) {
	matrix_time = 0;
	buffer_time = 0;
	hash_time = 0;
	addr_time = 0;
	
	int weight = 0;

	timeval ts1, te1, ts2, te2, ta1, ta2, ta3, ta4, ta5, ta6, tm1, tm2, tb1, tb2;
	gettimeofday( &ts1, NULL);
	unsigned int hash1 = (*hfunc[0])((unsigned char*)(s1.c_str()), s1.length());
	gettimeofday( &te1, NULL);
	double delta_t1 = (te1.tv_sec - ts1.tv_sec) * 1000000 +  (te1.tv_usec - ts1.tv_usec);
	hash_time += delta_t1;

	gettimeofday( &ts2, NULL);
	int tmp = pow(2, fingerprint_length) - 1;
	unsigned short g1 = hash1 & tmp;
	if (g1 == 0) g1 += 1;

	int* tmp1 = new int[r];
	tmp1[0] = g1;
	for (int i = 1; i < r; i++) {
		tmp1[i] = (tmp1[i - 1] * timer + prime) % bigger_p;
	}
	gettimeofday( &te2, NULL);
	double delta_t2 = (te2.tv_sec - ts2.tv_sec) * 1000000 +  (te2.tv_usec - ts2.tv_usec);
	addr_time += delta_t2;

	if (type == 0) {  		/*successor query*/
		gettimeofday( &ta1, NULL);
		unsigned int h1 = (hash1 >> fingerprint_length) % depth;
		gettimeofday( &ta2, NULL);
		double delta_t3 = (ta2.tv_sec - ta1.tv_sec) * 1000000 +  (ta2.tv_usec - ta1.tv_usec);
		addr_time += delta_t3;
		for (int i = 0; i < r; i++) {
			gettimeofday( &ta3, NULL);
			int p1 = (h1 + tmp1[i]) % depth;
			gettimeofday( &ta4, NULL);
			double delta_t4 = (ta4.tv_sec - ta3.tv_sec) * 1000000 +  (ta4.tv_usec - ta3.tv_usec);
			addr_time += delta_t4;
			for (int k = 0; k < width; k++) {
				gettimeofday( &ta5, NULL);
				int pos = p1 * width + k;
				gettimeofday( &ta6, NULL);
				double delta_t5 = (ta6.tv_sec - ta5.tv_sec) * 1000000 +  (ta6.tv_usec - ta5.tv_usec);
				addr_time += delta_t5;
				for (int j = 0; j < slot_num; ++j) {
					gettimeofday( &tm1, NULL);
					if ((((value[pos].idx >> ((j << 3)))&((1 << 4) - 1)) == i) && (value[pos].src[j] == g1)) {
						weight += value[pos].weight[j];
					}
					gettimeofday( &tm2, NULL);
					double delta_t6 = (tm2.tv_sec - tm1.tv_sec) * 1000000 +  (tm2.tv_usec - tm1.tv_usec);
					matrix_time += delta_t6;
					// if (((value[pos].idx[j] & 0xff) == i) && (value[pos].src[j] == g1)) {
					// 	weight += value[pos].weight[j];
					// }
				}
			}
		}
		gettimeofday( &tb1, NULL);
		unsigned int k1 = (h1 << fingerprint_length) + g1;
		map<unsigned int, int>::iterator it = index.find(k1);
		if (it != index.end()) {
			int tag = it->second;
			linknode* node = buffer[tag];
			// node = node->next;
			while (node != NULL) {
				weight += node->weight;
				node = node->next;
			}
		}
		gettimeofday( &tb2, NULL);
		double delta_t7 = (tb2.tv_sec - tb1.tv_sec) * 1000000 +  (tb2.tv_usec - tb1.tv_usec);
		buffer_time += delta_t7;
	}
	else if (type == 1) {	/*precursor query*/
		gettimeofday( &ta1, NULL);
		unsigned int h1 = (hash1 >> fingerprint_length) % width;
		gettimeofday( &ta2, NULL);
		double delta_t3 = (ta2.tv_sec - ta1.tv_sec) * 1000000 +  (ta2.tv_usec - ta1.tv_usec);
		addr_time += delta_t3;
		for (int i = 0; i < r; i++) {
			gettimeofday( &ta3, NULL);
			int p1 = (h1 + tmp1[i]) % width;
			gettimeofday( &ta4, NULL);
			double delta_t4 = (ta4.tv_sec - ta3.tv_sec) * 1000000 +  (ta4.tv_usec - ta3.tv_usec);
			addr_time += delta_t4;
			for (int k = 0; k < depth; k++) {
				gettimeofday( &ta5, NULL);
				int pos = k * width + p1;
				gettimeofday( &ta6, NULL);
				double delta_t5 = (ta6.tv_sec - ta5.tv_sec) * 1000000 +  (ta6.tv_usec - ta5.tv_usec);
				addr_time += delta_t5;
				for (int j = 0; j < slot_num; ++j) {
					gettimeofday( &tm1, NULL);
					if ((((value[pos].idx >> ((j << 3) + 4))&((1 << 4) - 1)) == i) && (value[pos].dst[j] == g1)) {
						weight += value[pos].weight[j];
					}
					gettimeofday( &tm2, NULL);
					double delta_t6 = (tm2.tv_sec - tm1.tv_sec) * 1000000 +  (tm2.tv_usec - tm1.tv_usec);
					matrix_time += delta_t6;
					// if (((value[pos].idx[j] >> 8) == i) && (value[pos].dst[j] == g1)) {
					// 	weight += value[pos].weight[j];
					// }
				}
			}
		}
		gettimeofday( &tb1, NULL);
		unsigned int k1 = (h1 << fingerprint_length) + g1;
		for (map<unsigned int, int>::iterator it = index.begin(); it != index.end(); ++it) {
			int tag = it->second;
			linknode* node = buffer[tag];
			// node = node->next;
			while (node != NULL) {
				if(node->key == k1)
					weight += node->weight;
				node = node->next;
			}
		}
		gettimeofday( &tb2, NULL);
		double delta_t7 = (tb2.tv_sec - tb1.tv_sec) * 1000000 +  (tb2.tv_usec - tb1.tv_usec);
		buffer_time += delta_t7;
	}
	delete []tmp1;
	return weight;
}
