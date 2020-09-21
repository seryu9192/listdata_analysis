//Imaging_5_C3+TriangleNotationToTriLin.cpp : TriangleNotationをもとに直鎖分子と環状分子を区別する
//

//アップデートログ
//2020.7.27:作成
//2020.9.18:入力フォルダ名をコマンドライン引数から読み取れるようにする

#include "./library.hpp"

//分けるsrの範囲
vector<double> SR_min = {0, 1.75};
vector<double> SR_max = {0.33, 100000};

//列
const int COL_SR = 13;

//outputfolderName
const vector<string> OUTPUT_FOLDERNAMES = {"\\5-2_Triangle", "\\5-1_Linear"};

int main(int argc, char* argv[])
{
	cout << "*****************************************************************" << endl;
	cout << "****       Imaging_5_C3+TriangleNotationToTriLin.cpp         ****" << endl;
	cout << "**** -TriangleNotation形式のリストデータを、SRごとに分けて   ****" << endl;
	cout << "****                 直鎖と環状に区別する-                   ****" << endl;
	cout << "****                 ver.2020.09.18 written by R. Murase     ****" << endl;
	cout << "*****************************************************************" << endl << endl;

	//入力イメージングデータパスの生成
	string inputDataFolder;
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
	
	//triangleとlinearについて処理
	for (int i = 0; i < SR_min.size(); i++)
	{
		string outputDataFolder = inputDataFolder + OUTPUT_FOLDERNAMES[i];
		if (_mkdir(outputDataFolder.c_str()) == 0)
		{
			cout << "新たなフォルダを作成し出力ファイルを保存します。" << endl;
		}
		else
		{
			cout << "既存のフォルダに出力ファイルを保存します。" << endl;
		}

		//入力ファイルごとにループ
		for (auto inputFilename : fNames)
		{
			//リストデータの読み込み
			string inputFilePath = inputDataFolder + "\\" + inputFilename;
			cout << inputFilename << "----読み込んでいます...";
			auto inputListData = readListFile<double>(inputFilePath);
			cout << "読み込みました --> 処理を開始します...";

			//出力ファイル
			vector<vector<double>> outputListData;
			for (int row = 0; row < inputListData.size(); row++)
			{
				//SRの値で抽出
				if (SR_min[i] <= inputListData[row][COL_SR] && inputListData[row][COL_SR] <= SR_max[i])
				{
					outputListData.push_back(inputListData[row]);
				}
			}

			//出力パス
			string outputFilePath = outputDataFolder + "\\" + inputFilename;
			cout << "処理終了 --> 書き込んでいます...";
			writeListFile<double>(outputFilePath, outputListData);
			cout <<  "書き込みました" << endl;
		}
		//ログにSR_linの値を残しておく
		string logFilePath = outputDataFolder + "\\SR.log";
		ofstream ofs(logFilePath, ios::app);
		time_t t = chrono::system_clock::to_time_t(chrono::system_clock::now());
		ofs << ctime(&t);
		ofs << "--- " << SR_min[i] << " <= SR <= " << SR_max[i] << endl;
		ofs.close();
		cout << "ログにSRの値を書き込みました。" << endl;
	}
	cout << "全ての処理が終了しました。" << endl;
	return 0;
}
