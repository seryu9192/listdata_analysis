// MCS6A_massAssignment.cpp : リストデータをchn表記からm/z表記に変換する
//

#include "pch.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
using namespace std;
typedef unsigned int ui;
typedef unsigned long long ull;

int columnInput;

//sqrt(m/z)=a*chn+bの係数（質量が既知のピークを用いて線形フィットで別途求める）
double a;
double b;

vector<int> inputListBuf;
vector<vector<int>> inputListData;
vector<vector<double>> outputListData;

bool readParameter(string);

int main()
{
	cout << "*****************************************************************************" << endl;
	cout << "****                       MCS6A_massAssignment.cpp                      ****" << endl;
	cout << "****      -TOFリストデータをchn表記からm/z表記に変換する(0.4ns/chn) -    ****" << endl;
	cout << "**** ※Parameter file は入力ファイルディレクトリに「ファイル名param.txt」****" << endl;
	cout << "****                                     の名前であらかじめ作成しておく  ****" << endl;
	cout << "****                               ver.2019.06.04 written by R. Murase   ****" << endl;
	cout << "*****************************************************************************" << endl << endl;

	//入力TOFデータパスの生成
	string listDataFolder, listDataName;
	cout << "TOFリストデータのフォルダを入力してください\n--->";
	cin >> listDataFolder;
	cout << "TOFリストデータのファイル名を入力してください\n--->";
	cin >> listDataName;


	//入力TOFデータの読み込み
	string listDataPath = listDataFolder + "\\" + listDataName + ".txt";

	ifstream ifs(listDataPath);
	if (!ifs)
	{
		cerr << "ファイルを開けませんでした" << endl;
		system("pause");
		return -1;
	}

	//入力パラメータパスの生成
	string parameterFilePath = listDataFolder + "\\" + listDataName + "param.txt";
	ifstream ifs2(parameterFilePath);
	if (!ifs2)
	{
		cerr << "パラメータファイルを開けませんでした" << endl;
		system("pause");
		return -1;
	}
	else if (!readParameter(parameterFilePath))//パラメータの読み込み
	{
		cerr << "パラメータファイルが不正です" << endl;
		system("pause");
		return -1;
	}

	//入力データのcolumn数を決定
	int connected;
	cout << "TOFリストデータはファイル番号で結合されたものですか？(はい：1,いいえ：2)\n--->";
	cin >> connected;
	if (connected==1)
	{
		columnInput = 4;
	}
	else if (connected==2)
	{
		columnInput = 3;
	}
	else
	{
		cerr << "入力が不正です！プログラムを終了します" << endl;
		system("pause");
		return -1;
	}

	//inputListBuf_tof(1Dvector)にTOFデータを読み込み
	while (!ifs.eof())
	{
		string s;
		ifs >> s;
		double tmp;
		if (s != "")
		{
			tmp = stoi(s);
			inputListBuf.push_back(tmp);
		}
	}

	//inputListBuf(1Dvector)を1行ずつinputLineにまとめてinputListData(2Dvector)に変換
	for (int i = 0; i < inputListBuf.size() / columnInput; i++)
	{
		vector<int> inputLine;
		for (int j = 0; j < columnInput; j++)
		{
			int index = columnInput * i + j;
			int tmp = inputListBuf[index];
			inputLine.push_back(tmp);
		}
		inputListData.push_back(inputLine);
	}
	inputListBuf.clear();
	cout << "----データの読み込み完了" << endl;

	//二次イオン種ごとに解析
	//inputListDataをはじめから1行ずつ見ていく
	for (int j = 0; j < inputListData.size(); j++)
	{
		vector<double> outputLine;
		//リストデータのcolumn数が4の時（結合されたリストデータ）
		if (columnInput==4)
		{
			int fileNum = inputListData[j][0];
			int sweepNum = inputListData[j][1];
			int chNum = inputListData[j][2];
			double chn = inputListData[j][3];
			double m_z = (a*chn + b)*(a*chn + b);//channelをm/zに変換

			//outputList1行を構成+outputListにpushback
			outputLine.push_back(fileNum);
			outputLine.push_back(sweepNum);
			outputLine.push_back(chNum);
			outputLine.push_back(m_z);
			outputListData.push_back(outputLine);
		}
		//リストデータのcolumn数が3の時
		else
		{
			int fileNum = inputListData[j][0];
			int chNum = inputListData[j][1];
			double chn = inputListData[j][2];
			double m_z = (a*chn + b)*(a*chn + b);//channelをm/zに変換

			//outputList1行を構成+outputListにpushback
			outputLine.push_back(fileNum);
			outputLine.push_back(chNum);
			outputLine.push_back(m_z);
			outputListData.push_back(outputLine);
		}
	}

	//出力ファイルにoutputListDataを書き込み
	string outputFileName = listDataName+"_msAssgnd";
	string outputFilePath = listDataFolder + "\\" + outputFileName + ".txt";

	ofstream ofs(outputFilePath);

	int columnOutput = outputListData[0].size();
	for (int j = 0; j < outputListData.size(); j++)
	{
		for (int k = 0; k < columnOutput; k++)
		{
			ofs << outputListData[j][k];
			if (k != columnOutput - 1) ofs << "\t";//デリミタ
		}
		ofs << endl;
	}
	cout << outputFilePath << "に書き込みました" << endl;
	cout << "a=" << a << endl;
	cout << "b=" << b << endl;
	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	return 0;
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

		double pValue;
		ss >> pValue;

		if (name == "a")
		{
			a = pValue;
		}
		else if (name == "b")
		{
			b = pValue;
		}
		else
		{
			isValid = false;
		}
	}
	return isValid;
}