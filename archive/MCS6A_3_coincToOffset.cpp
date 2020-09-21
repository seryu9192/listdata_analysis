// MCS6A_offsetTOF.cpp : TOFのリストデータに入射ビームの速度に応じたoffsetをかける。
//

//アップデートログ
//2019.8.1:出力ディレクトリ名を@入力時に自動で指定できるようにした
//2019.8.8:filename.txtから自動でファイル名を読み込ませるようにした
//2020.7.9:VS Codeに移植

#include "./library.hpp"

//TOFのオフセットするchn
int offset = 0;
int binWidth = 8;
int offsetAct = offset * binWidth;

vector<vector<ui>> inputListData;
vector<vector<int>> outputListData;
string inputDataName;

bool readParameter(string);
void offsetTOF();

int main()
{
	cout << "*****************************************************************************" << endl;
	cout << "****               -MCS6A_offsetTOF.cpp-(combined data only)             ****" << endl;
	cout << "****       TOFのリストデータに入射ビームの速度に応じたoffsetをかける     ****" << endl;
	cout << "****   ※Offset file は入力ファイルディレクトリに「ファイル名offset.txt」****" << endl;
	cout << "****                                     の名前であらかじめ作成しておく  ****" << endl;
	cout << "****                               ver.2020.07.16 written by R. Murase   ****" << endl;
	cout << "*****************************************************************************" << endl << endl;

	string inputDataFolder,inputDataPath;
	cout << "解析するリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder;

	//datanameを読み取り
	string datanamePath = searchFileFromUpperFolder(inputDataFolder, DATANAME_FILE);

	if (datanamePath == "")
	{
		cout << "dataname.txtファイルが見つかりませんでした\n";
		cout << "解析するリストデータのファイル名を入力してください\n--->";
		cin >> inputDataName;
	}
	if (readDataName(datanamePath) == "")
	{
		cout << "dataname.txtファイルが不正です\n";
		cout << "解析するリストデータのファイル名を入力してください\n--->";
		cin >> inputDataName;
	}
	else
	{
		inputDataName = readDataName(datanamePath);
		cout << "dataname.txtファイルを読み込みました\n";
		cout << "解析するリストデータのファイル名：" << inputDataName << endl;
	}

	//入力データパスの生成
	inputDataPath = inputDataFolder + "\\" + inputDataName + ".txt";
	ifstream ifs(inputDataPath);
	if (!ifs)
	{
		cerr << "ファイルを開けませんでした" << endl;
		system("pause");
		return -1;
	}

	//入力パラメータパスの生成
	string parameterFilePath = inputDataFolder + "\\" + inputDataName + "offset.txt";
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
	cout << "\n出力するフォルダのパスを入力してください (\"@\"入力で同じフォルダ内の\"3_offset\"に保存)\n--->";
	cin >> outputFolderName;
	if (outputFolderName == "@")
	{
		outputFolderName = upperPath(inputDataFolder) + "\\3_offset";
	}

	//出力データパスの生成
	string outputFilePath = outputFolderName + "\\" + inputDataName + ".txt";

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
	offsetTOF();

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

		int temp;
		ss >> temp;
		if (name == "offset")
		{
			offset = temp;
		}
		else if (name == "binWidth")
		{
			binWidth = temp;
		}
		else
		{
			isValid = false;
		}
	}
	if (isValid)
	{
		offsetAct = offset * binWidth;
	}
	return isValid;
}

void offsetTOF()
{
	for (int i = 0; i < inputListData.size(); i++)
	{
		ui fileNum = inputListData[i][0];
		ui sweepNum = inputListData[i][1];
		ui MCSchn = inputListData[i][2];
		ui chn = inputListData[i][3];
		//offset分だけchnに足す
		chn += offsetAct;

		vector<int> outputLine;
		outputLine.push_back(fileNum);
		outputLine.push_back(sweepNum);
		outputLine.push_back(MCSchn);
		outputLine.push_back(chn);

		outputListData.push_back(outputLine);
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