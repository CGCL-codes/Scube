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
	int width, range, p_num, size, f_num;	//GSS parameters
	string back_addr = "_GSS";
	int dataset = 3;
	int query_times = 1;						//查询的次数
	string filename, input_dir, output_dir;		//测试数据集文件路径  测试数据输入路径 测试结果输出路径
	string dataset_name, txt_name = "";
	int efflag = 0, eeflag = 0, nfflag = 0; 	//  edge frequence query,  edge existence query,  node frequence query
	bool writeflag = false;						//是否将测试结果写入到文件
	int node_query_flag = 0;					//1-node_in_query, 2-node_out_query
	//命令行参数
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-dataset") == 0) {
			dataset = atoi(argv[++i]);
		}
		if (strcmp(argv[i], "-range") == 0) {
			range = atoi(argv[++i]);
		}
		if (strcmp(argv[i], "-pnum") == 0) {
			p_num = atoi(argv[++i]);
		}
		if (strcmp(argv[i], "-size") == 0) {
			size = atoi(argv[++i]);
		}
		if (strcmp(argv[i], "-fplength") == 0) {
			f_num = atoi(argv[++i]);
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
		if (strcmp(argv[i], "-in") == 0) {
			node_query_flag = 1;
		}
		if (strcmp(argv[i], "-out") == 0) {
			node_query_flag = 2;
		}
		if (strcmp(argv[i], "-write") == 0) {
			writeflag = true;
		}
		if (strcmp(argv[i], "-qtimes") == 0) {
			query_times = atoi(argv[++i]);
		}
	}

	switch (dataset) { //数据集_查询类型_baseline/pgss_长度_res.txt
		case 1:
			filename = "..//..//Dataset//out";
			input_dir = "..//..//TestFiles//out//input//";
			output_dir = "..//..//TestFiles//out//output//";
			dataset_name = "out";
            width = 900;
			break;
		case 2:
			filename = "..//..//Dataset//lastfm_song";
			input_dir = "..//..//TestFiles//lastfm_song//input//";
			output_dir = "..//..//TestFiles//lastfm_song//output//";
			dataset_name = "lastfm_song";
            width = 15000;
			break;
		case 3:
			filename = "..//..//Dataset//stackoverflow";
			input_dir = "..//..//TestFiles//stackoverflow//input//";
			output_dir = "..//..//TestFiles//stackoverflow//output//";
			dataset_name = "stackoverflow";
			width = 5655;
			break;
		case 4:
			filename = "..//..//Dataset//caida";
			input_dir = "..//..//TestFiles//caida//input//";
			output_dir = "..//..//TestFiles//caida//output//";
			dataset_name = "caida";
			width = 15000;
			break;
		case 5:
			filename = "..//..//Dataset//wiki";
			input_dir = "..//..//TestFiles//wiki//input//";
			output_dir = "..//..//TestFiles//wiki//output//";
			dataset_name = "wiki";
			width = 1750;
			break;
		case 6:
			filename = "..//..//Dataset//lkml";
			input_dir = "..//..//TestFiles//lkml-10w-86400//input//";
			output_dir = "..//..//TestFiles//lkml-10w-86400//output//";
			dataset_name = "lkml";
			width = 1750;
			break;
		default:
			break;
	}

	//命令行参数
	for (int i = 0; i < argc; i++) {
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
	}
#if defined(DEBUG) || defined(HINT)
	cout << "*******************************************************" << endl;
	cout << "Print Infomation" << endl;
	cout << "GSS: width = " << width 
	<< ", range = " << range 
	<< ", p_num = " << p_num 
	<< ", size = " << size 
	<< ", f_num = " << f_num << endl;
#endif
    GSS gss(width, range, p_num, size, f_num);     //实例化GSS
	
    string test_situation_dir = dataset_name + "_" + to_string(width) + "_" + to_string(range) + "_" + to_string(p_num) + "_" + to_string(size) + "_" + to_string(f_num) + back_addr +"//";
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

// insertion process
#if defined(DEBUG) || defined(HINT)
			cout << "****************** GSS insert start *****************" << endl;
#endif
    insert(gss, filename);
	gss.countBuffer();
#if defined(DEBUG) || defined(HINT)
			cout << "****************** GSS insert end *******************" << endl << endl;
#endif
// query process
    if (efflag == 1) {
#if defined(DEBUG) || defined(HINT)
	    cout << "**************** GSS edge frequence start ****************" << endl;
#endif
		edgeFrequenceQueryTest(gss, input_dir, output_dir, dataset_name, query_times, writeflag);
#if defined(DEBUG) || defined(HINT)
		cout << "***************** GSS edge frequence end *****************" << endl << endl;
#endif
	}
	if (eeflag == 1) {
#if defined(DEBUG) || defined(HINT)
		cout << "**************** GSS existence start ****************" << endl;
#endif
		edgeFrequenceQueryTest(gss, input_dir, output_dir, dataset_name, query_times, writeflag);
#if defined(DEBUG) || defined(HINT)
		cout << "***************** GSS existence end *****************" << endl << endl;
#endif
	}
	if (nfflag == 1) {
#if defined(DEBUG) || defined(HINT)
		cout << "************* GSS node frequence start **************" << endl;
#endif
		nodeFrequenceQueryTest(gss, input_dir, output_dir, dataset_name, query_times, writeflag, node_query_flag);
#if defined(DEBUG) || defined(HINT)
		cout << "************** GSS node frequence end ***************" << endl << endl;
#endif
	}

#if defined(DEBUG) || defined(HINT)
	gettimeofday( &main_end, NULL);
	double main_time = (main_end.tv_sec - main_start.tv_sec) + (main_end.tv_usec - main_start.tv_usec) / 1000000.0;
	cout << endl << "This program lasts for " << main_time / 60.0 << " min" << endl;
#endif
	return 0;
}