// Imaging_separateMolAxis.cpp : molAxis形式のリストデータを角度ごとに分ける
//

//アップデートログ
//2019.10.12:作成

#include "pch.h"
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
using namespace std;
typedef unsigned int ui;
typedef unsigned long long ull;

vector<vector<double>> inputListData;
vector<vector<double>> outputListData;

//角度を分ける範囲を設定するvector
vector<int> thetaMin;
vector<int> thetaMax;

string inputDataName;

//分子軸ごとにデータを分ける関数
void separateMolAxis(double, double);
vector<string> getFileNameInDir(string);
string upperPath(string);
bool readFilename(string);
bool readParameter(string);
string stripExt(string);

int main()
{
	cout << "****************************************************************" << endl;
	cout << "**** -molAxisリストデータを角度ごとに分解するプログラム-    ****" << endl;
	cout << "****                molAxis形式のリストデータを             ****" << endl;
	cout << "****         分子軸角度ごとに別のファイルに保存する         ****" << endl;
	cout << "****                  ver.2019.10.12 written by R. Murase   ****" << endl;
	cout << "****************************************************************" << endl << endl;

	string inputDataFolder, inputDataPath;
	cout << "解析するリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder;

	//paramFile読み込み用にファイル名読み込む
	string filenamePath = upperPath(upperPath(inputDataFolder)) + "\\filename.txt";
	if (readFilename(filenamePath))
	{
		cout << "filename.txtファイルを読み込みました\n";
		cout << "解析するTOFリストデータのファイル名：" << inputDataName << endl;
	}
	else
	{
		cout << "filename.txtファイルが不正です\n";
		cout << "解析するTOFリストデータのファイル名を入力してください\n--->";
		cin >> inputDataName;
	}

	//入力イメージングデータフォルダ内のファイルの名前を取得
	vector<string> fName = getFileNameInDir(inputDataFolder);

	//フォルダにあるファイルのうち、param fileは除外
	for (int i = 0; i < fName.size(); i++)
	{
		if (fName[i].find("param") != string::npos)
		{
			fName.erase(fName.begin() + i);
		}
	}

	string outputDataFolder;
	cout << "出力するリストデータのフォルダを入力してください (\"@\"入力で同じフォルダ内の\"separated\"に保存)\n--->";
	cin >> outputDataFolder;
	if (outputDataFolder == "@")
	{
		outputDataFolder = inputDataFolder + "\\separated";
	}

	//入力パラメータパスの生成
	string parameterFilePath = outputDataFolder + "\\" + inputDataName + "param.txt";
	ifstream ifs2(parameterFilePath);
	if (!ifs2)
	{
		cerr << "パラメータファイルを開けませんでした。ファイル名を確認してください" << endl;
		cerr << "プログラムを終了します。" << endl;
		system("pause");
		return -1;
	}
	else if (!readParameter(parameterFilePath))//パラメータの読み取り
	{
		cerr << "パラメータファイルが不正です。プログラムを終了します。" << endl;
		system("pause");
		return -1;
	}
	else
	{
		cout << "パラメータファイルを読み取りました" << endl;
	}

	//入力ファイルの数だけループ
	for (int i = 0; i < fName.size(); i++)
	{
		//入力データパスの生成
		string file_now = stripExt(fName[i]);

		inputDataPath = inputDataFolder + "\\" + file_now + ".txt";
		ifstream ifs(inputDataPath);
		if (!ifs)
		{
			cerr << "ファイルを開けませんでした" << endl;
			system("pause");
			return -1;
		}

		//listDataを読み込み
		string tmp;
		while (getline(ifs, tmp))
		{
			stringstream ss(tmp);
			string tmp2;
			vector<double> inputLine;
			while (ss >> tmp2)
			{
				inputLine.push_back(stod(tmp2));
			}
			inputListData.push_back(inputLine);
		}

		cout << file_now + "----データの読み込み完了" << endl;

		//thetaの範囲の数だけループ
		for (int j = 0; j < thetaMin.size(); j++)
		{
			//分子軸角度ごとにファイルを分けて保存
			int tmin_now = thetaMin[j];
			int tmax_now = thetaMax[j];

			//範囲に入っているeventをpush_back
			separateMolAxis(tmin_now, tmax_now);
			cout << "theta = " << tmin_now << " - " << tmax_now << "degree ----データの処理終了" << endl;

			//出力データパスの生成
			string tmin_str = to_string(tmin_now);
			string tmax_str = to_string(tmax_now);
			string outputFilePath = outputDataFolder + "\\" + file_now + "_t=" + tmin_str + "-" + tmax_str + ".txt";

			//出力ファイルにoutputListDataを書き込み
			ofstream ofs(outputFilePath);
			int columnOutput = outputListData[0].size();

			//doubele型でイベント番号を扱う時、イベント数が1,000,000を超えるとデフォルト(6桁)では桁落ちするので、書き込み精度を7桁にする
			ofs << setprecision(7);

			for (int i = 0; i < outputListData.size(); i++)
			{
				for (int j = 0; j < columnOutput; j++)
				{
					ofs << outputListData[i][j];
					if (j != columnOutput - 1) ofs << "\t";//デリミタ
				}
				ofs << endl;
			}
			cout << outputFilePath << "に書き込みました" << endl << endl;

			//次の出力ファイルのために、outputリストデータに使用したメモリをクリア
			outputListData.clear();
		}
		//次の入力ファイルのために、inputリストデータに使用したメモリをクリア
		inputListData.clear();
	}
	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	return 0;
}

//分子軸ごとにデータを分けてoutputListDataにpush_back
void separateMolAxis(double thmin,double thmax)
{
	int columnMolAxis = inputListData[0].size() - 4;
	for (int i = 0; i < inputListData.size(); i++)
	{
		double th = inputListData[i][columnMolAxis];
		if (thmin<=th && th <= thmax)
		{
			outputListData.push_back(inputListData[i]);
		}
	}
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

string upperPath(string fullPath)
{
	int path_i = fullPath.find_last_of("\\");
	string res = fullPath.substr(0, path_i);//最後の'\'は含まない
	return res;
}

//拡張子外し
string stripExt(string fname)
{
	int path_e = fname.find_last_of(".");
	string res = fname.substr(0, path_e);
	return res;
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
		string temp;
		ss >> temp;

		if (name == "filename")
		{
			isValid = true;
			inputDataName = temp;
		}
	}
	return isValid;
}

bool readParameter(string pfilePath)
{
	//パラメータが有効かどうかのフラグ
	bool isValid = true;

	string filepath = pfilePath;
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

		//分けるthetaの範囲を読み込み
		if (name == "thetaMin")
		{
			int temp;
			while (ss >> temp)
			{
				thetaMin.push_back(temp);
				//'\t'に出会うまでの文字列を無視
				ss.ignore(line.size(), '\t');
			}
		}
		else if (name == "thetaMax")
		{
			int temp;
			while (ss >> temp)
			{
				thetaMax.push_back(temp);
				//'\t'に出会うまでの文字列を無視
				ss.ignore(line.size(), '\t');
			}
		}
		//いらんことが書いてあったら不正ファイル
		else
		{
			isValid = false;
		}
	}

	//ROIのminとmaxの数が合わないときはパラメータファイルが不正
	if (thetaMin.size() != thetaMax.size())
	{
		isValid = false;
	}
	return isValid;
}