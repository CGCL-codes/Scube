// 测试边查、点查等结果
#include <iomanip>
#include "Scube/ScubeKick.h"
#include "headers/QueryFunction.h"

int main(int argc, char* argv[]) {
#if defined(DEBUG) || defined(INFO)  	
    cout << fixed;  // 强制使用小数,防止使用科学计数法
	cout << setprecision(6);    // 控制显示的精度，控制小数点后面的位数
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
	uint32_t slot_nums, kick_times = 10;	// Scube parameters

	string head_scube_addr = "ScubeKick_";
	string head_degdetect_addr = "DegSlot_";
	string back_addr = "";
	int dataset = 1;
	int query_times = 1;					
	string filename, input_dir, output_dir;	
	string dataset_name, txt_name = "";
	int efflag = 0, eeflag = 0, nfflag = 0, rpqflag = 0; 	// edge frequence query,  edge existence query,  node frequence query
	int rpq = 0;		// reachability path query
	int alledgeflag = 0;
	int fun = 1; 		
	uint32_t edge_n = 1000;	
	bool writeflag = false;						
	int node_query_flag = 0;					// 1-node_out_query, 2-node_in_query
	int input_type = 0;
	bool timetest = false;
	int window = 10;
	int data_interval = 100;
	bool ins_bd = false;
	
	vector<int> win;
	// vector<int> win = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100};
	// vector<int> win = {50, 100, 150, 200, 250, 300, 350, 400, 450, 500, 550};
	// command parameters
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-dataset") == 0) {
			dataset = atoi(argv[++i]);
		}
		if (strcmp(argv[i], "-fplength") == 0) {
			fingerprint_length = atoi(argv[++i]);
		}
		if (strcmp(argv[i], "-edgeweight") == 0) {
			efflag = 1;
		}
		if (strcmp(argv[i], "-edgeexistence") == 0) {
			eeflag = 1;
		}
		if (strcmp(argv[i], "-nodeinweight") == 0) {
			nfflag = 1;
			node_query_flag = 2;
		}
		if (strcmp(argv[i], "-nodeoutweight") == 0) {
			nfflag = 1;
			node_query_flag = 1;
		}
		if (strcmp(argv[i], "-reachability") == 0) {
			rpqflag = 1;
		}
		if (strcmp(argv[i], "-rpq") == 0) {
			rpq = 1;
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
		if (strcmp(argv[i], "-kicks") == 0) {
            kick_times = atoi(argv[++i]);
        }
		if (strcmp(argv[i], "-slots") == 0) {
            slot_nums = atoi(argv[++i]);
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
	}
	for (int i = 1; i < 12; i++) {
		win.push_back(i * window);
	}
	switch (dataset) {
		case 1:
			filename = "..//..//Dataset//stackoverflow";
			input_dir = "..//..//TestFiles//stackoverflow//input//";
			output_dir = "..//..//TestFiles//stackoverflow//output//";
			dataset_name = "stackoverflow";
			width = 5656;
			depth = 5658;
			break;
		case 2:
			filename = "..//..//Dataset//caida";
			input_dir = "..//..//TestFiles//caida//input//";
			output_dir = "..//..//TestFiles//caida//output//";
			dataset_name = "caida";
			width = 15000;
			depth = 15000;
			break;
		case 3:
			filename = "..//..//Dataset//wiki";
			input_dir = "..//..//TestFiles//wiki//input//";
			output_dir = "..//..//TestFiles//wiki//output//";
			dataset_name = "wiki";
			width = 1750;
			depth = 1750;
			break;
		case 4:
			filename = "..//..//Dataset//lkml";
			input_dir = "..//..//TestFiles//lkml-10w-86400//input//";
			output_dir = "..//..//TestFiles//lkml-10w-86400//output//";
			dataset_name = "lkml";
			width = 1750;
			depth = 1750;
			break;
		case 5:
			filename = "..//..//Dataset//delicious-ui";
			input_dir = "..//..//TestFiles//delicious-ui//input//";
			output_dir = "..//..//TestFiles//delicious-ui//output//";
			dataset_name = "delicious-ui";
            width = 17354;
			depth = 17356;
			break;
		case 6:
			filename = "..//..//Dataset//man-made";
			input_dir = "..//..//TestFiles//man-made//input//";
			output_dir = "..//..//TestFiles//man-made//output//";
			dataset_name = "man-made";
            width = 17354;
			depth = 17356;
			break;
		case 7:
			filename = "..//..//Dataset//wiki-talk";
			input_dir = "..//..//TestFiles//wiki-talk//input//";
			output_dir = "..//..//TestFiles//wiki-talk//output//";
			dataset_name = "wiki-talk";
            width = 3536;
			depth = 3536;
			break;
		case 8:
			filename = "..//..//Dataset//dbpedia";
			input_dir = "..//..//TestFiles//dbpedia//input//";
			output_dir = "..//..//TestFiles//dbpedia//output//";
			dataset_name = "dbpedia";
            width = 9280;
			depth = 9280;
			break;
		default:
			break;
	}
	
	// command parameters
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
			"_fp_" + to_string(fingerprint_length) + "_win_" + to_string(window) + "_slotnums_" + to_string(slot_nums) + "_SLOTROOM_"+ to_string(SLOTROOM) + "_" + 
			to_string(k_width) + "_" + to_string(k_depth) + back_addr + "//";
		 
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
	cout << "[Info]-Scube: width = " << width << ", depth = " << depth << ", ROOM = " << ROOM << ", SLOTROOM = " << SLOTROOM;
	cout << ", degdetect.slot_nums = " << slot_nums;
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
    Scube* scube = new ScubeKick(width, depth, fingerprint_length, k_width, k_depth, kick_times, slot_nums);		// Init Scube
	
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