// ImagingC3+extract3points.cpp : イメージングリストデータからバックグラウンドを除去し、輝点が3点のもののみを抽出
//

//アップデートログ
//2019.9.28:作成
//2020.7.9:VS Codeに移植

#include "./library.hpp"

vector<int> inputListBuf;
vector<vector<int>> inputListData;
vector<vector<int>> intermediateListData;//BG除去したリストが一時的に入る
vector<vector<int>> outputListData;

string inputDataName;

void elimBG();
void extract3Points();
string upperPath(string);
bool readFilename(string);

int main()
{
	cout << "********************************************************************" << endl;
	cout << "****        Imaging C分解片個数解析プログラム for C3+           ****" << endl;
	cout << "****                  バックグラウンドを除去し、                ****" << endl;
	cout << "****         輝点が3つのもののみを抽出（連番処理）              ****" << endl;
	cout << "****                       ver.2019.09.28 written by R. Murase  ****" << endl;
	cout << "********************************************************************" << endl << endl;

	string inputDataFolder, inputDataPath;
	cout << "解析するリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder;

	string filenamePath = upperPath(upperPath(inputDataFolder)) + "\\filename.txt";
	cout << filenamePath << endl;
	if (readFilename(filenamePath))
	{
		cout << "filename.txtファイルを読み込みました\n";
		cout << "解析するリストデータのファイル名：" << inputDataName << endl;
	}
	else
	{
		cout << "filename.txtファイルが不正です\n";
		cout << "解析するリストデータのファイル名を入力してください\n--->";
		cin >> inputDataName;
	}

	int start, end;
	cout << "解析するファイル番号(start,end)を入力してください" << endl;
	cout << "start : ";
	cin >> start;
	cout << "end : ";
	cin >> end;

	//出力フォルダの指定
	string outputFolderName;
	cout << "\n出力するフォルダのパスを入力してください (\"@\"入力で同じフォルダ内の\"3_C3\"に保存)\n--->";
	cin >> outputFolderName;
	if (outputFolderName == "@")
	{
		outputFolderName = upperPath(inputDataFolder) + "\\3_C3";
	}

	for (int m = start; m <= end; m++)
	{
		//file indexを0詰め3桁の文字列suffixに変換
		ostringstream sout;
		sout << setfill('0') << setw(3) << m;
		string suf = sout.str();

		//入力データパスの生成
		inputDataPath = inputDataFolder + "\\" + inputDataName + suf + ".txt";
		ifstream ifs(inputDataPath);
		if (!ifs)
		{
			cerr << "ファイルを開けませんでした" << endl;
			return -1;
		}
		//出力データパスの生成
		string outputFilePath = outputFolderName + "\\" + inputDataName + suf + ".txt";

		//listDataの読み込み
		string tmp;
		while (getline(ifs, tmp))
		{
			stringstream ss(tmp);
			string tmp2;
			vector<int> inputLine;
			while (ss >> tmp2)
			{
				inputLine.push_back(stod(tmp2));
			}
			inputListData.push_back(inputLine);
		}
		cout << "----データの読み込み完了、処理を開始します。" << endl;

		//バックグラウンド(q=-1)を除去
		elimBG();

		//輝点が2つのフレームについて、重心と発散角と価数の積を計算
		extract3Points();

		cout << "----処理終了" << endl;

		//出力ファイルにoutputListDataを書き込み
		ofstream ofs(outputFilePath);
		int columnOutput = outputListData[0].size();

		for (int i = 0; i < outputListData.size(); i++)
		{
			for (int j = 0; j < columnOutput; j++)
			{
				ofs << outputListData[i][j];
				if (j != columnOutput - 1) ofs << "\t";//デリミタ
			}
			ofs << endl;
		}
		cout << outputFilePath << "に書き込みました" << endl << endl;

		//次のファイルの解析のために、リストデータに使用したメモリをクリア
		inputListBuf.clear();
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
		ui frameNum = inputListData[i][0];
		int x = inputListData[i][1];
		int y = inputListData[i][2];
		ui brightness = inputListData[i][3];
		int q = inputListData[i][4];

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

//輝点が3つのものをoutputListDataにpush_back
void extract3Points()
{
	int frameNum = intermediateListData[0][0];//今見ているフレームのインデックス
	int k = 0;//今見ている行のインデックス

	//リストデータの最後まで1つずつ見ていく
	while (k < intermediateListData.size())
	{
		vector<vector<int>> tmp;// 1フレーム分の部分配列
		frameNum = intermediateListData[k][0];
		tmp.push_back(intermediateListData[k]);

		//frame番号が変わるところまでで部分配列を抜き出す(tmp)
		while ((k + 1 < intermediateListData.size()) && (intermediateListData[k][0] == intermediateListData[k + 1][0]))
		{
			tmp.push_back(intermediateListData[k + 1]);
			k++;
		}

		//輝点の数が2つのフレームについてのみ解析を行う
		int pointNum = tmp.size();
		if (pointNum == 3)
		{
			//outputListDataにpush_back
			for (int i = 0; i < 3; i++)
			{
				outputListData.push_back(tmp[i]);
			}
		}
		k++;
	}
}

string upperPath(string fullPath)
{
	int path_i = fullPath.find_last_of("\\");
	string upperPath = fullPath.substr(0, path_i);//最後の'\'は含まない
	return upperPath;
}

bool readFilename(string fnamePath)
{
	//パラメータが有効かどうかのフラグ
	bool isValid = false;

	string filepath = fnamePath;
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

		//'='以降のパラメータをtempに格納
		string temp;
		ss >> temp;

		if (name == "filename")
		{
			isValid = true;
			inputDataName = temp;
		}
	}
	return isValid;
}