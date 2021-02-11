// ImagingMCS6A_resolveTOFwithImaging.cpp : ImagingデータとコインシデンスしたTOFイベントを抽出
//

//アップデートログ
//2020.7.9:VS Codeに移植
//2021.2.11:ログファイルを残す機能を追加

#include "./library.hpp"

//リストデータの列値
const int COL_FILENUM = 0;
const int COL_EVENTNUM = 1;

//outputfolder name
const string OUTPUT_FOLDER_NAME = "\\5_coinc_sep";

//resolve用の関数
template <typename T = int, typename U = int>
vector<vector<T>> extractEvents(vector<vector<T>>, vector<vector<U>>);

int main()
{
	cout << "******************************************************************" << endl;
	cout << "****          ImagingMCS6A_resolveTOFwithImaging.cpp          ****" << endl;
	cout << "****  -ImagingイベントとコインシデンスしたTOFイベントを出力-  ****" << endl;
	cout << "****                    ver.2021.02.11 written by R. Murase   ****" << endl;
	cout << "******************************************************************" << endl << endl;

	//入力イメージングデータパスの生成
	string inputDataFolder_img;
	cout << "イメージングリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder_img;

	//入力イメージングデータフォルダ内のファイルの名前を取得
	vector<string> fNames = getFileNameInDir(inputDataFolder_img);

	//入力TOFデータパスの生成
	string inputDataFolder_tof, inputDataName_tof;
	cout << "TOFリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder_tof;

	//dataname.txtを探しに行く
	string datanamePath = searchFileFromUpperFolder(inputDataFolder_tof, DATANAME_FILE);
	inputDataName_tof = readDataName(datanamePath);

	if (datanamePath == "")
	{
		cout << "dataname.txtファイルが見つかりませんでした\n";
		cout << "解析するTOFリストデータのファイル名を入力してください(拡張子なし)\n--->";
		cin >> inputDataName_tof;
	}
	else if(inputDataName_tof == "")
	{
		cout << "dataname.txtファイルが不正です\n";
		cout << "解析するTOFリストデータのファイル名を入力してください\n--->";
		cin >> inputDataName_tof;
	}
	else
	{
		cout << "dataname.txtファイルを読み込みました\n";
		cout << "解析するTOFリストデータのファイル名：" << inputDataName_tof << endl;
	}

	//入力TOFデータの読み込み
	string inputDataPath_tof = inputDataFolder_tof + "\\" + inputDataName_tof + ".txt";
	if (!checkFileExistence(inputDataPath_tof))
	{
		cerr << "TOFファイルを開けませんでした" << endl;
		system("pause");
		return -1;
	}

	//出力データパスの生成
	string outputDataFolder = upperFolder(inputDataFolder_tof) + OUTPUT_FOLDER_NAME;
	if (_mkdir(outputDataFolder.c_str()) == 0)
	{
		cout << "新たなフォルダを作成し出力ファイルを保存します。" << endl;
		outputDataFolder += "\\" + folderName(inputDataFolder_img);
		_mkdir(outputDataFolder.c_str());
	}
	else
	{
		outputDataFolder += "\\" + folderName(inputDataFolder_img);
		if(_mkdir(outputDataFolder.c_str()) == 0)
		{
			cout << "新たなフォルダを作成し出力ファイルを保存します。" << endl;
		}
		else
			cout << "既存のフォルダに出力ファイルを保存します。" << endl;
	}

	//TOFデータ読み込み
	cout << "TOFデータ読み込み中...";
	auto inputListData_tof = readListFile<int>(inputDataPath_tof);
	cout << "完了！" << endl;

	//入力ファイルごとにループ
	for (auto inputFilename : fNames)
	{
		string inputDataPath_img = inputDataFolder_img + "\\" + inputFilename;
		cout << inputFilename + " : Imagingデータ読み込み中...";
		auto inputListData_img = readListFile<double>(inputDataPath_img);
		cout << "読み込み完了 --> 処理開始...";
		
		//コインシデンスしているイベントを抽出
		auto outputListData = extractEvents(inputListData_tof, inputListData_img);

		//ファイルに書き出し(ファイル名は間違いがないように、TOFfilename + "&"" + Imagingfilename)
		string outputDataPath = outputDataFolder + "\\"+ parsePath(inputDataPath_tof)[1] + "&" + parsePath(inputDataPath_img)[1] + parsePath(inputDataPath_img)[2];
		writeListFile<int>(outputDataPath, outputListData);
		cout << "処理完了！" << endl;
	}

	//ログにコインシデンスに使ったイメージフォルダ名を残しておく
	string logFilePath = outputDataFolder + "\\coincidence.log";
	ofstream ofs(logFilePath, ios::app);
	time_t t = chrono::system_clock::to_time_t(chrono::system_clock::now());
	ofs << "--- " << ctime(&t);
	ofs << "Imaging folder used for coincidence: " + inputDataFolder_img << endl;
	ofs.close();
	cout << "全ての処理完了。プログラムを終了します" << endl;
	return 0;
}

//コインシデンスしたデータを抽出
template <typename T = int, typename U = int>
vector<vector<T>>extractEvents(vector<vector<T>> dataToResolve, vector<vector<U>> dataToRef)
{
	vector<vector<T>> res;
	if (dataToRef.empty())return res;

	//dataToResolveについて、線形探索
	for(int i = 0; i < dataToResolve.size(); i++)
	{
		int resolve_fileNum = int(dataToResolve[i][COL_FILENUM]);
		int resolve_eventNum = int(dataToResolve[i][COL_EVENTNUM]);
		//dataToRefについて二分探索(0列目：ファイル番号、1列目：イベント番号)
		int left = 0;
		int right = dataToRef.size() - 1;
		int mid, ref_fileNum, ref_eventNum;

		while (left <= right)
		{
			mid = (left + right) / 2;
			ref_fileNum = int(dataToRef[mid][COL_FILENUM]);
			ref_eventNum = int(dataToRef[mid][COL_EVENTNUM]);
			if (ref_fileNum == resolve_fileNum && ref_eventNum == resolve_eventNum)
			{
				res.push_back(dataToResolve[i]);
				break;
			}
			//fileNumの大小から評価
			else if (ref_fileNum < resolve_fileNum)
				left = mid + 1;
			else if (ref_fileNum > resolve_fileNum)
				right = mid - 1;
			//fileNumが等しかったら、eventNumの大小で評価
			else if (ref_eventNum < resolve_eventNum)
				left = mid + 1;
			else
				right = mid - 1;
		}
	}
	return res;
}