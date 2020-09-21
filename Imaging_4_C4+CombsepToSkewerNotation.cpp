// Imaging_4_C4+CombsepToSkewerNotation.cpp :C4_Combsepを、linear chainのイベントを抽出するためのパラメータ（SkewerNotation)に変換

//アップデートログ
//2020.7.30:Imaging_6_C3+LinearTomolAxis.cpp から作成
//2020.7.31:calcSkewer実装
//2020.9.18:外から別のプログラムで動かせるように、コマンドライン引数から入力フォルダパスを受け取れるようにする
//2020.9.20:リストデータのうち、特定のファイル番号についてのみ処理を行うため、readListFileを変更(int col, T keyMin, T keyMax)を引数に追加
//			keyMin,keyMaxに渡すためにstart, end変数を定義。コマンドライン引数から受け取れるようにする
//			書き込み時のwriteListFile関数を、start == 1の時は新規、それ以外は追加で書き込むようにする


#include "./library.hpp"

vector<vector<double>> inputListData;
vector<vector<double>> outputListData;
string inputDataName;

//outputfolderName
const string OUTPUT_FOLDER_NAME = "\\5_SkewerNotation";

//inputdata列
const int COL_FILE = 0;
const int COL_FRAME = 1;
const int COL_X = 2;
const int COL_Y = 3;
const int COL_BRIGHT = 4;
const int COL_CHARGE = 5;

//CLUSTER_SIZE
const int CLUSTER_SIZE = 4;

void calcSkewer();

int main(int argc, char* argv[])
{	
	cout << "*****************************************************************************" << endl;
	cout << "****             Imaging_4_C4+CombsepToSkewerNotation.cpp                ****" << endl;
	cout << "****         C4+ List data からlinear structureを抜き出すための          ****" << endl;
	cout << "****                  パラメータ(SkewerNotation)に変換する               ****" << endl;
	cout << "****                                 ver.2020.09.20 written by R. Murase ****" << endl;
	cout << "*****************************************************************************" << endl << endl;

	//入力イメージングデータパスの生成
	string inputDataFolder;
	int start, end;
	//別のプログラムから走らせる用にコマンドライン引数を定義
	//従来通り手動で走らせる
	if(argc == 1)
	{
		cout << "解析するリストデータのフォルダを入力してください\n--->";
		cin >> inputDataFolder;

		cout << "解析するファイル番号(start,end)を入力してください" << endl;
		cout << "start : ";
		cin >> start;
		cout << "end : ";
		cin >> end;
	}
	//コマンドライン引数で読み取る
	else if(argc == 4)
	{
		inputDataFolder = argv[1];
		start = stoi(argv[2]);
		end =  stoi(argv[3]);

		cout << "コマンドライン引数から読み取りました" << endl;
		cout << "inputDataFolder : " << inputDataFolder << endl;
		cout << "start = " << start << endl;
		cout << "end = "<< end << endl;
	}
	else
	{
		cerr << "コマンドライン引数の数が不正です" << endl;
		system("pause");
		return -1;
	}	
	
	//dataname.txtを読み込み
	string datanamePath = searchFileFromUpperFolder(inputDataFolder, DATANAME_FILE);
	if (datanamePath == "")
	{
		cout << "dataname.txtファイルが見つかりませんでした\n";
		cout << "解析するリストデータのファイル名を入力してください\n--->";
		cin >> inputDataName;
	}
	else if(readDataName(datanamePath) == "")
	{
		cout << "dataname.txtファイルが不正です\n";
		cout << "解析するリストデータのファイル名を入力してください\n--->";
		cin >> inputDataName;
	}
	else
	{
		cout << "dataname.txtファイルを読み込みました\n";
		inputDataName = readDataName(datanamePath);
		cout << "解析するリストデータのファイル名：" << inputDataName << endl;
	}

	//入力イメージングデータフォルダ内のファイルの名前を取得
	vector<string> fNames = getFileNameInDir(inputDataFolder);

	//出力データパスの生成
	string outputDataFolder, outputFileName;
	cout << " 同じフォルダ内の"+ OUTPUT_FOLDER_NAME +"に保存(なければ自動生成）" << endl;
	
	outputDataFolder = upperFolder(inputDataFolder) + OUTPUT_FOLDER_NAME;
	if (_mkdir(outputDataFolder.c_str()) == 0)
	{
		cout << "新たなフォルダを作成し出力ファイルを保存します。" << endl;
	}
	else
	{
		cout << "既存のフォルダに出力ファイルを保存します。" << endl;
	}

	//フォルダ内のすべてのファイルに対して処理
	for(auto inputfilename : fNames)
	{
		//param fileは読み飛ばし
		if (inputfilename.find("param") != string::npos)
			continue;

		//入力イメージングデータの読み込み
		string inputDataPath = inputDataFolder + "\\" + inputfilename;

		//inputDataの読み込み
		cout << inputDataPath + "----データを読み込んでいます...";
		inputListData = readListFile<double>(inputDataPath, COL_FILE, start, end+1);
		cout << "----読み込み完了 --> 処理を開始します...";

		//変換処理開始
		calcSkewer();
		cout << "処理終了 --> ファイルに書き込んでいます...";

		//出力ファイルにoutputListDataを書き込み
		string outputFilePath = outputDataFolder + "\\" + inputfilename;

		//start == 1の時は新規、それ以外はappend
		bool append = true;
		if (start == 1)append = false;
		writeListFile<double>(outputFilePath, outputListData, '\t', append);
		cout << "に書き込みました" << endl;

		//データクリア
		inputListData.clear();
		outputListData.clear();
	}

	//フォルダ内処理終了
	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	return 0;
}

void calcSkewer()
{
	//inputListDataをはじめからCLUSTER_SIZE行(1イベント)ずつ見ていく
	for (int i = 0; CLUSTER_SIZE * i < inputListData.size(); i++)
	{
		double fileNum = inputListData[CLUSTER_SIZE * i][COL_FILE];
		double frameNum = inputListData[CLUSTER_SIZE * i][COL_FRAME];
		vector<double> x;
		vector<double> y;
		vector<double> q;

		//4点のパラメータをpush_back
		for (int j = 0; j < CLUSTER_SIZE; j++)
		{
			x.push_back(inputListData[CLUSTER_SIZE * i + j][COL_X]);
			y.push_back(inputListData[CLUSTER_SIZE * i + j][COL_Y]);
			q.push_back(inputListData[CLUSTER_SIZE * i + j][COL_CHARGE]);
		}

		//4点から2点選び出す方法を全探索し、最も距離が離れている2点をF1, F4とする
		double l_max = 0;
		//ind[0]から近い順番に
		vector<int> ind(CLUSTER_SIZE);

		//2点を選ぶ組合せのリスト
		auto pairs = choose2FromN(CLUSTER_SIZE);
		for (int j = 0; j < pairs.size(); j++)
		{
			auto v_0 = make_pair(x[pairs[j].first], y[pairs[j].first]);
			auto v_1 = make_pair(x[pairs[j].second], y[pairs[j].second]);
			auto l = calcDist(v_0, v_1);
	
			//距離が最も離れている二点＝両端の二点のindexを覚えておく -> F1, F4とする
			if (chmax(l_max, l))
			{
				ind[0] = pairs[j].first;
				ind[CLUSTER_SIZE-1] = pairs[j].second;
			}
		}

		//d2, d3の計算とind[1], ind[2]の決定
		double x_1 = x[ind[0]];
		double y_1 = y[ind[0]];
		double x_4 = x[ind[CLUSTER_SIZE-1]];
		double y_4 = y[ind[CLUSTER_SIZE-1]];
		
		//線分mがx軸に垂直な場合はちょこっとxをずらす(実質的には影響がない程度)
		if(x_1 == x_4) x_1 += 1.;

		//F1, F4を結ぶ線分の方程式(y = ax + b)のパラメータ
		double a = (y_4 - y_1)/(x_4 - x_1);
		double b = (x_4*y_1 - x_1*y_4)/(x_4 - x_1);

		//F2, F3からH2, H3(垂線の足)の座標を計算
		auto comp_ind = complement(CLUSTER_SIZE, {ind[0], ind[CLUSTER_SIZE-1]});//まだ位置を決定していない、真ん中二点(F2, F3)
		//F2, F3(順不同)の座標
		double x_2 = x[comp_ind[0]];
		double y_2 = y[comp_ind[0]];
		double x_3 = x[comp_ind[1]];
		double y_3 = y[comp_ind[1]];

		//垂線の足H2, H3の座標のx座標（平面幾何で計算）
		double xi_2 =(a*(y_2-b)+x_2)/(a*a + 1);
		double xi_3 =(a*(y_3-b)+x_3)/(a*a + 1);

		//H2, H3の順番を決定(x_1とxi_2, xi_3を比較して近い方を新たにxi_2とする)
		double d_2, d_3;
		if(abs(xi_2-x_1) <= abs(xi_3-x_1))//x_1, xi_2, xi_3, x_4の並びのとき
		{
			ind[1] = comp_ind[0];
			ind[2] = comp_ind[1];
			d_2 = abs(a*x_2-y_2+b)/sqrt(a*a + 1);
			d_3 = abs(a*x_3-y_3+b)/sqrt(a*a + 1);
		}
		else//x_1, xi_3, xi_2, x_4の並びのとき
		{
			ind[1] = comp_ind[1];
			ind[2] = comp_ind[0];
			d_2 = abs(a*x_3-y_3+b)/sqrt(a*a + 1);
			d_3 = abs(a*x_2-y_2+b)/sqrt(a*a + 1);
		}
		
		//4点の重心の計算
		double x_c = average<double>(x);
		double y_c = average<double>(y);;

		//outputListdataに追加
		vector<double> outputLine = {fileNum, frameNum, x_c, y_c, l_max, d_2/l_max, d_3/l_max};
		//qを並び順にpush_back
		for (int j = 0; j < CLUSTER_SIZE; j++)
			outputLine.push_back(q[ind[j]]);		
		outputListData.push_back(outputLine);
	}
	return;
};


