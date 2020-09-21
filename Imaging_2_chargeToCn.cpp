// Imaging_2_chargeToCn.cpp : イメージングリストデータからバックグラウンドを除去し、輝点がn点のもののみを抽出
//

//アップデートログ
//2020.1.23:作成
//2020.2.12:出力フォルダの名前が"C"+clusterSizeになるように書き換え
//2020.7.9:VS Codeに移植
//2020.7.10:ImagingCn+Extract_nPoints -> Imaging_2_chargeToCn に改名
//2020.7.17:外から自動で動かせるように、inputDataFolder, start, endをコマンドライン引数から読み取れるようにした
//2020.7.20:ファイルが見つからなかった時の動作を強制終了(-1)からcontinueに変更
//


#include "./library.hpp"

vector<int> inputListBuf;
vector<vector<int>> inputListData;
vector<vector<int>> intermediateListData;//BG除去したリストが一時的に入る
vector<vector<int>> outputListData;

string inputDataName;
int clusterSize = 1;

//リストデータの列番号と対応するデータ
const int COL_FRAME = 0;
const int COL_X = 1;
const int COL_Y = 2;
const int COL_BRIGHT = 3;
const int COL_CHARGE = 4;

void elimBG();
void extract_nPoints();

int main(int argc, char* argv[])
{
	cout << "************************************************************************" << endl;
	cout << "****                ImagingCn+Extract_nPoints.cpp                   ****" << endl;
	cout << "****                    \"2_charge\" -> \"3_Cn\"                        ****" << endl;
	cout << "****  Imagingの輝点の数が指定した数のもののみを抽出するプログラム   ****" << endl;
	cout << "****                    バックグラウンドを除去し、                  ****" << endl;
	cout << "****      輝点が1フレーム当たりn個のもののみを抽出（連番処理）      ****" << endl;
	cout << "****                            ver.2020.7.18 written by R. Murase  ****" << endl;
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
		return -1;
	}
	else
		cout << "clustersize : " + to_string(clusterSize) << endl;
	

	//出力フォルダパスの決定
	string outputFolderName = "\\3_C" + to_string(clusterSize);
	string outputFolderPath = upperFolder(inputDataFolder) + outputFolderName;
	cout << "同じフォルダ内の\"" + outputFolderName + "\"フォルダに出力を保存します(存在しない場合は新規に作成します)" << endl;

	//出力フォルダの作成
	if (_mkdir(outputFolderPath.c_str()) == 0)
		cout << "新規にフォルダを作成しました" << endl;
	else
		cout << "既存のフォルダに書き込みます" << endl;

	//連番処理
	for (int m = start; m <= end; m++)
	{
		//file indexを0詰め3桁の文字列suffixに変換
		string suf = toSuffix(m);

		//入力データパスの生成
		inputDataPath = inputDataFolder + "\\" + inputDataName + suf + ".txt";
		
		if (!checkFileExistence(inputDataPath))
		{
			cerr << inputDataName + suf << "---ファイルを開けませんでした" << endl;			
			continue;
		}
		//出力データパスの生成
		string outputFilePath = outputFolderPath + "\\" + inputDataName + suf + ".txt";

		//listDataの読み込み
		cout << inputDataPath << "----データを読み込んでいます...";
		inputListData = readListFile<int> (inputDataPath);
		cout << "データの読み込み完了、処理を開始します... ";

		//バックグラウンド(q=-1)を除去
		elimBG();

		//輝点がclusterSize個のフレームを抽出
		extract_nPoints();

		cout << "処理終了" << endl;

		//出力ファイルにoutputListDataを書き込み
		cout << outputFilePath << "に書き込んでいます...";
		writeListFile<int> (outputFilePath, outputListData);
		cout << "書き込みました" << endl << endl;

		//次のファイルの解析のために、リストデータに使用したメモリをクリア
		inputListData.clear();
		intermediateListData.clear();
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
		ui frameNum = inputListData[i][COL_FRAME];
		int x = inputListData[i][COL_X];
		int y = inputListData[i][COL_Y];
		ui brightness = inputListData[i][COL_BRIGHT];
		int q = inputListData[i][COL_CHARGE];

		if (q != -1)
		{
			//intermediateListData1行(intermediateLine)を構成
			vector<int> intermediateLine;
			intermediateLine.push_back(frameNum);
			intermediateLine.push_back(x);
			intermediateLine.push_back(y);
			intermediateLine.push_back(brightness);
			intermediateLine.push_back(q);

			//intermediateListDataに追加
			intermediateListData.push_back(intermediateLine);
		}
	}
}

//輝点がclusterSize個のものをoutputListDataにpush_back
void extract_nPoints()
{
	//今見ている行のインデックス
	int k = 0;

	//リストデータの最後まで1つずつ見ていく
	while (k < intermediateListData.size())
	{
		vector<vector<int>> tmp;// 1フレーム分の部分配列
		tmp.push_back(intermediateListData[k]);

		//frame番号が変わるところまでで部分配列を抜き出す(tmp)
		while ((k + 1 < intermediateListData.size()) && (intermediateListData[k][COL_FRAME] == intermediateListData[k + 1][COL_FRAME]))
		{
			tmp.push_back(intermediateListData[k + 1]);
			k++;
		}

		//輝点の数がclusterSize個のフレームについてのみ解析を行う
		int pointNum = tmp.size();
		if (pointNum == clusterSize)
		{
			//outputListDataにpush_back
			for (int i = 0; i < pointNum; i++)
			{
				outputListData.push_back(tmp[i]);
			}
		}
		k++;
	}
}

