//ImagingMCS6A_buildCoincYieldTable_Cntotal.cpp: あるTOFのピークとコインシデンスするprojectile価数を数え、coincYield(chn表記)
//                                              テーブルを作成する。構造は分けず、クラスターサイズ個の輝点があるイベントとコインシデンス

//アップデートログ
//2021.01.21:ImagingMCS6A_buildCoincYieldTable_C1charge.cppをもとに作成

//[clustersize, E0, qcomb, m/z(0:projectile, -1:total), count]のテーブルを出力
// Imaging input folder: Compseparated ← 配向や構造はすべて丸めこむ
// TOF input folder: coinc

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
const int COL_Q = 4;

//TOF inputdata列(COL_FILEはimagingと共通)
const int COL_SWEEP = 1;
const int COL_TOF = 3;

//二点の輝度の差の絶対値で制限
int b_diff_max = 255;

//qを読み取るための正規表現(imaging fileに適用)
regex re_filename("\\d+_q=(\\d+)(.*).txt");

//着目する二次イオンのm/zを設定するベクター
vector<int> m_zList;
vector<int> roiMin;
vector<int> roiMax;
vector<int> thetaMin;
vector<int> thetaMax;
vector<vector<double>> inputListData_img;
vector<vector<int>> inputListData_tof;

vector<vector<int>> outputTableData;
vector<vector<double>> outputListData2;

//INPUT_FOLDER
string input_img_foldername = "\\imaging\\4_Combseparated";
string input_tof_foldername = "\\tof\\4_coinc";

//OUTPUT_DATANAME
string OUTPUT_DATANAME = "_coincYieldTable_tot.txt";

//dataname
string dataname = "yyyymmdd";

//Initial energy E0(keV/atom)
int E0 = 2000;

//Cluster size
int clusterSize = 1;

//param fileを読み込む
bool readParameter(string);

//指定したファイル番号、イベント番号をもつイベントが存在するかどうかを調べる(存在すればtrue)
bool Exists(int, int, int);

int main()
{
	cout << "*******************************************************************************" << endl;
	cout << "****           ImagingMCS6A_buildCoincYieldTable_Cntotal.cpp               ****" << endl;
	cout << "****    ※Parameter file は出力ファイルディレクトリに「ファイル名param.txt」****" << endl;
	cout << "****                                       の名前であらかじめ作成しておく  ****" << endl;
	cout << "****          [n, E, qcomb, m/z(0:projectile, -1:total), count]            ****" << endl;
	cout << "****                                    のテーブル(coincYieldTable)を出力  ****" << endl;
	cout << "****                                 ver.2021.01.21 written by R. Murase   ****" << endl;
	cout << "*******************************************************************************" << endl << endl;

	//入力イメージングデータパスの生成
	string inputDataFolder;
	cout << "データのフォルダを入力してください(ex. E:\\Cluster_sputtering\\MT2020\\MT2020#3\\1200keVC2_pos)\n--->";
	cin >> inputDataFolder;

	//同じフォルダ内の"dataname.txt"を開く
	string datanamePath = inputDataFolder + "\\" + DATANAME_FILE;
	//"dataname.txt"を読み取れたら
	if (!readDataName(datanamePath).empty())
	{
		cout << DATANAME_FILE + "ファイルを読み込みました" << endl;
		dataname = readDataName(datanamePath);
		cout << "解析するTOFリストデータのファイル名：" << dataname << endl;
	}
	else
	{
		cout << "dataname.txtファイルが不正です\n";
		cout << "解析するTOFリストデータのデータ名を入力してください\n--->";
		cin >> dataname;
	}
	//クラスターサイズを読み取り
	clusterSize = readClustersize(datanamePath);
	if(clusterSize == -1)
	{
		cout << "clustersizeを読み取れませんでした" << endl;
		return -1;
	}

	//入力パラメータパスの生成
	string parameterFilePath = inputDataFolder + "\\" + dataname + "param.txt";
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
		cout << "必要なパラメータを全て読み取りました" << endl;
		showSeries<int>("m_zList", m_zList);
		showSeries<int>("roiMin", roiMin);
		showSeries<int>("roiMax", roiMax);
		cout << "E0 = " << E0 << " keV/atom" << endl;
		cout << "clustersize = " << clusterSize << endl;
	}

	//入力パスの生成
	string inputDataFolder_img = inputDataFolder + input_img_foldername;
	string inputDataFolder_tof = inputDataFolder + input_tof_foldername;

	//入力イメージングデータフォルダ内のファイルの名前を取得
	vector<string> fNames = getFileNameInDir(inputDataFolder_img);
	//フォルダにあるファイルのうち、除外するファイルを指定
	for (int i = 0; i < fNames.size(); i++)
	{
		if (fNames[i].find("param") != string::npos)
		{
			fNames.erase(fNames.begin() + i);
		}
		if (fNames[i].find("chargeFraction") != string::npos)
		{
			fNames.erase(fNames.begin() + i);
		}
	}

	//入力TOFデータの読み込み
	string inputDataPath_tof = inputDataFolder_tof + "\\" + dataname + ".txt";
	if (!checkFileExistence(inputDataPath_tof))
	{
		cerr << "ファイルを開けませんでした" << endl;
		system("pause");
		return -1;
	}

	//total secondary ionのラベル(-1)を追加
	m_zList.push_back(-1);

	//TOF_listDataを読み込み
	cout << inputDataPath_tof + "----データを読み込んでいます...";
	inputListData_tof = readListFile<int>(inputDataPath_tof);
	cout << "データの読み込み完了" << endl;

	//出力データパスの生成(inputDataFolder)
	string outputDataFolder = inputDataFolder;

	//imagingフォルダのファイルについて一つ一つコインシデンスを数える
	for (auto inputfilename_img : fNames)
	{
		//入力イメージングデータの読み込み
		string inputDataPath_img = inputDataFolder_img + "\\" + inputfilename_img;
		smatch result;
		if (!checkFileExistence(inputDataPath_img))
		{
			cerr << "ファイルを開けませんでした" << endl;
			return -1;
		}
		else if (!regex_match(inputfilename_img, result, re_filename))
		{
			cerr << inputfilename_img <<"---ファイル名が不正です" << endl;
			continue;
		}
		//ファイル名からqcombを読み込む（正規表現を利用）
		int qcomb = stoi(result[1].str());
		
		//imaging_listDataを読み込み
		cout << inputDataPath_img + "----データを読み込んでいます...";
		inputListData_img = readListFile<double>(inputDataPath_img);
		cout << "データの読み込み完了 --> 処理を開始します" << endl;

		//イベントカウンター(m_zListの要素数)
		vector<int> eventCnt(m_zList.size(), 0);
		int incidentCnt = 0;

		//inputListData_tofをはじめから1行ずつ見ていく
		for (int row = 0; row < inputListData_tof.size(); row++)
		{
			int fileNum = inputListData_tof[row][COL_FILE];
			int frameNum = inputListData_tof[row][COL_FRAME];
			int chn = inputListData_tof[row][COL_TOF];

			//着目する二次イオン種(ROI)について条件を解析(イオン種の数+1しているのはtotalを求めるため)
			for (int siNum = 0; siNum < roiMin.size() + 1; siNum++)
			{
				//TOFデータ上でのROIの構成
				int rmin;
				int rmax;
				if (siNum < roiMin.size())
				{
					//はじめのroiMin.size()回はイオン種毎
					//TOFスペクトル内でのROIを設定
					rmin = roiMin[siNum];
					rmax = roiMax[siNum];
				}
				else
				{
					//最後にすべての二次イオンについてコインシデンスしてる収量を数える(ROIを全範囲に取る)
					rmin = 0;
					rmax = 64000;
				}

				//chnが着目する二次イオン(ROIに含まれる)でありなおかつ、同じファイル番号なるものが存在すれば
				if ((rmin <= chn && chn < rmax) && Exists(fileNum, frameNum, chn))
				{
					//カウントを+1する
					eventCnt[siNum]++;
				}
			}
		}
		//入射イベント数(incident):リストデータの行数 = 輝点の点の数 = イベント数 * clusterSizeなので
		// incident数を求めるためにclusterSizeで割る
		incidentCnt = inputListData_img.size()/clusterSize;

		//projectile
		vector<int> outputLine = {clusterSize, E0, qcomb, 0 ,incidentCnt};
		outputTableData.push_back(outputLine);
		
		//secondary ion
		for (int siNum = 0; siNum < eventCnt.size(); siNum++)
		{
			//siごとにoutputLineを構成し、TableDataにpush_back
			vector<int> outputLine = {clusterSize, E0, qcomb, m_zList[siNum] ,eventCnt[siNum]};
			outputTableData.push_back(outputLine);
		}

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
	}
	//出力ファイルにoutputListDataを書き込み
	string outputFileName = dataname + OUTPUT_DATANAME;
	string outputFilePath = outputDataFolder + "\\" + outputFileName;
	writeListFile<int> (outputFilePath, outputTableData);
	cout << outputFilePath << "に書き込みました" << endl << endl;
	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	return 0;
}

bool readParameter(string pfilePath)
{
	//読み取るべきパラメータのリスト
	vector<string> params = {"m/z", "roiMin", "roiMax", "E0"};
	
	//各パラメータをファイルから読み取ったかどうか
	map<string, bool> read;

	//パラメータが有効かどうかのフラグ
	bool isValid = false;

	//パラメータファイルを開く
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
			read["m/z"] = true;
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
			read["roiMin"] = true;
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
			read["roiMax"] = true;
		}
		else if (name == "E0")
		{
			int tmp;
			ss >> tmp;
			E0 = tmp;
			read["E0"] = true;
		}
	}

	//全てのパラメータが読み込まれたかチェック
	auto read_all_parameters = [&]{
		bool res = true;
		for(auto p : params)res &= read[p];
		return res;
	}();

	//全て読み込まれていたらOK
	if (read_all_parameters)
	{
		isValid = true;
	}
	else
	{
		cout << "以下のパラメータが読み込まれませんでした" << endl;
		for(auto p : params)
		{
			if(!read[p])
			{
				cout << p << endl;
			}
		}
		cout << "パラメータファイルを確認してください" << endl;
	}	
	//ただしROIのminとmaxの数が合わないときはパラメータファイルが不正
	if (roiMin.size() != roiMax.size() || m_zList.size() != roiMin.size() || thetaMin.size() != thetaMax.size())
	{
		isValid = false;
		cout << "minとmaxのサイズが等しくないパラメータがあります" << endl;
		cout << "パラメータファイルを確認してください" << endl;
	}
	return isValid;
}

//imagingデータのうち、fileNum,frameNumをインデックスに持ち、分子軸がthmin以上thmax以下のものが存在するとき、trueを返す
bool Exists(int fileNum, int frameNum, int chn)
{
	//探しているイベントが存在するかどうか？
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
			exist = true;
			/*
			//着目する二次イオンイベントのリストデータのみ抽出(デバッグ用)
			if (2090*8 < chn && chn < 2115*8)
			{
				outputListData2.push_back(inputListData_img[mid]);
			}
			*/
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