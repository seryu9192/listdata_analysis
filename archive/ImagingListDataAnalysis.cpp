// ImagingListDataAnalysis.cpp:Imagingの輝点リストデータ解析プログラム

//アップデートログ
//2019.8.8:出力ディレクトリ名を@入力時に自動で指定できるようにした
//2019.8.8:filename.txtから自動でファイル名を読み込ませるようにした

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

constexpr auto threshold = 100;
constexpr auto elimX = 5;
constexpr auto elimY = 5;

vector<ui> inputListBuf;
vector<vector<ui>> inputListData;
vector<vector<ui>> outputListData;
string inputDataName;

void solveOverflow();
void sortWithFrame();
void quicksort(int, int);
int med3(int, int, int);
void convPortToXY();
void coordinateOfPoint();
string upperPath(string);
bool readFilename(string);

int main()
{
	cout << "*************************************************" << endl;
	cout << "****** Imaging リストデータ解析プログラム *******" << endl;
	cout << "****** PCIe1473R Imaging輝点ピクセル処理 ********" << endl;
	cout << "** 0:overflowしたデータを直す,1:frameNumでsort **" << endl;
	cout << "** 2:port/pxNumをX/Yに変換, 3:CoordinateOfPoint**" << endl;
	cout << "**         ver.2019.08.08 written by R. Murase **" << endl;
	cout << "*************************************************" << endl << endl;
	
	string inputDataFolder, inputDataPath;
	cout << "解析するリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder;

	string filenamePath = upperPath(upperPath(inputDataFolder)) + "\\filename.txt";
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

	int start,end;
	cout << "解析するファイル番号(start,end)を入力してください" << endl;
	cout << "start : ";
	cin >> start;
	cout << "end : ";
	cin >> end;

	//出力フォルダの指定
	string outputFolderName;
	cout << "\nCOPフォルダのパスを入力してください (\"@\"入力で同じフォルダ内の\"1_COP\"に保存)\n--->";
	cin >> outputFolderName;
	if (outputFolderName == "@")
	{
		outputFolderName = upperPath(inputDataFolder) + "\\1_COP";
	}

	for (int m = start; m <= end; m++)
	{
		//file indexを0詰め3桁の文字列suffixに変換
		ostringstream sout;
		sout << setfill('0') << setw(3) << m;
		string suf = sout.str();

		//入力データパスの生成
		inputDataPath = inputDataFolder + "\\" + inputDataName + suf + ".txt";
		ifstream ifs(inputDataPath);
		if (!ifs)
		{
			cerr << "ファイルを開けませんでした" << endl;
			system("pause");
			return -1;
		}
		//出力データパスの生成
		string outputFilePath = outputFolderName + "\\" + inputDataName + suf + ".txt";
	
		//ifsから1行ずつ読み取ってinputListData(2Dvector)を構成
		string tmp;
		while (getline(ifs, tmp))
		{
			stringstream ss(tmp);
			string tmp2;
			vector<ui> inputLine;
			while (ss >> tmp2)
			{
				inputLine.push_back(stoi(tmp2));
			}
			inputListData.push_back(inputLine);
		}

		cout << inputDataName + suf + ".txt----データの読み込み完了、処理を開始します。" << endl;
	
		solveOverflow();
		inputListData = outputListData;
		outputListData.clear();
		cout << "overflow solved" << endl;

		sortWithFrame();
		inputListData = outputListData;
		outputListData.clear();
		cout << "sorted with frame number" << endl;

		convPortToXY();
		inputListData = outputListData;
		outputListData.clear();
		cout << "converted to XY" << endl;

		coordinateOfPoint();
		cout << "COP" << endl;
	
		cout << "----処理終了" << endl;

		//出力ファイルにoutputListDataを書き込み
		ofstream ofs(outputFilePath);
		for (int i = 0; i < outputListData.size(); i++)
		{
			for (int j = 0; j < outputListData[0].size(); j++)
			{
				ofs << outputListData[i][j];
				if (j != outputListData[0].size() - 1) ofs << "\t";
			}
			ofs << endl;
		}
		cout << outputFilePath << "に書き込みました" << endl << endl;

		//次のファイルの解析のために、リストデータに使用したメモリをクリア
		inputListBuf.clear();
		inputListData.clear();
		outputListData.clear();
	}
	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	return 0;
}

//0.NI PCIe1473Rから得られた画像データのframe番号は16bitまでであり、overflowしたデータはまた0から始まるので、二週目の画像番号に65536をたす
void solveOverflow()
{
	int lastFrame[8] = {};//最後に見たframe番号をport毎に記憶
	int overflowCnt[8] = {};//オーバーフローした回数
	for (int i = 0; i < inputListData.size(); i++)
	{
		int port = inputListData[i][1];
		//今のframe番号が前のframe番号より小さかったら、オーバーフローしているのでoverflowCnt++
		if (inputListData[i][0] < lastFrame[port]) //portをindexにする
		{
			overflowCnt[port]++;
		}
		lastFrame[port] = inputListData[i][0];

		//outputData1行を構成
		vector<ui> outputLine;
		//オーバーフローした回数だけframe番号に2^16=65536だけ足す
		outputLine.push_back(inputListData[i][0] + overflowCnt[port] * (int)pow(2, 16));
		outputLine.push_back(inputListData[i][1]);
		outputLine.push_back(inputListData[i][2]);
		outputLine.push_back(inputListData[i][3]);
		
		//outputListDataに追加
		outputListData.push_back(outputLine);
	}
}

//1.NI PCIe1473Rから得られた画像データのframe番号がバラバラなので昇順にソートする
void sortWithFrame()
{
	/*
	//bubble sortを利用
	bool isEnd = false;
	while (!isEnd)
	{
		bool loopSwap = false;//swapが実行されたかどうかを示すフラグ
		for (int i = 0; i < inputListData.size()-1; i++)
		{
			if (inputListData[i][0] > inputListData[i + 1][0])//前後でframe数が逆転している場合は入れ替える
			{
				swap(inputListData[i], inputListData[i + 1]);
				loopSwap = true;
			}
		}
		if (!loopSwap)//swapが一度も実行されなかった場合はソート終了
		{
			isEnd = true;
		}
	}
	*/

	//quick sortを利用

	quicksort(0, inputListData.size() - 1);

	//ソートされた配列をoutputListData
	outputListData = inputListData;
}

//quick sort
void quicksort(int left, int right)
{
	if (left<right)
	{
		int i = left, j = right;
		int pivot = med3(inputListData[i][0], inputListData[i + (j - i) / 2][0], inputListData[j][0]);
		while (true)
		{
			while (inputListData[i][0] < pivot)
				i++;
			while (inputListData[j][0] > pivot)
				j--;
			if (i >= j)
				break;
			swap(inputListData[i], inputListData[j]);
			i++;
			j--;
		}
		quicksort(left, i - 1);
		quicksort(j + 1, right);
	}
}

//x,y,zの中間値
int med3(int x, int y, int z)
{
	if (x < y)
	{
		if (y < z)
			return y;
		else if (z < x)
			return x;
		else
			return z;
	}
	else
	{
		if (z < y)
			return y;
		else if (x < z)
			return x;
		else
			return z;
	}
}


//2.NI PCIe1473Rから得られた画像データは、port番号とpixel番号で表示されているのでそれをXYに変換する
void convPortToXY()
{
	for (int i = 0; i < inputListData.size(); i++)
	{
		int portNum=inputListData[i][1];
		ui pxNum=inputListData[i][2];
		ui X, Y;
	
		switch (portNum)
		{
		case 0:
			X = (8 * pxNum) % 400;
			Y = (8 * pxNum) / 400;
			break;
		case 1:
			X = (8 * pxNum + 2) % 400;
			Y = (8 * pxNum + 2) / 400;
			break;
		case 2:
			X = (8 * pxNum + 4) % 400;
			Y = (8 * pxNum + 4) / 400;
			break;
		case 3:
			X = (8 * pxNum + 6) % 400;
			Y = (8 * pxNum + 6) / 400;
			break;
		case 4:
			X = (8 * pxNum + 1) % 400;
			Y = (8 * pxNum + 1) / 400;
			break;
		case 5:
			X = (8 * pxNum + 3) % 400;
			Y = (8 * pxNum + 3) / 400;
			break;
		case 6:
			X = (8 * pxNum + 5) % 400;
			Y = (8 * pxNum + 5) / 400;
			break;
		case 7:
			X = (8 * pxNum + 7) % 400;
			Y = (8 * pxNum + 7) / 400;
			break;
		default:
			break;
		}

		//outputData1行を構成
		vector<ui> outputLine;
		outputLine.push_back(inputListData[i][0]);
		outputLine.push_back(X);
		outputLine.push_back(Y);
		outputLine.push_back(inputListData[i][3]);

		//outputListDataに追加
		outputListData.push_back(outputLine);
	}

}

//3.coordinate of point解析（量研機構　千葉さんのLabVIEW VIと同じ機能)
void coordinateOfPoint()
{
	int frameNum = inputListData[0][0];//今見ているフレームのインデックス
	int k = 0;//今見ている行のインデックス

	//リストデータの最後まで1つずつ見ていく
	while (k < inputListData.size())
	{
		vector<vector<ui>> tmp;
		frameNum = inputListData[k][0];
		tmp.push_back(inputListData[k]);

		//frame番号が変わるところまでで部分配列を抜き出す(tmp)
		while ((k + 1 < inputListData.size()) && (inputListData[k][0] == inputListData[k+1][0]) )
		{
			tmp.push_back(inputListData[k + 1]);
			k++;
		}
		//そのフレームの明るさの最大値maxBrightがthreshold未満になるまで繰り返す
		while (true)
		{
		    //そのフレームで最も明るい点の座標と輝度を探す
			int Xm, Ym;
			ui maxBright = 0;
			for (int i = 0; i < tmp.size(); i++)
			{
				if (tmp[i][3]>=maxBright)
				{
					Xm = tmp[i][1];
					Ym = tmp[i][2];
					maxBright = tmp[i][3];
				}
			}
			//そのフレーム最大の輝度がthreshold未満ならばやめて、次のフレームに移る
			if (maxBright < threshold)
			{
				break;
			}
			//輝度がthreshold以上ならばその座標と輝度をoutpuLineに書き込みoutputListDataに追加したあと、
			//そのフレームのうち (X-Xm)<=elimX && (Y-Ym)<=elimYを満たす点の輝度を0にする
			else
			{
				vector<ui> outputLine;
				outputLine.push_back(frameNum);
				outputLine.push_back(Xm);
				outputLine.push_back(Ym);
				outputLine.push_back(maxBright);

				outputListData.push_back(outputLine);

				for (int i = 0; i < tmp.size(); i++)
				{
					if (abs((int)tmp[i][1]-Xm)<=elimX && abs((int)tmp[i][2] - Ym) <= elimY)
					{
						tmp[i][3] = 0;
					}
				}
			}
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