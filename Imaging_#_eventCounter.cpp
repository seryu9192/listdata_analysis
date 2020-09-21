// ImagingEventCounter.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

//アップデートログ
//2020.2.7:発掘、もとの機能は、電荷の組合せの個数分布を調べるものであった。（しかも二量体C2の時のみに対応）
//         しかし、電荷の組合せの個数分布は"ImagingSeparateByCharge_gen"で任意の入射クラスターサイズについて求めることができるようになったので、
//         このプログラムは、"輝点の個数分布"を求めるプログラムへと書き換える。
//2020.7.9:VS Codeに移植
//2020.7.30:"library.hpp"を導入

#include "./library.hpp"

//輝点の個数分布
const int MAX_POINT_NUM = 20;
vector<int> pointNum(MAX_POINT_NUM + 1 , 0);

vector<vector<int>> inputListData;
vector<vector<int>> outputListData;
string inputDataName;

void countPointNum();

int main()
{
	cout << "***********************************************************************" << endl;
	cout << "****                   ImagingEventCounter.cpp                     ****" << endl;
	cout << "**** -イメージングデータにおける輝点の個数分布を求めるプログラム-  ****" << endl;
	cout << "****                        ver.2020.07.30 written by R. Murase    ****" << endl;
	cout << "***********************************************************************" << endl << endl;

	//入力イメージングデータパスの生成
	string inputDataFolder, inputDataPath;
	cout << "イメージングリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder;

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

	//出力データパスの生成
	string outputFilePath = inputDataFolder + "\\" + inputDataName + "_eventCount.txt";

	//連番処理
	int start, end;
	cout << "解析するファイル番号(start,end)を入力してください" << endl;
	cout << "start : ";
	cin >> start;
	cout << "end : ";
	cin >> end;

	//出力ファイルにoutputListDataを書き込み
	if (start == 1)
	{
		//新規書き込みモード
		cout << "新規書き込みモードで書き込みます" << endl;
	}
	else
	{
		//追加書き込みモード
		cout << "追加書き込みモードで書き込みます" << endl;
	}

	//ヘッダー(新規書き込み時のみ)
	if (start == 1)
	{
		vector<int> header;
		for (int i = 0; i < pointNum.size(); i++)header.push_back(i);
		outputListData.push_back(header);
	}

	//連番データ処理開始
	for (int m = start; m <= end; m++)
	{
		//file indexを0詰め3桁の文字列suffixに変換
		string suf = toSuffix(m);

		//入力データパスの生成
		string extension = ".txt";
		inputDataPath = inputDataFolder + "\\" + inputDataName + suf + extension;
		if (!checkFileExistence(inputDataPath))
		{
			cerr << inputDataName + suf +  "---ファイルを開けませんでした" << endl;
			continue;
		}

		//listDataの読み込み
		cout << inputDataName + suf + extension + "----データを読み込んでいます...";
		inputListData = readListFile<int>(inputDataPath);	
		cout << "読み込みました --> 処理を開始します...";
		
		//輝点のデータ分布を数えてoutputListDataにpush_back
		pointNum[0] = m;//0列目はファイル番号
		countPointNum();
		outputListData.push_back(pointNum);

		//メモリのクリア
		inputListData.clear();
		for (int i = 0; i < pointNum.size(); i++)pointNum[i] = 0;

		cout << "処理終了" << endl;
	}
	
	//出力ファイルにoutputListDataを書き込み
	cout << "----全てのデータの処理終了。ファイルに書き込んでいます..." << endl;
	if (start == 1)
		writeListFile<int>(outputFilePath, outputListData);//新規書き込み
	else
		writeListFile<int>(outputFilePath, outputListData, '\t', true);//追加書き込み
	
	cout << outputFilePath << "に書き込みました" << endl << endl;

	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	return 0;
}

void countPointNum()
{
	int frameNum = inputListData[0][0];//今見ているフレームのインデックス
	int k = 0;//今見ている行のインデックス
	int colInput = inputListData[0].size();

	//リストデータの最後まで1つずつ見ていく
	while (k < inputListData.size())
	{
		vector<vector<int>> tmp;// 1フレーム分の部分配列
		frameNum = inputListData[k][0];
		tmp.push_back(inputListData[k]);

		//frame番号が変わるところまでで部分配列を抜き出す(tmp)
		while ((k + 1 < inputListData.size()) && (inputListData[k][0] == inputListData[k + 1][0]))
		{
			//もしBGでなければ(電荷が-1)、部分配列に追加
			if (inputListData[k + 1][colInput - 1] != -1)
			{
				tmp.push_back(inputListData[k + 1]);
			}
			k++;
		}

		//輝点の数をindexとするpointNumの要素に1足す
		int pNum = tmp.size();
		if (0 < pNum && pNum <= MAX_POINT_NUM)
		{
			pointNum[pNum]++;
		}
		k++;
	}
}