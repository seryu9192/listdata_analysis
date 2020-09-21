// ImagingMCS6A_countMolAxiswithTOF.cpp : あるTOFのピークとコインシデンスする分子軸の角度の収量を数える
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
#include <Windows.h>
#include <stdexcept>
using namespace std;
typedef unsigned int ui;
typedef unsigned long long ull;

constexpr auto columnInput_img = 6;
constexpr auto columnInput_tof = 4;

//分子軸の角度刻み
int dth = 15;

//二点の重心で制限
int x_c = 236;
int y_c = 872;
int R = 500;

//着目する二次イオンのm/zを設定するベクトル
vector<double> m_zList;
vector<double> inputListBuf_img;
vector<vector<double>> inputListData_img;

vector<double> inputListBuf_tof;
vector<vector<double>> inputListData_tof;

vector<vector<double>> outputTableData;
vector<vector<double>> outputListData2;

bool readParameter(string);
bool Exists(int, int, double, double, double);
vector<string> getFileName(string);

int main()
{
	cout << "*****************************************************************************" << endl;
	cout << "****                 ImagingMCS6A_countMolAxiswithTOF.cpp                ****" << endl;
	cout << "****                -TOFピークから分子軸を数えるプログラム-              ****" << endl;
	cout << "****                     TOFのピークから分子軸を逆算する                 ****" << endl;
	cout << "****               イベント番号を持つImagingのイベントを切り出す         ****" << endl;
	cout << "**** ※Parameter file は出力ファイルディレクトリに「ファイル名param.txt」****" << endl;
	cout << "****                                     の名前であらかじめ作成しておく  ****" << endl;
	cout << "****                               ver.2019.06.17 written by R. Murase   ****" << endl;
	cout << "*****************************************************************************" << endl << endl;

	//入力イメージングデータパスの生成
	string listDataFolder_img;
	cout << "イメージングリストデータのフォルダを入力してください\n--->";
	cin >> listDataFolder_img;

	//入力イメージングデータフォルダ内のファイルの名前を取得
	vector<string> fName = getFileName(listDataFolder_img);

	//入力TOFデータパスの生成
	string listDataFolder_tof, listDataName_tof;
	cout << "TOFリストデータのフォルダを入力してください\n--->";
	cin >> listDataFolder_tof;
	cout << "TOFリストデータのファイル名を入力してください(_msAssndより前)\n--->";
	cin >> listDataName_tof;


	//入力TOFデータの読み込み
	string listDataPath_tof = listDataFolder_tof + "\\" + listDataName_tof + "_msAssgnd.txt";

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

	//入力パラメータパスの生成
	string parameterFilePath = outputDataFolder + "\\" + listDataName_tof + "param.txt";
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

	//inputListBuf_tof(1Dvector)にTOFデータを読み込み
	while (!ifs_tof.eof())
	{
		string s;
		ifs_tof >> s;
		double tmp;
		if (s != "")
		{
			tmp = stod(s);
			inputListBuf_tof.push_back(tmp);
		}
	}

	//inputListBuf_tof(1Dvector)を1行ずつinputLineにまとめてinputListData(2Dvector)に変換
	for (int i = 0; i < inputListBuf_tof.size() / columnInput_tof; i++)
	{
		vector<double> inputLine;
		for (int j = 0; j < columnInput_tof; j++)
		{
			int index = columnInput_tof * i + j;
			double tmp = inputListBuf_tof[index];
			inputLine.push_back(tmp);
		}
		inputListData_tof.push_back(inputLine);
	}
	inputListBuf_tof.clear();
	cout << listDataPath_tof + "----データの読み込み完了" << endl;


	//フォルダのファイルについて一つ一つコインシデンスを数える
	for (int i = 0; i < fName.size(); i++)
	{
		//入力イメージングデータの読み込み
		string listDataPath_img = listDataFolder_img + "\\" + fName[i];
		ifstream ifs_img(listDataPath_img);
		if (!ifs_img)
		{
			cerr << "ファイルを開けませんでした" << endl;
			return -1;
		}

		//inputListBuf_img(1Dvector)にイメージングデータを読み込み
		while (!ifs_img.eof())
		{
			string s;
			ifs_img >> s;
			double tmp;
			if (s != "")
			{
				tmp = stod(s);
				inputListBuf_img.push_back(tmp);
			}
		}

		//inputListBuf_img(1Dvector)を1行ずつinputLineにまとめてinputListData(2Dvector)に変換
		for (int i = 0; i < inputListBuf_img.size() / columnInput_img; i++)
		{
			vector<double> inputLine;
			for (int j = 0; j < columnInput_img; j++)
			{
				int index = columnInput_img * i + j;
				double tmp = inputListBuf_img[index];
				inputLine.push_back(tmp);
			}
			inputListData_img.push_back(inputLine);
		}
		cout << listDataPath_img + "----データの読み込み完了" << endl;



		//m/z列を識別する行(header)を構成 {0(thmin),0(thmax),0(incident),m_zList,-1(total)}
		vector<double> m_zHeader;
		for (int i = 0; i < 3; i++)
		{
			m_zHeader.push_back(0);
		}
		for (int i = 0; i < m_zList.size(); i++)
		{
			m_zHeader.push_back(m_zList[i]);
		}
		m_zHeader.push_back(-1);
		outputTableData.push_back(m_zHeader);

		// 切り出す角ごとにカウント
		for (int i = 0; i < 6; i++)
		{
			//角度の範囲を決定
			int thetaMin = dth * i;
			int thetaMax = dth * (i + 1);

			//イベントカウンター
			vector<int> eventCnt(m_zList.size() + 1, 0);
			int incidentCnt = 0;

			//inputListData_tofをはじめから1行ずつ見ていく
			for (int j = 0; j < inputListData_tof.size(); j++)
			{
				int fileNum = inputListData_tof[j][0];
				int frameNum = inputListData_tof[j][1];
				double m_z = inputListData_tof[j][3];

				//着目する二次イオン種について条件を解析
				for (int k = 0; k < m_zList.size() + 1; k++)
				{
					double roimin;
					double roimax;
					if (k < m_zList.size())
					{
						//m_zList.size()の分は着目する二次イオンの収量を数える
						roimin = m_zList[k] - 0.49;
						roimax = m_zList[k] + 0.49;
					}
					else
					{
						//最後にすべての二次イオンの収量を数える(ROIを全範囲に取る)
						roimin = 0;
						roimax = 2320;
					}
					//m_zが着目する二次イオンなら
					if (roimin < m_z && m_z < roimax)
					{
						//同じファイル番号で、分子軸がROIに収まるものが存在すれば、
						if (Exists(fileNum, frameNum, thetaMin, thetaMax, m_z))
						{
							//カウントを+1する
							eventCnt[k]++;
						}
					}

				}
			}

			//入射イベント数(incident)を数える（ROI角度に含まれている＋重心がROI円内に含まれている）
			for (int j = 0; j < inputListData_img.size(); j++)
			{
				int x = inputListData_img[j][2];
				int y = inputListData_img[j][3];
				int r2 = (x - x_c) * (x - x_c) + (y - y_c) * (y - y_c);
				double molAxis = inputListData_img[j][4];
				if (thetaMin < molAxis && molAxis < thetaMax && r2 < R*R)
				{
					incidentCnt++;
				}
			}

			//outputTableDataの1行を構成
			vector<double> outputLine;
			outputLine.push_back(thetaMin);//ROI角度最小値
			outputLine.push_back(thetaMax);//ROI角度最大値
			outputLine.push_back(incidentCnt);//ROI角度に入っている入射イベントの数		
			//二次イオン種の数だけpush_back
			for (int j = 0; j < eventCnt.size(); j++)
			{
				outputLine.push_back(eventCnt[j]);
			}
			outputTableData.push_back(outputLine);
			cout << to_string(thetaMin) + "-" + to_string(thetaMax) << "解析終了" << endl;
		}


		//出力ファイルにoutputListDataを書き込み
		string outputFileName = listDataName_tof + "_" + fName[i];
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
		cout << outputFilePath << "に書き込みました" << endl << endl;

		//条件を満たすリストデータを抜き出して保存する
		
		string outputFileName2 = listDataName_tof + "_" + fName[i] + "_list";
		string outputFilePath2 = outputDataFolder + "\\" + outputFileName2 + ".txt";

		ofstream ofs2(outputFilePath2);

		columnOutput = outputListData2[0].size();
		for (int j = 0; j < outputListData2.size(); j++)
		{
			for (int k = 0; k < columnOutput; k++)
			{
				ofs2 << outputListData2[j][k];
				if (k != columnOutput - 1) ofs2 << "\t";//デリミタ
			}
			ofs2 << endl;
		}
		cout << outputFilePath2 << "に書き込みました" << endl << endl;
		
		inputListBuf_img.clear();
		inputListData_img.clear();
		outputTableData.clear();
		outputListData2.clear();
	}
		


	
	cerr << "m/z_list\n";
	for (int i = 0; i < m_zList.size(); i++)
	{
		cerr << m_zList[i] << " ";
	}
	cerr << "\ndth=" << dth << endl;

	cerr << "x_c=" << x_c << " ";
	cerr << "y_c=" << y_c << " ";
	cerr << "R=" << R << " ";

	cerr << "\nでした" << endl;

	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	system("pause");
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
			int temp;
			while (ss >> temp)
			{
				m_zList.push_back(temp);
				//','に出会うまでの文字列を無視
				ss.ignore(line.size(), ',');
			}
		}
		else if (name == "dth")
		{
			int temp;
			ss >> temp;
			dth = temp;
		}
		else if (name == "CM")
		{
			int temp;
			vector<int> CM;
			while (ss >> temp)
			{
				CM.push_back(temp);
				//','に出会うまでの文字列を無視
				ss.ignore(line.size(), ',');
			}
			x_c = CM[0];
			y_c = CM[1];
			R = CM[2];
		}
		else
		{
			isValid = false;
		}
	}
	return isValid;
}

//imagingデータのうち、fileNum,frameNumをインデックスに持ち、分子軸がthmin以上thmax以下のものが存在するとき、trueを返す
bool Exists(int fileNum, int frameNum,double thmin,double thmax,double m_z)
{
	bool exist = false;

	//binary searchアルゴリズムを使用
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
			int x = inputListData_img[mid][2];
			int y = inputListData_img[mid][3];
			int r2 = (x - x_c) * (x - x_c) + (y - y_c) * (y - y_c);
			double molAxis = inputListData_img[mid][4];

			if (thmin<molAxis && molAxis<thmax && r2 < R*R)
			{
				exist = true;
				//着目する二次イオンイベントのリストデータのみ抽出
				if (163.51<m_z&& m_z<164.49)
				{
					outputListData2.push_back(inputListData_img[mid]);
				}
			}
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
	return exist;
}

vector<string> getFileName(string dir_name)
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