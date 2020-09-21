// ImagingMCS6A_C2+countMolAxiswithTOF.cpp : あるTOFのピークとコインシデンスする分子軸の角度の収量を数える(chn表記)
//

//アップデートログ
//2019.8.8:filename.txtから自動でファイル名を読み込ませるようにした
//2020.6.18:"filename.txt"の名前を"dataname.txtにした + ファイルの中身もdataname = の形にした
//2020.6.18:"library.hpp"から読み込むようにする+ファイル入出力の関数もここから読み込む
//2020.6.18:VS 2019に対応するプロジェクトファイルを作り直した
//2020.7.9:VS Codeに移植
//2020.7.29:入力データの列番号をマジックナンバーでなく、"COL_***"変数で表示するようにする
//2020.7.29:b_diffで入力データにフィルターをかける機能をいったん廃止(C3+, C4+の分子軸を計算するため)
//ImagingMCS6A_C2+countMolAxiswithTOF.cpp -> ImagingMCS6A_countMolAxiswithTOF.cpp に改名
//2020.8.15:入力パスをimagingとtof両方入力せずに、rootパスだけですべての処理を行えるようにする

#include "./library.hpp"

//TOFのbin width
constexpr auto binWidth = 8;

//二点の重心で制限
int x_c = 236;
int y_c = 872;
int R = 5000;

//imaging inputdata列
const int COL_FILE = 0;
const int COL_FRAME = 1;
const int COL_X = 2;
const int COL_Y = 3;
const int COL_MOLAXIS = 4;

//TOF inputdata列(COL_FILEはimagingと共通)
const int COL_SWEEP = 1;
const int COL_TOF = 3;

//二点の輝度の差の絶対値で制限
int b_diff_max = 255;


//FOLDER_NAMES
const string IMG_FOLDERNAME = "\\imaging\\6_molAxis";
const string TOF_FOLDERNAME = "\\tof\\4_coinc";
const string COINC_FOLDERNAME = "\\coincYield";

//着目する二次イオンのm/zを設定するベクトル
vector<int> m_zList;
vector<int> roiMin;
vector<int> roiMax;
vector<int> thetaMin;
vector<int> thetaMax;
vector<vector<double>> inputListData_img;
vector<vector<int>> inputListData_tof;

vector<vector<int>> outputTableData;
vector<vector<double>> outputListData2;
string inputDataName_tof;

//param fileを読み込む
bool readParameter(string);

//指定したファイル番号、イベント番号、ある分子軸角度をもつイベントが存在するかどうかを調べる(存在すればtrue)
bool Exists(int, int, double, double, int, int);

int main()
{
	cout << "*********************************************************************************" << endl;
	cout << "****               ImagingMCS6A_C2+countMolAxiswithTOF_chn.cpp               ****" << endl;
	cout << "****     -TOFピーク内のイベントとコインシデンスしている分子軸を数える-       ****" << endl;
	cout << "****            入力データ：TOF_stop(chn表示)、imaging_molAxis               ****" << endl;
	cout << "****            TOF,imaging共通: 0列目にfileNum,1列目にeventNum              ****" << endl;
	cout << "****               TOFchannel:3列目、imaging_molAxis:4列目                   ****" << endl;
	cout << "****   imaging_center of mass:2(x), 3(y)列目, b_diff:6列目にあるとして評価   ****" << endl;
	cout << "**** パラメータ：m/z(ヘッダ用),roiMin/roiMax(TOFのROIchn), thetaMin/thetaMax ****" << endl;
	cout << "****      ※Parameter file は出力ファイルディレクトリに「ファイル名param.txt」****" << endl;
	cout << "****                                         の名前であらかじめ作成しておく  ****" << endl;
	cout << "****                                   ver.2020.08.15 written by R. Murase   ****" << endl;
	cout << "*********************************************************************************" << endl << endl;

	//入力イメージングデータパスの読み取り
	string inputDataFolder;
	cout << "データのrootフォルダのパス(ex.E:\\Cluster_sputtering\\MT2020\\MT2020#3\\1800keVC2_pos)を入力してください\n--->";
	cin >> inputDataFolder;

	//入力Imaging,TOFデータパスの構成
	string inputDataFolder_img = inputDataFolder + IMG_FOLDERNAME;
	string inputDataFolder_tof = inputDataFolder + TOF_FOLDERNAME;
	string outputDataFolder = inputDataFolder + COINC_FOLDERNAME;

	//入力イメージングデータフォルダ内のファイルの名前を取得
	vector<string> fNames = getFileNameInDir(inputDataFolder_img);

	//フォルダにあるファイルのうち、param fileは除外
	for (int i = 0; i < fNames.size(); i++)
	{
		if (fNames[i].find("param") != string::npos)
		{
			fNames.erase(fNames.begin() + i);
		}
	}

	//上の階層に"dataname.txt"を探しに行く
	string datanamePath = searchFileFromUpperFolder(inputDataFolder_tof, DATANAME_FILE);

	//"dataname.txt"を読み取れたら
	if (!readDataName(datanamePath).empty())
	{
		cout << DATANAME_FILE + "ファイルを読み込みました" << endl;
		inputDataName_tof = readDataName(datanamePath);
		cout << "解析するTOFリストデータのファイル名：" << inputDataName_tof << endl;
	}
	else
	{
		cout << "filename.txtファイルが不正です\n";
		cout << "解析するTOFリストデータのデータ名を入力してください\n--->";
		cin >> inputDataName_tof;
	}

	//入力TOFデータの読み込み
	string inputDataPath_tof = inputDataFolder_tof + "\\" + inputDataName_tof + ".txt";
	if (!checkFileExistence(inputDataPath_tof))
	{
		cerr << "ファイルを開けませんでした" << endl;
		system("pause");
		return -1;
	}

	//入力パラメータパスの生成
	string parameterFilePath = outputDataFolder + "\\" + inputDataName_tof + "param.txt";
	if (!checkFileExistence(parameterFilePath))
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
		cout << "パラメータファイルを読み取りました" << endl;
		showSeries<int>("m_zList", m_zList);
		showSeries<int>("roiMin", roiMin);
		showSeries<int>("roiMax", roiMax);
		showSeries<int>("thetaMin", thetaMin);
		showSeries<int>("thetaMax", thetaMax);
	}
	//TOF_listDataを読み込み
	cout << inputDataPath_tof + "----データを読み込んでいます...";
	inputListData_tof = readListFile<int>(inputDataPath_tof);
	cout <<"データの読み込み完了" << endl;

	//imagingフォルダのファイルについて一つ一つコインシデンスを数える
	for (auto inputfilename : fNames)
	{
		//入力イメージングデータの読み込み
		string listDataPath_img = inputDataFolder_img + "\\" + inputfilename;
		if (!checkFileExistence(listDataPath_img))
		{
			cerr << "ファイルを開けませんでした" << endl;
			return -1;
		}

		//imaging_listDataを読み込み
		cout << listDataPath_img + "----データを読み込んでいます...";
		inputListData_img = readListFile<double>(listDataPath_img);
		cout << "データの読み込み完了 --> 処理を開始します" << endl;

		//m/z列を識別する行(header)を構成 {0(thmin),0(thmax),0(incident),m_zList,-1(total)}
		vector<int> m_zHeader;
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

		// 切り出す角度ごとにカウント
		for (int i = 0; i < thetaMin.size(); i++)
		{
			//角度の範囲を決定
			int tmin = thetaMin[i];
			int tmax = thetaMax[i];

			//イベントカウンター(m_zListの要素数+1個（+1はすべての二次イオン数）)
			vector<int> eventCnt(m_zList.size() + 1, 0);
			int incidentCnt = 0;

			//inputListData_tofをはじめから1行ずつ見ていく
			for (int j = 0; j < inputListData_tof.size(); j++)
			{
				int fileNum = inputListData_tof[j][COL_FILE];
				int frameNum = inputListData_tof[j][COL_FRAME];
				int chn = inputListData_tof[j][COL_TOF];

				//着目する二次イオン種(ROI)について条件を解析(イオン種の数+1しているのはtotalを求めるため)
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

					//chnが着目する二次イオン(ROIに含まれる)でありなおかつ、同じファイル番号で、分子軸がROIに収まるものが存在すれば
					if ((rmin <= chn && chn < rmax) && Exists(fileNum, frameNum, tmin, tmax, chn, b_diff_max))
					{
						//カウントを+1する
						eventCnt[k]++;
					}
				}
			}
			//入射イベント数(incident)を数える（ROI角度に含まれている＋重心がROI円内に含まれている）
			for (int j = 0; j < inputListData_img.size(); j++)
			{
				int x = inputListData_img[j][COL_X];
				int y = inputListData_img[j][COL_Y];
				int r2 = (x - x_c) * (x - x_c) + (y - y_c) * (y - y_c);
				double molAxis = inputListData_img[j][COL_MOLAXIS];
				if (tmin <= molAxis && molAxis < tmax && r2 < R * R)
				{
					incidentCnt++;
				}
			}

			//outputTableDataの1行を構成
			vector<int> outputLine;
			outputLine.push_back(tmin);//ROI角度最小値
			outputLine.push_back(tmax);//ROI角度最大値
			outputLine.push_back(incidentCnt);//ROI角度に入っている入射イベントの数		
			//二次イオン種の数だけeventCntをpush_back
			for (int j = 0; j < eventCnt.size(); j++)
			{
				outputLine.push_back(eventCnt[j]);
			}
			//TableDataにpush_back
			outputTableData.push_back(outputLine);
			cout << to_string(tmin) + "-" + to_string(tmax) << " deg---解析終了" << endl;
		}

		//出力ファイルにoutputListDataを書き込み
		string outputFileName = inputDataName_tof + "_" + inputfilename;
		string outputFilePath = outputDataFolder + "\\" + outputFileName;
		writeListFile<int> (outputFilePath, outputTableData);
		cout << outputFilePath << "に書き込みました" << endl << endl;

		//条件を満たすリストデータを抜き出して保存する(デバッグ用)
		/*
		string outputFileName2 = listDataName_tof + "_" + fName[i] + "_list";
		string outputFilePath2 = outputDataFolder + "\\" + outputFileName2 + ".txt";

		ofstream ofs(outputFilePath2);
		columnOutput = outputListData2[0].size();
		for (int j = 0; j < outputListData2.size(); j++)
		{
			for (int k = 0; k < columnOutput; k++)
			{
				ofs << outputListData2[j][k];
				if (k != columnOutput - 1) ofs << "\t";//デリミタ
			}
			ofs << endl;
		}
		cout << outputFilePath2 << "に書き込みました" << endl;
		*/

		inputListData_img.clear();
		outputTableData.clear();
	}

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
				roiMin.push_back(tmp * binWidth);
				//'\t'に出会うまでの文字列を無視
				ss.ignore(line.size(), '\t');
			}
		}
		else if (name == "roiMax")
		{
			int tmp;
			while (ss >> tmp)
			{
				roiMax.push_back(tmp * binWidth);
				//'\t'に出会うまでの文字列を無視
				ss.ignore(line.size(), '\t');
			}
		}
		else if (name == "thetaMin")
		{
			int tmp;
			while (ss >> tmp)
			{
				thetaMin.push_back(tmp);
				//'\t'に出会うまでの文字列を無視
				ss.ignore(line.size(), '\t');
			}
		}
		else if (name == "thetaMax")
		{
			int tmp;
			while (ss >> tmp)
			{
				thetaMax.push_back(tmp);
				//'\t'に出会うまでの文字列を無視
				ss.ignore(line.size(), '\t');
			}
		}
		else if (name == "CM")
		{
			int tmp;
			vector<int> CM;
			while (ss >> tmp)
			{
				CM.push_back(tmp);
				//'\t'に出会うまでの文字列を無視
				ss.ignore(line.size(), '\t');
			}
			x_c = CM[0];
			y_c = CM[1];
			R = CM[2];
		}
		else if (name == "b_diff_max")
		{
			int tmp;
			ss >> tmp;
			b_diff_max = tmp;
		}
		else
		{
			isValid = false;
		}
	}

	//ROIのminとmaxの数が合わないときはパラメータファイルが不正
	if (roiMin.size() != roiMax.size() || m_zList.size() != roiMin.size() || thetaMin.size() != thetaMax.size())
	{
		isValid = false;
	}

	return isValid;
}

//imagingデータのうち、fileNum,frameNumをインデックスに持ち、分子軸がthmin以上thmax以下のものが存在するとき、trueを返す
bool Exists(int fileNum, int frameNum, double thmin, double thmax, int chn, int b_diff_max)
{
	bool exist = false;

	//binary searchアルゴリズムを使用
	int left = 0;
	int right = inputListData_img.size() - 2;
	int mid, file, frame;

	while (left <= right)
	{
		mid = (left + right) / 2;
		file = inputListData_img[mid][COL_FILE];
		frame = inputListData_img[mid][COL_FRAME];

		if (file == fileNum && frame == frameNum)
		{
			//2点の重心の位置を評価
			int x = inputListData_img[mid][COL_X];
			int y = inputListData_img[mid][COL_Y];
			int r2 = (x - x_c) * (x - x_c) + (y - y_c) * (y - y_c);
			//分子軸の角度を評価
			double molAxis = inputListData_img[mid][COL_MOLAXIS];

			if (thmin <= molAxis && molAxis < thmax && r2 < R * R)
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