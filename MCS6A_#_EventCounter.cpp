// MCS6A_EventCounter.cpp : MCS6Aのstart信号とstop信号の数を数えるプログラム
//

//アップデートログ
//2019.8.8:ファイル読み込みをcolumn数に依存しないようにした(inputListBufの廃止)
//2019.8.8:filename.txtから自動でファイル名を読み込ませるようにした
//2019.8.25:paramファイルからroiを読み取り、roiごとの積算数も記録するようにした
//2019.10.10:tofのchannelが大きすぎる（MCSのバグ？）イベントを除外するためにtofMaxを導入
//2020.1.18:ファイルを書き込むときに書き込みモードを指定できるようにした(新規書き込みモード/追加書き込みモード)
//2020.1.18:各ROIについてstart-endのカウントの総和を最後の行に追加する機能を廃止
//2020.2.20:filename.txtを探す範囲を2つ上と3つ上のディレクトリの両方にした(これまでは2つ上のディレクトリのみ)
//2020.7.9:VS Codeに移植
//2020.7.19:library.hppを導入
//2020.7.21:start信号であるという判定を（COL_CHNが6である && ひとつ前のsweep番号と異なる）に変更（同じsweepで）
//2020.9.18:読み込んだパラメータを表示するように修正

#include "./library.hpp"

constexpr auto binWidth = 8;

//リストデータの列番号
const int COL_SWEEP = 0;
const int COL_CHN = 1;
const int COL_TOF = 2;

//バグデータはじき用の定数
constexpr auto tofMax = 1000000;

vector<vector<ull>> inputListData;
vector<vector<ull>> outputListData;
string inputDataName;
vector<string> m_zList;
vector<int> roiMin;
vector<int> roiMax;
int roiNum = 0;

ull startSum;
ull stopSum;
vector<ull> roiSum;

bool readParameter(string);

int main()
{
	cout << "******************************************************" << endl;
	cout << "****      MCS6A イベントカウンタープログラム      ****" << endl;
	cout << "****      Start信号とStop信号の積算量を数える     ****" << endl;
	cout << "****          ver.2020.9.18  written by R. Murase ****" << endl;
	cout << "******************************************************" << endl << endl;

	//inputファイルの指定
	string inputDataFolder, inputDataPath;
	cout << "解析するリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder;

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

	//入力パラメータパスの生成
	string parameterFilePath = inputDataFolder + "\\" + inputDataName + "param.txt";
	if (!checkFileExistence(parameterFilePath))
	{
		cerr << "パラメータファイルを開けませんでした" << endl;
		cerr << "startとstopのみ数えます" << endl;
	}
	else if (!readParameter(parameterFilePath))//パラメータの読み取り
	{
		cerr << "パラメータファイルが不正です" << endl;
		cerr << "startとstopのみ数えます" << endl;
	}
	else
	{
		cout << "パラメータファイルを読み取りました" << endl;
		showSeries<string>("m_zList", m_zList);
		showSeries<int>("roiMin", roiMin);
		showSeries<int>("roiMax", roiMax);
	}

	//出力データパスの生成
	string outputFilePath = inputDataFolder + "\\" + inputDataName + "_eventCount.txt";

	//ファイル番号
	int start, end;
	cout << "解析するファイル番号(start,end)を入力してください" << endl;
	cout << "start : ";
	cin >> start;
	cout << "end : ";
	cin >> end;

	//出力ファイルにoutputListDataを書き込み
	ofstream ofs;

	//もしstartが1なら、新規書き込み。それ以外は追加書き込み
	if (start == 1)
	{
		//新規書き込みモード
		cout << "新規書き込みモードで書き込みます" << endl;
		ofs = ofstream(outputFilePath);
	}
	else
	{
		//追加書き込みモード
		cout << "追加書き込みモードで書き込みます" << endl;
		ofs = ofstream(outputFilePath, ios::app);
	}

	//ヘッダー
	vector<string> header = { "fileNum","start","stop" };
	for (int i = 0; i < roiNum; i++)
	{
		//ヘッダーの追加
		header.push_back(m_zList[i]);

		//roiSumの初期化
		roiSum.push_back(0);
	}

	//連番ファイルの処理
	for (int m = start; m <= end; m++)
	{
		//file indexを0詰め3桁の文字列suffixに変換
		string suf = toSuffix(m);

		//入力データパスの生成
		string extension = ".txt";//拡張子
		inputDataPath = inputDataFolder + "\\" + inputDataName + suf + extension;
		if (!checkFileExistence(inputDataPath))
		{
			cerr << "ファイルを開けませんでした" << endl;
			continue;
		}

		//listDataの読み込み
		cout << inputDataName + suf + extension + "----読み込み中...";
		inputListData = readListFile<ull>(inputDataPath);
		cout << "読み込み完了 -> 処理中...";

		//start信号とstop信号とroiの数を数える
		ull start = 0;
		ull stop = 0;
		vector<ull> roiCount(m_zList.size(), 0);

		//inputListDataを1行ずつ見ていく
		for (int i = 0; i < inputListData.size(); i++)
		{
			//i列目のchnとtofを読み込み
			int chn = inputListData[i][COL_CHN];
			int tof = inputListData[i][COL_TOF];

			//i列目のイベントの評価(chn = 6かつひとつ前との番号が異なる場合はトリガーのstart信号と判断)
			if (chn == 6 && (i == 0 || inputListData[i][COL_SWEEP] != inputListData[i-1][COL_SWEEP]))//start信号
			{
				start++;
			}
			else if (chn == 1 && tof < tofMax)//stop信号(バグデータははじく)
			{
				stop++;
				//roiの数だけループ
				for (int j = 0; j < roiNum; j++)
				{
					//tofがroiに含まれていたらroiCountとroiSumに1足す
					if (roiMin[j] <= tof && tof <= roiMax[j])
					{
						roiCount[j]++;
						roiSum[j]++;
					}
				}
			}
		}

		//結果の表の1列を作成
		vector<ull> outputLine;
		outputLine.push_back(m);
		outputLine.push_back(start);
		outputLine.push_back(stop);
		for (int i = 0; i < roiNum; i++)
		{
			outputLine.push_back(roiCount[i]);
		}

		outputListData.push_back(outputLine);

		//次のファイルの解析のために、リストデータに使用したメモリをクリア
		inputListData.clear();
		cout << "処理完了" << endl;
	}

	int columnOutput = outputListData[0].size();

	//ヘッダーの書き込み(新規書き込みモードのみ)
	if (start == 1)
	{
		for (int j = 0; j < header.size(); j++)
		{
			ofs << header[j];
			if (j != header.size() - 1)ofs << "\t";
		}
		ofs << endl;
	}

	//データの書き込み
	for (int i = 0; i < outputListData.size(); i++)
	{
		for (int j = 0; j < columnOutput; j++)
		{
			ofs << outputListData[i][j];
			if (j != columnOutput - 1) ofs << "\t";
		}
		ofs << endl;
	}
	return 0;
}

bool readParameter(string pfilePath)
{
	//パラメータが有効かどうかのフラグ
	bool isValid = true;

	string filepath = pfilePath;
	ifstream ifs(filepath);

	//パラメータファイルを1行ずつ読み取る
	string line;
	while (getline(ifs, line))
	{
		//0文字目が'#'の行は読み飛ばす
		if (line[0] == '#')
			continue;
		//'='がない行も読み飛ばす
		if (line.find('=') == string::npos)
			continue;

		//stringstreamインスタンスに読み取った文字列を入れる
		stringstream ss(line);

		//読み取った文字列のうち、スペースの前までの部分をname文字列に入れる
		string name;
		ss >> name;

		//'='に出会うまでの文字列を無視
		ss.ignore(line.size(), '=');

		if (name == "m/z")
		{
			string temp;
			while (ss >> temp)
			{
				m_zList.push_back(temp);
				//'\t'に出会うまでの文字列を無視
				ss.ignore(line.size(), '\t');
			}
		}
		else if (name == "roiMin")
		{
			int tmp;
			while (ss >> tmp)
			{
				//リストデータに書かれているchnはヒストグラムで読み取ったchnのbinWidth倍なので
				//その分をかけた値をpush_back
				roiMin.push_back(tmp * binWidth);
				//'\t'に出会うまでの文字列を無視
				ss.ignore(line.size(), '\t');
			}
		}
		else if (name == "roiMax")
		{
			int tmp;
			while (ss >> tmp)
			{
				roiMax.push_back(tmp * binWidth);
				//'\t'に出会うまでの文字列を無視
				ss.ignore(line.size(), '\t');
			}
		}
		else
		{
			//いらないことが書いてあったらパラメータファイルが不正
			isValid = false;
		}
	}

	//ROIのminとmaxの数が合わないときはパラメータファイルが不正
	if (roiMin.size() != roiMax.size() || m_zList.size() != roiMin.size())
	{
		isValid = false;
	}
	//roiの数を指定
	roiNum = m_zList.size();

	return isValid;
}