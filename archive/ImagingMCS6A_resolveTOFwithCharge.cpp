// ImagingMCS6A_resolveTOFwithCharge.cpp : TOFリスト(stop全部)のうち、imagingの価数ごとにコインシデンスが取れているものを選びだす（連番処理）
//

//アップデートログ
//2019.8.26:ファイル読み込みをcolumn数に依存しないようにした(inputListBufの廃止)


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

vector<vector<double>> inputListData_img;
vector<vector<double>> inputListData_tof;
vector<vector<int>> outputListData;
vector<int> invalidList;
string inputDataName;

int chargeDist[4][2] = {};

bool readParameter(string);
bool searchEvent(int, int);
string upperPath(string);
bool readFilename(string);

int main()
{
	cout << "***********************************************************************" << endl;
	cout << "****                    -TOF切り出しプログラム-                    ****" << endl;
	cout << "****          イメージングの電荷とコインシデンスしている           ****" << endl;
	cout << "****        TOFのイベントを取り出して結合し、一つのリストにする    ****" << endl;
	cout << "****     ※使えないファイルがある場合は「non_validファイル」を     ****" << endl;
	cout << "**** あらかじめ入力imagingフォルダに入れておく(default:すべて処理) ****" << endl;
	cout << "****                           ver.2019.08.26 written by R. Murase ****" << endl;
	cout << "***********************************************************************" << endl << endl;

	//入力イメージングデータパスの生成
	string inputDataFolder_img;
	cout << "イメージングリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder_img;

	//入力TOFデータパスの生成
	string inputDataFolder_tof;
	cout << "TOFリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder_tof;

	//入力ファイル名
	string filenamePath = upperPath(upperPath(inputDataFolder_tof)) + "\\filename.txt";
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

	//連番処理用の番号を入力
	int start, end;
	cout << "解析するファイル番号(start,end)を入力してください" << endl;
	cout << "start : ";
	cin >> start;
	cout << "end : ";
	cin >> end;

	//入力パラメータパスの生成
	string parameterFilePath = inputDataFolder_img + "\\" + inputDataName + "non_valid.txt";
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

		//入力イメージングデータの読み込み
		string listDataPath_img = inputDataFolder_img + "\\" + inputDataName + suf + ".txt";
		ifstream ifs_img(listDataPath_img);
		if (!ifs_img)
		{
			cerr << "ファイルを開けませんでした" << endl;
			system("pause");
			return -1;
		}

		//入力TOFデータの読み込み
		string listDataPath_tof = inputDataFolder_tof + "\\" + inputDataName + suf + ".txt";
		ifstream ifs_tof(listDataPath_tof);
		if (!ifs_tof)
		{
			cerr << "ファイルを開けませんでした" << endl;
			system("pause");
			return -1;
		}

		//listData_imgの読み込み
		string tmp;
		while (getline(ifs_img, tmp))
		{
			stringstream ss(tmp);
			string tmp2;
			vector<double> inputLine;
			while (ss >> tmp2)
			{
				inputLine.push_back(stoi(tmp2));
			}
			inputListData_img.push_back(inputLine);
		}
		cout << listDataPath_img + "----データの読み込み完了" << endl;

		//listData_tofの読み込み
		while (getline(ifs_tof, tmp))
		{
			stringstream ss(tmp);
			string tmp2;
			vector<double> inputLine;
			while (ss >> tmp2)
			{
				inputLine.push_back(stoi(tmp2));
			}
			inputListData_tof.push_back(inputLine);
		}
		cout << listDataPath_tof + "----データの読み込み完了" << endl;

		//TOFデータの切り出し処理開始
		for (int j = 0; j < inputListData_img.size(); j++)
		{
			//inputListData_imgをはじめから1行ずつ見ていく
			int frameNum = inputListData_img[j][0];

			//イメージングの価数の分布を記録
			int q = inputListData_img[j][4];
			chargeDist[q - 1][1]++;

			//TOFのデータからそのインデックスをもつイベントを探し、あればoutputListDataにpush_backする
			if (searchEvent(m, frameNum))
			{
				chargeDist[q - 1][0]++;
			}
		}
		//次のファイル用にメモリをクリア
		inputListData_img.clear();
		inputListData_tof.clear();
	}

	//出力ファイルにoutputListDataを書き込み
	string outputFileName = inputDataName + "_" + inputDataName + "_chargeDist";
	string outputFilePath = inputDataFolder_tof + "\\" + outputFileName + ".txt";

	//outputListDataの構成
	for (int i = 0; i < 4; i++)
	{
		vector<int> outputLine;
		outputLine.push_back(i + 1);
		outputLine.push_back(chargeDist[i][0]);
		outputLine.push_back(chargeDist[i][1]);
		outputListData.push_back(outputLine);
	}

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

bool searchEvent(int fileNum, int frameNum)
{
	bool exist = false;

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
			exist = true;
			break;
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
	return exist;
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