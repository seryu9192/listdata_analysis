// MCS6A_DataAnalysis.cpp : MCS6Aの.lstデータデコード

//アップデートログ
//2019.8.1:出力ディレクトリ名を@入力時に自動で指定できるようにした
//2019.8.8:filename.txtから自動でファイル名を読み込ませるようにした
//2019.10.10:tofのchannelが大きすぎる（MCSのバグ？）イベントを除外するためにTOF_MAXを導入
//2020.2.25: 1_decoded, 2_stopフォルダを自動生成するようにした
//2020.7.9:VS Codeに移植, MCS6A_DataAnalysis　-> MCS6A_0_rawToDecodeStopに改名
//2020.7.16:出力フォルダの名前を固定
//2020.7.17:外から自動で動かせるように、inputDataFolder, start, endをコマンドライン引数から読み取れるようにした

#include "./library.hpp"

int overflowCnt = 0;
int lastSweep = 0;

vector<ull> inputListBuf;
vector<vector<ull>> inputListData;
vector<vector<ull>> decodedListData;
vector<vector<ull>> stopListData;

string inputDataName;

void decodeEvent(string);
void extractStop();

int main(int argc, char* argv[])
{
	cout << "**********************************************" << endl;
	cout << "****      MCS6A データ解析プログラム      ****" << endl;
	cout << "****  0:lstデータデコード, 1:stopのみ抽出 ****" << endl;
	cout << "****  ver.2020.7.17 written by R. Murase  ****" << endl;
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
		inputDataPath = argv[1];
		start = stoi(argv[2]);
		end =  stoi(argv[3]);

		cout << "コマンドライン引数から読み取りました" << endl;
		cout << "inputDataPath : " << inputDataPath << endl;
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

	//1_decodeフォルダ
	string decodeFolderName;
	cout << "decodedデータを入力フォルダ内の\"1_decoded\"に保存(フォルダが存在しなければ自動作成)\n";
	decodeFolderName = upperFolder(inputDataFolder) + "\\1_decoded";
	_mkdir(decodeFolderName.c_str());

	//2_stopフォルダ
	string stopFolderName;
	cout << "stopデータを入力フォルダ内の\"2_stop\"に保存します(フォルダが存在しなければ自動作成)\n";
	stopFolderName = upperFolder(inputDataFolder) + "\\2_stop";
	_mkdir(stopFolderName.c_str());
	
	//連番処理
	for (int m = start; m <= end; m++)
	{
		//file indexを0詰め3桁の文字列suffixに変換
		string suf = toSuffix(m);

		//入力データパスの生成
		string extension = ".lst";
		inputDataPath = inputDataFolder + "\\" + inputDataName + suf + extension;
		ifstream ifs(inputDataPath);
		if (!ifs)
		{
			cout << inputDataName + suf + extension + ": ファイルを開けませんでした" << endl;
			system("pause");
			continue;
		}

		//出力データパスの生成
		string decodeFilePath = decodeFolderName + "\\" + inputDataName + suf + ".txt";
		string stopFilePath = stopFolderName + "\\" + inputDataName + suf + ".txt";
		
		//1_decode処理
		//.lstデータ読み込み
		bool isData = false;
		while (!ifs.eof())
		{
			//.lstファイルを1行ずつ読み込み
			string tmp;
			getline(ifs, tmp);

			//"[DATA]"以降を読み込み
			if (isData)
			{
				decodeEvent(tmp);//1行デコードしてoutputListData vectorにpushback
			}
			//"[DATA]"以降の文字列(＝データ)に対して変換を行うためのフラグ
			if (tmp == "[DATA]") isData = true;
		}

		//decodedファイルにdecodeListDataを書き込み
		writeListFile<ull>(decodeFilePath, decodedListData);
		cout << decodeFilePath << "に書き込みました" << endl;

		//2_stop抽出処理
		//stop(ch=1)の行のみを取り出しoutputListDataに書き込み
		extractStop();

		//stopファイルにstopListDataを書き込み
		writeListFile<ull>(stopFilePath, stopListData);
		cout << stopFilePath << "に書き込みました" << endl;

		//次のファイルの解析のために、リストデータに使用したメモリをクリア
		inputListBuf.clear();
		inputListData.clear();
		decodedListData.clear();
		stopListData.clear();
		overflowCnt = 0;
		lastSweep = 0;
	}

	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	return 0;
}

//MCS6Aの文字列をデコードして、解析できる形式{sweepNum, chn(start:6/stop:1),time}に変換する関数
void decodeEvent(string tmp)
{
	//リストデータの最後の行が改行されていて（= 何も書いてない行がある:MCS6Aの仕様）stoullに例外が出るので
	//それを避けるためにif文の中に処理を書いている
	if (tmp != "") //最後の空行以外を処理
	{
		//デコード
		ull eventHex = stoull(tmp, nullptr, 16);
		ull event = eventHex % (ull)pow(2, 48);
		ull sweepNum = event / (ull)pow(2, 32);
		if (sweepNum == 0 && sweepNum < lastSweep)//最後に見たsweep数よりも今のsweep数が小さいときはoverflow
		{
			overflowCnt++;
		}
		lastSweep = sweepNum;
		sweepNum += (int)pow(2, 16)*overflowCnt;//overflowした分だけ65536足す
		ui time = event % (ull)pow(2, 32);
		time /= (int)pow(2, 4);
		ui chn = event % (int)pow(2, 4);
		chn %= (int)pow(2, 3);

		//list1行を構成
		vector<ull> eventVector = { sweepNum,chn,time };
		
		//decodeListdata配列に追加
		decodedListData.push_back(eventVector);
	}
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
