// ImagingMCS6A_IntegrationAnalysis.cpp : Imaging or MCS6Aのlistデータから積算データを構成

//アップデートログ
//2019.8.8:ファイル読み込みをcolumn数に依存しないようにした(inputListBufの廃止)
//2019.8.8:出力ディレクトリ名を@入力時に自動で指定できるようにした
//2019.8.8:filename.txtから自動でファイル名を読み込ませるようにした
//2019.10.13:imagingデータの積算ファイルの拡張子を.csvにした
//2020.2.20:filename.txtを2つ上と3つ上のフォルダに探しに行くようにした（これまでは2つ上のみ）
//2020.6.11:"library.hpp"から関数などを読み込むようにした
//2020.6.12:これまで"filename"と呼んでいたもの(e.g.20200612)の呼び方を"dataname"に変更
//2020.7.9:VS Codeに移植
//2020.7.17:出力フォルダを固定（入力フォルダの一つ上に保存）
//2020.7.17:出力フォルダを入力フォルダと同じにする
//2020.7.20:invalidの処理を変更（invalid.txtファイルの上に置く)

#include "./library.hpp"

//イメージングのpixel数
constexpr auto IMAGE_WIDTH = 400;
constexpr auto IMAGE_HEIGHT = 1024;

//実験時のMCSのbin幅とchannel範囲の決定
constexpr auto BIN_WIDTH_EXP = 8;
constexpr auto MCS_CHRANGE_EXP = 8000;

//リストデータからTOFスペクトルを再構成するときのchannelとbin幅の決定
constexpr auto BIN_WIDTH = 8;
constexpr auto CH = BIN_WIDTH_EXP * MCS_CHRANGE_EXP / BIN_WIDTH;

vector<vector<ui>> inputListData;
string inputDataName;

vector<vector<ui>> imageIntegrated(IMAGE_WIDTH,vector<ui>(IMAGE_HEIGHT, 0));
vector<ui> tofIntegrated(CH, 0);

//列の値
//imaging
const int COL_X = 1;
const int COL_Y = 2;

//tof
const int COL_CHN = 1;
const int COL_TOF = 2;


//ナンバリングがずれていて使えないデータを除外するためのファイル番号リスト
vector<int> invalidList;//実験時にナンバリングがずれていたファイル#

void integrateImaging();
void integrateTOF();

int main()
{
	cout << "******************************************************************" << endl;
	cout << "****              Imaging or MCS6A listデータから             ****" << endl;
	cout << "****   積算データを構成するプログラム  (0:imaging, 1:MCS6A)   ****" << endl;
	cout << "****      使えないファイルがある場合は\"invalid.txt\"を参照     ****" << endl;
	cout << "****                      ver.2020.07.20 written by R. Murase ****" << endl;
	cout << "******************************************************************" << endl << endl;

	//入力データの指定
	string inputDataFolder, inputDataPath;
	cout << "解析するリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder;

	//いくつか上のフォルダに"dataname.txt"を探しに行く
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

	//連番データ処理用
	int start, end;
	cout << "解析するファイル番号(start,end)を入力してください" << endl;
	cout << "start : ";
	cin >> start;
	cout << "end : ";
	cin >> end;

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

	//出力フォルダの指定
	cout << "入力フォルダに保存します" << endl;
	string outputFolderName = inputDataFolder;

	//連番のデータを処理
	bool isImg = true;
	for (int m = start; m <= end; m++)
	{
		//invalidListにmが入っていたら、continue
		if(find(invalidList.begin(), invalidList.end(), m) != invalidList.end())
			continue;
		
		//file indexを0詰め3桁の文字列suffixに変換
		string suf = toSuffix(m);

		//入力データパスの生成
		inputDataPath = inputDataFolder + "\\" + inputDataName + suf + ".txt";
		if (!checkFileExistence(inputDataPath))
		{
			cerr << inputDataName + suf << "---ファイルを開けませんでした" << endl;
			continue;
		}

		//listDataの読み込み
		cout << inputDataName + suf + ".txt" + "----データを読み込んでいます...";
		inputListData = readListFile<ui>(inputDataPath);

		//行数、列数を表示
		cerr << "\nline:" << inputListData.size()<< ", row:" << inputListData[0].size()<< endl;
		
		//イメージングデータかどうかを、列の数で判断（3列：TOF, それ以外：イメージング）
		if(inputListData[0].size() == 3)isImg = false;
		cout << "データの読み込み完了、処理を開始します...";
		if (isImg)
		{
			integrateImaging();
		}
		else
		{
			integrateTOF();				
		}
		cout << "処理終了" << endl;

		//次のファイルの解析のために、リストデータ入力に使用したメモリをクリア
		inputListData.clear();
	}
	//出力データパスの生成
	string outputFilePath = outputFolderName + "\\" + inputDataName + "integrated.csv";
	if (!isImg)
	{
		outputFilePath = outputFolderName + "\\" + inputDataName + "bin=" + to_string(BIN_WIDTH) + "integrated.txt";
	}

	//出力ファイルへの書き込み処理
	if (isImg)
	{
		writeListFile<ui>(outputFilePath, imageIntegrated, ',');
	}
	else
	{
		writeListFile<ui> (outputFilePath, tofIntegrated);
	}

	cout << outputFilePath << "に書き込みました" << endl << endl;
	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	return 0;
}

// 1行ずつx/yを読み取ってimageIntegratedに積算
void integrateImaging()
{
	for (int i = 0; i < inputListData.size(); i++)
	{
		ui x = inputListData[i][COL_X];
		ui y = inputListData[i][COL_Y];
		imageIntegrated[x][y]++;
	}
}

// 1行ずつchを読み取ってtofIntegratedに積算
void integrateTOF()
{
	for (int i = 0; i < inputListData.size(); i++)
	{
		if (CH <= inputListData[i][COL_TOF] / BIN_WIDTH)
		{
			continue;
		}
		//stop信号(1)のみ積算
		if (inputListData[i][COL_CHN] == 1)
		{
			ui chn = inputListData[i][COL_TOF] / BIN_WIDTH;//binWidthごとに1チャンネルにまとめる
			tofIntegrated[chn]++;
		}
	}
}

