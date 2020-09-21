// ImagingMCS6A_resolveTOFwithChargePair.cpp : Imagingにおいて、ある電荷ペアとコインシデンスするevent番号を持つTOFのイベントを抽出する
//

//アップデートログ
//2019.8.23 作成 
//2019.8.27:tofのchannelが大きすぎる（MCSのバグ？）イベントを除外するためにTOF_MAXを導入
//2020.2.23:coincidenceしたリストデータを入れるフォルダ"coinc"を自動で生成するようにした
//2020.2.23:filename.txtを2つ上と3つ上のフォルダに探しに行くようにした（今までは2つ上のみ）
//2020.3.10:library.hppに関数をまとめる
//2020.7.9:VS Codeに移植

#include "./library.hpp"

//連番ファイルの数
int fileNum_tof_MAX = 1;

//データを扱う二次元配列をファイル番号ごとに分けた三次元配列(ListData[fileNum][eventNum][column])
vector<vector<vector<double>>> inputListData_img;
vector<vector<vector<int>>> inputListData_tof;
vector<vector<vector<int>>> outputListData;

string inputDataName_tof;

void searchEvent(int, int, int);//(fileNum,sweepNum,lineNum)

int main()
{
	cout << "************************************************************" << endl;
	cout << "****       ImagingMCS6A_resolveTOFwithChargePair.cpp    ****" << endl;
	cout << "****                -TOF切り出しプログラム-             ****" << endl;
	cout << "****        イメージングとコインシデンスしている        ****" << endl;
	cout << "****     イベント番号を持つTOFのイベントを切り出す      ****" << endl;
	cout << "****              ver.2020.03.10 written by R. Murase   ****" << endl;
	cout << "************************************************************" << endl << endl;

	//入力imagingデータパスの入力
	string inputDataFolder_img;
	cout << "イメージングリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder_img;

	//入力imagingフォルダ内のファイルの名前を取得
	vector<string> fNames = getFileNameInDir(inputDataFolder_img, DATA_EXTENSION);

	//imagingフォルダにあるファイルのうち、param fileは除外
	for (int i = 0; i < fNames.size(); i++)
	{
		if (fNames[i].find("param") != string::npos)
		{
			fNames.erase(fNames.begin() + i);
		}
	}

	//入力TOFデータパスの入力
	string inputDataFolder_tof;
	cout << "TOFリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder_tof;

	//inputDataFolder_tofから最大 UPPERFOLDER_MAX 個上のフォルダまで"dataname.txt"を探しに行く
	string dataNamePath = searchFileFromUpperFolder(inputDataFolder_tof, DATANAME_FILE);

	//"dataname.txt"が見つかれば（dataNamePathが空文字列でなければ）そのファイルを読む
	if (dataNamePath != "")
	{
		inputDataName_tof = readDataName(dataNamePath);
		cout << "dataname.txtファイルを読み込みました\n";
		cout << "解析するTOFリストデータのファイル名：" << inputDataName_tof << endl;
	}
	else
	{
		cout << "dataname.txtファイルが不正です\n";
		cout << "解析するTOFリストデータのファイル名を入力してください\n--->";
		cin >> inputDataName_tof;
	}

	//入力TOFデータ数の読み込み
	fileNum_tof_MAX = lastFileNum(inputDataFolder_tof);
	cout << "データ数:" << fileNum_tof_MAX << endl;

	//inputListData_tofのvectorのサイズを連番数だけ確保
	inputListData_tof.resize(fileNum_tof_MAX + 1);
	
	//出力データパスの生成(TOFのinputFolderに保存)
	string outputDataFolder, outputFileName;
	cout << "出力フォルダのパスを入力してください (\"@\"入力で同じフォルダ内の\"coinc\"フォルダに保存（存在しなければ新たに生成）)\n--->";
	cin >> outputDataFolder;
	if (outputDataFolder == "@")
	{
		outputDataFolder = inputDataFolder_tof + "\\coinc";
		_mkdir(outputDataFolder.c_str());
	}

	//入力tofファイルをHDDから読み込み
	for (int fileNum_tof = 1; fileNum_tof <= fileNum_tof_MAX; fileNum_tof++)
	{	
		//入力tofファイルパス作成
		string inputDataPath_tof = inputDataFolder_tof + "\\" + inputDataName_tof + toSuffix(fileNum_tof) + ".txt";
		ifstream ifs_tof(inputDataPath_tof);
		if (!ifs_tof)
		{
			cerr << inputDataName_tof + toSuffix(fileNum_tof) <<"---ファイルが開けませんでした" << endl;
			system("pause");
			return -1;
		}
		//ファイル読み込み
		inputListData_tof[fileNum_tof] = readListFile(inputDataPath_tof);
		cout << inputDataName_tof + toSuffix(fileNum_tof) << "---読み込み完了" << endl;
	}
	cout << "全てのTOFデータ読み込み完了。処理を開始します。" << endl;
	
	//imagingデータのファイル数だけループ
	for (int fileNum_img = 0; fileNum_img < fNames.size(); fileNum_img++)
	{
		//outputListDataのvectorのサイズを連番数だけ確保
		outputListData.resize(fileNum_tof_MAX + 1);

		string inputDataName_img = fNames[fileNum_img];

		//入力イメージングデータの読み込み
		string inputDataPath_img = inputDataFolder_img + "\\" + inputDataName_img;
		ifstream ifs_img(inputDataPath_img);
		if (!ifs_img)
		{
			cerr << "ファイルを開けませんでした" << endl;
			system("pause");
			return -1;
		}
		inputListData_img = readListFile_sep<double>(inputDataPath_img, fileNum_tof_MAX);
		cout << inputDataPath_img + "----データの読み込み完了" << endl;

		//TOFデータの切り出し処理開始(連番処理)
		for (int fileNum_tof = 1; fileNum_tof <= fileNum_tof_MAX; fileNum_tof++)
		{
			for (int j = 0; j < inputListData_tof[fileNum_tof].size(); j++)
			{
				//inputListData_tofをはじめから1行ずつ見ていく
				int sweepNum = inputListData_tof[fileNum_tof][j][0];

				//Imagingのデータからそのインデックスをもつイベントを探し、
				//あればinputListData_tofのeventをoutputListDataにpush_backする
				searchEvent(fileNum_tof, sweepNum, j);
			}
		}

		//出力ファイルにoutputListDataを書き込み
		outputFileName = inputDataName_tof + "_" + inputDataName_img;
		string outputFilePath = outputDataFolder + "\\" + outputFileName;
		writeListFile_sep(outputFilePath, outputListData);
		cout << outputFilePath << "に書き込みました" << endl << endl;

		//次の出力データのためにoutputListDataをクリア
		outputListData.clear();
		inputListData_img.clear();
	}
	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	return 0;
}

void searchEvent(int fileNum_tof, int sweepNum_tof, int line_tof)
{
	if (inputListData_img[fileNum_tof].empty())return;
	//二分探索アルゴリズムを使用(imaging dataのサイズをnとすると、計算量はO(logn))
	//imaging dataを二分探索する(tofデータ一つよりimagingの方がサイズが大きいため)
	int left = 0;
	int right = inputListData_img[fileNum_tof].size() - 2;
	int mid, fileNum_img, frameNum_img;

	while (left <= right)
	{
		mid = (left + right) / 2;
		fileNum_img = inputListData_img[fileNum_tof][mid][0];
		frameNum_img = inputListData_img[fileNum_tof][mid][1];

		//イメージングのデータにTOFのデータと同じインデックスを持つものを見つけたら
		if (fileNum_img == fileNum_tof && frameNum_img == sweepNum_tof)
		{
			int tof = inputListData_tof[fileNum_tof][line_tof][2];
			if (tof < TOF_MAX)//バグデータをはじくためのif文
			{
				auto line = inputListData_tof[fileNum_tof][line_tof];
				
				//TOFのデータの0列目にfileNumを追加
				reverse(all(line));
				line.push_back(fileNum_tof);
				reverse(all(line));

				//TOFのデータをoutputListDataに追加
				outputListData[fileNum_tof].push_back(line);
			}
			break;
		}
		//ファイル番号の大小から評価
		else if (fileNum_img < fileNum_tof)
		{
			left = mid + 1;
		}
		else if (fileNum_img > fileNum_tof)
		{
			right = mid - 1;
		}
		//ファイル番号が等しい場合はフレーム番号の大小を評価
		else if (frameNum_img < sweepNum_tof)
		{
			left = mid + 1;
		}
		else
		{
			right = mid - 1;
		}
	}
	return;
}