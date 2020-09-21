// MCS6A_extractEvent.cpp : 着目する二次イオンが放出されたイベントごとにリストデータを分ける
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

constexpr auto columnInput = 4;

//ROIを決めるベクトル
vector<string> SI = { "C6H5","C7H7","[Phe-COOH]","[Phe+H]" };//二次イオンの名前
vector<int> roimin = { 22664,24720,28512,33672 };//着目するTOFのROIのチャンネル m/z = {77,91,120,166}
vector<int> roimax = { 22816,24856,28632,33776 };

vector<int> inputListBuf;
vector<vector<int>> inputListData;
vector<vector<int>> outputListData;

int main()
{
	cout << "*************************************************************" << endl;
	cout << "****                MCS6A_extractEvent.cpp               ****" << endl;
	cout << "****  -TOFリストデータを着目する二次イオンごとに分ける-  ****" << endl;
	cout << "****               ver.2019.01.30 written by R. Murase   ****" << endl;
	cout << "*************************************************************" << endl << endl;


	//入力TOFデータパスの生成
	string listDataFolder_tof, listDataName;
	cout << "TOFリストデータのフォルダを入力してください\n--->";
	cin >> listDataFolder_tof;
	cout << "TOFリストデータのファイル名を入力してください\n--->";
	cin >> listDataName;

	//入力TOFデータの読み込み
	string listDataPath_tof = listDataFolder_tof + "\\" + listDataName + ".txt";

	ifstream ifs_tof(listDataPath_tof);
	if (!ifs_tof)
	{
		cerr << "ファイルを開けませんでした" << endl;
		return -1;
	}

	//出力データパスの生成
	string outputDataFolder;
	cout << "出力データのフォルダ名を入力してください\n--->";
	cin >> outputDataFolder;

	//inputListBuf_tof(1Dvector)にTOFデータを読み込み
	while (!ifs_tof.eof())
	{
		string s;
		ifs_tof >> s;
		double tmp;
		if (s != "")
			tmp = stod(s);
		inputListBuf.push_back(tmp);
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

	cout << listDataPath_tof + "----データの読み込み完了" << endl;

	//二次イオン種ごとに解析
	for (int i = 0; i < roimin.size(); i++)
	{
		//inputListDataをはじめから1行ずつ見ていく
		for (int j = 0; j < inputListData.size(); j++)
		{
			int tof = inputListData[j][3];
			if (roimin[i] < tof && tof < roimax[i])
			{
				outputListData.push_back(inputListData[j]);
			}
		}

		//出力ファイルにoutputListDataを書き込み
		string outputFileName = listDataName + "_" + SI[i];
		string outputFilePath = outputDataFolder + "\\" + outputFileName + ".txt";

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
		outputListData.clear();
	}

	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	return 0;
}
