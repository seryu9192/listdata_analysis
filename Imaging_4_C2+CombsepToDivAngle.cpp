// ImagingC2+divAngleAnalysis.cpp : Charge形式のイメージングリストデータからバックグラウンド(q=-1)を消した後、
//                                  2点だけ光っているフレームについて　2点の重心/放出角/電荷積/輝度の差の絶対値 のリストデータ(C2+Notation)を構成

//アップデートログ
//2019.8.1:出力ディレクトリ名を@入力時に自動で指定できるようにした
//2019.8.8:filename.txtから自動でファイル名を読み込ませるようにした
//2019.10.9:ファイルを出力するときに、イベント番号が1,000,000を超えるとデフォルトのdouble型では桁落ちするので、setprecision関数で精度を7桁に設定した
//2020.4.2:"library.hpp"を導入
//2020.4.2:リストデータの列を表すグローバルを導入
//2020.7.9:VS Codeに移植
//2020.7.17:ImagingC2+divAngleAnalysis - >Imaging_4_C2+CombsepToDivAngleに改名
//2020.7.17:外から自動で動かせるように、inputDataFolder, start, endをコマンドライン引数から読み取れるようにした
//2020.7.18:出力フォルダを固定化(5_C2divangle)
//2020.9.20:リストデータのうち、特定のファイル番号についてのみ処理を行うため、readListFileを変更(int col, T keyMin, T keyMax)を引数に追加
//			keyMin,keyMaxに渡すためにstart, end変数を定義。コマンドライン引数から受け取れるようにする
//			書き込み時のwriteListFile関数を、start == 1の時は新規、それ以外は追加で書き込むようにする

#include "./library.hpp"

vector<vector<int>> inputListData;
//BG除去したリストが一時的に入る配列
vector<vector<int>> bgElimListData;
vector<vector<double>> outputListData;

//データ名
string inputDataName;

//outputfolderName
const string OUTPUT_FOLDER_NAME = "5_C2divangle";

//リストデータの列
const int COL_FILE = 0;
const int COL_FRAME = 1;
const int COL_X = 2;
const int COL_Y = 3;
const int COL_BRIGHT = 4;
const int COL_CHARGE = 5;

//データの変換に使う関数
void elimBG();
void compC2Notation();

int main(int argc, char* argv[])
{
	cout << "************************************************************************" << endl;
	cout << "****              -Imaging C分解片電荷解析プログラム-               ****" << endl;
	cout << "****                   バックグラウンドを除去し、                   ****" << endl;
	cout << "****  輝点が2つのものについて重心/発散角/電荷積/輝度の差の絶対値    ****" << endl;
	cout << "****               形式のファイル(C2+Notation)を出力する            ****" << endl;
	cout << "****                          ver.2020.09.20 written by R. Murase   ****" << endl;
	cout << "************************************************************************" << endl << endl;

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
	
	//dataname.txtを探す
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

	//出力フォルダの指定
	cout << "同じフォルダ内の" + OUTPUT_FOLDER_NAME +"に保存（なければ自動作成）)\n";
	string outputFolderPath = upperFolder(inputDataFolder) + "\\" + OUTPUT_FOLDER_NAME;
	_mkdir(outputFolderPath.c_str());

	//フォルダ内のデータ
	auto fnames = getFileNameInDir(inputDataFolder);
	cout << "inputfile list" << endl;
	for (auto inputfilename:fnames)
	{
		cerr << inputfilename << endl;
	}
	cout << "処理を開始します" << endl;
	for (auto inputfilename:fnames)
	{

		//入力データパスの生成
		inputDataPath = inputDataFolder + "\\" + inputfilename;
		if (!checkFileExistence(inputDataPath))
		{
			cerr << "ファイルを開けませんでした。プログラムを終了します。" << endl;
			system("pause");
			return -1;
		}
		//出力データパスの生成
		string outputFilePath = outputFolderPath + "\\" + inputfilename;

		//listDataの読み込み
		cout << inputfilename << "----データを読み込んでいます....";
		inputListData = readListFile<int>(inputDataPath, COL_FILE, start, end+1);
		cout << "読み込み完了、処理を開始します....";

		//バックグラウンド(q=-1)を除去
		elimBG();

		//輝点が2つのフレームについて、重心と発散角と価数の積を計算
		compC2Notation();
		cout << "処理終了" << endl;


		//出力ファイルにoutputListDataを書き込み
		cout << outputFilePath << "に書き込んでいます...";
		//start == 1の時は新規、それ以外はappend
		bool append = true;
		if (start == 1)append = false;
		writeListFile<double>(outputFilePath, outputListData, '\t', append);
		cout << "に書き込みました" << endl << endl;

		//次のファイルの解析のために、リストデータに使用したメモリをクリア
		inputListData.clear();
		bgElimListData.clear();
		outputListData.clear();
	}
	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	return 0;
}

//バックグラウンド(q=-1)を除去
void elimBG()
{
	for (int i = 0; i < inputListData.size(); i++)
	{
		int q = inputListData[i][COL_CHARGE];
		//qが-1でなかったら(BGでなかったら)
		if (q != -1)
		{
			//bgElimListDataに追加
			bgElimListData.push_back(inputListData[i]);
		}
	}
}

//輝点が2つのものについて重心と距離と発散角を計算(C2+Notation)
void compC2Notation()
{
	int fileNum = bgElimListData[0][COL_FILE];//今見ているフレームのファイル番号
	int frameNum = bgElimListData[0][COL_FRAME];//今見ているフレームのインデックス
	int k = 0;//今見ている行のインデックス

	//リストデータの最後まで1つずつ見ていく
	while (k < bgElimListData.size())
	{
		vector<vector<int>> tmp;// 1フレーム分の部分配列
		fileNum = bgElimListData[k][COL_FILE];
		frameNum = bgElimListData[k][COL_FRAME];
		tmp.push_back(bgElimListData[k]);

		//frame番号が変わるところまでで部分配列を抜き出す(tmp)
		while ((k + 1 < bgElimListData.size()) && (bgElimListData[k][COL_FRAME] == bgElimListData[k + 1][COL_FRAME]))
		{
			tmp.push_back(bgElimListData[k + 1]);
			k++;
		}

		//輝点の数が2つのフレームについてのみ解析を行う
		int pointNum = tmp.size();
		if (pointNum == 2)
		{
			//2点の座標の読み込み
			int x_0 = tmp[0][COL_X];
			int y_0 = tmp[0][COL_Y];
			int x_1 = tmp[1][COL_X];
			int y_1 = tmp[1][COL_Y];

			//重心の計算
			int x_c = (x_0 + x_1) / 2;
			int y_c = (y_0 + y_1) / 2;

			//2点の距離と発散角の計算
			int dx = (x_0 - x_1);
			int dy = (y_0 - y_1);
			double d = sqrt(dx * dx + dy * dy);
			//pixel単位からmm単位に変換
			d *= mmPerPx;
			//距離からmrad単位に変換
			double divAngle = d / (double)L * 1000.;

			//2点の電荷の読み込みと電荷積の計算
			int q_0 = tmp[0][COL_CHARGE];
			int q_1 = tmp[1][COL_CHARGE];
			int q_prod = q_0 * q_1;

			//(1,4)ペアは除外
			//if ((q_0 + q_1 == 5) && (q_0 * q_1 == 4))
			//{
			//	continue;
			//}

			//2点の輝度の差の絶対値を計算
			int bright_0 = tmp[0][COL_BRIGHT];
			int bright_1 = tmp[1][COL_BRIGHT];
			int bright_diff = abs(bright_0 - bright_1);

			//outputListData1行(outputLine)を構成
			vector<double> outputLine;
			outputLine.push_back(fileNum);
			outputLine.push_back(frameNum);
			outputLine.push_back(x_c);
			outputLine.push_back(y_c);
			outputLine.push_back(divAngle);
			outputLine.push_back(q_prod);
			outputLine.push_back(bright_diff);

			//outputListDataにpush_back
			outputListData.push_back(outputLine);
		}
		k++;
	}
}
