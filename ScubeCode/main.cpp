#include <iomanip>
#include "Scube/ScubeKick.h"
#include "headers/QueryFunction.h"

int main(int argc, char* argv[]) {
#if defined(DEBUG) || defined(INFO)  	
    cout << fixed;  				// 强制使用小数,防止使用科学计数法
	cout << setprecision(6);    	// 控制显示的精度，控制小数点后面的位数
	timeval main_start, main_end;
	gettimeofday( &main_start, NULL);
	cout << "[Info]-";
	for (int i = 0; i < argc; i++) {
		cout << argv[i] << " ";
	}
	cout << "ROOM = " << ROOM;
	cout << " SLOTROOM = " << SLOTROOM << endl;
	cout << endl << endl;
#endif
	int width, depth, fingerprint_length;	// Scube parameters
	uint16_t k_width, k_depth;				// Scube parameters
	double alpha = 0.8;						// Scube parameters
	uint32_t slot_nums, kick_times = 10;	// Scube parameters

	string head_scube_addr = "";
	string head_degdetect_addr = "";
	string back_addr = "";
	int dataset = 1;
	int query_times = 1;						// 查询的次数
	string filename, input_dir, output_dir;		// 测试数据集文件路径  测试数据输入路径 测试结果输出路径
	string dataset_name, txt_name = "";
	int efflag = 0, eeflag = 0, nfflag = 0, rpqflag = 0; 	// edge frequence query,  edge existence query,  node frequence query
	int rpq = 0;		// reachability path query
	int alledgeflag = 0;
	int fun = 1; 		// 边查，batch还是repeat
	uint32_t edge_n = 1000;		//边查，重复n次或者n为一个batch
	bool writeflag = false;						// 是否将测试结果写入到文件
	int node_query_flag = 0;					// 1-node_out_query, 2-node_in_query
	int input_type = 0;
	bool timetest = false;
	int scube_flag = 0;
	int degdetect = 0;
	int window = 10;
	int degSlotHash = 0;
	int data_interval = 100;
	bool ins_bd = false;
	int ignore_bits = 8;
	int reserved_bits = 2;
	double exp_deg;
	bool time_tp = false;
	
	vector<int> win;
	// vector<int> win = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100};
	// vector<int> win = {50, 100, 150, 200, 250, 300, 350, 400, 450, 500, 550};
	//命令行参数
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-dataset") == 0) {
			dataset = atoi(argv[++i]);
		}
		if (strcmp(argv[i], "-fplength") == 0) {
			fingerprint_length = atoi(argv[++i]);
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
		if (strcmp(argv[i], "-out") == 0) {
			node_query_flag = 1;
		}
		if (strcmp(argv[i], "-in") == 0) {
			node_query_flag = 2;
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
		if (strcmp(argv[i], "-time") == 0) {
			timetest = true;
		}
		if (strcmp(argv[i], "-scube") == 0) {
            scube_flag = atoi(argv[++i]);
        }
		if (strcmp(argv[i], "-degdetect") == 0) {
            degdetect = atoi(argv[++i]);
        }
		if (strcmp(argv[i], "-kicks") == 0) {
            kick_times = atoi(argv[++i]);
        }
		if (strcmp(argv[i], "-slots") == 0) {
            slot_nums = atoi(argv[++i]);
        }
		if (strcmp(argv[i], "-slothash") == 0) {
            degSlotHash = atoi(argv[++i]);
        }
		if (strcmp(argv[i], "-k_width") == 0) {
            k_width = atoi(argv[++i]);
        }
		if (strcmp(argv[i], "-k_depth") == 0) {
            k_depth = atoi(argv[++i]);
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
		if (strcmp(argv[i], "-ins-bd") == 0) {
            ins_bd = true;
        }
		if (strcmp(argv[i], "-ignore_bits") == 0) {
            ignore_bits = atoi(argv[++i]);
        }
		if (strcmp(argv[i], "-reserved_bits") == 0) {
            reserved_bits = atoi(argv[++i]);
        }
		if (strcmp(argv[i], "-alpha") == 0) {
            alpha = atof(argv[++i]);
        }
		if (strcmp(argv[i], "-exp") == 0) {
            exp_deg = atof(argv[++i]);
        }
		if (strcmp(argv[i], "-time_tp") == 0) {
            time_tp = true;
        }
	}
	for (int i = 1; i < 12; i++) {
		win.push_back(i * window);
	}

	head_scube_addr = "ScubeKick_";

	if (degdetect == 0) {
		cout << "请指定DegDetect: -degdetect 1--DegDetectorSlot, -degdetect 2--DegDetectorNewSlot, -degdetect 3--DegDetectorSlot2bit, -degdetect 4--DegDetectorUMap2bit" << endl;
		return -1;
	}
	else if (degdetect == 1) {
		head_degdetect_addr = "DegSlot_";
	}
	else if (degdetect == 2) {
		head_degdetect_addr = "DegNewSlot_";
	}
	else if (degdetect == 3) {
		head_degdetect_addr = "DegSlot2bit_";
	}
	else if (degdetect == 4) {
		head_degdetect_addr = "DegUMap2bit_";
	}

	switch (dataset) {
		case 1:
			filename = "..//..//Dataset//wiki-talk";
			dataset_name = "wiki-talk";
            width = 3536;
			depth = 3536;
			break;
		case 2:
			filename = "..//..//Dataset//stackoverflow";
			dataset_name = "stackoverflow";
			width = 5656;
			depth = 5656;
			break;
		case 3:
			filename = "..//..//Dataset//dbpedia";
			dataset_name = "dbpedia";
            width = 9280;
			depth = 9280;
			break;
		case 4:
			filename = "..//..//Dataset//caida";
			dataset_name = "caida";
			width = 15000;
			depth = 15000;
			break;
		case 5:
			filename = "..//..//Dataset//delicious-ui";
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

	if (writeflag) {
		string test_situation_dir = head_scube_addr + head_degdetect_addr + dataset_name + "_" + to_string(width) + "x" + to_string(depth) + "_ROOM_" + to_string(ROOM) +
		"_fp_" + to_string(fingerprint_length) + "_win_" + to_string(window);
		
		if (degdetect == 1) {
			test_situation_dir += ("_slotnums_" + to_string(slot_nums) + "_SLOTROOM_"+ to_string(SLOTROOM) + "_" + to_string(k_width) + "_" + to_string(k_depth) + back_addr + "//");
		}
		else if (degdetect == 2) {
			test_situation_dir += ("_slotnums_" + to_string(slot_nums) + "_SLOTROOM_"+ to_string(SLOTROOM) + "_" + to_string(k_width) + "_" + to_string(k_depth) + "_" + to_string(ignore_bits) + "_" + to_string(alpha) + back_addr + "//");
		}
		else if (degdetect == 3) {
			test_situation_dir += ("_slotnums_" + to_string(slot_nums) + "_SLOTROOM_"+ to_string(SLOTROOM) + "_" + to_string(k_width) + "_" + to_string(k_depth) + "_" + to_string(ignore_bits) + "_" + to_string(reserved_bits) + "_" + to_string(alpha) + back_addr + "//");
		}
		else if (degdetect == 4) {
			test_situation_dir += ("_" + to_string(ignore_bits) + "_" + to_string(reserved_bits) + "_" + to_string(alpha) + back_addr + "//");
		}
		 
		output_dir += test_situation_dir;
		char dir_path[FILENAME_MAX];
		strcpy(dir_path, output_dir.c_str());
		if (createDirectory(dir_path) != 0) {
			cout << "Create Directory error" << endl;
			return -1;
		}
	}
#if defined(DEBUG) || defined(INFO)
	cout << "************************** Print Parameters **************************" << endl;
	cout << "[Info]-Scube: width = " << width << ", depth = " << depth << ", ROOM = " << ROOM;
	if (degdetect == 1) {	// DegDetectorSlot
		cout << ", SLOTROOM = " << SLOTROOM << ", degdetect.slot_nums = " << slot_nums;
	}
	if (degdetect == 2) {	// DegDetectorNewSlot
		cout << ", SLOTROOM = " << SLOTROOM << ", degdetect.slot_nums = " << slot_nums;
		cout << ", ignore_bits = " << ignore_bits << ", alpha = " << alpha;
	}
	if (degdetect == 3) {	// DegDetectorSlot2bit
		cout << ", SLOTROOM = " << SLOTROOM << ", degdetect.slot_nums = " << slot_nums;
		cout << ", ignore_bits = " << ignore_bits << ", reserved_bits = " << reserved_bits << ", alpha = " << alpha;
	}
	if (degdetect == 4) {	// DegDetectorUMap2bit
		cout << ", ignore_bits = " << ignore_bits << ", reserved_bits = " << reserved_bits << ", alpha = " << alpha;
	}
	cout << ", kick_times = " << (uint32_t)kick_times;
	cout << ", fingerprint_length = " << fingerprint_length << endl << endl;
	cout << "[Info]-dataset: " << filename << endl;
	cout << "[Info]-input_dir: " << input_dir << endl;
	cout << "[Info]-output_dir: " << output_dir << endl;
	cout << "[Info]-write flag = " << writeflag << endl;
	cout << "[Info]-data interval = " << data_interval << endl;
	cout << "[Info]-win = { " ;
	for (int i = 0; i < win.size(); i++) {
		cout << win[i] << " ";
	}
	cout << "}" << endl;
	cout << "**********************************************************************" << endl << endl;
#endif
#if defined(DEBUG)
	cout << "~~~~~~~~~~~~~~~~~~ Init Scube ~~~~~~~~~~~~~~~~~~" << endl;
#endif
    Scube* scube = NULL;					// Init Scube
	if (degdetect == 1) {			// DegDetectSlot
		scube = new ScubeKick(width, depth, fingerprint_length, k_width, k_depth, kick_times, slot_nums, degSlotHash);
	}
	else if (degdetect == 2) {		// DegDetectNewSlot
		scube = new ScubeKick(width, depth, fingerprint_length, k_width, k_depth, kick_times, slot_nums, degSlotHash, ignore_bits, alpha);
	}
	else if (degdetect == 3) {		// DegDetectSlot2bit
		scube = new ScubeKick(width, depth, fingerprint_length, kick_times, slot_nums, degSlotHash, exp_deg, ignore_bits, reserved_bits, alpha);
	}
	else if (degdetect == 4) {		// DegDetectUMap2bit
		scube = new ScubeKick(width, depth, fingerprint_length, kick_times, degSlotHash, exp_deg, ignore_bits, reserved_bits, alpha);
	}

#if defined(DEBUG)
	cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
#endif
	if (rpq == 1) {
		cout << "**************** Scube Reachability Query While Insert Start ****************" << endl;
		reachabilityQueryWhileInsert(*scube, filename, input_type, data_interval, input_dir, output_dir, dataset_name, query_times, writeflag);
		cout << "****************  Scube Reachability Query While Insert End  ****************" << endl;
	}
	else if (ins_bd) {
		cout << "**************** Scube Insert Breakdown Start ****************" << endl;
		insertbd(*((ScubeKick*)scube), filename, input_type, data_interval);
		cout << "****************  Scube Insert Breakdown End  ****************" << endl;
	}
	else if (time_tp) {
		cout << "**************** Scube Insert Time Throughput Start ****************" << endl;
		insert_tp_time(*((ScubeKick*)scube), filename, input_type);
		cout << "****************  Scube Insert Time Throughput End  ****************" << endl;
	}
	else {
		// insertion process
	#if defined(DEBUG) || defined(INFO)
		cout << "****************** Scube insert start *****************" << endl;
	#endif
		insert(*scube, filename, input_type, data_interval);
	#if defined(DEBUG) || defined(INFO)
		cout << "****************** Scube insert end *******************" << endl << endl;
	#endif
	#if defined(DEBUG)
		scube->printUsageInfo();
		scube->printExtendAddrInfo();
	#endif

	// query all edges
	// allEdgeFrequenceQuery(*scube, filename, output_dir, dataset_name);

	// query process
		if (efflag == 1) {
	#if defined(DEBUG) || defined(INFO)
			cout << "**************** Scube edge frequence start ****************" << endl;
	#endif
			// if (timetest)
			// 	edgeFrequenceQueryTestTime(*scube, input_dir, output_dir, dataset_name, query_times, writeflag);
			// else
				edgeFrequenceQueryTest(*scube, input_dir, output_dir, dataset_name, query_times, writeflag);
	#if defined(DEBUG) || defined(INFO)
			cout << "***************** Scube edge frequence end *****************" << endl << endl;
	#endif
		}
		if (eeflag == 1) {
	#if defined(DEBUG) || defined(INFO)
			cout << "**************** Scube existence start ****************" << endl;
	#endif
			// if (timetest)
			// 	edgeFrequenceQueryTestTime(*scube, input_dir, output_dir, dataset_name, query_times, writeflag);
			// else
				edgeFrequenceQueryTest(*scube, input_dir, output_dir, dataset_name, query_times, writeflag);
	#if defined(DEBUG) || defined(INFO)
			cout << "***************** Scube existence end *****************" << endl << endl;
	#endif
		}
		if (nfflag == 1) {
	#if defined(DEBUG) || defined(INFO)
			cout << "************* Scube node frequence start **************" << endl;
	#endif
			if (timetest)
				nodeFrequenceQueryTestTime(*((ScubeKick*)scube), input_dir, output_dir, dataset_name, win, query_times, writeflag, node_query_flag);
			else
				nodeFrequenceQueryTest(*scube, input_dir, output_dir, dataset_name, win, query_times, writeflag, node_query_flag);
	#if defined(DEBUG) || defined(INFO)
			cout << "************** Scube node frequence end ***************" << endl << endl;
	#endif
		}
		if (rpqflag == 1) {
	#if defined(DEBUG) || defined(INFO)
			cout << "************* Scube reachability query start **************" << endl;
	#endif
			reachabilityQueryTest(*scube, input_dir + "PRQ_" + dataset_name, output_dir + "//PRQ_" + dataset_name, query_times, writeflag);
	#if defined(DEBUG) || defined(INFO)
			cout << "************** Scube node frequence end ***************" << endl << endl;
	#endif
		}
		if (alledgeflag == 1) {
	#if defined(DEBUG) || defined(INFO)
			cout << "************* Scube all edge query start **************" << endl;
	#endif
			allEdgeFrequenceQuery1(*scube, input_dir, output_dir, dataset_name, writeflag, edge_n, fun);
	#if defined(DEBUG) || defined(INFO)
			cout << "************** Scube all edge query end ***************" << endl << endl;
	#endif
		}
	}

#if defined(DEBUG)
	cout << "~~~~~~~~~~~~~~~~~~ Del Scube ~~~~~~~~~~~~~~~~~~" << endl;
#endif
	delete scube;
#if defined(DEBUG)
	cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
#endif
#if defined(DEBUG) || defined(INFO)
	gettimeofday( &main_end, NULL);
	double main_time = (main_end.tv_sec - main_start.tv_sec) + (main_end.tv_usec - main_start.tv_usec) / 1000000.0;
	cout << endl << "This program lasts for " << main_time / 60.0 << " min" << endl;
#endif
	return 0;
}