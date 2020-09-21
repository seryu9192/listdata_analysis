// ImagingMCS6A_countC3StructWithTOF.cpp : C3+ビーム用のコインシデンス解析プログラム
//

//アップデートログ
//2019.9.29:作成
//2019.10.16:SRmin,SRmaxをdouble型に変更
//2020.7.9:VS Codeに移植

#include "./library.hpp"

constexpr auto binWidth = 8;

//二点の重心で制限
int x_c = 236;
int y_c = 872;
int r_c = 5000;

//着目する二次イオンのm/zを設定するベクトル
vector<int> m_zList;
vector<int> roiMin;//TOFスペクトル上のROI
vector<int> roiMax;
vector<double> SRmin;//S2,S3対象座標系の原点からの距離
vector<double> SRmax;
vector<vector<double>> inputListData_img;
vector<vector<int>> inputListData_tof;

vector<vector<double>> outputTableData;
vector<vector<double>> outputListData2;
string inputDataName_tof;

bool readParameter(string);
bool Exists(int, int, double, double);
vector<string> getFileNameInDir(string);
string upperPath(string);
bool readFilename(string);

int main()
{
	cout << "*********************************************************************************" << endl;
	cout << "****                 ImagingMCS6A_countC3StructWithTOF.cpp　                 ****" << endl;
	cout << "****     -TOFピーク内のイベントとコインシデンスしているC3+の構造を数える-    ****" << endl;
	cout << "****      入力データフォルダ：TOF:2_stop(chn表示)、imaging:4_shape           ****" << endl;
	cout << "**** パラメータ：m/z(ヘッダ用),roiMin/roiMax(TOFのROIchn), SRmin/SRmax       ****" << endl;
	cout << "****     ※Parameter file は出力ファイルディレクトリに「ファイル名param.txt」****" << endl;
	cout << "****                                         の名前であらかじめ作成しておく  ****" << endl;
	cout << "****                                   ver.2019.10.16 written by R. Murase   ****" << endl;
	cout << "*********************************************************************************" << endl << endl;

	//入力イメージングデータパスの生成
	string inputDataFolder_img;
	cout << "イメージングリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder_img;

	//入力イメージングデータフォルダ内のファイルの名前を取得
	vector<string> fName = getFileNameInDir(inputDataFolder_img);

	//フォルダにあるファイルのうち、param fileは除外
	for (int i = 0; i < fName.size(); i++)
	{
		if (fName[i].find("param") != string::npos)
		{
			fName.erase(fName.begin() + i);
		}
	}

	//入力TOFデータパスの生成
	string inputDataFolder_tof;
	cout << "TOFリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder_tof;
	string filenamePath = upperPath(upperPath(inputDataFolder_tof)) + "\\filename.txt";
	if (readFilename(filenamePath))
	{
		cout << "filename.txtファイルを読み込みました\n";
		cout << "解析するTOFリストデータのファイル名：" << inputDataName_tof << endl;
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

	//imagingフォルダのファイルについて一つ一つコインシデンスを数える
	for (int m = 0; m < fName.size(); m++)
	{
		//入力イメージングデータの読み込み
		string listDataPath_img = inputDataFolder_img + "\\" + fName[m];
		ifstream ifs_img(listDataPath_img);
		if (!ifs_img)
		{
			cerr << "ファイルを開けませんでした" << endl;
			return -1;
		}

		//imaging_listDataを読み込み
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

		//m/z列を識別する行(header)を構成 {0(SRmin),0(SRmax),0(incident),m_zList,-1(total)}
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

		// 切り出す対象座標系S2,S3の原点からの距離SRの範囲ごとにカウント
		for (int i = 0; i < SRmin.size(); i++)
		{
			//角度の範囲を決定
			double SRmin_now = SRmin[i];
			double SRmax_now = SRmax[i];

			//イベントカウンター
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
					//chnが着目する二次イオン(ROIに含まれる)でありなおかつ、同じファイル番号で、SRがROIに収まるものが存在すれば
					if ((rmin < chn && chn < rmax) && Exists(fileNum, frameNum, SRmin_now, SRmax_now))
					{
						//カウントを+1する
						eventCnt[k]++;
					}
				}
			}
			//入射イベント数(incident)を数える（SRがROIに含まれている＋重心がROI円内に含まれている）
			for (int j = 0; j < inputListData_img.size(); j++)
			{
				int x = inputListData_img[j][2];
				int y = inputListData_img[j][3];
				int r2 = (x - x_c) * (x - x_c) + (y - y_c) * (y - y_c);
				double sR = inputListData_img[j][13];
				if (SRmin_now <= sR && sR < SRmax_now && r2 < r_c*r_c)
				{
					incidentCnt++;
				}
			}

			//outputTableDataの1行を構成
			vector<double> outputLine;
			outputLine.push_back(SRmin_now);//sRのROI最小値
			outputLine.push_back(SRmax_now);//sRのROI最大値
			outputLine.push_back(incidentCnt);//RのROIに入っている入射イベントの数		
			//二次イオン種の数だけpush_back
			for (int j = 0; j < eventCnt.size(); j++)
			{
				outputLine.push_back(eventCnt[j]);
			}
			outputTableData.push_back(outputLine);
			cout <<"R = "<<to_string(SRmin_now) + "-" + to_string(SRmax_now) << " ---解析終了" << endl;
		}

		//出力ファイルにoutputListDataを書き込み
		string outputFileName = inputDataName_tof + "_" + fName[m];
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

		//条件を満たすリストデータを抜き出して保存する(デバッグ用)
		/*
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
		*/
		inputListData_img.clear();
		outputTableData.clear();
	}

	cerr << "m/z_list\n";
	for (int i = 0; i < m_zList.size(); i++)
	{
		cerr << m_zList[i] << " ";
		if (i == m_zList.size() - 1)
		{
			cerr << endl;
		}
	}
	cerr << "x_c=" << x_c << " ";
	cerr << "y_c=" << y_c << " ";
	cerr << "r=" << r_c << " ";

	cerr << "\nでした" << endl;
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
			int temp;
			while (ss >> temp)
			{
				m_zList.push_back(temp);
				//'\t'に出会うまでの文字列を無視
				ss.ignore(line.size(), '\t');
			}
		}
		else if (name == "roiMin")
		{
			int temp;
			while (ss >> temp)
			{
				//リストデータに書かれているchnはヒストグラムで読み取ったchnのbinWidth倍なので
				//その分をかけた値をpush_back
				roiMin.push_back(temp*binWidth);
				//'\t'に出会うまでの文字列を無視
				ss.ignore(line.size(), '\t');
			}
		}
		else if (name == "roiMax")
		{
			int temp;
			while (ss >> temp)
			{
				roiMax.push_back(temp*binWidth);
				//'\t'に出会うまでの文字列を無視
				ss.ignore(line.size(), '\t');
			}
		}
		else if (name == "SRmin")
		{
			double temp;
			while (ss >> temp)
			{
				SRmin.push_back(temp);
				//'\t'に出会うまでの文字列を無視
				ss.ignore(line.size(), '\t');
			}
		}
		else if (name == "SRmax")
		{
			double temp;
			while (ss >> temp)
			{
				SRmax.push_back(temp);
				//'\t'に出会うまでの文字列を無視
				ss.ignore(line.size(), '\t');
			}
		}
		else if (name == "CM")
		{
			int temp;
			vector<int> CM;
			while (ss >> temp)
			{
				CM.push_back(temp);
				//'\t'に出会うまでの文字列を無視
				ss.ignore(line.size(), '\t');
			}
			x_c = CM[0];
			y_c = CM[1];
			r_c = CM[2];
		}
		else
		{
			isValid = false;
		}
	}

	//ROIのminとmaxの数が合わないときはパラメータファイルが不正
	if (roiMin.size() != roiMax.size() || m_zList.size() != roiMin.size() || SRmin.size() != SRmax.size())
	{
		isValid = false;
	}

	return isValid;
}

//imagingデータのうち、fileNum,frameNumをインデックスに持ち、分子軸がthmin以上thmax以下のものが存在するとき、trueを返す
bool Exists(int fileNum, int frameNum, double sRmin, double sRmax)
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
			//2点の重心の位置を評価
			int x = inputListData_img[mid][2];
			int y = inputListData_img[mid][3];
			int r2 = (x - x_c) * (x - x_c) + (y - y_c) * (y - y_c);
			//分子軸の角度を評価
			double SR = inputListData_img[mid][13];

			if (sRmin <= SR && SR < sRmax && r2 < r_c*r_c)//切り出す条件
			{
				exist = true;
				/*
				//着目する二次イオンイベントのリストデータのみ抽出(デバッグ用)
				if (2090*8 < chn && chn < 2115*8)
				{
					outputListData2.push_back(inputListData_img[mid]);
				}
				*/
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
		string temp;
		ss >> temp;

		if (name == "filename")
		{
			isValid = true;
			inputDataName_tof = temp;
		}
	}
	return isValid;
}