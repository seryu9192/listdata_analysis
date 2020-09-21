// ImagingNumberOfPointDistribution.cpp : イメージング画像に映っている輝点の個数分布を出力


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
typedef unsigned int ull;

constexpr auto columnInput = 5;
constexpr auto maxNum = 15;

vector<ui> inputListBuf;
vector<vector<ui>> inputListData;
vector<vector<int>> outputListData;
vector<int> numDist(maxNum, 0);

void countNumDist();

int main()
{
	cout << "***************************************************************" << endl;
	cout << "****          ImagingNumberOfPointDistribution.cpp         ****" << endl;
	cout << "**** イメージング画像に映っている輝点の個数分布解析プログラム   ****" << endl;
	cout << "****                  ver.2019.06.15 written by R. Murase  ****" << endl;
	cout << "***************************************************************" << endl << endl;

	//入力ファイルの指定
	string listDataFolder, listDataName, listDataPath;
	cout << "解析するリストデータのフォルダを入力してください\n--->";
	cin >> listDataFolder;
	cout << "解析するリストデータのファイル名を入力してください\n--->";
	cin >> listDataName;

	//出力フォルダの指定
	string outputFolderName = listDataFolder;

	//入力データパスの生成
	listDataPath = listDataFolder + "\\" + listDataName + ".txt";
	ifstream ifs(listDataPath);
	if (!ifs)
	{
		cerr << "ファイルを開けませんでした" << endl;
		return -1;
	}
	//出力データパスの生成
	string outputFilePath = outputFolderName + "\\" + listDataName + "numDist.txt";

	//inputListBuf(1Dvector)にリストデータを読み込み
	while (!ifs.eof())
	{
		string s;
		ifs >> s;
		ui tmp;
		if (s != "")
			tmp = stoi(s);
		inputListBuf.push_back(tmp);
	}

	//inputListBuf(1Dvector)を1行ずつinputLineにまとめてinputListData(2Dvector)に変換
	for (int i = 0; i < inputListBuf.size() / columnInput; i++)
	{
		vector<ui> inputLine;
		for (int j = 0; j < columnInput; j++)
		{
			int index = columnInput * i + j;
			int tmp = inputListBuf[index];
			inputLine.push_back(tmp);
		}
		inputListData.push_back(inputLine);
	}

	cout << "----データの読み込み完了、処理を開始します。" << endl;

	//個数分布を数える
	countNumDist();

	//outputListDataを構成
	for (int i = 0; i < maxNum; i++)
	{
		vector<int> outputLine = { i,numDist[i] };
		outputListData.push_back(outputLine);
	}

	cout << "----処理終了" << endl;

	//出力ファイルにoutputListDataを書き込み
	ofstream ofs(outputFilePath);
	int columnOutput = outputListData[0].size();
	for (int i = 0; i < outputListData.size(); i++)
	{
		for (int j = 0; j < columnOutput; j++)
		{
			ofs << outputListData[i][j];
			if (j != columnOutput - 1) ofs << "\t";
		}
		ofs << endl;
	}
	cout << outputFilePath << "に書き込みました" << endl;
	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	return 0;
}

//countNumDist():輝点の個数分布を構成
void countNumDist()
{
	int sweepNum = inputListData[0][0];//今見ているsweepのインデックス
	int k = 0;//今見ている行のインデックス

	//リストデータの最後まで1つずつ見ていく
	while (k < inputListData.size())
	{
		vector<vector<ui>> tmp;
		sweepNum = inputListData[k][0];
		tmp.push_back(inputListData[k]);

		//sweep番号が変わるところまでで部分配列を抜き出す(tmp)
		while ((k + 1 < inputListData.size()) && (inputListData[k][0] == inputListData[k + 1][0]))
		{
			tmp.push_back(inputListData[k + 1]);
			k++;
		}
		int num = tmp.size();
		numDist[num]++;
		k++;
	}
}
