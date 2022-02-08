//插入、边查、点查等函数
#ifndef QUERYFUNCTION_H
#define QUERYFUNCTION_H

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <thread>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h> //_access fun
#include "TCM.h"
#include "GSS.h"

// const int query_data_pairs = 100000;
uint64_t query_data_pairs = 0;

struct QueryData {
	uint32_t a;
	uint32_t b;
};

/***************** function declaration ***********************/
int isFolderExist(char* folder);
int createDirectory(char* sPathName);
int readRandomFileToDataArray(string file, QueryData dataArray[]);
/***************** function declaration ***********************/
int isFolderExist(char* folder) {
	int ret = 0;
	ret = access(folder, R_OK | W_OK);
	if (ret == 0)
		ret = 1;
	else
		ret = 0;
	return ret;
}
int createDirectory(char* sPathName) {
	char DirName[256];
	strcpy(DirName, sPathName);
	int i, len = strlen(DirName);
	if (DirName[len - 1] != '/')
		strcat(DirName, "/");
	len = strlen(DirName);
	for (i = 1; i < len; i++) {
		if (DirName[i] == '/') {
			DirName[i] = 0;
			int a = access(DirName, F_OK);
			if (a == -1) {
				mkdir(DirName, 0755);
			}
			DirName[i] = '/';
		}
	}
	return 0;
}
#if defined(DEBUG) || defined(HINT)
uint64_t count_lines(string file) {  //统计文本行数
    ifstream readFile;
    uint64_t n = 0;
    char line[512];
    string temp;
    readFile.open(file, ios::in);//ios::in 表示以只读的方式读取文件
    if(readFile.fail()) { //文件打开失败:返回0
        cout << "error in opening file" << endl;
        return 0;
    }
    else { //文件存在
        while(getline(readFile,temp))
            n++;
    }
    readFile.close();
    return n;
}
void progress_bar(int n) {
	int i = 0;
	char bar[102];
	const char *lable = "|/-\\";
	bar[0] = 0;
	while (i < n) {
	    bar[i] = '#';
		i++;
		bar[i] = 0;
	}
	printf("\r[%-100s][%d%%][%c]", bar, i, lable[i%4]);
	fflush(stdout);
	return;
}
#endif
int readRandomFileToDataArray(string file, QueryData dataArray[]) {
	ifstream randomFile;
	randomFile.open(file);
	if (!randomFile.is_open()) {
		cout << "Error in open file, Path = " << file << endl;
		return -1;
	}
	int datanum = 0;
	uint32_t a, b;
	while (!randomFile.eof()) {
		randomFile >> a >> b;
		if(randomFile.fail())
			break;
		dataArray[datanum].a = a;
		dataArray[datanum].b = b;
		datanum++;
		if(datanum > query_data_pairs) {
			cout << "输入数据 > 数组范围" << endl;
			break;
		}
	}
	randomFile.close();
	return datanum;
}

template <class T>
int insert(T& tcm, string filename, int type, int data_interval) {		// int data_interval-10000为单位
	ifstream ifs;
	ifs.open(filename);
	if (!ifs.is_open()) {
		cout << "Error in open file, Path = " << filename << endl;
		return -1;
	}
	uint32_t s, d, w, t;
#if defined(DEBUG) || defined(HINT)
	cout << "Inserting..." << endl;
	double total = count_lines(filename);
	if(total == 0)
		cout << "ERROR--QueryFunction--134" <<endl;
#endif
#if defined(DEBUG) || defined(TINSTIME) || defined(BINSTIME) || defined(HINT)
	timeval ins_start, ins_end;	
	gettimeofday( &ins_start, NULL);
#endif
#if defined(DEBUG) || defined(BINSTIME)
	timeval bpoint_start, bpoint_end;
	gettimeofday( &bpoint_start, NULL);
#endif
	int datanum = 0;
	int newv = 0, oldv = 0;
	int dn = 0;
	while (!ifs.eof()) {
		if (type == 0) {
			ifs >> s >> d >> w >> t;
			tcm.insert(to_string(s), to_string(d), w);
		}
		else if (type == 1) {
			ifs >> s >> d >> t;
			tcm.insert(to_string(s), to_string(d), 1);
		}
		else if (type == 2) {
			ifs >> s >> d;
			tcm.insert(to_string(s), to_string(d), 1);
		}
		if(ifs.fail())
			break;
		datanum++;
#if defined(DEBUG) || defined(BINSTIME)
		if (datanum % (data_interval * 10000) == 0) {
			dn++;
			gettimeofday( &bpoint_end, NULL);
			double bpoint_time = (bpoint_end.tv_sec - bpoint_start.tv_sec) +  (bpoint_end.tv_usec - bpoint_start.tv_usec) / 1000000.0;
			cout << datanum << ", Break Point Time = " << bpoint_time << " s" << endl;
			cout << "Throughput: " << (double) ((data_interval * 10000) / (double) (bpoint_time)) << " tuples/s" << endl;
			tcm.countBuffer();

			gettimeofday( &bpoint_start, NULL);
		}
#endif
#if defined(DEBUG) || defined(HINT)
		if (datanum % 100000 == 0) {
			newv = (int) ((double) datanum / total * 100);
			if (newv != oldv) {
				progress_bar(newv);
				oldv = newv;
			}
		}
		if (datanum == total) {
			progress_bar(100);
			cout << endl;
		}
#endif
	}
#if defined(DEBUG) || defined(BINSTIME)
	gettimeofday( &bpoint_end, NULL);
	double bpoint_time = (bpoint_end.tv_sec - bpoint_start.tv_sec) +  (bpoint_end.tv_usec - bpoint_start.tv_usec) / 1000000.0;
	cout << endl << datanum << ", Break Point Time = " << bpoint_time << " s" << endl;
	cout << "Throughput: " << (double) ((datanum - (dn * data_interval * 10000)) / (double) (bpoint_time)) << " tuples/s" << endl;
	tcm.countBuffer();

#endif
#if defined(DEBUG) || defined(TINSTIME) || defined(BINSTIME) || defined(HINT)
	gettimeofday( &ins_end, NULL);
	double ins_time = (ins_end.tv_sec - ins_start.tv_sec) +  (ins_end.tv_usec - ins_start.tv_usec) / 1000000.0;
	cout << "Insertion Time = " << ins_time << " s" << endl;
	cout << "Insertion Finished!" << endl;
	cout << "Datanum = " << datanum << endl;
#endif
	ifs.close();
	return 0;
}

void count_num(uint64_t* datanum, uint64_t* end_num) {
	while(*datanum != *end_num) {
		printf("datanum = %lu\n", *datanum);
		sleep(1);
	}
}

template <class T>
int insert_tp_time(T& tcm, string filename, int type) {		// int data_interval-10000为单位
	ifstream ifs;
	ifs.open(filename);
	if (!ifs.is_open()) {
		cout << "Error in open file, Path = " << filename << endl;
		return -1;
	}
	uint32_t s, d, w, t;
#if defined(DEBUG) || defined(INFO)
	cout << "Inserting..." << endl;
	uint64_t total = count_lines(filename);
	if(total == 0)
		cout << "ERROR--QueryFunction--258" <<endl;
#endif
	uint64_t datanum = 0;
	int newv = 0, oldv = 0;
	int dn = 0;	

	thread* child = new thread(count_num, &datanum, &total);

	while (!ifs.eof()) {
		if (type == 0) {
			ifs >> s >> d >> w >> t;
			tcm.insert(to_string(s), to_string(d), w);
		}
		else if (type == 1) {
			ifs >> s >> d >> t;
			tcm.insert(to_string(s), to_string(d), 1);
		}
		else if (type == 2) {
			ifs >> s >> d;
			tcm.insert(to_string(s), to_string(d), 1);
		}
		if(ifs.fail())
			break;
		datanum++;
	}
#if defined(DEBUG) || defined(INFO)
	cout << "Insertion Finished!" << endl;
	cout << "Datanum = " << datanum << endl;
#endif
	ifs.close();
	child->join();
	return 0;
}

template <class T>
int insert(T& tcm, string filename, int type, int data_interval, int fplen, int width, int depth, map<uint32_t, set<uint32_t>>& acc_out_deg, map<uint32_t, set<uint32_t>>& acc_in_deg) {		// int data_interval-10000为单位
	ifstream ifs;
	ifs.open(filename);
	if (!ifs.is_open()) {
		cout << "Error in open file, Path = " << filename << endl;
		return -1;
	}
	uint32_t s, d, w, t;
#if defined(DEBUG) || defined(HINT)
	cout << "Inserting..." << endl;
	double total = count_lines(filename);
	if(total == 0)
		cout << "ERROR--QueryFunction--134" <<endl;
#endif
#if defined(DEBUG) || defined(TINSTIME) || defined(BINSTIME) || defined(HINT)
	timeval ins_start, ins_end;	
	gettimeofday( &ins_start, NULL);
#endif
#if defined(DEBUG) || defined(BINSTIME)
	timeval bpoint_start, bpoint_end;
	gettimeofday( &bpoint_start, NULL);
#endif
	int datanum = 0;
	int newv = 0, oldv = 0;
	int dn = 0;
	while (!ifs.eof()) {
		if (type == 0) {
			ifs >> s >> d >> w >> t;
			tcm.insert(to_string(s), to_string(d), w);
		}
		else if (type == 1) {
			ifs >> s >> d >> t;
			tcm.insert(to_string(s), to_string(d), 1);
		}
		else if (type == 2) {
			ifs >> s >> d;
			tcm.insert(to_string(s), to_string(d), 1);
		}

		unsigned int hash1 = (*hfunc[0])((unsigned char*)(to_string(s).c_str()), to_string(s).length());
		unsigned int hash2 = (*hfunc[0])((unsigned char*)(to_string(d).c_str()), to_string(d).length());
		unsigned int tmp = pow(2, fplen) - 1;
		unsigned short g1 = hash1 & tmp;
		if(g1 == 0)  g1 += 1;
		unsigned int h1 = (hash1 >> fplen) % depth;
		unsigned short g2 = hash2 & tmp;
		if(g2==0)  g2 += 1;
		unsigned int h2 = (hash2 >> fplen) % width;
		
		unsigned int k1 = (h1 << fplen) + g1;
		unsigned int k2 = (h2 << fplen) + g2;

		map<uint32_t, set<uint32_t>>::iterator iter1 = acc_out_deg.find(k1);
        if (iter1 != acc_out_deg.end()) {
            iter1->second.insert(k2);
        }
        else {
            set<uint32_t> temp;
            temp.insert(k2);
            acc_out_deg.insert(pair<uint32_t, set<uint32_t>>(k1, temp));
        }
        map<uint32_t, set<uint32_t>>::iterator iter2 = acc_in_deg.find(k2);
        if (iter2 != acc_in_deg.end()) {
            iter2->second.insert(k1);
        }
        else {
            set<uint32_t> temp;
            temp.insert(k1);
            acc_in_deg.insert(pair<uint32_t, set<uint32_t>>(k2, temp));
        }

		if(ifs.fail())
			break;
		datanum++;
#if defined(DEBUG) || defined(BINSTIME)
		if (datanum % (data_interval * 10000) == 0) {
			dn++;
			gettimeofday( &bpoint_end, NULL);
			double bpoint_time = (bpoint_end.tv_sec - bpoint_start.tv_sec) +  (bpoint_end.tv_usec - bpoint_start.tv_usec) / 1000000.0;
			cout << datanum << ", Break Point Time = " << bpoint_time << " s" << endl;

			cout << "Throughput: " << (double) ((data_interval * 10000) / (double) (bpoint_time)) << " tuples/s" << endl;

			gettimeofday( &bpoint_start, NULL);
		}
#endif
#if defined(DEBUG) || defined(HINT)
		if (datanum % 100000 == 0) {
			newv = (int) ((double) datanum / total * 100);
			if (newv != oldv) {
				progress_bar(newv);
				oldv = newv;
			}
		}
		if (datanum == total) {
			progress_bar(100);
			cout << endl;
		}
#endif
	}
#if defined(DEBUG) || defined(BINSTIME)
	gettimeofday( &bpoint_end, NULL);
	double bpoint_time = (bpoint_end.tv_sec - bpoint_start.tv_sec) +  (bpoint_end.tv_usec - bpoint_start.tv_usec) / 1000000.0;
	cout << endl << datanum << ", Break Point Time = " << bpoint_time << " s" << endl;

	cout << "Throughput: " << (double) ((datanum - (dn * data_interval * 10000)) / (double) (bpoint_time)) << " tuples/s" << endl;

#endif
#if defined(DEBUG) || defined(TINSTIME) || defined(BINSTIME) || defined(HINT)
	gettimeofday( &ins_end, NULL);
	double ins_time = (ins_end.tv_sec - ins_start.tv_sec) +  (ins_end.tv_usec - ins_start.tv_usec) / 1000000.0;
	cout << "Insertion Time = " << ins_time << " s" << endl;
	cout << "Insertion Finished!" << endl;
	cout << "Datanum = " << datanum << endl;
#endif
	ifs.close();
	return 0;
}

template <class T>
uint32_t edgeFrequenceQueryTest(T& tcm, string input_dir, string output_dir, string dataset_name, vector<int> num, int query_times, bool write) {
	string input_file_prefix = dataset_name + "-edge-input-";
	string input_file_suffix = ".txt";
	string output_file_prefix = dataset_name + "-edge-output-";
	string output_file_suffix = ".txt";
	string time_file_suffix = "-time.txt";

	//edge query process
	for (int i = 0; i < num.size(); i++) {
		query_data_pairs = count_lines(input_dir + input_file_prefix + to_string(num[i]) + input_file_suffix) + 10;
		QueryData* dataArray = new QueryData[query_data_pairs];
		int datanum = readRandomFileToDataArray(input_dir + input_file_prefix + to_string(num[i]) + input_file_suffix, dataArray);

		ofstream resultFile, timeFile;
		if (write) {
			char dir_path[FILENAME_MAX];
			strcpy(dir_path, output_dir.c_str());
			if (createDirectory(dir_path) != 0) {
				cout << "CreateDirectory Error, Path = " << dir_path << endl;
				return -1;
			}
			resultFile.open(output_dir + output_file_prefix + to_string(num[i]) + output_file_suffix);
			if (!resultFile.is_open()) {
				cout << "Error in open file, Path = " << (output_dir + output_file_prefix + to_string(num[i]) + output_file_suffix) << endl;
				return -1;
			}
			timeFile.open(output_dir + output_file_prefix + to_string(num[i]) + time_file_suffix);
			if (!timeFile.is_open()) {
				cout << "Error in open file, Path = " << (output_dir + output_file_prefix + to_string(num[i]) + time_file_suffix) << endl;
				return -1;
			}
		}

		double sumTime = 0, sumTime_perquery = 0;
		timeval tp1, tp2;
		for (int m = 0; m < query_times; m++) {
			sumTime_perquery = 0;
			for (int n = 0; n < datanum; n++) {
				gettimeofday( &tp1, NULL);
				uint32_t res = tcm.edgeWeightQuery(to_string(dataArray[n].a), to_string(dataArray[n].b));
				gettimeofday( &tp2, NULL);
				double delta_t = (tp2.tv_sec - tp1.tv_sec) * 1000000 +  (tp2.tv_usec - tp1.tv_usec);
				sumTime_perquery += delta_t;
				if (write && m == 0) {
					if(n == (datanum - 1)) {
						resultFile << res;
						timeFile  << delta_t;
						break;
					}
					else {
						resultFile << res << endl;
						timeFile  << delta_t << endl;
					}
				}
			}
			sumTime += (sumTime_perquery / (double)datanum);
		}

		if (write) {
			resultFile.flush();
			timeFile.flush();
			resultFile.close();
			timeFile.close();
		}
		delete[] dataArray;
#if defined(DEBUG) || defined(HINT)
		cout << "win = " << num[i] << endl;
		double mseconds = (double)(sumTime / (double)query_times) / 1000; 
		printf("Query Times = %d, Query Avg Time = %lf ms\n\n", query_times, mseconds);
#endif
	}
	return 0;
}

template <class T>
uint32_t allEdgeFrequenceQuery(T& egss, string dataset, string output_dir, string dataset_name) {

	ifstream randomFile;
	randomFile.open(dataset);
	if (!randomFile.is_open()) {
		cout << "Error in open file, Path = " << dataset << endl;
		return -1;
	}
	ofstream resultFile;
	char dir_path[FILENAME_MAX];
	strcpy(dir_path, output_dir.c_str());
	if (createDirectory(dir_path) != 0) {
		cout << "CreateDirectory Error, Path = " << dir_path << endl;
		return -1;
	}
	resultFile.open(output_dir + dataset_name + "_all_edge_frequence_test.txt");
	if (!resultFile.is_open()) {
		cout << "Error in open file, Path = " << (output_dir + dataset_name + "_all_edge_frequence_test.txt") << endl;
		return -1;
	}


	int datanum = 0;
	uint64_t a, b, c, d;
	while (!randomFile.eof()) {
		randomFile >> a >> b >> c >> d;
		if(randomFile.fail())
			break;
		// edge query
		uint32_t res = egss.edgeWeightQuery(to_string(a), to_string(b));
		resultFile << res << endl;
		datanum++;
	}
	randomFile.close();

	resultFile.flush();
	resultFile.close();
#if defined(DEBUG) || defined(INFO)
	printf("datanum = %d\n\n", datanum);
#endif
	return 0;
}

template <class T>
uint32_t allEdgeFrequenceQuery1(T& egss, string input_dir, string output_dir, string dataset_name, bool write, uint32_t n, int fun) {
	string tail = (fun == 1) ? ("repeat-" + to_string(n)) : ("batch-" + to_string(n));
	string input_file = input_dir + dataset_name + "-all-edge-input.txt";
	string output_file = output_dir + dataset_name + "-all-edge-weight-output-" + tail + ".txt";
	string time_file = output_dir + dataset_name + "-all-edge-weight-time-" + tail + ".txt";
	ifstream randomFile;
	randomFile.open(input_file);
	if (!randomFile.is_open()) {
		cout << "Error in open file, Path = " << input_file << endl;
		return -1;
	}
	ofstream resultFile, timeFile;
	if (write) {
		char dir_path[FILENAME_MAX];
		strcpy(dir_path, output_dir.c_str());
		if (createDirectory(dir_path) != 0) {
			cout << "CreateDirectory Error, Path = " << dir_path << endl;
			return -1;
		}
		resultFile.open(output_file);
		if (!resultFile.is_open()) {
			cout << "Error in open file, Path = " << output_file << endl;
			return -1;
		}
		timeFile.open(time_file);
		if (!timeFile.is_open()) {
			cout << "Error in open file, Path = " << time_file << endl;
			return -1;
		}
	}

	int datanum = 0;
	uint64_t a, b;
	timeval tp1, tp2;
	uint32_t res = 0;
	double sumTime = 0;
	double avgTime = 0;
	double delta_t = 0;
	int cnt = 0;
	int group = 0;
	while (!randomFile.eof()) {
		randomFile >> a >> b;
		if(randomFile.fail())
			break;
		// edge query
		if (fun == 1) {		// 同一组重复query n次
			avgTime = 0;
			for (int i = 0; i < n; i++) {
				gettimeofday( &tp1, NULL);
				res = egss.edgeWeightQuery(to_string(a), to_string(b));
				gettimeofday( &tp2, NULL);
				delta_t = (tp2.tv_sec - tp1.tv_sec) * 1000000 +  (tp2.tv_usec - tp1.tv_usec);
				avgTime += delta_t;
			}
			avgTime /= n;
			sumTime += avgTime;
			if (write) {
				resultFile << res << endl;
				timeFile  << avgTime << endl;
			}
		}
		else if (fun == 2) {		// n条数据为1个batch
			gettimeofday( &tp1, NULL);
			res = egss.edgeWeightQuery(to_string(a), to_string(b));
			gettimeofday( &tp2, NULL);
			delta_t = (tp2.tv_sec - tp1.tv_sec) * 1000000 +  (tp2.tv_usec - tp1.tv_usec);
			avgTime += delta_t;
			cnt++;
			if (write && (cnt == n)) {
				group++;
				avgTime /= n;
				sumTime += avgTime;
				cnt = 0;
				resultFile << res << endl;
				timeFile  << avgTime << endl;
				avgTime = 0;
			} 
		}
		datanum++;
	}
	if (write) {
		resultFile.flush();
		timeFile.flush();
		resultFile.close();
		timeFile.close();
	}
	randomFile.close();
#if defined(DEBUG) || defined(INFO)
	if (fun == 1) {
		printf("datanum = %d\n\n", datanum);
		double mseconds = (double)(sumTime / (double)datanum) / 1000;
		printf("Sum_Time = %lf ms, Query Avg Time = %lf ms\n\n", sumTime / 1000.0, mseconds);
	}
	else if (fun == 2) {
		printf("group = %d\n\n", group);
		double mseconds = (double)(sumTime / (double)group) / 1000;
		printf("Sum_Time = %lf ms, Query Avg Time = %lf ms\n\n", sumTime / 1000.0, mseconds);
	}
#endif
	return 0;
}

// template <class T>
// uint32_t allEdgeFrequenceQuery1(T& egss, string input_dir, string output_dir, string dataset_name, bool write) {
// 	string input_file = input_dir + dataset_name + "-all-edge-input.txt";
// 	string output_file = output_dir + dataset_name + "-all-edge-weight-output.txt";
// 	string time_file = output_dir + dataset_name + "-all-edge-weight-time.txt";
// 	ifstream randomFile;
// 	randomFile.open(input_file);
// 	if (!randomFile.is_open()) {
// 		cout << "Error in open file, Path = " << input_file << endl;
// 		return -1;
// 	}
// 	ofstream resultFile, timeFile;
// 	if (write) {
// 		char dir_path[FILENAME_MAX];
// 		strcpy(dir_path, output_dir.c_str());
// 		if (createDirectory(dir_path) != 0) {
// 			cout << "CreateDirectory Error, Path = " << dir_path << endl;
// 			return -1;
// 		}
// 		resultFile.open(output_file);
// 		if (!resultFile.is_open()) {
// 			cout << "Error in open file, Path = " << output_file << endl;
// 			return -1;
// 		}
// 		timeFile.open(time_file);
// 		if (!timeFile.is_open()) {
// 			cout << "Error in open file, Path = " << time_file << endl;
// 			return -1;
// 		}
// 	}

// 	int datanum = 0;
// 	uint64_t a, b;
// 	timeval tp1, tp2;
// 	double sumTime = 0;
// 	while (!randomFile.eof()) {
// 		randomFile >> a >> b;
// 		if(randomFile.fail())
// 			break;
// 		// edge query
// 		gettimeofday( &tp1, NULL);
// 		uint32_t res = egss.edgeWeightQuery(to_string(a), to_string(b));
// 		gettimeofday( &tp2, NULL);
// 		double delta_t = (tp2.tv_sec - tp1.tv_sec) * 1000000 +  (tp2.tv_usec - tp1.tv_usec);
// 		sumTime += delta_t;
// 		if (write) {
// 			resultFile << res << endl;
// 			timeFile  << delta_t << endl;
// 		}
// 		datanum++;
// 	}
// 	if (write) {
// 		resultFile.flush();
// 		timeFile.flush();
// 		resultFile.close();
// 		timeFile.close();
// 	}
// 	randomFile.close();
// #if defined(DEBUG) || defined(INFO)
// 	printf("datanum = %d\n\n", datanum);
// 	double mseconds = (double)(sumTime / (double)datanum) / 1000;
// 	printf("Sum_Time = %lf ms, Query Avg Time = %lf ms\n\n", sumTime / 1000.0, mseconds);
// #endif
// 	return 0;
// }


template <class T>
uint32_t edgeExistenceQueryTest(T& tcm, string input_dir, string output_dir, string dataset_name, int query_times, bool write) {
	string input_file_prefix = "_bool_";
	string input_file_suffix = ".txt";
	string output_file_prefix = "_bool_baseline_";
	string output_file_suffix = "_res.txt";
	string time_file_suffix = "_time.txt";
	//edge query process
	query_data_pairs = count_lines(input_dir + dataset_name + input_file_prefix + input_file_suffix) + 10;
	QueryData* dataArray = new QueryData[query_data_pairs];
	int datanum = readRandomFileToDataArray(input_dir + dataset_name + input_file_prefix + input_file_suffix, dataArray);
	ofstream resultFile, timeFile;
	if (write) {
		char dir_path[FILENAME_MAX];
		strcpy(dir_path, output_dir.c_str());
		if (createDirectory(dir_path) != 0) {
			cout << "createDirectory error" << endl;
			return -1;
		}
		resultFile.open(output_dir + dataset_name + output_file_prefix + output_file_suffix);
		if (!resultFile.is_open()) {
			cout << "error in open file " << (output_dir + dataset_name + output_file_prefix + output_file_suffix) << endl;
			return -1;
		}
		timeFile.open(output_dir + dataset_name + output_file_prefix + time_file_suffix);
		if (!timeFile.is_open()) {
			cout << "Error in open file, Path = " << (output_dir + dataset_name + output_file_prefix + time_file_suffix) << endl;
			return -1;
		}
	}

	double sumTime = 0, sumTime_perquery = 0;
	int ones = 0;
	timeval tp1, tp2;
	for (int m = 0; m < query_times; m++) {
		sumTime_perquery = 0;
		for (int n = 0; n < datanum; n++) {
			gettimeofday( &tp1, NULL);
			uint32_t res = tcm.edgeWeightQuery(to_string(dataArray[n].a), to_string(dataArray[n].b));
			gettimeofday( &tp2, NULL);
			double delta_t = (tp2.tv_sec - tp1.tv_sec) * 1000000 +  (tp2.tv_usec - tp1.tv_usec);
			sumTime_perquery += delta_t;
			if (write && m == 0) {
				if (res > 0)   
					ones++;
				if(n == (datanum - 1)) {
					resultFile << ((res > 0) ? 1 : 0);
					timeFile  << delta_t;
					break;
				}
				else {
					resultFile << ((res > 0) ? 1 : 0) << endl;
					timeFile  << delta_t << endl;
				}
			}
		}
		sumTime += (sumTime_perquery / (double)datanum);
	}
	
	if (write) {
		resultFile.flush();
		timeFile.flush();
		resultFile.close();
		timeFile.close();
	}
	delete[] dataArray;
#if defined(DEBUG) || defined(HINT)
	double mseconds = (double)(sumTime / (double)query_times) / 1000; 
	printf("Query Times = %d, Query Avg Time = %lf ms\n\n", query_times, mseconds);
#endif
	return 0;
}
template <class T>
uint32_t nodeFrequenceQueryTest(T& tcm, string input_dir, string output_dir, string dataset_name, vector<int> num, int query_times, bool write, int flag) {
	string file_suffix = ".txt";
	string input_file = "";
    string output_file = "";
    string time_file = "";
	switch (flag) {
		case 1:
			input_file = dataset_name + "-out-deg-input-";
			output_file = dataset_name + "-out-weight-output-";
			time_file = dataset_name + "-out-weight-time-";
			break;
		case 2:
			input_file = dataset_name + "-in-deg-input-";
            output_file = dataset_name + "-in-weight-output-";
            time_file = dataset_name + "-in-weight-time-";
			break;
		default:
			break;
	}
	//node query process
	for (int i = 0; i < num.size(); i++) {
		query_data_pairs = count_lines(input_dir + input_file + to_string(num[i]) + file_suffix) + 10;
		QueryData* dataArray = new QueryData[query_data_pairs];
		int datanum = readRandomFileToDataArray(input_dir + input_file + to_string(num[i]) + file_suffix, dataArray);
		ofstream resultFile, timeFile;
		if (write) {
			char dir_path[FILENAME_MAX];
			strcpy(dir_path, output_dir.c_str());
			if (createDirectory(dir_path) != 0) {
				cout << "createDirectory error" << endl;
				return -1;
			}
			resultFile.open(output_dir + output_file + to_string(num[i]) + file_suffix);
			if (!resultFile.is_open()) {
				cout << "Error in open file, Path = " << (output_dir + output_file + to_string(num[i]) + file_suffix) << endl;
				return -1;
			}
			timeFile.open(output_dir + time_file + to_string(num[i]) + file_suffix);
			if (!timeFile.is_open()) {
				cout << "Error in open file, Path = " << (output_dir + time_file + to_string(num[i]) + file_suffix) << endl;
				return -1;
			}
		}

		double sumTime = 0, sumTime_perquery = 0;	
		timeval tp1, tp2;
		for (int m = 0; m < query_times; m++) {
			sumTime_perquery = 0;
			for (int n = 0; n < datanum; n++) {			
				gettimeofday( &tp1, NULL);
				int32_t res = tcm.nodeWeightQuery(to_string(dataArray[n].a), (int)dataArray[n].b);
				gettimeofday( &tp2, NULL);
				double delta_t = (tp2.tv_sec - tp1.tv_sec) * 1000000 +  (tp2.tv_usec - tp1.tv_usec);
				sumTime_perquery += delta_t;
				
				if (write && m == 0) {
					if(n == (datanum - 1)) {
						resultFile << res;
						timeFile  << delta_t;
						break;
					}
					else {
						resultFile << res << endl;
						timeFile  << delta_t << endl;
					}
				}
			}
			sumTime += (sumTime_perquery / (double)datanum);
		}

		if (write) {
			resultFile.flush();
			timeFile.flush();
			resultFile.close();
			timeFile.close();
		}
#if defined(DEBUG) || defined(HINT)
 	cout << "win = " << num[i] << endl;
	double mseconds = (double)(sumTime / (double)query_times) / 1000; 
	printf("Query Times = %d, Query Avg Time = %lf ms\n\n", query_times, mseconds);
#endif
		delete[] dataArray;
	}
	return 0;
}
template <class T>
uint32_t reachabilityQueryTest(T& tcm, string input_file_header, string output_file_header, int query_times, bool write) {
	string input_file_suffix = ".txt";
    string output_file_suffix = "_output.txt";
    string time_file_suffix = "_time.txt";
	//node query process
	query_data_pairs = count_lines(input_file_header + input_file_suffix) + 10;
	QueryData* dataArray = new QueryData[query_data_pairs];
	int datanum = readRandomFileToDataArray(input_file_header + input_file_suffix, dataArray);
	ofstream resultFile, timeFile;
	if (write) {
		resultFile.open(output_file_header + output_file_suffix);
		if (!resultFile.is_open()) {
			cout << "Error in open file, Path = " << (output_file_header + output_file_suffix) << endl;
			return -1;
		}
		timeFile.open(output_file_header + time_file_suffix);
		if (!timeFile.is_open()) {
			cout << "Error in open file, Path = " << (output_file_header + time_file_suffix) << endl;
			return -1;
		}
	}

	double sumTime = 0, sumTime_perquery = 0;	
	timeval tp1, tp2;
	int ones = 0;
	for (int m = 0; m < query_times; m++) {
		sumTime_perquery = 0;
		for (int n = 0; n < datanum; n++) {
			gettimeofday( &tp1, NULL);
			uint32_t res = tcm.reachabilityQuery(to_string(dataArray[n].a), to_string(dataArray[n].b));
			gettimeofday( &tp2, NULL);
			double delta_t = (tp2.tv_sec - tp1.tv_sec) * 1000000 +  (tp2.tv_usec - tp1.tv_usec);
			sumTime_perquery += delta_t;
			if (write && m == 0) {
				if (res > 0)   
					ones++;
				if(n == (datanum - 1)) {
					resultFile << ((res > 0) ? 1 : 0);
					timeFile  << delta_t;
					break;
				}
				else {
					resultFile << ((res > 0) ? 1 : 0) << endl;
					timeFile  << delta_t << endl;
				}
			}
		}
		sumTime += (sumTime_perquery / (double)datanum);
	}

	if (write) {
		resultFile.flush();
		timeFile.flush();
		resultFile.close();
		timeFile.close();
	}
#if defined(DEBUG) || defined(INFO)
	double mseconds = (double)(sumTime / (double)query_times) / 1000;
	printf("Ones = %d, Query Times = %d, Query Avg Time = %lf ms\n\n", ones, query_times, mseconds);
#endif
	delete[] dataArray;
	return 0;
}


uint32_t reachabilityQueryTest1(GSS gss, string input_file_header, string output_file_header, int query_times, bool write) {
	string input_file_suffix = ".txt";
    string output_file_suffix = "_output.txt";
    string time_file_suffix = "_time.txt";
	//node query process
	query_data_pairs = count_lines(input_file_header + input_file_suffix) + 10;
	QueryData* dataArray = new QueryData[query_data_pairs];
	int datanum = readRandomFileToDataArray(input_file_header + input_file_suffix, dataArray);
	ofstream resultFile, timeFile;
	if (write) {
		resultFile.open(output_file_header + output_file_suffix);
		if (!resultFile.is_open()) {
			cout << "Error in open file, Path = " << (output_file_header + output_file_suffix) << endl;
			return -1;
		}
		timeFile.open(output_file_header + time_file_suffix);
		if (!timeFile.is_open()) {
			cout << "Error in open file, Path = " << (output_file_header + time_file_suffix) << endl;
			return -1;
		}
	}

	double sumTime = 0, sumTime_perquery = 0;	
	timeval tp1, tp2;
	int ones = 0;
	int points = 0, lines = 0;
	for (int m = 0; m < query_times; m++) {
		sumTime_perquery = 0;
		for (int n = 0; n < datanum; n++) {
			gettimeofday( &tp1, NULL);
			uint32_t res = gss.reachabilityQuery(to_string(dataArray[n].a), to_string(dataArray[n].b), points, lines);
			gettimeofday( &tp2, NULL);
			double delta_t = (tp2.tv_sec - tp1.tv_sec) * 1000000 +  (tp2.tv_usec - tp1.tv_usec);
			sumTime_perquery += delta_t;
			if (write && m == 0) {
				if (res > 0)   
					ones++;
				if(n == (datanum - 1)) {
					resultFile << ((res > 0) ? 1 : 0);
					timeFile  << delta_t << " " << points << " " << lines;
					break;
				}
				else {
					resultFile << ((res > 0) ? 1 : 0) << endl;
					timeFile  << delta_t << " " << points << " " << lines << endl;
				}
			}
		}
		sumTime += (sumTime_perquery / (double)datanum);
	}

	if (write) {
		resultFile.flush();
		timeFile.flush();
		resultFile.close();
		timeFile.close();
	}
#if defined(DEBUG) || defined(INFO)
	double mseconds = (double)(sumTime / (double)query_times) / 1000;
	printf("Ones = %d, Query Times = %d, Query Avg Time = %lf ms\n\n", ones, query_times, mseconds);
#endif
	delete[] dataArray;
	return 0;
}

// -----------------------insert while query--------------------------------- //
template <class T>	// int data_interval-10000为单位
int reachabilityQueryWhileInsert(T& tcm, string filename, int type, int data_interval, string input_dir, string output_dir, string dataset_name, int query_times, bool write) {
	ifstream ifs;
	ifs.open(filename);
	if (!ifs.is_open()) {
		cout << "Error in open file, Path = " << filename << endl;
		return -1;
	}
	
	if (write) {
		char dir_path[FILENAME_MAX];
		strcpy(dir_path, output_dir.c_str());
		if (createDirectory(dir_path) != 0) {
			cout << "createDirectory error" << endl;
			return -1;
		}
	}
	
	uint32_t s, d, w, t;
#if defined(DEBUG) || defined(INFO)
	cout << "Inserting..." << endl;
	double total = count_lines(filename);
	if(total == 0)
		cout << "ERROR--QueryFunction--442" <<endl;
#endif
#if defined(DEBUG) || defined(INFO)
	timeval ins_start, ins_end;
	gettimeofday( &ins_start, NULL);
#endif
#if defined(DEBUG)
	timeval bpoint_start, bpoint_end;
	gettimeofday( &bpoint_start, NULL);
#endif
	int datanum = 0;
	int newv = 0, oldv = 0;
	int flag = 1;
	while (!ifs.eof()) {
		if (type == 0) {
			ifs >> s >> d >> w >> t;
			tcm.insert(to_string(s), to_string(d), w);
		}
		else if (type == 1) {
			ifs >> s >> d >> t;
			tcm.insert(to_string(s), to_string(d), 1);
		}
		else if (type == 2) {
			ifs >> s >> d;
			tcm.insert(to_string(s), to_string(d), 1);
		}
		if(ifs.fail())
			break;
		datanum++;
#if defined(DEBUG)
		if (datanum % (data_interval * 10000) == 0) {
			gettimeofday( &bpoint_end, NULL);
			double bpoint_time = (bpoint_end.tv_sec - bpoint_start.tv_sec) +  (bpoint_end.tv_usec - bpoint_start.tv_usec) / 1000000.0;
			cout << endl << datanum << ", Break Point Time = " << bpoint_time << " s" << endl;

			string input_file_header = input_dir + "//PRQ_" + dataset_name + "_" + to_string(flag);
			string output_file_header = output_dir + "//PRQ_" + dataset_name + "_" + to_string(flag);
			//query...
			reachabilityQueryTest(tcm, input_file_header, output_file_header, query_times, write);
			flag++;

			gettimeofday( &bpoint_start, NULL);
		}
#endif
#if defined(DEBUG) || defined(INFO)
		if (datanum % 100000 == 0) {
			newv = (int) ((double) datanum / total * 100);
			if (newv != oldv) {
				progress_bar(newv);
				oldv = newv;
			}
		}
		if (datanum == total) {
			progress_bar(100);
			cout << endl;
		}
#endif
	}
#if defined(DEBUG)
	gettimeofday( &bpoint_end, NULL);
	double bpoint_time = (bpoint_end.tv_sec - bpoint_start.tv_sec) +  (bpoint_end.tv_usec - bpoint_start.tv_usec) / 1000000.0;
	cout << endl << datanum << ", Break Point Time = " << bpoint_time << " s" << endl;
#endif
#if defined(DEBUG) || defined(INFO)
	gettimeofday( &ins_end, NULL);
	double ins_time = (ins_end.tv_sec - ins_start.tv_sec) +  (ins_end.tv_usec - ins_start.tv_usec) / 1000000.0;
	cout << "Insertion Time = " << ins_time << " s" << endl;
	cout << "Insertion Finished!" << endl;
	cout << "Datanum = " << datanum << endl;
#endif
	ifs.close();
	return 0;
}

uint32_t nodeFrequenceQueryTestTime(GSS& gss, string input_dir, string output_dir, string dataset_name, vector<int> num, int query_times, bool write, int flag) {
	string file_suffix = ".txt";
	string input_file = "";
    string output_file = "";
    string time_file = "";
	switch (flag) {
		case 1:
			input_file = dataset_name + "-out-deg-input-";
			output_file = dataset_name + "-out-weight-output-";
			time_file = dataset_name + "-out-weight-time-";
			break;
		case 2:
			input_file = dataset_name + "-in-deg-input-";
            output_file = dataset_name + "-in-weight-output-";
            time_file = dataset_name + "-in-weight-time-";
			break;
		default:
			break;
	}
	
	//node query process
	for (int i = 0; i < num.size(); i++) {
		query_data_pairs = count_lines(input_dir + input_file + to_string(num[i]) + file_suffix) + 10;
		QueryData* dataArray = new QueryData[query_data_pairs];
		int datanum = readRandomFileToDataArray(input_dir + input_file + to_string(num[i]) + file_suffix, dataArray);
		ofstream resultFile, timeFile;
		if (write) {
			char dir_path[FILENAME_MAX];
			strcpy(dir_path, output_dir.c_str());
			if (createDirectory(dir_path) != 0) {
				cout << "createDirectory error" << endl;
				return -1;
			}
			resultFile.open(output_dir + output_file + to_string(num[i]) + file_suffix);
			if (!resultFile.is_open()) {
				cout << "Error in open file, Path = " << (output_dir + output_file + to_string(num[i]) + file_suffix) << endl;
				return -1;
			}
			timeFile.open(output_dir + time_file + to_string(num[i]) + file_suffix);
			if (!timeFile.is_open()) {
				cout << "Error in open file, Path = " << (output_dir + time_file + to_string(num[i]) + file_suffix) << endl;
				return -1;
			}
		}

		double sumTime = 0, sumTime_perquery = 0;
		double t_m = 0, t_b = 0;
		double matrix_t = 0, buffer_t = 0;
		double t_all = 0;
		timeval tp1, tp2;
		for (int m = 0; m < query_times; m++) {
			sumTime_perquery = 0;
			for (int n = 0; n < datanum; n++) {			
				gettimeofday( &tp1, NULL);
				int32_t res = gss.nodeWeightQueryTime(to_string(dataArray[n].a), (int)dataArray[n].b, matrix_t, buffer_t);
				gettimeofday( &tp2, NULL);
				double delta_t = (tp2.tv_sec - tp1.tv_sec) * 1000000 +  (tp2.tv_usec - tp1.tv_usec);
				sumTime_perquery += delta_t;
				t_all += delta_t;
				t_m += matrix_t;
				t_b += buffer_t;
				
				if (write && m == 0) {
					if(n == (datanum - 1)) {
						resultFile << res;
						timeFile  << delta_t << " " << matrix_t << " " << buffer_t;
						break;
					}
					else {
						resultFile << res << endl;
						timeFile  << delta_t << " " << matrix_t << " " << buffer_t << endl;
					}
				}
			}
			sumTime += (sumTime_perquery / (double)datanum);
		}

		if (write) {
			resultFile.flush();
			timeFile.flush();
			resultFile.close();
			timeFile.close();
		}
#if defined(DEBUG) || defined(HINT)
 	cout << "win = " << num[i] << endl;
	double mseconds = (double)(sumTime / (double)query_times) / 1000; 
	printf("Query Times = %d, Query Avg Time = %lf ms\n", query_times, mseconds);
	// t_all = t_m + t_b;
	printf("Matrix time = %lf, Buffer Time = %lf, Total Time = %lf\n", t_m, t_b, t_all);
	printf("Avg Matrix time = %lf, Avg Buffer Time = %lf, Avg Total Time = %lf\n\n", t_m / (datanum * query_times) , t_b / (datanum * query_times), t_all / (datanum * query_times));
#endif
		delete[] dataArray;
	}
	return 0;
}

uint32_t edgeFrequenceQueryTestTime(GSS& gss, string input_dir, string output_dir, string dataset_name, int query_times, bool write) {
	string input_file_prefix = dataset_name + "_random_edge_frequence_";
	string input_file_suffix = "_sorted.txt";
	string output_file_prefix = dataset_name + "_edge_frequence_baseline_";
	string output_file_suffix = "_res.txt";
	string time_file_suffix = "_time.txt";
	
	//edge query process
	query_data_pairs = count_lines(input_dir + input_file_prefix + input_file_suffix) + 10;
	QueryData* dataArray = new QueryData[query_data_pairs];
	int datanum = readRandomFileToDataArray(input_dir + input_file_prefix + input_file_suffix, dataArray);

	ofstream resultFile, timeFile;
	if (write) {
		char dir_path[FILENAME_MAX];
		strcpy(dir_path, output_dir.c_str());
		if (createDirectory(dir_path) != 0) {
			cout << "CreateDirectory Error, Path = " << dir_path << endl;
			return -1;
		}
		resultFile.open(output_dir + output_file_prefix + output_file_suffix);
		if (!resultFile.is_open()) {
			cout << "Error in open file, Path = " << (output_dir + output_file_prefix + output_file_suffix) << endl;
			return -1;
		}
		timeFile.open(output_dir + output_file_prefix + time_file_suffix);
		if (!timeFile.is_open()) {
			cout << "Error in open file, Path = " << (output_dir + output_file_prefix + time_file_suffix) << endl;
			return -1;
		}
	}

	double sumTime = 0, sumTime_perquery = 0;
	timeval tp1, tp2;
	double t_m = 0, t_b = 0, t_h = 0, t_a = 0;
	double matrix_t = 0, buffer_t = 0, hash_t = 0, addr_t = 0;
	double t_all = 0;
	for (int m = 0; m < query_times; m++) {
		sumTime_perquery = 0;
		for (int n = 0; n < datanum; n++) {
			gettimeofday( &tp1, NULL);
			uint32_t res = gss.edgeWeightQueryTime(to_string(dataArray[n].a), to_string(dataArray[n].b), matrix_t, buffer_t, hash_t, addr_t);
			gettimeofday( &tp2, NULL);
			double delta_t = (tp2.tv_sec - tp1.tv_sec) * 1000000 +  (tp2.tv_usec - tp1.tv_usec);
			sumTime_perquery += delta_t;
			t_all += delta_t;
			t_m += matrix_t;
			t_b += buffer_t;
			t_h += hash_t;
			t_a += addr_t;
			if (write && m == 0) {
				if(n == (datanum - 1)) {
					resultFile << res;
					timeFile  << delta_t << " " << matrix_t << " " << buffer_t <<  " " << hash_t <<  " " << addr_t;
					break;
				}
				else {
					resultFile << res << endl;
					timeFile  << delta_t << " " << matrix_t << " " << buffer_t <<  " " << hash_t <<  " " << addr_t << endl;
				}
			}
		}
		sumTime += (sumTime_perquery / (double)datanum);
	}

	if (write) {
		resultFile.flush();
		timeFile.flush();
		resultFile.close();
		timeFile.close();
	}
	delete[] dataArray;
#if defined(DEBUG) || defined(HINT)
	double mseconds = (double)(sumTime / (double)query_times) / 1000;
	printf("Query Times = %d, Query Avg Time = %lf ms\n", query_times, mseconds);
	t_all = t_m + t_b + t_h + t_a;
	printf("Matrix time = %lf, Buffer Time = %lf, Hash Time = %lf, Addr Time = %lf\n\n", (t_m / t_all), (t_b / t_all), (t_h / t_all), (t_a / t_all));
#endif
	return 0;
}
uint32_t edgeExistenceQueryTestTime(GSS& gss, string input_dir, string output_dir, string dataset_name, int query_times, bool write) {
	string input_file_prefix = "_bool_";
	string input_file_suffix = ".txt";
	string output_file_prefix = "_bool_baseline_";
	string output_file_suffix = "_res.txt";
	string time_file_suffix = "_time.txt";

	//edge query process
	query_data_pairs = count_lines(input_dir + dataset_name + input_file_prefix + input_file_suffix) + 10;
	QueryData* dataArray = new QueryData[query_data_pairs];
	int datanum = readRandomFileToDataArray(input_dir + dataset_name + input_file_prefix + input_file_suffix, dataArray);
	ofstream resultFile, timeFile;
	if (write) {
		char dir_path[FILENAME_MAX];
		strcpy(dir_path, output_dir.c_str());
		if (createDirectory(dir_path) != 0) {
			cout << "createDirectory error" << endl;
			return -1;
		}
		resultFile.open(output_dir + dataset_name + output_file_prefix + output_file_suffix);
		if (!resultFile.is_open()) {
			cout << "error in open file " << (output_dir + dataset_name + output_file_prefix + output_file_suffix) << endl;
			return -1;
		}
		timeFile.open(output_dir + dataset_name + output_file_prefix + time_file_suffix);
		if (!timeFile.is_open()) {
			cout << "Error in open file, Path = " << (output_dir + dataset_name + output_file_prefix + time_file_suffix) << endl;
			return -1;
		}
	}

	double sumTime = 0, sumTime_perquery = 0;
	int ones = 0;
	timeval tp1, tp2;
	double t_m = 0, t_b = 0, t_h = 0, t_a = 0;
	double matrix_t = 0, buffer_t = 0, hash_t = 0, addr_t = 0;
	double t_all = 0;
	for (int m = 0; m < query_times; m++) {
		sumTime_perquery = 0;
		for (int n = 0; n < datanum; n++) {
			gettimeofday( &tp1, NULL);
			uint32_t res = gss.edgeWeightQueryTime(to_string(dataArray[n].a), to_string(dataArray[n].b), matrix_t, buffer_t, hash_t, addr_t);
			gettimeofday( &tp2, NULL);
			double delta_t = (tp2.tv_sec - tp1.tv_sec) * 1000000 +  (tp2.tv_usec - tp1.tv_usec);
			sumTime_perquery += delta_t;
			t_all += delta_t;
			t_m += matrix_t;
			t_b += buffer_t;
			t_h += hash_t;
			t_a += addr_t;
			if (write && m == 0) {
				if (res > 0)   
					ones++;
				if(n == (datanum - 1)) {
					resultFile << ((res > 0) ? 1 : 0);
					timeFile  << delta_t << " " << matrix_t << " " << buffer_t <<  " " << hash_t <<  " " << addr_t;
					break;
				}
				else {
					resultFile << ((res > 0) ? 1 : 0) << endl;
					timeFile  << delta_t << " " << matrix_t << " " << buffer_t <<  " " << hash_t <<  " " << addr_t << endl;
				}
			}
		}
		sumTime += (sumTime_perquery / (double)datanum);
	}
	
	if (write) {
		resultFile.flush();
		timeFile.flush();
		resultFile.close();
		timeFile.close();
	}
	delete[] dataArray;
#if defined(DEBUG) || defined(HINT)
	double mseconds = (double)(sumTime / (double)query_times) / 1000; 
	printf("Query Times = %d, Query Avg Time = %lf ms\n", query_times, mseconds);	
	t_all = t_m + t_b + t_h + t_a;
	printf("Matrix time = %lf, Buffer Time = %lf, Hash Time = %lf, Addr Time = %lf\n\n", (t_m / t_all), (t_b / t_all), (t_h / t_all), (t_a / t_all));
#endif
	return 0;
}
// uint32_t nodeFrequenceQueryTestTime(GSS& gss, string input_dir, string output_dir, string dataset_name, vector<int> num, int query_times, bool write, int flag) {
// 	string file_suffix = ".txt";
// 	string input_file = "";
//     string output_file = "";
//     string time_file = "";
// 	switch (flag) {
// 		case 1:
// 			input_file = dataset_name + "-out-deg-input-";
// 			output_file = dataset_name + "-out-weight-output-";
// 			time_file = dataset_name + "-out-weight-time-";
// 			break;
// 		case 2:
// 			input_file = dataset_name + "-in-deg-input-";
//             output_file = dataset_name + "-in-weight-output-";
//             time_file = dataset_name + "-in-weight-time-";
// 			break;
// 		default:
// 			break;
// 	}
	
// 	//node query process
// 	for (int i = 0; i < num.size(); i++) {
// 		query_data_pairs = count_lines(input_dir + input_file + to_string(num[i]) + file_suffix) + 10;
// 		QueryData* dataArray = new QueryData[query_data_pairs];
// 		int datanum = readRandomFileToDataArray(input_dir + input_file + to_string(num[i]) + file_suffix, dataArray);
// 		ofstream resultFile, timeFile;
// 		if (write) {
// 			char dir_path[FILENAME_MAX];
// 			strcpy(dir_path, output_dir.c_str());
// 			if (createDirectory(dir_path) != 0) {
// 				cout << "createDirectory error" << endl;
// 				return -1;
// 			}
// 			resultFile.open(output_dir + output_file + to_string(num[i]) + file_suffix);
// 			if (!resultFile.is_open()) {
// 				cout << "Error in open file, Path = " << (output_dir + output_file + to_string(num[i]) + file_suffix) << endl;
// 				return -1;
// 			}
// 			timeFile.open(output_dir + time_file + to_string(num[i]) + file_suffix);
// 			if (!timeFile.is_open()) {
// 				cout << "Error in open file, Path = " << (output_dir + time_file + to_string(num[i]) + file_suffix) << endl;
// 				return -1;
// 			}
// 		}

// 		double sumTime = 0, sumTime_perquery = 0;
// 		double t_m = 0, t_b = 0, t_h = 0, t_a = 0;
// 		double matrix_t = 0, buffer_t = 0, hash_t = 0, addr_t = 0;
// 		double t_all = 0;	
// 		timeval tp1, tp2;
// 		for (int m = 0; m < query_times; m++) {
// 			sumTime_perquery = 0;
// 			for (int n = 0; n < datanum; n++) {			
// 				gettimeofday( &tp1, NULL);
// 				int32_t res = gss.nodeWeightQueryTime(to_string(dataArray[n].a), (int)dataArray[n].b, matrix_t, buffer_t, hash_t, addr_t);
// 				gettimeofday( &tp2, NULL);
// 				double delta_t = (tp2.tv_sec - tp1.tv_sec) * 1000000 +  (tp2.tv_usec - tp1.tv_usec);
// 				sumTime_perquery += delta_t;
// 				t_all += delta_t;
// 				t_m += matrix_t;
// 				t_b += buffer_t;
// 				t_h += hash_t;
// 				t_a += addr_t;
				
// 				if (write && m == 0) {
// 					if(n == (datanum - 1)) {
// 						resultFile << res;
// 						timeFile  << delta_t << " " << matrix_t << " " << buffer_t <<  " " << hash_t <<  " " << addr_t;
// 						break;
// 					}
// 					else {
// 						resultFile << res << endl;
// 						timeFile  << delta_t << " " << matrix_t << " " << buffer_t <<  " " << hash_t <<  " " << addr_t << endl;
// 					}
// 				}
// 			}
// 			sumTime += (sumTime_perquery / (double)datanum);
// 		}

// 		if (write) {
// 			resultFile.flush();
// 			timeFile.flush();
// 			resultFile.close();
// 			timeFile.close();
// 		}
// #if defined(DEBUG) || defined(HINT)
//  	cout << "win = " << num[i] << endl;
// 	double mseconds = (double)(sumTime / (double)query_times) / 1000; 
// 	printf("Query Times = %d, Query Avg Time = %lf ms\n", query_times, mseconds);
// 	t_all = t_m + t_b + t_h + t_a;
// 	printf("Matrix time = %lf, Buffer Time = %lf, Hash Time = %lf, Addr Time = %lf\n\n", (t_m / t_all), (t_b / t_all), (t_h / t_all), (t_a / t_all));
// #endif
// 		delete[] dataArray;
// 	}
// 	return 0;
// }

#endif // #ifndef QUERYFUNCTION_H