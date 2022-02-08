//测试边查、点查等结果
#include <iomanip>
#include "headers/QueryFunction.h"

int main(int argc, char* argv[]) {
#if defined(DEBUG) || defined(HINT)  	
    cout << fixed;  //强制使用小数,防止使用科学计数法
	cout << setprecision(7);    //控制显示的精度，控制小数点后面的位数
	timeval main_start, main_end;
	gettimeofday( &main_start, NULL);
	for (int i = 0; i < argc; i++) {
		cout << argv[i] << " ";
	}
	cout << endl << endl;
#endif
    int width, depth, hashnum;      //TCM parameters
	string head_addr = "TCM_";
	string back_addr = "";
	int dataset = 3;
	int query_times = 1;						//查询的次数
	string filename, input_dir, output_dir;		//测试数据集文件路径  测试数据输入路径 测试结果输出路径
	string dataset_name, txt_name = "";
	int efflag = 0, eeflag = 0, nfflag = 0, rpqflag = 0; 	//  edge frequence query,  edge existence query,  node frequence query
	int rpq = 0;		// reachability path query
	int alledgeflag = 0;
	int fun = 1; 		// 边查，batch还是repeat
	uint32_t edge_n = 1000;		//边查，重复n次或者n为一个batch
	bool writeflag = false;						//是否将测试结果写入到文件
	int node_query_flag = 0;					//1-node_in_query, 2-node_out_query
	int input_type = 0;
	int window = 0;
	int data_interval = 100;
	bool time_tp = false;
	vector<int> win;
    // vector<int> win = {1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000};
	// vector<int> win = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100};
	//命令行参数
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-dataset") == 0) {
			dataset = atoi(argv[++i]);
		}
		if (strcmp(argv[i], "-hashnum") == 0) {
			hashnum = atoi(argv[++i]);
		}
		if (strcmp(argv[i], "-edgefrequence") == 0) {
			efflag = 1;
		}
		if (strcmp(argv[i], "-edgeexistence") == 0) {
			eeflag = 1;
		}
		if (strcmp(argv[i], "-nodefrequence") == 0) {
			nfflag = 1;
		}
		if (strcmp(argv[i], "-reachability") == 0) {
			rpqflag = 1;
		}
		if (strcmp(argv[i], "-rpq") == 0) {
			rpq = 1;
		}
		if (strcmp(argv[i], "-in") == 0) {
			node_query_flag = 2;
		}
		if (strcmp(argv[i], "-out") == 0) {
			node_query_flag = 1;
		}
		if (strcmp(argv[i], "-write") == 0) {
			writeflag = true;
		}
		if (strcmp(argv[i], "-qtimes") == 0) {
			query_times = atoi(argv[++i]);
		}
		if (strcmp(argv[i], "-input") == 0) {
			input_type = atoi(argv[++i]);
		}
		if (strcmp(argv[i], "-win") == 0) {
            window = atoi(argv[++i]);
        }
		if (strcmp(argv[i], "-datainterval") == 0) {
            data_interval = atoi(argv[++i]);
        }
		if (strcmp(argv[i], "-tail") == 0) {
			back_addr = argv[++i];
		}
		if (strcmp(argv[i], "-alledge") == 0) {
			alledgeflag = 1;
		}
		if (strcmp(argv[i], "-fun") == 0) {
			fun = atoi(argv[++i]);
		}
		if (strcmp(argv[i], "-edge_n") == 0) {
			edge_n = atoi(argv[++i]);
		}
		if (strcmp(argv[i], "-time_tp") == 0) {
            time_tp = true;
        }
	}
	if (window > 0) {
		for (int i = 1; i < 12; i++) {
			win.push_back(i * window);
		}
	}

	switch (dataset) {
		case 1:
			filename = "..//..//Dataset//wiki-talk";
			input_dir = "..//..//TestFiles//wiki-talk//input//";
			output_dir = "..//..//TestFiles//wiki-talk//output//";
			dataset_name = "wiki-talk";
            width = 3536;
			depth = 3536;
			break;
		case 2:
			filename = "..//..//Dataset//stackoverflow";
			input_dir = "..//..//TestFiles//stackoverflow//input//";
			output_dir = "..//..//TestFiles//stackoverflow//output//";
			dataset_name = "stackoverflow";
			width = 5656;
			depth = 5656;
			break;
		case 3:
			filename = "..//..//Dataset//dbpedia";
			input_dir = "..//..//TestFiles//dbpedia//input//";
			output_dir = "..//..//TestFiles//dbpedia//output//";
			dataset_name = "dbpedia";
            width = 9280;
			depth = 9280;
			break;
		case 4:
			filename = "..//..//Dataset//caida";
			input_dir = "..//..//TestFiles//caida//input//";
			output_dir = "..//..//TestFiles//caida//output//";
			dataset_name = "caida";
			width = 15000;
			depth = 15000;
			break;
		case 5:
			filename = "..//..//Dataset//delicious-ui";
			input_dir = "..//..//TestFiles//delicious-ui//input//";
			output_dir = "..//..//TestFiles//delicious-ui//output//";
			dataset_name = "delicious-ui";
            width = 17354;
			depth = 17356;
			break;
		default:
			break;
	}

	//命令行参数
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-vector") == 0) {
			win.clear();
			while(true) {
				if((i + 1) >= argc) {
					break;
				}
				if(argv[i + 1][0] < '0' || argv[i + 1][0] > '9') {
					break;
				}
				else {
					win.push_back(atoi(argv[++i]));
				}
			}
		}
		if (strcmp(argv[i], "-input_dir") == 0) {
			input_dir = argv[++i];
			input_dir += "//";
		}
		if (strcmp(argv[i], "-output_dir") == 0) {
			output_dir = argv[++i];
			output_dir += "//";
		}
		if (strcmp(argv[i], "-filename") == 0) {
			filename = argv[++i];
		}
		if (strcmp(argv[i], "-width") == 0) {
			width = atoi(argv[++i]);
		}
		if (strcmp(argv[i], "-depth") == 0) {
			depth = atoi(argv[++i]);
		}
	}
#if defined(DEBUG) || defined(HINT)
	cout << "*******************************************************" << endl;
	cout << "Print Infomation" << endl;
	cout << "TCM: width = " << width << ", depth = " << depth << ", hashnum = " << hashnum << endl;
	cout << "data interval = " << data_interval << endl;
	cout << "win = { " ;
	for (int i = 0; i < win.size(); i++) {
		cout << win[i] << " ";
	}
	cout << "}" << endl;
#endif
    TCM tcm(width, depth, hashnum);     //实例化TCM
	
    string test_situation_dir = head_addr + dataset_name + "_" + to_string(width) + "x" + to_string(depth) + "_hashnum_" + to_string(hashnum) + "_win_" + to_string(window) + back_addr +"//";
	output_dir += test_situation_dir;
	char dir_path[FILENAME_MAX];
	strcpy(dir_path, output_dir.c_str());
	if (createDirectory(dir_path) != 0) {
		cout << "Create Directory error" << endl;
		return -1;
	}
#if defined(DEBUG) || defined(HINT)
	cout << "dataset: " << filename << endl;
	cout << "input_dir: " << input_dir << endl;
	cout << "output_dir: " << output_dir << endl;
	cout << "write flag = " << writeflag << endl;
	cout << endl;
	cout << "*******************************************************" << endl << endl;
#endif

	if (rpq == 1) {
		cout << "**************** TCM Reachability Query While Insert Start ****************" << endl;
		reachabilityQueryWhileInsert(tcm, filename, input_type, data_interval, input_dir, output_dir, dataset_name, query_times, writeflag);
		cout << "****************  TCM Reachability Query While Insert End  ****************" << endl;
	}
	else if (time_tp) {
		cout << "**************** Scube Insert Time Throughput Start ****************" << endl;
		insert_tp_time(tcm, filename, input_type);
		cout << "****************  Scube Insert Time Throughput End  ****************" << endl;
	}
	else {
		// insertion process
	#if defined(DEBUG) || defined(HINT)
		cout << "****************** TCM insert start *****************" << endl;
	#endif
		insert(tcm, filename, input_type, data_interval);
	#if defined(DEBUG) || defined(HINT)
		cout << "****************** TCM insert end *******************" << endl << endl;
	#endif
	// query process
		if (efflag == 1) {
	#if defined(DEBUG) || defined(HINT)
			cout << "**************** TCM edge frequence start ****************" << endl;
	#endif
			edgeFrequenceQueryTest(tcm, input_dir, output_dir, dataset_name, win, query_times, writeflag);
	#if defined(DEBUG) || defined(HINT)
			cout << "***************** TCM edge frequence end *****************" << endl << endl;
	#endif
		}
		if (eeflag == 1) {
	#if defined(DEBUG) || defined(HINT)
			cout << "**************** TCM existence start ****************" << endl;
	#endif
			edgeFrequenceQueryTest(tcm, input_dir, output_dir, dataset_name, win, query_times, writeflag);
	#if defined(DEBUG) || defined(HINT)
			cout << "***************** TCM existence end *****************" << endl << endl;
	#endif
		}
		if (nfflag == 1) {
	#if defined(DEBUG) || defined(HINT)
			cout << "************* TCM node frequence start **************" << endl;
	#endif
			nodeFrequenceQueryTest(tcm, input_dir, output_dir, dataset_name, win, query_times, writeflag, node_query_flag);
	#if defined(DEBUG) || defined(HINT)
			cout << "************** TCM node frequence end ***************" << endl << endl;
	#endif
		}
		if (rpqflag == 1) {
	#if defined(DEBUG) || defined(HINT)
			cout << "************* TCM reachability query start **************" << endl;
	#endif
			reachabilityQueryTest(tcm, input_dir + "PRQ_" + dataset_name, output_dir + "//PRQ_" + dataset_name, query_times, writeflag);
	#if defined(DEBUG) || defined(HINT)
			cout << "************** TCM node frequence end ***************" << endl << endl;
	#endif
		}
		if (alledgeflag == 1) {
	#if defined(DEBUG) || defined(HINT)
			cout << "************* TCM all edge query start **************" << endl;
	#endif
			allEdgeFrequenceQuery1(tcm, input_dir, output_dir, dataset_name, writeflag, edge_n, fun);
	#if defined(DEBUG) || defined(HINT)
			cout << "************** TCM all edge query end ***************" << endl << endl;
	#endif
		}
	}

#if defined(DEBUG) || defined(HINT)
	gettimeofday( &main_end, NULL);
	double main_time = (main_end.tv_sec - main_start.tv_sec) + (main_end.tv_usec - main_start.tv_usec) / 1000000.0;
	cout << endl << "This program lasts for " << main_time / 60.0 << " min" << endl;
#endif
	return 0;
}