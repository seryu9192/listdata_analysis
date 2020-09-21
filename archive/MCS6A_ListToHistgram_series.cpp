// MCS6A_ListToHistgram_series.cpp : リストファイル(2_stop)からTOFヒストグラムを作成。フォルダ内のすべてのファイルに対して処理
//

//アップデートログ
//MCS6A_ListToMPAseries.cppとして作成
//2019.8.23:ファイル名をfilename.txtから読み込むようにした
//2019.8.24:連番処理 -> フォルダ内のデータを一括処理に変更
//2020.2.25:#_spectraフォルダを自動生成するようにした
//2020.2.25:MCS6A_ListToHistgram_series.cppに名前を変更

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <Windows.h>
#include <stdexcept>
#include <direct.h>

using namespace std;

typedef unsigned int ui;

int columnInput = 4;

//実験時のMCSのbin幅とchannel範囲の決定
constexpr auto BIN_WIDTH_EXP = 8;
constexpr auto MCS_CH_Exp = 8000;

//リストデータからTOFスペクトルを再構成するときのchannelとbin幅の決定
constexpr auto BIN_WIDTH = 8;
constexpr auto chNum = BIN_WIDTH_EXP * MCS_CH_Exp / BIN_WIDTH;

vector<vector<ui>> inputListData;
string inputDataName;

//TOFのチャンネルごとのyield
vector<ui> tofIntegrated(chNum, 0);

void integrateTOF();
string upperPath(string);
bool readFilename(string);
vector<string> getFileNameInDir(string);

int main()
{
	cout << "******************************************************" << endl;
	cout << "***           MCS6A listデータ(stop)から           ***" << endl;
	cout << "***       TOFスペクトルを構成するプログラム        ***" << endl;
	cout << "***   (フォルダ内のすべてのリストデータを処理)     ***" << endl;
	cout << "***             ver.2020.2.25 written by R. Murase ***" << endl;
	cout << "******************************************************" << endl << endl;

	string inputDataFolder, inputDataPath;
	cout << "解析するリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder;

	//入力イメージングデータフォルダ内のファイルの名前を取得
	vector<string> fName = getFileNameInDir(inputDataFolder);

	//出力フォルダの指定
	string outputFolderName;
	cout << "出力フォルダのパスを入力してください (\"@\"入力で一つ上(\"@@\"入力で二つ上)のフォルダ内の\"#_spectra\"に保存)\n--->";
	cin >> outputFolderName;
	if (outputFolderName == "@")
	{
		outputFolderName = upperPath(inputDataFolder) + "\\#_spectra";
	}
	else if (outputFolderName == "@@")
	{
		outputFolderName = upperPath(upperPath(inputDataFolder)) + "\\#_spectra";
	}
	_mkdir(outputFolderName.c_str());

	for (int i = 0; i < fName.size(); i++)
	{
		inputDataName = fName[i];
		//入力データパスの生成
		inputDataPath = inputDataFolder + "\\" + inputDataName;
		ifstream ifs(inputDataPath);
		if (!ifs)
		{
			cerr << "ファイルを開けませんでした" << endl;
			system("pause");
			return -1;
		}
		//リストデータを1行ずつ読み取り、\tで分けてinputListDataにpush_back
		string tmp;
		while (getline(ifs, tmp))
		{
			stringstream ss(tmp);
			string tmp2;
			vector<ui> inputLine;
			while (ss >> tmp2)
			{
				inputLine.push_back(stoi(tmp2));
			}
			inputListData.push_back(inputLine);
		}
		columnInput = inputListData[0].size();
		cout << inputDataPath + ".txt" + "----データの読み込み完了、処理を開始します" << endl;

		integrateTOF();

		cout << inputDataPath + ".txt" + "----処理終了" << endl;

		//出力データパスの生成
		string outputFilePath = outputFolderName + "\\" + inputDataName;

		//出力ファイルにoutputListDataを書き込み
		ofstream ofs(outputFilePath);

		//出力ファイルへの書き込み処理
		for (int i = 0; i < chNum; i++)
		{
			ofs << tofIntegrated[i] << endl;
		}

		cout << outputFilePath << "に書き込みました" << endl << endl;

		//次のファイルのためにメモリをクリア
		inputListData.clear();
		for (int i = 0; i < tofIntegrated.size(); i++)
		{
			tofIntegrated[i] = 0;
		}
	}
	cout << "全ての処理が完了しました。プログラムを終了します" << endl;
	return 0;
}

// 1行ずつchを読み取ってtofIntegratedに積算
void integrateTOF()
{
	for (int i = 0; i < inputListData.size(); i++)
	{
		//stop信号(1)のみ積算
		if (inputListData[i][columnInput - 2] == 1)
		{
			ui ch = inputListData[i][columnInput - 1] / BIN_WIDTH;//binWidthごとに1チャンネルにまとめる
			if (ch >= chNum)//chNumを超えるデータ（バグ）は読み飛ばす
			{
				continue;
			}
			tofIntegrated[ch]++;
		}
	}
}

string upperPath(string fullPath)
{
	int path_i = fullPath.find_last_of("\\");
	string upperPath = fullPath.substr(0, path_i);//最後の'\'は含まない
	return upperPath;
}

bool readFilename(string fnamePath)
{
	//パラメータが有効かどうかのフラグ
	bool isValid = false;

	string filepath = fnamePath;
	ifstream ifs(filepath);

	//パラメータファイルを1行ずつ読み取る
	string line;
	while (getline(ifs, line))
	{
		//0文字目が'#'の行は読み飛ばす
		if (line[0] == '#')
			continue;
		//'='がない行も読み飛ばす
		if (line.find('=') == string::npos)
			continue;

		//stringstreamインスタンスに読み取った文字列を入れる
		stringstream ss(line);

		//読み取った文字列のうち、スペースの前までの部分をname文字列に入れる
		string name;
		ss >> name;

		//'='に出会うまでの文字列を無視
		ss.ignore(line.size(), '=');

		//'='以降のパラメータをtempに格納
		string tmp;
		ss >> tmp;

		if (name == "filename")
		{
			isValid = true;
			inputDataName = tmp;
		}
	}
	return isValid;
}

vector<string> getFileNameInDir(string dir_name)
{
	HANDLE hFind;
	WIN32_FIND_DATA win32fd;
	vector<string> file_names;

	string extension = "txt";
	string search_name = dir_name + "\\*." + extension;
	hFind = FindFirstFile(search_name.c_str(), &win32fd);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		throw runtime_error("file not found");
	}

	do
	{
		if (win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
		}
		else
		{
			file_names.push_back(win32fd.cFileName);
		}
	} while (FindNextFile(hFind, &win32fd));

	FindClose(hFind);
	return file_names;
}
