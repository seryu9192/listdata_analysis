// MCS6A_3_stopToCoinc.cpp : ImagingデータとコインシデンスしたTOFイベントを抽出(連番)
//

//アップデートログ
//2020.7.28:ImagingMCS6A_resolveTOFwithImaging.cppから作成
//2020.9.18:参照するイメージングフォルダの名前(imagingfolder_img)をコマンドライン引数から読む様に変更
//          (一般のクラスターサイズに適合できるようにした)

#include "./library.hpp"

//リストデータの列値
const int COL_EVENTNUM = 0;

//outputfolderName
const string OUTPUT_FOLDER_NAME = "\\4_coinc";
string inputfolder_img = "\\imaging\\";

string inputDataName;

template <typename T = int, typename U = int>
vector<vector<T>> extractEvents(vector<vector<T>>, vector<vector<U>>);

int main(int argc, char* argv[])
{
	cout << "*****************************************************************" << endl;
	cout << "****                   MCS6A_3_stopToCoinc.cpp               ****" << endl;
	cout << "****  -ImagingイベントとコインシデンスしたTOFイベントを出力- ****" << endl;
	cout << "****                     ver.2020.09.18 written by R. Murase ****" << endl;
	cout << "*****************************************************************" << endl << endl;

	//入力TOFデータフォルダ内のファイルの名前を取得
	string inputDataFolder, inputDataPath, imageFoldername;
	int start, end;

	//別のプログラムから走らせる用にコマンドライン引数を定義
	//従来通り手動で走らせる
	if(argc == 1)
	{
		cout << "解析するリストデータのフォルダを入力してください\n--->";
		cin >> inputDataFolder;
		cout << "参照するイメージングフォルダ名を入力してください\n--->";
		cin >> imageFoldername;
		inputfolder_img += imageFoldername;
		cout << "解析するファイル番号(start,end)を入力してください" << endl;
		cout << "start : ";
		cin >> start;
		cout << "end : ";
		cin >> end;
	}
	//コマンドライン引数で読み取る
	else if(argc == 5)
	{
		inputDataFolder = argv[1];
		start = stoi(argv[2]);
		end =  stoi(argv[3]);
		//"3_C2"など、参照するイメージングフォルダの名前をコマンドライン引数から指定
		imageFoldername = argv[4];
		inputfolder_img += imageFoldername;

		cout << "コマンドライン引数から読み取りました" << endl;
		cout << "inputDataFolder : " << inputDataFolder << endl;
		cout << "start = " << start << endl;
		cout << "end = "<< end << endl;
		cout << "imageFoldername= " << imageFoldername << endl;
	}
	else
	{
		cerr << "コマンドライン引数の数が不正です" << endl;
		system("pause");
		return -1;
	}	

	//dataname.txtを読み込み
	string datanamePath = searchFileFromUpperFolder(inputDataFolder, DATANAME_FILE);
	if (datanamePath == "")
	{
		cout << "dataname.txtファイルが見つかりませんでした\n";
		cout << "解析するリストデータのファイル名を入力してください\n--->";
		cin >> inputDataName;
	}
	else if(readDataName(datanamePath) == "")
	{
		cout << "dataname.txtファイルが不正です\n";
		cout << "解析するリストデータのファイル名を入力してください\n--->";
		cin >> inputDataName;
	}	
	else
	{
		cout << "dataname.txtファイルを読み込みました\n";
		inputDataName = readDataName(datanamePath);
		cout << "解析するリストデータのファイル名：" << inputDataName << endl;
	}

	//Imaging folder
	string inputDataFolder_img = upperFolder(inputDataFolder, 2) + inputfolder_img;
	cout << "imagingdata:" + inputDataFolder_img + "とコインシデンスしたデータを抽出します" << endl;

	//出力データパスの生成
	string outputDataFolder = upperFolder(inputDataFolder) + OUTPUT_FOLDER_NAME;
	cout << "出力データを入力フォルダ内" + OUTPUT_FOLDER_NAME +"に保存します(フォルダが存在しなければ自動作成)\n";
	if (_mkdir(outputDataFolder.c_str()) == 0)
	{
		cout << "新たなフォルダを作成し出力ファイルを保存します。" << endl;
		_mkdir(outputDataFolder.c_str());
	}
	else
	{
		if(_mkdir(outputDataFolder.c_str()) == 0)
		{
			cout << "新たなフォルダを作成し出力ファイルを保存します。" << endl;
		}
		else
		{
			cout << "既存のフォルダに出力ファイルを保存します。" << endl;
		}
	}

	//連番処理
	for (int m = start; m <= end; m++)
	{
		//0埋め3桁のsuffixに変換
		string suf = toSuffix(m);

		//入力TOFデータの読み込み
		string inputDataPath = inputDataFolder + "\\" + inputDataName + suf +".txt";
		if (!checkFileExistence(inputDataPath))
		{
			cerr << "TOFファイルを開けませんでした" << endl;
			continue;
			return -1;
		}
		//TOFデータ読み込み
		cout << inputDataName + suf + "---TOFデータ読み込み中...";
		auto inputListData_tof = readListFile<int>(inputDataPath);
		cout << "読み込みました --> ";

		//imagingデータ読み込み
		string inputDataPath_img = inputDataFolder_img + "\\" + inputDataName + suf +".txt";
		cout << "Imagingデータ読み込み中...";
		auto inputListData_img = readListFile<int>(inputDataPath_img);
		cout << "読み込み完了 --> 処理開始...";
		
		//コインシデンスしているイベントを抽出
		auto outputListData = extractEvents(inputListData_tof, inputListData_img);

		//ファイルに書き出し
		cout << "処理完了 --> 書き込んでいます...";
		string outputDataPath = outputDataFolder + "\\"+ inputDataName + suf +".txt";
		writeListFile<int>(outputDataPath, outputListData);
		cout << "書き込みました" << endl;
	}
	//ログにコインシデンスに使ったイメージフォルダを残しておく
	string logFilePath = outputDataFolder + "\\coincindence.log";
	ofstream ofs(logFilePath, ios::app);
	time_t t = chrono::system_clock::to_time_t(chrono::system_clock::now());
	ofs << "--- " << ctime(&t);
	ofs << "Imaging folder used for coincidence: " + inputDataFolder_img << endl;
	ofs << "file# : " << start << "-" << end << endl;
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
		int resolve_eventNum = int(dataToResolve[i][COL_EVENTNUM]);
		//dataToRefについて二分探索(0列目：ファイル番号、1列目：イベント番号)
		int left = 0;
		int right = dataToRef.size() - 1;
		int mid, ref_fileNum, ref_eventNum;
		while (left <= right)
		{
			mid = (left + right) / 2;
			ref_eventNum = int(dataToRef[mid][COL_EVENTNUM]);
			if (ref_eventNum == resolve_eventNum)
			{
				res.push_back(dataToResolve[i]);
				break;
			}
			//eventNumの大小で評価
			else if (ref_eventNum < resolve_eventNum)
				left = mid + 1;
			else
				right = mid - 1;
		}
	}
	return res;
}