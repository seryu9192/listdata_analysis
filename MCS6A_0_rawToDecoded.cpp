//MCS6A_0_rawToDecode.cpp : MCS6Aの.lstデータデコード

//アップデートログ
//2019.8.1:出力ディレクトリ名を@入力時に自動で指定できるようにした
//2019.8.8:filename.txtから自動でファイル名を読み込ませるようにした
//2019.10.10:tofのchannelが大きすぎる（MCSのバグ？）イベントを除外するためにTOF_MAXを導入
//2020.2.25: 1_decoded, 2_stopフォルダを自動生成するようにした
//2020.7.9:VS Codeに移植, MCS6A_DataAnalysis　-> MCS6A_0_rawToDecodeStopに改名
//2020.7.16:出力フォルダの名前を固定
//2020.7.17:外から自動で動かせるように、inputDataFolder, start, endをコマンドライン引数から読み取れるようにした
//2020.7.21:MCS6A_0_rawToDecode.cpp として作成(MCS6A_01_rawToDecode.cppから分割)

#include "./library.hpp"

//outputfolderName
const string OUTPUT_FOLDER_NAME = "\\1_decoded";

int overflowCnt = 0;
int lastSweep = 0;

vector<ull> inputListBuf;
vector<vector<ull>> inputListData;
vector<vector<ull>> decodedListData;

string inputDataName;

void decodeEvent(string);
void extractStop();

int main(int argc, char* argv[])
{
	cout << "**********************************************" << endl;
	cout << "****           MCS6A_0_rawToDecode        ****" << endl;
	cout << "****            lstデータデコード         ****" << endl;
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
	
	//datanameをフォルダ名から読み取り
	inputDataName = folderName(inputDataFolder);
	
	cout << "解析するリストデータのファイル名：" << inputDataName << endl;

	//1_decodeフォルダ
	string decodeFolderName;
	cout << "decodedデータを入力フォルダ内の\"decoded\"に保存(フォルダが存在しなければ自動作成)\n";
	decodeFolderName = inputDataFolder + OUTPUT_FOLDER_NAME;
	_mkdir(decodeFolderName.c_str());
	
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
			continue;
		}

		//出力データパスの生成
		string decodeFilePath = decodeFolderName + "\\" + inputDataName + suf + ".txt";

		cout << inputDataName + suf + extension + "---処理しています...";
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
		cout << "処理完了 -> 書き込んでいます...";

		//decodedファイルにdecodeListDataを書き込み
		writeListFile<ull>(decodeFilePath, decodedListData);
		cout << "書き込みました" << endl;

		//次のファイルの解析のために、リストデータに使用したメモリをクリア
		inputListBuf.clear();
		inputListData.clear();
		decodedListData.clear();
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
