// MCS6A_integrateCoincTOF.cpp : TOFの切り分けたlistデータからスペクトルを生成

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
constexpr auto columnTOF = 4;
constexpr auto binWidth = 8;
constexpr auto mcsCh = 16000;

vector<ui> inputListBuf;
vector<vector<ui>> inputListData;

ui tofIntegrated[mcsCh] = {};

void integrateTOF();

int main()
{
	cout << "***************************************************************" << endl;
	cout << "*********　　  MCS6A listデータ → スペクトル   ***************" << endl;
	cout << "**** 解析後のリストデータからスペクトルを構成するプログラム ***" << endl;
	cout << "****                   ver.2019.01.14 written by R. Murase ****" << endl;
	cout << "***************************************************************" << endl << endl;

	//入力データの指定
	string listDataFolder, listDataName, listDataPath;
	cout << "解析するリストデータのフォルダを入力してください\n--->";
	cin >> listDataFolder;
	cout << "解析するリストデータのファイル名を入力してください\n--->";
	cin >> listDataName;

	//出力フォルダの指定
	string outputFolderName;
	cout << "\n出力するフォルダのパスを入力してください\n--->";
	cin >> outputFolderName;

	//入力データパスの生成
	listDataPath = listDataFolder + "\\" + listDataName + ".txt";
	ifstream ifs(listDataPath);
	if (!ifs)
	{
		cerr << "ファイルを開けませんでした" << endl;
		return -1;
	}

	//inputListBuf(1Dvector)にリストデータを読み込み
	while (!ifs.eof())
	{
		string s;
		ifs >> s;
		ui tmp;
		//読み取った文字列が空でなければ、sをint型に変換
		if (s != "")
			tmp = stoi(s);
		inputListBuf.push_back(tmp);
	}

	//inputListBuf(1Dvector)を1行ずつinputLineにまとめてinputListData(2Dvector)に変換
	for (int i = 0; i < inputListBuf.size() / columnTOF; i++)
	{
		vector<ui> inputLine;
		for (int j = 0; j < columnTOF; j++)
		{
			int index = columnTOF * i + j;
			int tmp = inputListBuf[index];
			inputLine.push_back(tmp);
		}
		inputListData.push_back(inputLine);
	}

	cout << listDataName + ".txt" + "----データの読み込み完了、処理を開始します。" << endl;

	//スペクトル構成処理
	integrateTOF();

	cout << listDataName + ".txt" + "----処理終了" << endl;

	//出力データパスの生成
	string outputFilePath = outputFolderName + "\\" + listDataName + "_mpa.txt";
	//出力ファイルにoutputListDataを書き込み
	ofstream ofs(outputFilePath);

	//出力ファイルへの書き込み処理
	for (int i = 0; i < mcsCh; i++)
	{
		ofs << i + 1 << "\t" << tofIntegrated[i] << endl;
	}
	cout << outputFilePath << "に書き込みました" << endl << endl;

	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	return 0;
}

// 1行ずつchを読み取ってtofIntegratedに積算
void integrateTOF()
{
	for (int i = 0; i < inputListData.size(); i++)
	{
		//stop信号(1)のみ積算
		if (inputListData[i][2] == 1)
		{
			ui ch = inputListData[i][3] / binWidth;//binWidthごとに1チャンネルにまとめる
			tofIntegrated[ch]++;
		}
	}
}