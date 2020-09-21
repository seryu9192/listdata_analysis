// SIMION_tof_ListToHistogram.cpp : SIMIONのTOFsimulationデータからヒストグラムを作成
//
//アップデートログ
//2019.8.19:出力チャンネルを20000chn->8000chnに変更
//2019.8.25:チャンネルナンバーを出力しないようにする
//2019.8.25:出力チャンネルを8000chn->4000chnに変更

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

double binWidth = 0.0032;//us

vector<vector<double>> inputListData;
vector<int> outputHistogram(4032,0);

int main()
{
	cout << "***********************************************************" << endl;
	cout << "****  SIMION TOF List -> Histogram(3.2ns/chn,4032chn)  ****" << endl;
	cout << "****               ver.2019.08.19 Written by R. Murase ****" << endl;
	cout << "***********************************************************" << endl << endl;

	string inputDataPath;
	cout << "Write full path of input data\n--->";
	cin >> inputDataPath;

	int path_i = inputDataPath.find_last_of("\\") + 1;
	int ext_i = inputDataPath.find_last_of(".");

	string pathname = inputDataPath.substr(0, path_i);
	string extname = inputDataPath.substr(ext_i, inputDataPath.size() - ext_i);
	string filename = inputDataPath.substr(path_i, ext_i - path_i);

	ifstream ifs(inputDataPath);
	if (!ifs)
	{
		cerr << "File open failed" << endl;
		system("pause");
		return -1;
	}

	//データ読み込み
	bool isData = false;
	int line = 0;
	while (!ifs.eof())
	{
		line++;
		string tmp;
		getline(ifs, tmp);

		//"[DATA]"が書いてある行以降か、13行目以降を読み込み
		if (isData)
		{
			if (tmp == "" || tmp == "") continue;
			//リストデータを読み込み
			stringstream ss(tmp);
			string tmp2;
			vector<double> inputLine;
			while (ss >> tmp2)
			{
				inputLine.push_back(stod(tmp2));
			}
			inputListData.push_back(inputLine);
		}
		//"[DATA]"以降の文字列(＝データ)に対して変換を行うためのフラグ
		if (tmp == "[DATA]"|| line >= 12) isData = true;
	}
	
	for (int i = 0; i < inputListData.size(); i++)
	{
		double tof = inputListData[i][2];
		int chn = tof / binWidth;
		if (inputListData[i][1]==4)
		{
			outputHistogram[chn]++;
		}
	}

	string outputDataPath = pathname + filename + "_histogram(3.2ns_chn)" + extname;
	ofstream ofs(outputDataPath);
	if (!ofs)
	{
		cerr << "File write failed" << endl;
		system("pause");
		return -1;
	}
	
	for (int i = 0; i < outputHistogram.size(); i++)
	{
		ofs << outputHistogram[i] << endl;
	}
	return 0;
}
