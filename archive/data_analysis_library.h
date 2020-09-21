#pragma once

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

void elimBG();
void extractOne();
string upperPath(string);
bool readFilename(string);


//ImagingC+rejectPileupAnalysis
//バックグラウンド(q=-1)を除去
void elimBG()
{
	for (int i = 0; i < inputListData.size(); i++)
	{
		ui frameNum = inputListData[i][0];
		int x = inputListData[i][1];
		int y = inputListData[i][2];
		ui brightness = inputListData[i][3];
		int q = inputListData[i][4];

		if (q != -1)
		{
			//intermediateListData1行(intermediateLine)を構成
			vector<int> intermediateLine;
			intermediateLine.push_back(frameNum);
			intermediateLine.push_back(x);
			intermediateLine.push_back(y);
			intermediateLine.push_back(brightness);
			intermediateLine.push_back(q);

			//intermediateListDataに追加
			intermediateListData.push_back(intermediateLine);
		}
	}
}

//輝点が1つのもののみをoutputListDataに保存
void extractOne()
{
	int frameNum = intermediateListData[0][0];//今見ているフレームのインデックス
	int k = 0;//今見ている行のインデックス

	//リストデータの最後まで1つずつ見ていく
	while (k < intermediateListData.size())
	{
		vector<vector<int>> tmp;// 1フレーム分の部分配列
		frameNum = intermediateListData[k][0];
		intermediateListData[k].push_back(0);//列数調整のため0を挿入
		tmp.push_back(intermediateListData[k]);

		//frame番号が変わるところまでで部分配列を抜き出す(tmp)
		while ((k + 1 < intermediateListData.size()) && (intermediateListData[k][0] == intermediateListData[k + 1][0]))
		{
			tmp.push_back(intermediateListData[k + 1]);
			k++;
		}

		//輝点の数が1つのフレームについてのみ解析を行う
		int pointNum = tmp.size();
		if (pointNum == 1)
		{
			//outputListDataにpush_back
			outputListData.push_back(tmp[0]);
		}
		k++;
	}
}

string upperPath(string fullPath)
{
	int path_i = fullPath.find_last_of("\\");
	string upperPath = fullPath.substr(0, path_i);//最後の'\'は含まない
	return upperPath;
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