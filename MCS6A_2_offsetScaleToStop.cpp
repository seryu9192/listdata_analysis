// MCS6A_2_offsetScaleToStop : MCS6Aデコードデータからストップのみを抽出

//アップデートログ
//2019.8.1:出力ディレクトリ名を@入力時に自動で指定できるようにした
//2019.8.8:filename.txtから自動でファイル名を読み込ませるようにした
//2019.10.10:tofのchannelが大きすぎる（MCSのバグ？）イベントを除外するためにTOF_MAXを導入
//2020.2.25: 1_decoded, 2_stopフォルダを自動生成するようにした
//2020.7.9:VS Codeに移植, MCS6A_DataAnalysis　-> MCS6A_0_rawToDecodeStopに改名
//2020.7.16:出力フォルダの名前を固定
//2020.7.17:外から自動で動かせるように、inputDataFolder, start, endをコマンドライン引数から読み取れるようにした
//2020.7.21:MCS6A_2_offsetScaleToStop.cpp として作成(MCS6A_01_rawToDecode.cppから分割)

#include "./library.hpp"

vector<vector<ull>> decodedListData;
vector<vector<ull>> stopListData;

string inputDataName;

//outputfolderName
const string OUTPUT_FOLDER_NAME = "\\3_stop";

void extractStop();

int main(int argc, char* argv[])
{
	cout << "**********************************************" << endl;
	cout << "****     MCS6A_2_offsetScaleToStop.cpp    ****" << endl;
	cout << "****              stopのみ抽出            ****" << endl;
	cout << "****  ver.2020.7.21 written by R. Murase  ****" << endl;
	cout << "**********************************************" << endl << endl;

	string inputDataFolder, inputDataPath;
	int start, end;
	
	//別のプログラムから走らせる用にコマンドライン引数を定義
	//従来通り手動で走らせる
	if(argc == 1)
	{
		cout << "解析するリストデータのフォルダを入力してください\n--->";
		cin >> inputDataFolder;

		cout << "解析するファイル番号(start,end)を入力してください" << endl;
		cout << "start : ";
		cin >> start;
		cout << "end : ";
		cin >> end;
	}
	//コマンドライン引数で読み取る
	else if(argc == 4)
	{
		inputDataFolder = argv[1];
		start = stoi(argv[2]);
		end =  stoi(argv[3]);

		cout << "コマンドライン引数から読み取りました" << endl;
		cout << "inputDataFolder : " << inputDataFolder << endl;
		cout << "start = " << start << endl;
		cout << "end = "<< end << endl;
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

	//stopフォルダ
	string stopFolderPath;
	cout << "stopデータを入力フォルダ内の" + OUTPUT_FOLDER_NAME + "に保存します(フォルダが存在しなければ自動作成)\n";
	stopFolderPath = upperFolder(inputDataFolder) + OUTPUT_FOLDER_NAME;
	_mkdir(stopFolderPath.c_str());
	
	//連番処理
	for (int m = start; m <= end; m++)
	{
		//file indexを0詰め3桁の文字列suffixに変換
		string suf = toSuffix(m);

		//入力データパスの生成
		string extension = ".txt";
		inputDataPath = inputDataFolder + "\\" + inputDataName + suf + extension;
		if (!checkFileExistence(inputDataPath))
		{
			cout << inputDataName + suf + extension + ": ファイルを開けませんでした" << endl;
			continue;
		}

		//ファイルの読み込み
		cout << inputDataName + suf + extension + "---読み込んでいます...";
		decodedListData = readListFile<ull>(inputDataPath);
		cout << "読み込みました --> 処理を開始します...";

		//出力データパスの生成
		string stopFilePath = stopFolderPath + "\\" + inputDataName + suf + ".txt";
		
		//stop抽出処理
		//stop(ch=1)の行のみを取り出しoutputListDataに書き込み
		extractStop();
		cout << "処理終了 --> 書き込んでいます...";

		//stopファイルにstopListDataを書き込み
		writeListFile<ull>(stopFilePath, stopListData);
		cout << "書き込みました" << endl;

		//次のファイルの解析のために、リストデータに使用したメモリをクリア
		decodedListData.clear();
		stopListData.clear();
	}

	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	return 0;
}

//stop信号だけ抽出する関数
void extractStop()
{
	for (int i = 0; i < decodedListData.size(); i++)
	{
		//MCS6Aの入力チャンネル(6:start, 1:stop)
		int ch = decodedListData[i][1];
		ull tof = decodedListData[i][2];
		//stop信号のみ出力(バグデータははじく)
		if (ch==1 && tof < TOF_MAX)
		{
			stopListData.push_back(decodedListData[i]);
		}
	}
}
