//library.hpp : データ解析プログラムに共通の関数をまとめたヘッダファイル

//アップデートログ
//2020.2.27:作成
//2020.4.2:"writeListFile"の書き込み時の有効数字を10桁に設定
//2020.4.2:"getFileNameInDir"のextensionのデフォルト引数を".txt"に設定
//2020.6.12:"writeListFile"のデリミタを可変にした
//2020.7.6 :デバッグ用関数"showListData"を作成した
//2020.7.9:VS Codeに移植

#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <string>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <Windows.h>
#include <stdexcept>
#include <direct.h>
#include <regex>
#define all(x) (x).begin(), (x).end()
using namespace std;
typedef unsigned int ui;
typedef long long ll;
typedef unsigned long long ull;

//--------------------------    CONSTANTS    ------------------------------------//

const string DATANAME_FILE = "dataname.txt";
const string DATA_EXTENSION = ".txt";
const int UPPERFOLDER_MAX = 4;//どれだけ上の階層までファイルを探しに行くか？の上限
const int FILENUM_MAX = 1000;//連番ファイルの上限
const int TOF_MAX = 1000000;//MCSのバグデータをはじくための定数
const auto PI = acos(-1);//円周率

//--------------------------    file path and directory    ------------------------------------//

//parsePath : パスをpathname/filename/extensionに分割した文字列のリストを返す
vector<string> parsePath(string fullpath)
{
	vector<string> res(3);
	int path_i = fullpath.find_last_of("\\") + 1;
	int ext_i = fullpath.find_last_of(".");
	res[0] = fullpath.substr(0, path_i);
	res[1] = fullpath.substr(path_i,ext_i - path_i);
	res[2] = fullpath.substr(ext_i, fullpath.size()-ext_i);
	return res;
}

//upperFolder : 今のファイルのi階層上のフォルダのフルパスを返す(デフォルトは一つ上)
string upperFolder(string fullPath, int i = 1)
{
	if (i == 0) return fullPath;
	else
	{
		int path_i = fullPath.find_last_of("\\");
		string res = fullPath.substr(0, path_i);//最後の'\'は含まない
		return upperFolder(res, i - 1);
	}
}

//folderName : 今のファイルが属しているフォルダの名前を返す
string folderName(string fullPath)
{
	int path_i = fullPath.find_last_of("\\");
	string folderName = fullPath.substr(int(path_i) + 1, fullPath.size());
	return folderName;
}

//指定したフォルダ内のファイル名を拡張子付きで取得(第二引数は拡張子(e.g.".txt"))
vector<string> getFileNameInDir(string dirPath, string extension = ".txt")
{
	HANDLE hFind;
	WIN32_FIND_DATA win32fd;
	vector<string> file_names;

	string search_name = dirPath + "\\*" + extension;
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

//ファイルの存在確認
bool checkFileExistence(string path)
{
	ifstream ifs(path);
	return ifs.is_open();
}

//"現在のフォルダ(curDir)から上の階層のフォルダにfileToFindの名前のついたファイルを探索し、そのファイルのパスを返す
string searchFileFromUpperFolder(string curDir, string fileToFind)
{
	//見つかれなければ空文字列を返す
	string res;

	//近い階層から早い者勝ちで見つける（違う階層に複数同じ同じ名前のファイルがある場合は近い階層のファイルのパスが返される）
	for (int up = 1; up <= UPPERFOLDER_MAX; up++)
	{
		string dataNameFolder = upperFolder(curDir, up);
		string path = dataNameFolder + "\\" + fileToFind;
		//fileToFind"がその階層に存在すれば、パスを返して探索終了
		if (checkFileExistence(path))
		{
			res = path;
			break;
		}
	}
	return res;
}

//file indexを0埋め3桁の文字列suffixに変換
string toSuffix(int ind)
{
	ostringstream sout;
	sout << setfill('0') << setw(3) << ind;
	string suffix = sout.str();
	return suffix;
}

//--------------------------    read and write list files    ------------------------------------//

//文字列を任意の形式の数値に変換(デフォルト：int)
template <typename T = int> T stoT(string src)
{
	T res;
	istringstream(src) >> res;
	return res;
}

//"dataname.txt"を読み取り、読み取ったファイル名(e.g. "20200227")を返す(見つからない場合は空文字列を返す)
string readDataName(string dNamePath)
{
	//"dataname.txt"のパス
	string filepath = dNamePath;
	ifstream ifs(filepath);

	//返り値(見つからない場合は空文字列を返す)
	string res;

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

		//'='以降のパラメータをtmpに格納
		string tmp;
		ss >> tmp;

		if (name == "dataname")
		{
			res = tmp;
		}
	}
	return res;
}

//"dataname.txt"を読み取り、読み取ったクラスターサイズを返す(見つからない場合は-1を返す)
int readClustersize(string fnamePath)
{
	//パラメータが有効かどうかのフラグ
	bool isValid = false;

	string filepath = fnamePath;
	ifstream ifs(filepath);

	//返り値(見つからない場合は-1)
	int res = -1;

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

		//'='以降のパラメータをtmpに格納
		int tmp;
		ss >> tmp;

		if (name == "clustersize")
		{
			isValid = true;
			res = tmp;
		}
	}
	return res;
}

//フォルダにある連番ファイルの最後の番号を読む
int lastFileNum(string dataFolder)
{
	//データ名を読んでくる
	string dataName = readDataName(searchFileFromUpperFolder(dataFolder, DATANAME_FILE));
	//データ名に不備がある場合は-1を返す
	if (dataName == "")return -1;
	//1から順番にsuffixを増やして、存在しなくなる番号を探す
	for (int i = 1; i <= FILENUM_MAX; i++)
	{
		ostringstream oss_i;
		oss_i << setfill('0') << setw(3) << i;
		string suf = oss_i.str();
		string fileName = dataFolder + "\\" + dataName + suf + DATA_EXTENSION;
		//1から順番に存在を確認。存在するなら次の番号へ、なければそのひとつ前の番号(i-1)を出力
		if (checkFileExistence(fileName))continue;
		return i - 1;
	}
	return -1;
}

//テキスト形式のリストデータを読み取り、二次元配列変数に入れる(全部読み込む)
template<typename T = int>
vector<vector<T>> readListFile(string filePath)
{
	ifstream ifs(filePath);
	string line;
	vector<vector<T>> res;
	while (getline(ifs, line))
	{
		stringstream ss(line);
		string tmp;
		vector<T> inputLine;
		while (ss >> tmp)
		{
			inputLine.push_back(stoT<T>(tmp));
		}
		//全部読み込み
		res.push_back(inputLine);
	}
	return res;
}

//テキスト形式のリストデータを読み取り、二次元配列変数に入れる(bstart <= inputList[colEventNum] < bendを満たすデータのみ読み取り)
template<typename T = int>
vector<vector<T>> readListFile(string filePath, int colEventNum, int bstart, int bend)
{
	ifstream ifs(filePath);
	string line;
	vector<vector<T>> res;
	while (getline(ifs, line))
	{
		stringstream ss(line);
		string tmp;
		vector<T> inputLine;
		while (ss >> tmp)
		{
			inputLine.push_back(stoT<T>(tmp));
		}

		//指定した範囲のeventNumの行のみ読み込み(バッチ処理用)
		int eventNum = inputLine[colEventNum];
		if (bstart <= eventNum && eventNum < bend)
		{
			res.push_back(inputLine);
		}
		if (bend <= eventNum)
			break;
	}
	return res;
}

//二次元配列をテキストデータに書き込む
template<typename T = int>
void writeListFile(string outputFilePath, vector<vector<T>> outputListData, char delim = '\t')
{
	//outputListDataが空配列でなければ
	if (outputListData.size())
	{
		//ファイルに書き込む
		ofstream ofs(outputFilePath);
		int columnOutput = outputListData[0].size();
		
		//有効数字10桁で書き込み
		ofs << setprecision(10);
		for (int r = 0; r < outputListData.size(); r++)
		{
			for (int c = 0; c < columnOutput; c++)
			{
				ofs << outputListData[r][c];
				if (c != columnOutput - 1) ofs << delim; //デリミタ
			}
			ofs << endl;
		}
	}
	else
	{
		cerr << "! caution : " << outputFilePath << "is empty !" << endl;
	}
	return;
}

//一次元データをテキストファイルに書き込む
template<typename T = int>
void writeListFile(string outputFilePath, vector<T> outputListData)
{
	//outputListDataが空配列でなければ
	if (outputListData.size())
	{
		//ファイルに書き込む
		ofstream ofs(outputFilePath);

		//有効数字10桁で書き込み
		ofs << setprecision(10);
		for (int i = 0; i < outputListData.size(); i++)
		{
			ofs << outputListData[i] << endl;
		}
	}
	else
	{
		cerr << "! caution : " << outputFilePath << "is empty !" << endl;
	}
	return;
}

template<typename T = int>
void writeListFile_sep(string outputFilePath, vector<vector<vector<T>>> outputListData)
{
	//outputListDataが空配列でなければ
	if (outputListData.size())
	{
		//ファイルに書き込む
		ofstream ofs(outputFilePath);

		//ファイル番号のループ
		for (int fileNum = 1; fileNum < outputListData.size(); fileNum++)
		{
			if (outputListData[fileNum].empty())continue;
			int columnOutput = outputListData[fileNum][0].size();
			for (int j = 0; j < outputListData[fileNum].size(); j++)
			{
				for (int k = 0; k < columnOutput; k++)
				{
					ofs << outputListData[fileNum][j][k];
					if (k != columnOutput - 1) ofs << "\t";//デリミタ
				}
				ofs << endl;
			}
		}
	}
	else
	{
		//cout << outputFilePath << "は空です" << endl;
	}
	return;
}
//--------------------------    debug    ------------------------------------//


//二次元配列をコンソールに表示
template<typename T = int>
void showListData(vector<vector<T>> outputListData, char delim = '\t', int rowMax = 10)
{
	//outputListDataが空配列でなければ
	if (outputListData.size())
	{
		//コンソールに表示
		int columnOutput = outputListData[0].size();

		//行数と列数を表示
		cerr << "row : " << outputListData.size() << ", col : " << columnOutput << endl;
		cerr << "Showing first" << rowMax << " rows" << endl;

		//有効数字10桁で書き込み
		cerr << setprecision(10);
		for (int j = 0; j < rowMax; j++)
		{
			for (int k = 0; k < columnOutput; k++)
			{
				cerr << outputListData[j][k];
				if (k != columnOutput - 1) cerr << delim; //デリミタ
			}
			cerr << endl;
		}
	}
	else
	{
		cerr << "! caution : The list data is empty!" << endl;
	}
	return;
}