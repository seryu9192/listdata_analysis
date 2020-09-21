//Imaging_5_C4+SkewerNotationToLin.cpp: SkewerNotationをもとに、C4+の直鎖分子とそれ以外を区別する
//

//アップデートログ
//2020.7.31:Imaging_5_C3+TriangleNotationToTriLin.cppから作成
//2020.9.18:外から別のプログラムで動かせるように、コマンドライン引数から入力フォルダパスを受け取れるようにする
//2020.9.20:リストデータのうち、特定のファイル番号についてのみ処理を行うため、readListFileを変更(int col, T keyMin, T keyMax)を引数に追加
//			keyMin,keyMaxに渡すためにstart, end変数を定義。コマンドライン引数から受け取れるようにする
//			書き込み時のwriteListFile関数を、start == 1の時は新規、それ以外は追加で書き込むようにする

#include "./library.hpp"

//分けるd/lの範囲
double d_l = 0.15;//これ以下を直鎖

//d2/l, d3/l 列
const int COL_FILE = 0;
const int COL_d2 = 5;
const int COL_d3 = 6;

//outputfolderName
const string OUTPUT_FOLDER_NAME = "\\5-1_Linear";

int main(int argc, char* argv[])
{
	cout << "*****************************************************************" << endl;
	cout << "****        Imaging_5_C4+SkewerNotationToLinear.cpp          ****" << endl;
	cout << "****         -SkewerNotation形式のリストデータを、           ****" << endl;
	cout << "****      d2/l, d3/lでに分けて 直鎖と環状に区別する-         ****" << endl;
	cout << "****                 ver.2020.09.18 written by R. Murase     ****" << endl;
	cout << "*****************************************************************" << endl << endl;

	//入力イメージングデータパスの生成
	string inputDataFolder_img;
	int start, end;
	//別のプログラムから走らせる用にコマンドライン引数を定義
	//従来通り手動で走らせる
	if(argc == 1)
	{
		cout << "解析するリストデータのフォルダを入力してください\n--->";
		cin >> inputDataFolder_img;
		cout << "解析するファイル番号(start,end)を入力してください" << endl;
		cout << "start : ";
		cin >> start;
		cout << "end : ";
		cin >> end;
	}
	//コマンドライン引数で読み取る
	else if(argc == 4)
	{
		inputDataFolder_img = argv[1];
		start = stoi(argv[2]);
		end =  stoi(argv[3]);

		cout << "コマンドライン引数から読み取りました" << endl;
		cout << "inputDataFolder : " << inputDataFolder_img << endl;
		cout << "start = " << start << endl;
		cout << "end = "<< end << endl;
	}
	else
	{
		cerr << "コマンドライン引数の数が不正です" << endl;
		system("pause");
		return -1;
	}

	//入力イメージングデータフォルダ内のファイルの名前を取得
	vector<string> fNames = getFileNameInDir(inputDataFolder_img);
	
	//フォルダにあるファイルのうち、param fileは除外
	for (int i = 0; i < fNames.size(); i++)
	{
		if (fNames[i].find("param") != string::npos)
		{
			fNames.erase(fNames.begin() + i);
		}
	}
	
	string outputDataFolder = inputDataFolder_img + OUTPUT_FOLDER_NAME;
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
		string inputDataPath = inputDataFolder_img + "\\" + inputFilename;
		cout << inputFilename << "----読み込んでいます...";
		auto inputListData = readListFile<double>(inputDataPath, COL_FILE, start, end+1);
		cout << "読み込みました --> 処理を開始します...";

		//出力ファイル
		vector<vector<double>> outputListData;
		for (int row = 0; row < inputListData.size(); row++)
		{
			//d2/l or d3/lの値で抽出
			auto d = max(inputListData[row][COL_d2], inputListData[row][COL_d3]);
			if (d_l >= d)
			{
				outputListData.push_back(inputListData[row]);
			}
		}

		//出力パス
		string outputFilePath = outputDataFolder + "\\" + inputFilename;
		cout << "処理終了 --> 書き込んでいます...";

		//start == 1の時は新規、それ以外はappend
		bool append = true;
		if (start == 1)append = false;
		writeListFile<double>(outputFilePath, outputListData, '\t', append);
		cout <<  "書き込みました" << endl;
	}

	//ログにd/lの値を残しておく
	string logFilePath = outputDataFolder + "\\d_l.log";
	ofstream ofs(logFilePath, ios::app);
	time_t t = chrono::system_clock::to_time_t(chrono::system_clock::now());
	ofs << ctime(&t);
	ofs << "file#: " << start << " - " << end << " --- d/l = " << d_l << endl;
	ofs.close();

	cout << "ログにd/lの値を書き込みました。" << endl;
	cout << "全ての処理が終了しました。" << endl;
	return 0;
}
