// MCS6A_rescaleTOF.cpp : TOFのchannelのスケールを変化させる（定数倍する）
//

//アップデートログ
//2019.8.23:filename.txtから自動でファイル名を読み込ませるようにした(3つ上の階層)
//2019.10.9:filename.txtについてコメントを追加
//2020.2.12:rescaleフォルダを自動で生成するようにした
//2020.2.21:"scale"の呼び方を"rescale"に変更。ファイル名も"dataname+_rescale.txt"に変更
//2020.7.9:VS Codeに移植

#include "./library.hpp"

//TOFのchnにかける定数
double scale = 1;

vector<vector<ui>> inputListData;
vector<vector<int>> outputListData;
string inputDataName;

bool readParameter(string);
void scaleTOF();
string upperPath(string);
bool readFilename(string);

int main()
{
	cout << "*******************************************************************************" << endl;
	cout << "****                -MCS6A_scaleTOF.cpp-(serial data only)                 ****" << endl;
	cout << "****                     TOFのリストデータchnを定数倍する                  ****" << endl;
	cout << "****  ※rescale file は入力ファイルディレクトリに「ファイル名_rescale.txt」****" << endl;
	cout << "****                                       の名前であらかじめ作成しておく  ****" << endl;
	cout << "****              \"filename.txt\"ファイルは2つ上の階層に作っておく          ****" << endl;
	cout << "****                                  ver.2020.2.21 written by R. Murase   ****" << endl;
	cout << "*******************************************************************************" << endl << endl;

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

	//入力パラメータパスの生成
	string parameterFilePath = inputDataFolder + "\\" + inputDataName + "_rescale.txt";
	ifstream ifs2(parameterFilePath);
	if (!ifs2)
	{
		cerr << "パラメータファイルを開けませんでした" << endl;
		system("pause");
		return -1;
	}
	else if (!readParameter(parameterFilePath))//パラメータの読み取り
	{
		cerr << "パラメータファイルが不正です" << endl;
		system("pause");
		return -1;
	}

	//出力フォルダの指定
	string outputFolderName;
	cout << "\n出力するフォルダのパスを入力してください (\"@\"入力で同じフォルダ内の\"rescaled\"フォルダに保存(なければ自動作成))\n--->";
	cin >> outputFolderName;
	if (outputFolderName == "@")
	{
		outputFolderName = inputDataFolder + "\\rescaled";
		_mkdir(outputFolderName.c_str());
	}

	//連番処理開始
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
			system("pause");
			return -1;
		}
		//出力データパスの生成
		string outputFilePath = outputFolderName + "\\" + inputDataName + suf + ".txt";

		//リストデータを1行ずつ読み取り、\tで分けてinputListDataにpush_back
		string tmp;
		while (getline(ifs, tmp))
		{
			stringstream ss(tmp);
			string tmp2;
			vector<ui> inputLine;
			while (ss >> tmp2)
			{
				inputLine.push_back(stoi(tmp2));
			}
			inputListData.push_back(inputLine);
		}
		cout << "----データの読み込み完了、処理を開始します。" << endl;

		//chn
		scaleTOF();

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
		inputListData.clear();
		outputListData.clear();
	}
	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
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

		double temp;
		ss >> temp;
		if (name == "scale")
		{
			scale = temp;
		}
		else
		{
			isValid = false;
		}
	}
	return isValid;
}

void scaleTOF()
{
	for (int i = 0; i < inputListData.size(); i++)
	{
		ui sweepNum = inputListData[i][0];
		ui MCSchn = inputListData[i][1];
		ui chn = inputListData[i][2];
		//scale分だけchnにかける
		chn *= scale;

		vector<int> outputLine;
		outputLine.push_back(sweepNum);
		outputLine.push_back(MCSchn);
		outputLine.push_back(chn);

		outputListData.push_back(outputLine);
	}
}

string upperPath(string fullPath)
{
	int path_i = fullPath.find_last_of("\\");
	string upperPath = fullPath.substr(0, path_i);
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