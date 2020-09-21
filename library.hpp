//library.hpp : データ解析プログラムに共通の関数をまとめたヘッダファイル

//アップデートログ
//2020.2.27:作成
//2020.4.2:"writeListFile"の書き込み時の有効数字を10桁に設定
//2020.4.2:"getFileNameInDir"のextensionのデフォルト引数を".txt"に設定
//2020.6.12:"writeListFile"のデリミタを可変にした
//2020.7.6 :デバッグ用関数"showListData"を作成した
//2020.7.9:VS Codeに移植
//2020.7.16:"readClustersize"関数を作成
//2020.7.16:"doubletoString"関数を作成
//2020.7.16:"stripExtension"関数を作成
//2020.7.16:"writeListFile"関数にappend modeを追加（append引数で選択）
//2020.7.20:"readInvalid"関数を作成
//2020.7.28:"showSeries"関数を作成
//2020.7.30:"chmin/chmax"関数を追加
//2020.7.30:"average"関数を追加
//2020.7.31:"writeListFile_sep"を削除
//2020.7.31:"choose2FromN"関数を追加
//2020.7.31:"complement"関数を追加
//2020.8.25:"writeListFile"関数で、デリミタ、append modeオンオフ、書き込み精度を引数から追加できるように変更
//2020.9.19:"readListFile"関数で、値で読み込み条件を設定できるように、col、keyMin、keyMaxを引数に追加したものを作成

#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <Windows.h>
#include <stdexcept>
#include <direct.h>
#include <regex>
#include <chrono>
#include <filesystem>
#define all(x) (x).begin(), (x).end()
using namespace std;
namespace fs = std::filesystem;
typedef unsigned int ui;
typedef long long ll;
typedef unsigned long long ull;
typedef pair<int, int> P;

//--------------------------    CONSTANTS    ------------------------------------//

const string DATANAME_FILE = "dataname.txt";
const string INVALID_FILE = "invalid.txt";
const string DATA_EXTENSION = ".txt";
const int UPPERFOLDER_MAX = 4;//どれだけ上の階層までファイルを探しに行くか？の上限
const int FILENUM_MAX = 1000;//連番ファイルの上限
const int TOF_MAX = 1000000;//MCSのバグデータをはじくための定数
const auto PI = acos(-1);//円周率

const auto M_CARBON = 12;     //炭素の質量数(amu)
const auto e2 = 14.4;//素電荷の二乗(eV・Å)
const auto V_B = 2.19e6; //Bohr velocity(m/s)
const auto KE_AU = 25.; //25 keV/amu => 1 au
const auto L = 1025.;//Distance between detector-target(mm)
const auto mmPerPx = 0.08;//pixel -> mm

//--------------------------    chmax / chmin    ------------------------------------//
template<class T>inline bool chmax(T& a, T b) { if (a < b) { a = b; return true; }return false; }
template<class T>inline bool chmin(T& a, T b) { if (a > b) { a = b; return true; }return false; }

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

//stripExtension : ファイル名から拡張子を取り除く
string stripExtension(string withExt)
{
	int i = withExt.find_last_of('.');
	string res = withExt.substr(0, i);
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

//dを小数点n桁までの文字列に変換する
string doubletoString(double d, int n)
{
	stringstream ss;
	ss << fixed << setprecision(n) << d;
	string res = ss.str();
	return res;
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

//"invalid.txt"を読み取り、読み取ったリストをvectorにして返す
vector<int> readInvalid(string invalidFilepath)
{
	vector<int> res;

	//ファイルがなかったら空vectorを返す
	if(!checkFileExistence(invalidFilepath))
		return res;

	ifstream ifs(invalidFilepath);
	//invalidファイルを1行ずつ読み取る
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
		if (name == "invalid")
		{
			int tmp;
			while (ss >> tmp)
			{
				res.push_back(tmp);
				//','に出会うまでの文字列を無視
				ss.ignore(line.size(), ',');
			}
		}
	}
	return res;
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

//テキスト形式のリストデータを読み取り、二次元配列変数に入れる(col列目がkeyMin以上、keyMax未満の行のみ読み出す)
//unsigned short型のインデックスのオーバーフローも同時に直す
template<typename T = int>
vector<vector<T>> readListFile(string filePath, int col, int keyMin, int keyMax)
{
	ifstream ifs(filePath);
	string line;
	vector<vector<T>> res;
	
	//overflowの管理
	int overflowCnt = 0;
	int previous = 0;
	const int US_MAX = pow(2, 16);

	//ファイルから１行ずつ見ていく
	while (getline(ifs, line))
	{
		stringstream ss(line);
		string tmp;
		vector<T> inputLine;
		//一行分の配列を構成
		while (ss >> tmp)
		{
			inputLine.push_back(stoT<T>(tmp));
		}

		//大小関係が反転していたらoverflowと認識してcnt++
		if (previous > inputLine[col])
		{
			overflowCnt++;
		}
		//現在の値を覚えておく
		previous = inputLine[col];

		//overflowした回数だけUS_MAX=65536を足す
		inputLine[col] += overflowCnt * US_MAX;

		//条件を満たすもののみ読み込み
		if(keyMin <= inputLine[col] && inputLine[col] < keyMax)
		{
			res.push_back(inputLine);
		}
		else if(keyMax <= inputLine[col])
		{
			//範囲の上限よりも先のデータは見ない
			break;
		}
	}
	return res;
}

//二次元配列をテキストデータに書き込む(デリミタとappend modeの有無、書き込みの有効数字を引数から指定できるようにデフォルト引数に設定)
template<typename T = int>
void writeListFile(string outputFilePath, vector<vector<T>> outputListData, char delim = '\t', bool append = false, int prec = 10)
{
	//outputListDataが空配列でなければ
	if (outputListData.size())
	{
		//ファイルに書き込む
		ofstream ofs;
		if (!append)
		{
			//新規書き込みモード
			ofs = ofstream(outputFilePath);
		}
		else
		{
			//追加書き込みモード
			ofs = ofstream(outputFilePath, ios::app);
		}

		int columnOutput = outputListData[0].size();
		
		//有効数字10桁で書き込み
		ofs << setprecision(prec);
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
		cerr << "! caution : " << outputFilePath << " is empty !" << endl;
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
		//0埋め3桁の文字列に変換
		string suf = toSuffix(i);
		string fileName = dataFolder + "\\" + dataName + suf + DATA_EXTENSION;
		//1から順番に存在を確認。存在するなら次の番号へ、なければそのひとつ前の番号(i-1)を出力
		if (checkFileExistence(fileName))continue;
		return i - 1;
	}
	return -1;
}

//--------------------------    math    ------------------------------------//

//配列の平均値を求める
template<typename T = int>
double average(vector<T> array)
{
	double res = 0;
	int n = array.size();
	for (int i = 0; i < n; i++)
	{
		res += double(array[i]);
	}
	res /= double(n);
	return res;
}

//Nから2個選ぶ組合せを全探索しindex pairのリストを返す
vector<P> choose2FromN(int n)
{
	vector<P> res;
	for (int i = 0; i < n; i++)
	{
		for (int j = i + 1; j < n; j++)
		{
			res.push_back({i, j});
		}		
	}
	return res;
}

//補集合の要素を返す
vector<int> complement(int n, vector<int> set)
{
	vector<int> res;
	for (int i = 0; i < n; i++)
	{
		if(find(set.begin(), set.end(),i) == set.end())
			res.push_back(i);
	}
	return res;
}

//2点間の座標からユークリッド距離を返す
double calcDist(pair<double,double> p, pair<double, double> q)
{
	double x_0 = p.first;
	double y_0 = p.second;
	double x_1 = q.first;
	double y_1 = q.second;
	double res = sqrt((x_0 - x_1) * (x_0 - x_1) + (y_0 - y_1) * (y_0 - y_1));
	return res;
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
		cerr << "Showing first " << rowMax << " rows" << endl;

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

//一次元配列をコンソールに表示
template<typename T = int>
void showSeries(string seriesName, vector<T> series)
{
	//固定小数点で表示（小数点以下2桁）
	cout << fixed << setprecision(2);
	cout << seriesName + " = {";
	for (int i = 0; i < series.size(); i++)
	{
		cout << series[i] << ", ";
	}
	cout << "}\n";
} 