// MCS6AListData.cpp : MCS6Aのリストデータを平田さんのプログラムで解析できる形式に変換する
#include "pch.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
constexpr auto LinesPerFile = 2000000;//1ファイルあたりのデータ行数
typedef unsigned int ui;
typedef unsigned long long ull;
using namespace std;

void decodeEvent(string); //デコード用の関数
vector<vector<unsigned int>> DATA;

int main()
{
	cout << "*********************************************" << endl;
	cout << "***** MCS6A　リストデータ変換プログラム *****" << endl;
	cout << "** MCS6Aのリスト形式->平田さんの形式に変換 **" << endl;
	cout << "*********************************************" << endl << endl;

	string listDataFolder, listDataName, listDataPath;
	cout << "解析するリストデータのフォルダを入力してください\n--->";
	cin >> listDataFolder;
	cout << "解析するリストデータのファイル名を入力してください(.lst)\n--->";
	cin >> listDataName;

	listDataPath = listDataFolder + "\\" + listDataName + ".lst";
	ifstream ifs(listDataPath);
	if (!ifs)
	{
		cerr << "ファイルを開けませんでした" << endl;
		return -1;
	}

	string outputFolderName,outputFilePath;
	cout << "\n出力するフォルダのパスを入力してください\n--->";
	cin >> outputFolderName;
	outputFilePath = outputFolderName + "\\" + listDataName;

	bool isData = false;
	ull lineCnt = 0;
	int fileCnt = 1;

	while (!ifs.eof())
	{
		//.lstファイルを1行ずつ読み込み
		string tmp;
		getline(ifs, tmp); 

		//"[DATA]"以降を読み込み
		if (isData)
		{
			decodeEvent(tmp);//1行デコードしてDATA vectorにpushback
			if (lineCnt == LinesPerFile - 1)//行数が上限に達したら書き込む
			{
				lineCnt = -1;
				string outputDataPath = outputFilePath + to_string(fileCnt) + ".txt";
				ofstream ofs(outputDataPath);
				if (!ofs)
				{
					cout << "ファイルを保存できませんでした" << endl;
					return -1;
				}
				//ファイルに1行ずつ書き込み
				for (int i = 0; i < DATA.size(); i++)
				{
					ofs << DATA[i][0] << "\t" << DATA[i][1] << "\t" << DATA[i][2] << "\t" << DATA[i][3] << endl;
				}
				fileCnt++;
				cout << outputDataPath << "に書き込みました" << endl;
				DATA.clear();
			}
			lineCnt++;
		}
		//"[DATA]"以降の文字列(＝データ)に対して変換を行うためのフラグ
		if (tmp == "[DATA]") isData = true;
	}
	cout << "ファイルの読み込み終わり" << endl;

	//cout << "このデータは" << --lineCnt << "行です" << endl;

	//最後に、DATAに残った余りのデータをファイルに書き込み
	string outputDataPath = outputFilePath + to_string(fileCnt) + ".txt";
	ofstream ofs(outputDataPath);
	if (!ofs)
	{
		cout << "ファイルが保存できませんでした" << endl;
		return -1;
	}
	for (int i = 0; i < DATA.size(); i++)
	{
		ofs << DATA[i][0] << "\t" << DATA[i][1] << "\t" << DATA[i][2] << "\t" << DATA[i][3] << endl;
	}
	cout << outputDataPath << "に書き込みました" << endl;
	DATA.clear();

	return 0;
}


//MCS6Aの文字列をデコードして、平田さんが解析できる形式{time, chn(start:1/stop:0),0,0}に変換する関数
void decodeEvent(string tmp)
{
	//リストデータの最後の行が改行されていて（= 何も書いてない行がある:MCS6Aの仕様？）stoullに例外が出るので
	//それを避けるためにif文の中に処理を書いている
	if (tmp != "") //最後の空行以外を処理
	{
		//デコード
		unsigned eventHex = stoull(tmp, nullptr, 16);
		ull eventHex2 = eventHex % (int)pow(2, 48);
		eventHex2 %= (int)pow(2, 24);
		ui time = eventHex2 / (int)pow(2, 4);
		ui chn = eventHex2 % (int)pow(2, 4);
		chn %= (int)pow(2, 3);

		//start/stop信号を書き換える(1->0 / 6->1)
		if (chn == 1) chn = 0;
		if (chn == 6) chn = 1;

		//末尾に0だけの列を2つ追加
		vector<unsigned int> eventVector = { time,chn,0,0 };

		//DATA配列に追加
		DATA.push_back(eventVector);
	}
}