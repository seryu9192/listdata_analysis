// ImagingMCS6A_resolveTOFwithPoints.cpp : TOFリストのうち、imagingとコインシデンスが取れているものを選びだす

//アップデートログ
//


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

constexpr auto columnInput_img = 5;
constexpr auto columnInput_tof = 3;

vector<double> inputListBuf_img;
vector<vector<double>> inputListData_img;
vector<double> inputListBuf_tof;
vector<vector<double>> inputListData_tof;
vector<vector<double>> outputListData;
vector<int> invalidList;

bool readParameter(string);
void searchEvent(int, int);

int main()
{
	cout << "***********************************************************************" << endl;
	cout << "****             -TOF切り出しプログラム(連番処理)-                 ****" << endl;
	cout << "****            イメージングとコインシデンスしている               ****" << endl;
	cout << "****        TOFのイベントを取り出して結合し、一つのリストにする    ****" << endl;
	cout << "****     ※使えないファイルがある場合は「non_validファイル」を     ****" << endl;
	cout << "**** あらかじめ入力imagingフォルダに入れておく(default:すべて処理) ****" << endl;
	cout << "****                           ver.2019.07.19 written by R. Murase ****" << endl;
	cout << "***********************************************************************" << endl << endl;

	//入力イメージングデータパスの生成
	string listDataFolder_img, listDataName;
	cout << "イメージングリストデータのフォルダを入力してください\n--->";
	cin >> listDataFolder_img;
	
	//入力TOFデータパスの生成
	string listDataFolder_tof;
	cout << "TOFリストデータのフォルダを入力してください\n--->";
	cin >> listDataFolder_tof;

	//入力ファイル名
	cout << "イメージングリストデータのファイル名を入力してください\n--->";
	cin >> listDataName;

	//連番処理用の番号を入力
	int start, end;
	cout << "解析するファイル番号(start,end)を入力してください" << endl;
	cout << "start : ";
	cin >> start;
	cout << "end : ";
	cin >> end;

	//入力パラメータパスの生成
	string parameterFilePath = listDataFolder_img + "\\" + listDataName + "non_valid.txt";
	ifstream ifs2(parameterFilePath);
	if (!ifs2)
	{
		cout << "全てのファイルを処理します。" << endl;
	}
	else if (!readParameter(parameterFilePath))//パラメータの読み取り
	{
		cerr << "パラメータファイルが不正です" << endl;
		system("pause");
		return -1;
	}

	//連番処理開始
	for (int m = start; m <= end; m++)
	{
		//データが使えるかどうか判定
		bool valid = true;
		for (int i = 0; i < invalidList.size(); i++)
		{
			if (invalidList[i] == m) valid = false;
		}
		if (!valid)	continue;//使えないデータ（non_validListに入っているデータ）の時は処理しない

		//file indexを0詰め3桁の文字列suffixに変換
		ostringstream sout;
		sout << setfill('0') << setw(3) << m;
		string suf = sout.str();

		//入力イメージングデータパスの読み込み
		string listDataPath_img = listDataFolder_img + "\\" + listDataName + suf + ".txt";
		ifstream ifs_img(listDataPath_img);
		if (!ifs_img)
		{
			cerr << "イメージングファイルを開けませんでした" << endl;
			system("pause");
			return -1;
		}

		//入力TOFデータパスの読み込み
		string listDataPath_tof = listDataFolder_tof + "\\" + listDataName + suf +".txt";
		ifstream ifs_tof(listDataPath_tof);
		if (!ifs_tof)
		{
			cerr << "TOFファイルを開けませんでした" << endl;
			system("pause");
			return -1;
		}

		//Imaging_listDataを読み込み
		string tmp;
		while (getline(ifs_img, tmp))
		{
			stringstream ss(tmp);
			string tmp2;
			vector<double> inputLine;
			while (ss >> tmp2)
			{
				inputLine.push_back(stod(tmp2));
			}
			inputListData_img.push_back(inputLine);
		}
		cout << listDataPath_img + "----データの読み込み完了" << endl;

		//TOF_listDataを読み込み
		while (getline(ifs_tof, tmp))
		{
			stringstream ss(tmp);
			string tmp2;
			vector<double> inputLine;
			while (ss >> tmp2)
			{
				inputLine.push_back(stod(tmp2));
			}
			inputListData_tof.push_back(inputLine);
		}
		cout << listDataPath_tof + "----データの読み込み完了" << endl;

		//TOFデータの切り出し処理開始
		for (int j = 0; j < inputListData_img.size(); j++)
		{
			//inputListData_imgをはじめから1行ずつ見ていく
			int frameNum = inputListData_img[j][0];
			//TOFのデータからそのインデックスをもつイベントを探し、あればoutputListDataにpush_backする
			searchEvent(m, frameNum);
		}

		//次のファイル用にメモリをクリア
		inputListData_img.clear();
		inputListData_tof.clear();
	}

	//出力ファイルにoutputListDataを書き込み
	string outputFileName = listDataName + "_" + listDataName+"_coinc";
	string outputFilePath = listDataFolder_tof + "\\" + outputFileName + ".txt";

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
	cout << outputFilePath << "に書き込みました" << endl << endl;

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
		if (name == "non_valid")
		{
			int temp;
			while (ss >> temp)
			{
				invalidList.push_back(temp);
				//','に出会うまでの文字列を無視
				ss.ignore(line.size(), ',');
			}
		}
		else
		{
			isValid = false;
		}
	}
	return isValid;
}

void searchEvent(int fileNum, int frameNum)
{
	//binary searchアルゴリズムを使用
	int left = 0;
	int right = inputListData_tof.size() - 2;
	int mid, file, sweepNum;

	while (left <= right)
	{
		mid = (left + right) / 2;
		sweepNum = inputListData_tof[mid][0];

		//フレーム番号の大小を評価
		if (sweepNum == frameNum)
		{
			//見つけたときはoutputListDataにpush_back
			vector<double> outputLine;
			outputLine.push_back(fileNum);
			outputLine.push_back(inputListData_tof[mid][0]);
			outputLine.push_back(inputListData_tof[mid][1]);
			outputLine.push_back(inputListData_tof[mid][2]);
			outputListData.push_back(outputLine);
			
			//条件を満たすイベントが複数ある（同じイベントに二次イオンが複数ある）場合のために、はじめに見つけた要素をinputListData_tofから除外
			inputListData_tof.erase(inputListData_tof.begin() + mid);

			//もう一度同じインデックスを持つイベントがないかどうか探す
			searchEvent(fileNum,frameNum);
		}
		else if (sweepNum < frameNum)
		{
			left = mid + 1;
		}
		else
		{
			right = mid - 1;
		}
	}
}