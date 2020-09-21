// ImagingMCS6A_countCoincYield.cpp : あるTOFのピークとコインシデンスするイメージングのイベントを数える(chn表記)
//

//アップデートログ
//2020.1.23:ImagingMCS6A_countMolAxiswithTOF_chn.cppをもとに作成
//2020.1.24:とりあえず電荷状態とコインシデンスしているTOFのイベントを数える機能をつけた
//2020.2.23:filename.txtを2つ上と3つ上のフォルダに探しに行くようにした（これまでは2つ上のみ）

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <Windows.h>
#include <stdexcept>
#include <regex>
using namespace std;
typedef unsigned int ui;
typedef unsigned long long ull;

constexpr auto BIN_WIDTH = 8;

//着目する二次イオンのm/zを設定するベクトル
vector<int> m_zList;
vector<int> roiMin;
vector<int> roiMax;
vector<int> thetaMin;
vector<int> thetaMax;
vector<vector<double>> inputListData_img;
vector<vector<int>> inputListData_tof;

vector<vector<string>> outputTableData;
vector<vector<double>> outputListData2;
string inputDataName_tof;
int clusterSize = 1;

bool readParameter(string);
bool exists(int, int);
vector<string> getFileNameInDir(string);
string upperPath(string);
bool readFilename(string);
bool readClustersize(string);

int main()
{
	cout << "****************************************************************" << endl;
	cout << "****            ImagingMCS6A_countCoincYield.cpp            ****" << endl;
	cout << "****     -TOFピーク内のイベントとコインシデンスしている     ****" << endl;
	cout << "****               イメージングイベントを数える-            ****" << endl;
	cout << "****     ※Parameter file は出力ファイルディレクトリに      ****" << endl;
	cout << "**** 「ファイル名param.txt」の名前であらかじめ作成しておく  ****" << endl;
	cout << "****                  ver.2020.02.23 written by R. Murase   ****" << endl;
	cout << "****************************************************************" << endl << endl;

	//入力イメージングデータパスの生成
	string inputDataFolder_img;
	cout << "イメージングリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder_img;

	//入力イメージングデータフォルダ内のファイルの名前を取得
	vector<string> fNames = getFileNameInDir(inputDataFolder_img);

	//ファイル識別用の正規表現 : "(日付d+)_q=(荷電状態d+).txt"
	regex re_filenames("(\\d+)_q=(\\d+).txt");

	//フォルダにあるファイルのうち、param fileは除外
	for (int i = 0; i < fNames.size(); i++)
	{
		if (fNames[i].find("param") != string::npos)
		{
			fNames.erase(fNames.begin() + i);
		}
	}

	//入力TOFデータパスの生成
	string inputDataFolder_tof;
	cout << "TOFリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder_tof;

	string filenameFolder = upperPath(upperPath(inputDataFolder_tof));
	string filenamePath = filenameFolder + "\\filename.txt";
	if (readFilename(filenamePath)|| readFilename(upperPath(filenameFolder)+ "\\filename.txt"))
	{
		cout << "filename.txtファイルを読み込みました\n";
		cout << "解析するTOFリストデータのファイル名：" << inputDataName_tof << endl;
		if (readClustersize(filenamePath)|| readClustersize(upperPath(filenameFolder)+ "\\filename.txt"))
		{
			cout << "clustersize：" << clusterSize << endl;
		}
		else
		{
			cout << "filename.txtからにclustersizeが読み取れませんでした" << endl;
			cout << "clustersizeを入力してください:";
			cin >> clusterSize;
		}
	}
	else
	{
		cout << "filename.txtファイルが不正です\n";
		cout << "解析するTOFリストデータのファイル名を入力してください\n--->";
		cin >> inputDataName_tof;
	}

	//入力TOFデータの読み込み
	string inputDataPath_tof = inputDataFolder_tof + "\\" + inputDataName_tof + ".txt";

	ifstream ifs_tof(inputDataPath_tof);
	if (!ifs_tof)
	{
		cerr << "ファイルを開けませんでした" << endl;
		system("pause");
		return -1;
	}

	//出力データパスの生成
	string outputDataFolder;
	cout << "出力データのフォルダ名を入力してください\n--->";
	cin >> outputDataFolder;

	//入力パラメータパスの生成
	string parameterFilePath = outputDataFolder + "\\" + inputDataName_tof + "param.txt";
	ifstream ifs2(parameterFilePath);
	if (!ifs2)
	{
		cerr << "パラメータファイルを開けませんでした" << endl;
		system("pause");
		return -1;
	}
	else if (!readParameter(parameterFilePath))//パラメータの読み取り
	{
		cerr << "パラメータファイルが不正です" << endl;
		system("pause");
		return -1;
	}
	else
	{
		cout << "パラメータファイルを読み込みました" << endl;
		cout << "m/z_list = ";
		for (int i = 0; i < m_zList.size(); i++)
		{
			cout << m_zList[i] << " ";
			if (i == m_zList.size() - 1)
			{
				cout << endl;
			}
		}
	}
	
	cout << "TOFリストデータを読み込んでいます..." << endl;

	//TOF_listDataを読み込み
	string tmp;
	while (getline(ifs_tof, tmp))
	{
		stringstream ss(tmp);
		string tmp2;
		vector<int> inputLine;
		while (ss >> tmp2)
		{
			inputLine.push_back(stod(tmp2));
		}
		inputListData_tof.push_back(inputLine);
	}
	cout << inputDataPath_tof + "----データの読み込み完了" << endl;

	//m/z列を識別する行(header)を構成 {0(filename),0(incident),m_zList,-1(total)}
	vector<string> m_zHeader = { "filename","incident" };
	for (int i = 0; i < m_zList.size(); i++)
	{
		m_zHeader.push_back(to_string(m_zList[i]));
	}
	m_zHeader.push_back("total");
	outputTableData.push_back(m_zHeader);

	//imagingフォルダのファイルについて一つ一つコインシデンスを数える
	for (int m = 0; m < fNames.size(); m++)
	{
		//入力イメージングデータの読み込み
		string listDataPath_img = inputDataFolder_img + "\\" + fNames[m];
		ifstream ifs_img(listDataPath_img);
		if (!ifs_img)
		{
			cerr << "ファイルを開けませんでした" << endl;
			return -1;
		}

		//imaging_listDataを読み込み
		cout << listDataPath_img + "----データを読み込んでいます..." << endl;
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
		cout << "読み込み完了" << endl;

		//イベントカウンター(m_zROIの数+1個（+1はすべての二次イオン数）)
		vector<int> eventCnt(m_zList.size() + 1, 0);
		int incidentCnt = 0;

		//inputListData_tofをはじめから1行ずつ見ていく
		for (int j = 0; j < inputListData_tof.size(); j++)
		{
			int fileNum = inputListData_tof[j][0];
			int frameNum = inputListData_tof[j][1];
			int chn = inputListData_tof[j][3];

			//着目する二次イオン種(ROI)について条件を解析
			for (int k = 0; k < m_zList.size() + 1; k++)
			{
				//TOFデータ上でのROIの構成
				int rmin;
				int rmax;
				if (k < roiMin.size())
				{
					//はじめのroiMin.size()回はイオン種毎
					//TOFスペクトル内でのROIを設定
					rmin = roiMin[k];
					rmax = roiMax[k];
				}
				else
				{
					//最後にすべての二次イオンについてコインシデンスしてる収量を数える(ROIを全範囲に取る)
					rmin = 0;
					rmax = 64000;
				}

				//chnが着目する二次イオン(ROIに含まれる)でありなおかつ、同じファイル番号のものが存在すれば
				if ((rmin <= chn && chn <= rmax) && exists(fileNum, frameNum))
				{
					//カウントを+1する
					eventCnt[k]++;
				}
			}
		}
		//入射イベント数(incident)を数える(clustersizeで割る)
		incidentCnt = inputListData_img.size() / clusterSize;

		//今解析しているイメージングデータの荷電状態を正規表現で抽出
		smatch result;
		regex_match(fNames[m], result, re_filenames);

		//outputTableDataの1行を構成
		vector<string> outputLine;
		outputLine.push_back(result[2].str());//荷電状態を書き込み
		outputLine.push_back(to_string(incidentCnt));//ROI角度に入っている入射イベントの数

		//二次イオン種の数だけpush_back
		for (int j = 0; j < eventCnt.size(); j++)
		{
			outputLine.push_back(to_string(eventCnt[j]));
		}
		outputTableData.push_back(outputLine);

		inputListData_img.clear();
	}

	//今解析しているイメージングデータの荷電状態を正規表現で抽出
	smatch result;
	regex_match(fNames[0], result, re_filenames);

	//出力ファイルにoutputListDataを書き込み
	string outputFileName = inputDataName_tof + "_" + result[1].str() + ".txt";
	string outputFilePath = outputDataFolder + "\\" + outputFileName;

	ofstream ofs(outputFilePath);
	int columnOutput = outputTableData[0].size();
	for (int j = 0; j < outputTableData.size(); j++)
	{
		for (int k = 0; k < columnOutput; k++)
		{
			ofs << outputTableData[j][k];
			if (k != columnOutput - 1) ofs << "\t";//デリミタ
		}
		ofs << endl;
	}
	cout << outputFilePath << "に書き込みました" << endl;
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

		if (name == "m/z")
		{
			int tmp;
			while (ss >> tmp)
			{
				m_zList.push_back(tmp);
				//'\t'に出会うまでの文字列を無視
				ss.ignore(line.size(), '\t');
			}
		}
		else if (name == "roiMin")
		{
			int tmp;
			while (ss >> tmp)
			{
				//リストデータに書かれているchnはヒストグラムで読み取ったchnのbinWidth倍なので
				//その分をかけた値をpush_back
				roiMin.push_back(tmp * BIN_WIDTH);
				//'\t'に出会うまでの文字列を無視
				ss.ignore(line.size(), '\t');
			}
		}
		else if (name == "roiMax")
		{
			int tmp;
			while (ss >> tmp)
			{
				roiMax.push_back(tmp * BIN_WIDTH);
				//'\t'に出会うまでの文字列を無視
				ss.ignore(line.size(), '\t');
			}
		}
		else
		{
			isValid = false;
		}
	}

	//ROIのminとmaxの数が合わないときはパラメータファイルが不正
	if (roiMin.size() != roiMax.size() || m_zList.size() != roiMin.size())
	{
		cerr << "パラメータファイルのROIの数がminとmaxで異なります。" << endl;
		isValid = false;
	}

	return isValid;
}

//imagingデータのうち、fileNum,frameNumをインデックスに持つときtrueを返す
bool exists(int fileNum, int frameNum)
{
	bool res = false;

	//二分探索アルゴリズムを使用
	int left = 0;
	int right = inputListData_img.size() - 2;
	int mid, file, frame;

	while (left <= right)
	{
		mid = (left + right) / 2;
		file = inputListData_img[mid][0];
		frame = inputListData_img[mid][1];

		if (file == fileNum && frame == frameNum)
		{
			res = true;
			break;
		}
		//ファイル番号の大小から評価
		else if (file < fileNum)
		{
			left = mid + 1;
		}
		else if (file > fileNum)
		{
			right = mid - 1;
		}
		//ファイル番号が等しい場合はフレーム番号の大小を評価
		else if (frame < frameNum)
		{
			left = mid + 1;
		}
		else
		{
			right = mid - 1;
		}
	}
	return res;
}

vector<string> getFileNameInDir(string dir_name)
{
	HANDLE hFind;
	WIN32_FIND_DATA win32fd;
	vector<string> file_names;

	string extension = "txt";
	string search_name = dir_name + "\\*." + extension;
	hFind = FindFirstFile(search_name.c_str(), &win32fd);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		throw runtime_error("file not found");
	}

	do
	{
		if (win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
		}
		else
		{
			file_names.push_back(win32fd.cFileName);
		}
	} while (FindNextFile(hFind, &win32fd));

	FindClose(hFind);
	return file_names;
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
		string tmp;
		ss >> tmp;

		if (name == "dataname")
		{
			isValid = true;
			inputDataName_tof = tmp;
		}
	}
	return isValid;
}

bool readClustersize(string fnamePath)
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
		int tmp;
		ss >> tmp;

		if (name == "clustersize")
		{
			isValid = true;
			clusterSize = tmp;
		}
	}
	return isValid;
}