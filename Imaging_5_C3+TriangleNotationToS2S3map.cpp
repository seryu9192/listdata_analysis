//Imaging_5_C3+TriangleNotationToS2S3map.cpp : TriangleNotationのリストからS2-S3二次元グラフを作成
//

//アップデートログ
//2020.10.04:Imaging_5_C3+TriangleNotationToTriLin.cppから作成

#include "./library.hpp"

//2次元に変換するときのS2-S3の範囲
const double S_MIN = -5;
const double S_MAX =  5;
const int CHN_NUM = 50;

//列
const int COL_S2 = 11;
const int COL_S3 = 12;

//outputfolderName
const string OUTPUT_FOLDERNAME = "\\5-#_map";

int main(int argc, char* argv[])
{
	cout << "*****************************************************************" << endl;
	cout << "****       Imaging_5_C3+TriangleNotationToS2S3map.cpp        ****" << endl;
	cout << "****    -TriangleNotation形式のリストデータから、S2-S3の     ****" << endl;
	cout << "****                  二次元分布のデータを作成する-          ****" << endl;
	cout << "****                 ver.2020.10.04 written by R. Murase     ****" << endl;
	cout << "*****************************************************************" << endl << endl;

	//入力イメージングデータパスの生成
	string inputDataFolder, inputDataName;

	//別のプログラムから走らせる用にコマンドライン引数を定義
	//従来通り手動で走らせる
	if(argc == 1)
	{
		cout << "解析するリストデータのフォルダを入力してください\n--->";
		cin >> inputDataFolder;
	}
	//コマンドライン引数で読み取る
	else if(argc == 2)
	{
		inputDataFolder = argv[1];
		cout << "コマンドライン引数から読み取りました" << endl;
		cout << "inputDataFolder : " << inputDataFolder << endl;
	}
	else
	{
		cerr << "コマンドライン引数の数が不正です" << endl;
		system("pause");
		return -1;
	}

	//dataname.txtの読み取り
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
	
	//フォルダにあるファイルのうち、param fileは除外
	for (int i = 0; i < fNames.size(); i++)
	{
		if (fNames[i].find("param") != string::npos)
		{
			fNames.erase(fNames.begin() + i);
		}
	}
	
	//outputdatafolderを作成
	string outputDataFolder = inputDataFolder + OUTPUT_FOLDERNAME;
	if (_mkdir(outputDataFolder.c_str()) == 0)
	{
		cout << "新たなフォルダを作成し出力ファイルを保存します。" << endl;
	}
	else
	{
		cout << "既存のフォルダに出力ファイルを保存します。" << endl;
	}

	//フォルダ内の全てのリストデータについての総和
	vector<vector<int>> tot(CHN_NUM, vector<int>(CHN_NUM, 0)); 

	//入力ファイルごとにループ
	for (auto inputFilename : fNames)
	{
		//リストデータの読み込み
		string inputFilePath = inputDataFolder + "\\" + inputFilename;
		cout << inputFilename << "----読み込んでいます...";
		auto inputListData = readListFile<double>(inputFilePath);
		cout << "読み込みました --> 処理を開始します...";

		//出力ファイル
		vector<vector<int>> indiv(CHN_NUM, vector<int>(CHN_NUM, 0));
		for (int row = 0; row < inputListData.size(); row++)
		{
			auto s2 = inputListData[row][COL_S2];
			auto s3 = inputListData[row][COL_S3];
			//SRの値で抽出
			int x = (s2- S_MIN)/(S_MAX-S_MIN)*CHN_NUM;
			int y = (s3- S_MIN)/(S_MAX-S_MIN)*CHN_NUM;
			if (0 <= x && x < CHN_NUM && 0 <= y && y < CHN_NUM)
			{
				indiv[x][y]++;
				tot[x][y]++;
			}
		}

		//出力パス
		inputFilename = parsePath(inputFilename)[1];
		string outputFilePath = outputDataFolder + "\\" + inputFilename + ".csv";
		cout << "処理終了 --> 書き込んでいます...";
		writeListFile<int>(outputFilePath, indiv, ',');
		cout <<  "書き込みました" << endl;
	}
	
	//最後にtotalを保存
	string outputFilePath_tot = outputDataFolder + "\\" + inputDataName + "_q=tot.csv";
	writeListFile<int>(outputFilePath_tot, tot, ',');

	//ログにSR_linの値を残しておく(追加書き込み)
	string logFilePath = outputDataFolder + "\\S.log";
	ofstream ofs(logFilePath, ios::app);
	time_t t = chrono::system_clock::to_time_t(chrono::system_clock::now());
	ofs << ctime(&t);
	ofs << "--- " << S_MIN << " <= S2, S3 <= " << S_MAX << ", CHN_NUM = " << CHN_NUM <<endl;
	ofs.close();
	cout << "ログにS_MIN, S_MAX, CHN_NUMの値を書き込みました。" << endl;
	cout << "全ての処理が終了しました。" << endl;
	return 0;
}
