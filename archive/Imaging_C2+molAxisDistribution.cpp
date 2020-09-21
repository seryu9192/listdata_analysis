// ImagingC2+molAxisDistribution.cpp : 価数ペアごとにmolAxisの個数分布を求める
//

//アップデートログ
//2020.4.3:作成
//2020.7.9:VS Codeに移植

#include "./library.hpp"

//リストデータを扱う配列
vector<vector<double>> inputListData;
vector<vector<int>> outputData;


//データ名
string inputDataName;

//outputfolderName
const string OUTPUT_FOLDER_NAME = "6_molAxis";

//リストデータの列
const int COL_FILE = 0;
const int COL_FRAME = 1;
const int COL_X = 2;
const int COL_Y = 3;
const int COL_MOLAXIS = 4;
const int COL_CHARGEPROD = 5;
const int COL_BRIGHTDIFF = 6;

int main()
{
	cout << "*************************************************************" << endl;
	cout << "****              -分子軸分布集計プログラム-               **" << endl;
	cout << "****                   ver.2020.04.03 written by R. Murase **" << endl;
	cout << "*************************************************************" << endl << endl;

	//入力イメージングデータパスの生成
	string inputDataFolder;
	cout << "イメージングリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder;

	//入力フォルダ内のファイルを列挙
	auto fileNames = getFileNameInDir(inputDataFolder);
	for (auto fileName : fileNames)
	{
		//"param"ファイルは処理しない
		if (fileName.find("param") != string::npos)continue;
		
		string inputDataPath = inputDataFolder + "\\" + fileName;
		inputListData = readListFile<double>(inputDataPath);
		cout << fileName + "----データの読み込み完了" << endl;

	}
	return 0;
}


