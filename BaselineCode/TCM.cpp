// the main method of tcm
#include <iomanip>
#include "headers/queryfunctions.h"

int main(int argc, char* argv[]) {
#if defined(DEBUG) || defined(HINT)
    cout << fixed;
	cout << setprecision(7);
	timeval main_start, main_end;
	gettimeofday( &main_start, NULL);
	for (int i = 0; i < argc; i++) {
		cout << argv[i] << " ";
	}
	cout << endl << endl;
#endif
    int width, depth, hashnum;      	// the parameters of TCM
	int dataset = 2;					// specify the dataset for testing, (1) wiki-talk; (2) stackoverflow; (3) dbpedia; (4) caida.
	string dataset_file_path;			// the dataset file path
	string input_dir;					// the inputfile dir path
	string output_dir;					// the outputfile dir path
	string dataset_name;				// the short name of the testing dataset
	int efflag = 0;						// the flag of edge frequence query
	int eeflag = 0;						// the flag of edge existence query
	int nfflag = 0;						// the flag of node frequence query
	int rpqflag = 0; 					// the flag of reachability path query
	int alledgeflag = 0;				// edge query for all unique edges
	uint32_t edge_n = 1000;				// repeat edge_n times for edge queries
	bool writeflag = false;				// write test results to a file
	int node_query_flag = 0;			// 1-node out query, 2-node in query
	int query_times = 1;				// query times
	int input_type = 0;
	string head_addr = "TCM_";
	string back_addr = "";
	int window = 0;
	int data_interval = 100;
	bool time_tp = false;
	vector<int> win;
    // vector<int> win = {1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000};
	// vector<int> win = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100};

	// command line parameters
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
			dataset_file_path = "..//..//Dataset//wiki-talk";
			input_dir = "..//..//TestFiles//wiki-talk//input//";
			output_dir = "..//..//TestFiles//wiki-talk//output//";
			dataset_name = "wiki-talk";
            width = 3536;
			depth = 3536;
			break;
		case 2:
			dataset_file_path = "..//..//Dataset//stackoverflow";
			input_dir = "..//..//TestFiles//stackoverflow//input//";
			output_dir = "..//..//TestFiles//stackoverflow//output//";
			dataset_name = "stackoverflow";
			width = 5656;
			depth = 5656;
			break;
		case 3:
			dataset_file_path = "..//..//Dataset//dbpedia";
			input_dir = "..//..//TestFiles//dbpedia//input//";
			output_dir = "..//..//TestFiles//dbpedia//output//";
			dataset_name = "dbpedia";
            width = 9280;
			depth = 9280;
			break;
		case 4:
			dataset_file_path = "..//..//Dataset//caida";
			input_dir = "..//..//TestFiles//caida//input//";
			output_dir = "..//..//TestFiles//caida//output//";
			dataset_name = "caida";
			width = 15000;
			depth = 15000;
			break;
		default:
			break;
	}

	// command line parameters
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
		if (strcmp(argv[i], "-dataset_file_path") == 0) {
			dataset_file_path = argv[++i];
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
    TCM tcm(width, depth, hashnum);     // initialize TCM

	if (writeflag) {
		string test_situation_dir = head_addr + dataset_name + "_" + to_string(width) + "x" + to_string(depth) + "_hashnum_" + to_string(hashnum) + "_win_" + to_string(window) + back_addr +"//";
		output_dir += test_situation_dir;
		char dir_path[FILENAME_MAX];
		strcpy(dir_path, output_dir.c_str());
		if (createDirectory(dir_path) != 0) {
			cout << "Create Directory error" << endl;
			return -1;
		}
#if defined(DEBUG) || defined(HINT)
	cout << "input_dir: " << input_dir << endl;
	cout << "output_dir: " << output_dir << endl;
#endif
	}
#if defined(DEBUG) || defined(HINT)
	cout << "dataset: " << dataset_file_path << endl;
	cout << "write flag = " << writeflag << endl;
	cout << endl;
	cout << "*******************************************************" << endl << endl;
#endif

	if (time_tp) {
		cout << "**************** Scube Insert Time Throughput Start ****************" << endl;
		insert_tp_time(tcm, dataset_file_path, input_type);
		cout << "****************  Scube Insert Time Throughput End  ****************" << endl;
	}
	else {
		// insert process
	#if defined(DEBUG) || defined(HINT)
		cout << "****************** TCM insert start *****************" << endl;
	#endif
		insert(tcm, dataset_file_path, input_type, data_interval);
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
			allEdgeFrequenceQuery(tcm, input_dir, output_dir, dataset_name, writeflag, edge_n);
	#if defined(DEBUG) || defined(HINT)
			cout << "************** TCM all edge query end ***************" << endl << endl;
	#endif
		}
	}

#if defined(DEBUG) || defined(HINT)
	gettimeofday( &main_end, NULL);
	double main_time = (main_end.tv_sec - main_start.tv_sec) + (main_end.tv_usec - main_start.tv_usec) / 1000000.0;
	cout << endl << "This program lasts for " << main_time / 60.0 << " min." << endl;
#endif
	return 0;
}