// ImagingMCS6A_C3+countStructurewithTOF.cpp : あるTOFのピークとコインシデンスするC3+の立体構造を数える
//

//アップデートログ
//2020.7.29:ImagingMCS6A_C2+countMolAxiswithTOF.cppからコピー作成(molAxisの代わりにC3+の立体構造をSRで分ける)

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
const int COL_SR = 13;

//TOF inputdata列(COL_FILEはimagingと共通)
const int COL_SWEEP = 1;
const int COL_TOF = 3;

//二点の輝度の差の絶対値で制限
int b_diff_max = 255;

//着目する二次イオンのm/zを設定するベクトル
vector<int> m_zList;
vector<int> roiMin;
vector<int> roiMax;
vector<double> srMin;
vector<double> srMax;
vector<vector<double>> inputListData_img;
vector<vector<int>> inputListData_tof;

vector<vector<double>> outputTableData;
vector<vector<double>> outputListData2;
string inputDataName_tof;

//param fileを読み込む
bool readParameter(string);

//指定したファイル番号、イベント番号、ある分子軸角度をもつイベントが存在するかどうかを調べる(存在すればtrue)
bool Exists(int, int, double, double, int, int);

int main()
{
	cout << "*********************************************************************************" << endl;
	cout << "****                ImagingMCS6A_C3+countStructurewithTOF.cpp                ****" << endl;
	cout << "****     -TOFピーク内のイベントとコインシデンスしている分子軸を数える-       ****" << endl;
	cout << "****            入力データ：TOF_stop(chn表示)、imaging_molAxis               ****" << endl;
	cout << "****            TOF,imaging共通: 0列目にfileNum,1列目にeventNum              ****" << endl;
	cout << "****               TOFchannel:3列目、imaging_molAxis:4列目                   ****" << endl;
	cout << "****   imaging_center of mass:2(x), 3(y)列目, b_diff:6列目にあるとして評価   ****" << endl;
	cout << "****   パラメータ：m/z(ヘッダ用),roiMin/roiMax(TOFのROIchn), srMin/srMax     ****" << endl;
	cout << "****      ※Parameter file は出力ファイルディレクトリに「ファイル名param.txt」****" << endl;
	cout << "****                                         の名前であらかじめ作成しておく  ****" << endl;
	cout << "****                                   ver.2020.07.29 written by R. Murase   ****" << endl;
	cout << "*********************************************************************************" << endl << endl;

	//入力イメージングデータパスの生成
	string inputDataFolder_img;
	cout << "イメージングリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder_img;

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

	//入力TOFデータパスの生成
	string inputDataFolder_tof;
	cout << "TOFリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder_tof;

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

	//出力データパスの生成
	string outputDataFolder;
	cout << "出力データのフォルダ名を入力してください\n--->";
	cin >> outputDataFolder;

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
		showSeries<double>("srMin", srMin);
		showSeries<double>("srMax", srMax);
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
		cout << "読み込み完了 --> 処理を開始します..." << endl;

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

		// 切り出すsrごとにカウント
		for (int i = 0; i < srMin.size(); i++)
		{
			//角度の範囲を決定
			double srmin = srMin[i];
			double srmax = srMax[i];

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

					//chnが着目する二次イオン(ROIに含まれる)でありなおかつ、同じファイル番号で、がROIに収まるものが存在すれば
					if ((rmin <= chn && chn < rmax) && Exists(fileNum, frameNum, srmin, srmax, chn, b_diff_max))
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
				double sr = inputListData_img[j][COL_SR];
				if (srmin <= sr && sr < srmax && r2 < R * R)
				{
					incidentCnt++;
				}
			}

			//outputTableDataの1行を構成
			vector<double> outputLine;
			outputLine.push_back(srmin);//ROI角度最小値
			outputLine.push_back(srmax);//ROI角度最大値
			outputLine.push_back(incidentCnt);//ROI角度に入っている入射イベントの数		
			//二次イオン種の数だけeventCntをpush_back
			for (int j = 0; j < eventCnt.size(); j++)
			{
				outputLine.push_back(eventCnt[j]);
			}
			//TableDataにpush_back
			outputTableData.push_back(outputLine);
			cout << "SR = " <<to_string(srmin) + "-" + to_string(srmax) << "---解析終了" << endl;
		}

		//出力ファイルにoutputListDataを書き込み
		string outputFileName = inputDataName_tof + "_" + inputfilename;
		string outputFilePath = outputDataFolder + "\\" + outputFileName;
		writeListFile<double> (outputFilePath, outputTableData);
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
		else if (name == "srMin")
		{
			double tmp;
			while (ss >> tmp)
			{
				srMin.push_back(tmp);
				//'\t'に出会うまでの文字列を無視
				ss.ignore(line.size(), '\t');
			}
		}
		else if (name == "srMax")
		{
			double tmp;
			while (ss >> tmp)
			{
				srMax.push_back(tmp);
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
	if (roiMin.size() != roiMax.size() || m_zList.size() != roiMin.size() || srMin.size() != srMax.size())
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
			//SRを評価
			double sr = inputListData_img[mid][COL_SR];

			if (thmin <= sr && sr < thmax && r2 < R * R)
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