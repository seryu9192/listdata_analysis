// Imaging_extractByPointNum.cpp : イメージングデータを、1枚に映っている輝点の数で抽出する
//

//アップデートログ
//2019.12.12:作成
//2020.7.9:VS Codeに移植

#include "./library.hpp"

vector<vector<int>> inputListData;
vector<vector<int>> outputListData;

string inputDataName;
int numOfPointsToExtract;

void extractWithPointNum();
string upperPath(string);
bool readFilename(string);

int main()
{
	cout << "**************************************************************" << endl;
	cout << "****          Imaging_extractByPointNum.cpp               ****" << endl;
	cout << "****      -Imaging 1枚に移っている輝点の数で              ****" << endl;
	cout << "****             リストデータを分解するプログラム-        ****" << endl;
	cout << "****                ver.2019.12.12 written by R. Murase   ****" << endl;
	cout << "**************************************************************" << endl << endl;

	string inputDataFolder, inputDataPath;
	cout << "解析するリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder;

	string filenamePath = upperPath(upperPath(inputDataFolder)) + "\\filename.txt";

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

	cout << "フレームあたりの輝点の数を入力してください" << endl;
	cin >> numOfPointsToExtract;
	cout << "\n1枚あたりの輝点の数 : " << numOfPointsToExtract <<"のフレームを抽出します。"<< endl;
	
	//extracted用のフォルダをまず作成
	string extracted_dir = inputDataFolder + "\\points="+to_string(numOfPointsToExtract);
	cout << extracted_dir + "フォルダを作成し、そこに保存します。" << endl;
	_mkdir(extracted_dir.c_str());


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
		string outputFilePath = extracted_dir + "\\" + inputDataName + suf + ".txt";

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

		extractWithPointNum();

		cout << "----処理終了。ファイルに書き込んでいます..." << endl;

		//出力ファイルにoutputListDataを書き込み
		ofstream ofs(outputFilePath);
		int columnOutput = outputListData[0].size();

		//double型でイベント番号を扱う時、イベント数が1,000,000を超えるとデフォルト(6桁)では桁落ちするので、書き込み精度を7桁にする
		ofs << setprecision(7);

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
		inputListData.clear();
		outputListData.clear();
	}
	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;

}

void extractWithPointNum()
{
	int frameNum = inputListData[0][0];//今見ているフレームのインデックス
	int k = 0;//今見ている行のインデックス

	//リストデータの最後まで1つずつ見ていく
	while (k < inputListData.size())
	{
		vector<vector<int>> tmp;// 1フレーム分の部分配列
		frameNum = inputListData[k][0];
		tmp.push_back(inputListData[k]);

		//frame番号が変わるところまでで部分配列を抜き出す(tmp)
		while ((k + 1 < inputListData.size()) && (inputListData[k][0] == inputListData[k + 1][0]))
		{
			tmp.push_back(inputListData[k + 1]);
			k++;
		}

		//輝点の数が、最初に設定したフレームあたりの輝点の数と一致したら、outputListDataに出力
		int pointNum = tmp.size();
		if (pointNum == numOfPointsToExtract)
		{
			for (int i = 0; i < pointNum; i++)
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