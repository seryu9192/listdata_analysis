//Imaging_3_CnToCombseparated.cpp : 電荷ペア毎にlistファイルを分ける

//アップデートログ
//2020.1.23:ImagingSeparateByCharge.cppの改良版として作成
//          電荷の積としてではなく、電荷の組み合わせに着目して価数を分ける(generalized)
//2020.7.9:VS Codeに移植
//2020.7.10:ImagingSeparateByCharge_gen -> Imaging_3_CnToCombseparated に改名
//2020.7.16:"library.hpp"を導入
//2020.7.16:combinedデータではなくserialデータに対して処理を行うように変更（appendモードでcombined機能も兼ねる）
//2020.7.17:外から自動で動かせるように、inputDataFolder, start, endをコマンドライン引数から読み取れるようにした
//2020.7.17:appendモードをstartが1かどうかで判断する
//2020.7.20:ファイルが見つからなかった時の動作を強制終了(-1)からcontinueに変更
//2020.7.21:invalidListの導入

#include "./library.hpp"

vector<vector<int>> inputListData;
vector<vector<int>> outputListData;

string inputDataName;

//outputfolderName
const string OUTPUT_FOLDER_NAME = "4_Combseparated";

//リストデータの列番号と対応するデータ
const int COL_FRAME = 0;
const int COL_X = 1;
const int COL_Y = 2;
const int COL_BRIGHT = 3;
const int COL_CHARGE = 4;

//測定に不備があり使えないファイル番号のリスト
vector<int> invalidList;

//クラスターサイズ
int clusterSize = 1;
//charge stateの範囲
const int chargeMin = 1;
const int chargeMax = 4;

//電荷の組み合わせを表す文字列の集合
set<string> chargeComb;

//電荷の積ごとにデータを分ける関数
void separateCharge(string, int);
void buildChargeComb(string);

int main(int argc, char* argv[])
{
	cout << "*************************************************************" << endl;
	cout << "****           ImagingSeparateByCharge_gen.cpp           ****" << endl;
	cout << "****             \"3_Cn\" -> \"4_Combseparated\"             ****" << endl;
	cout << "****      -リストデータを電荷毎に分解するプログラム-     ****" << endl;
	cout << "****　             結合されたリストデータを              ****" << endl;
	cout << "****         電荷の組み合わせごとに分けて保存する        ****" << endl;
	cout << "****                ver.2020.07.21 written by R. Murase  ****" << endl;
	cout << "*************************************************************" << endl << endl;

	int start, end;
	string inputDataFolder, inputDataPath;

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

	//dataname.txtの読み取り
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

	//クラスターサイズを読み取り
	clusterSize = readClustersize(datanamePath);
	if(clusterSize == -1)
	{
		cout << "clustersizeを読み取れませんでした" << endl;
		system("pause");
		return -1;
	}
	else
		cout << "clustersize : " + to_string(clusterSize) << endl;
	
	//chargeCombの集合を構成
	buildChargeComb("");
	cout << "chargeComb：{";
	for (auto e : chargeComb)
	{
		cout << e << ", ";
	}
	cout << "\b\b}" << endl;

	//invalidの処理
	string invalidFilePath = searchFileFromUpperFolder(inputDataFolder, INVALID_FILE);
	if (!checkFileExistence(invalidFilePath))
	{
		cout << "invalidファイルは見つかりませんでした" << endl;
		cout << "全てのファイルを結合します" << endl;
	}
	else
	{
		invalidList = readInvalid(invalidFilePath);
		cout << "invalidファイルを読み取りました" << endl;
		if(invalidList.empty())
		{
			cout << "invalidなデータはありません" << endl;
		}
		else
		{
			cout << "invalid_list = {";
			for (int i = 0; i < invalidList.size(); i++)
			{
				cout << invalidList[i];
				if (i != invalidList.size() - 1)cout << ", ";
			}
			cout << "}"<< endl;
		}
	}

	//書き込みモード
	//startが1なら新規書き込みモード
	bool append;
	if(start == 1)
	{
		append = false;
		cout << "新規書き込みモードで書き込みます" << endl;
	}
	//それ以外なら追加書き込みモード
	else
	{
		append = true;
		cout << "追加書き込みモードで書き込みます" << endl;
	}
	
	//出力フォルダパスの決定
	string outputFolderPath = upperFolder(inputDataFolder) + "\\" + OUTPUT_FOLDER_NAME;
	cout << "同じフォルダ内の\"" + OUTPUT_FOLDER_NAME + "\"フォルダに出力を保存します(存在しない場合は新規に作成します)" << endl;
	
	//出力フォルダの作成
	if (_mkdir(outputFolderPath.c_str()) == 0)
		cout << "新規にフォルダを作成しました" << endl;
	else
		cout << "既存のフォルダに書き込みます" << endl;

	//連番処理
	for (int m = start; m <= end; m++)
	{
		//invalidListにmが入っていたら、continue
		if(find(invalidList.begin(), invalidList.end(), m) != invalidList.end())
			continue;

		//suffix
		string suf = toSuffix(m);

		//入力データパスの生成
		inputDataPath = inputDataFolder + "\\" + inputDataName + suf + ".txt";
		if (!checkFileExistence(inputDataPath))
		{
			cerr << inputDataName + suf << "---ファイルを開けませんでした" << endl;
			continue;
		}

		//listDataを読み込み
		cout << inputDataName + suf + "----ファイルを読み込んでいます...";
		inputListData = readListFile<int> (inputDataPath);
		cout << "ファイルの読み込み完了" << endl;

		//電荷の組み合わせ毎に処理
		for (auto cComb : chargeComb)
		{
			//電荷の組み合わせごとにファイルを分けて保存
			auto q = cComb;
			cout << "q = " << q << "----データの処理開始...";
			separateCharge(q, m);
			cout << "データの処理終了" << endl;

			//電荷積の組合わせがない場合はoutputListDataをファイルに書き込まない
			if (outputListData.size() == 0)
			{
				cout << "q = " << q << "----イベントがありませんでした" << endl;
				continue;
			}

			//出力データパスの生成
			string outputFilePath = outputFolderPath + "\\" + inputDataName + "_q=" + q + ".txt";

			cout << outputFilePath << "に書き込んでいます...";
			writeListFile<int> (outputFilePath, outputListData, '\t' , append);
			cout << "書き込みました" << endl;

			//次の出力ファイルのために、outputリストデータに使用したメモリをクリア
			outputListData.clear();
		}
		//2つ目以降のファイルは追加書き込みで行う
		append = true;
	}

	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	return 0;
}

//電荷の積ごとにデータを分けてoutputListDataにpush_back
void separateCharge(string q, int m)
{
	int k = 0;//今見ている行のインデックス

	//リストデータの最後まで1つずつ見ていく
	while (k < inputListData.size())
	{
		int from = k;

		// 1フレーム分のchargeCombを表す文字列
		string tmp = to_string(inputListData[k][COL_CHARGE]);

		int cnt = 1;
		//frame番号が変わるところまでで電荷の組み合わせを調べ、引数と等しかったら出力
		while ((k + 1 < inputListData.size()) && (inputListData[k][COL_FRAME] == inputListData[k + 1][COL_FRAME]))
		{
			tmp += to_string(inputListData[k + 1][COL_CHARGE]);
			k++;
		}
		int to = k;
		sort(tmp.begin(), tmp.end());
		if (tmp == q)
		{
			for (int i = from; i <= to; i++)
			{
				vector<int> outputLine = {m};
				outputLine.insert(outputLine.end(), inputListData[i].begin(), inputListData[i].end());
				outputListData.push_back(outputLine);
			}
		}
		k++;
	}
}

void buildChargeComb(string sNow)
{
	//もしsNowの文字数がclusterSizeと同じなら、辞書式にソートしてchargeComb集合に挿入
	if (sNow.size() == clusterSize)
	{
		sort(sNow.begin(), sNow.end());
		chargeComb.insert(sNow);
	}
	//そうでない場合は、chargeMin-chargeMaxまでの数字をsNowに追加して再帰関数の引数にする
	else
	{
		for (int i = chargeMin; i <= chargeMax; i++)
		{
			string sNext = sNow + to_string(i);
			buildChargeComb(sNext);
		}
	}
	return;
}