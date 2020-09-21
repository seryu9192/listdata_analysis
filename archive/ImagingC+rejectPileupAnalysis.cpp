// ImagingC+rejectPileupAnalysis.cpp : Charge形式のイメージングリストデータからバックグラウンド(q=-1)を消した後、
//　　　　　　　　　　　　　　　　　　 1つだけ光っているフレームのみを切り出してくる(1枚に複数の点が光っているPileupを除去)

//アップデートログ
//2019.10.12:filename.txtから自動でファイル名を読み込ませるようにした


#include "pch.h"
#include "data_analysis_library.h"
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

vector<vector<int>> inputListData;
vector<vector<int>> intermediateListData;//BG除去したリストが一時的に入る
vector<vector<int>> outputListData;

string inputDataName;

int main()
{
	cout << "*******************************************************" << endl;
	cout << "****        -Imaging C1 pileup除去プログラム-      ****" << endl;
	cout << "****           バックグラウンドを除去し、          ****" << endl;
	cout << "**** 輝点が1つのもののみを抽出しファイルに書き込み ****" << endl;
	cout << "****         ver.2019.10.12 written by R. Murase   ****" << endl;
	cout << "*******************************************************" << endl << endl;

	string inputDataFolder;
	cout << "解析するリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder;

	string filenamePath = upperPath(upperPath(inputDataFolder)) + "\\filename.txt";
	cout << filenamePath << endl;
	if (readFilename(filenamePath))
	{
		cout << "filename.txtファイルを読み込みました\n";
		cout << "解析するリストデータのファイル名：" << inputDataName << endl;
	}
	else
	{
		cout << "filename.txtファイルが不正です\n";
		cout << "解析するリストデータのファイル名を入力してください\n--->";
		cin >> inputDataName;
	}

	int start, end;
	cout << "解析するファイル番号(start,end)を入力してください" << endl;
	cout << "start : ";
	cin >> start;
	cout << "end : ";
	cin >> end;

	//出力フォルダの指定
	string outputFolderName;
	cout << "\n出力するフォルダのパスを入力してください  (\"@\"入力で同じフォルダ内の\"3_C1\"に保存) \n--->";
	cin >> outputFolderName;
	if (outputFolderName == "@")
	{
		outputFolderName = upperPath(inputDataFolder) + "\\3_C1";
	}

	for (int m = start; m <= end; m++)
	{
		//file indexを0詰め3桁の文字列suffixに変換
		ostringstream sout;
		sout << setfill('0') << setw(3) << m;
		string suf = sout.str();

		//入力データパスの生成
		string inputDataPath = inputDataFolder + "\\" + inputDataName + suf + ".txt";
		ifstream ifs(inputDataPath);
		if (!ifs)
		{
			cerr << "ファイルを開けませんでした" << endl;
			return -1;
		}
		//出力データパスの生成
		string outputFilePath = outputFolderName + "\\" + inputDataName + suf + ".txt";

		//リストデータを1行ずつ読み取り、\tで分けてinputListDataにpush_back
		string tmp;
		while (getline(ifs, tmp))
		{
			stringstream ss(tmp);
			string tmp2;
			vector<int> inputLine;
			while (ss >> tmp2)
			{
				inputLine.push_back(stoi(tmp2));
			}
			inputListData.push_back(inputLine);
		}
		cout << "----データの読み込み完了、処理を開始します。" << endl;

		//バックグラウンド(q=-1)を除去
		elimBG();

		//輝点が1つのフレームのみ抜き出す
		extractOne();

		cout << "----処理終了" << endl;

		//出力ファイルにoutputListDataを書き込み
		ofstream ofs(outputFilePath);
		int columnOutput = outputListData[0].size();

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

		//次のファイルの解析のために、リストデータに使用したメモリをクリア
		inputListData.clear();
		intermediateListData.clear();
		outputListData.clear();
	}
	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	return 0;
}
