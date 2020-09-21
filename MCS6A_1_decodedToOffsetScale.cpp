// MCS6A_3_coincToOffsetScale.cpp : TOFのリストデータに入射ビームの速度に応じたoffsetをかける。
//

//アップデートログ
//2019.8.1:出力ディレクトリ名を@入力時に自動で指定できるようにした
//2019.8.8:filename.txtから自動でファイル名を読み込ませるようにした
//2020.7.9:VS Codeに移植
//2020.7.19:MCS6A_offsetTOF.cpp -> MCS6A_3_coincToOffsetScale.cppに改名
//2020.7.19:tofのscaleを変える機能を追加(chn' = a * chn + b)
//2020.7.19:"MCS6A_3_coincToOffsetScale.cpp" -> "MCS6A_2_stopToOffsetScale.cpp"に改名
//2020.7.21:出力フォルダ名を"2_offset"に変更
//2020.7.21:外から自動で動かせるように、inputDataFolder, start, endをコマンドライン引数から読み取れるようにした
//

#include "./library.hpp"

//TOFのoffset/scaleするchn
int offset = 0;
double scale = 1;
int binWidth = 8;
int offsetAct = offset * binWidth;

//リストデータの列番号
const int COL_SWEEP = 0;
const int COL_CHN = 1;
const int COL_TOF = 2;

//outputfolderName
const string OUTPUT_FOLDER_NAME = "2_offsetScale";

vector<vector<int>> inputListData;
vector<vector<int>> outputListData;
string inputDataName;

bool readParameter(string);
void offsetTOF();

int main(int argc, char* argv[])
{
	cout << "*****************************************************************************" << endl;
	cout << "****         -MCS6A_1_decodeToOffsetScale.cpp-(serial data only)         ****" << endl;
	cout << "****       TOFのリストデータに入射ビームの速度に応じたoffsetをかける     ****" << endl;
	cout << "****    ※Offset file は入力ファイルディレクトリに「ファイル名offset.txt」****" << endl;
	cout << "****                                     の名前であらかじめ作成しておく  ****" << endl;
	cout << "****                               ver.2020.07.21 written by R. Murase   ****" << endl;
	cout << "*****************************************************************************" << endl << endl;

	string inputDataFolder,inputDataPath;
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




	//入力パラメータパスの生成
	string parameterFilePath = inputDataFolder + "\\" + inputDataName + "offset.txt";
	ifstream ifs2(parameterFilePath);
	if (!ifs2)
	{
		cerr << "offsetファイルを開けませんでした" << endl;
		system("pause");
		return -1;
	}
	else if (!readParameter(parameterFilePath))//パラメータの読み取り
	{
		cerr << "offsetファイルが不正です" << endl;
		system("pause");
		return -1;
	}

	//TOFのオフセットするchn
	cout << "\noffset : "<< offset << endl;
	cout << "binWidth : " << binWidth << endl;
	cout << "scale : " << scale << endl;
	cout << "offsetAct( = offset * binWidth) : "  << offsetAct << endl;

	//出力フォルダの指定
	string outputFolderPath;
	cout << "\n同じフォルダ内の\"" + OUTPUT_FOLDER_NAME + "\"に保存します\n--->";
	outputFolderPath = upperFolder(inputDataFolder) + "\\" + OUTPUT_FOLDER_NAME;
	_mkdir(outputFolderPath.c_str());

	//連番処理
	for(int m = start; m <= end; m++)
	{
		//suffix
		string suf = toSuffix(m);

		//入力データパスの生成
		inputDataPath = inputDataFolder + "\\" + inputDataName  + suf + ".txt";
		if (!checkFileExistence(inputDataPath))
		{
			cerr << inputDataPath  <<"---ファイルを開けませんでした" << endl;
			system("pause");
			continue;
		}

		//リストデータを1行ずつ読み取り、\tで分けてinputListDataにpush_back
		cout <<  inputDataPath + "----データを読み込んでいます...";
		inputListData = readListFile<int> (inputDataPath);
		cout << "データの読み込み完了、処理を開始します...";

		//chn' = a * chn + b
		offsetTOF();

		cout << "----処理終了" << endl;

		//出力ファイルにoutputListDataを書き込み
		string outputFilePath = outputFolderPath + "\\" + inputDataName + suf +".txt";
		cout << outputFilePath << "に書き込んでいます...";
		writeListFile<int> (outputFilePath, outputListData);
		cout << "書き込みました" << endl;
		
		//次のファイルのためにデータをクリア
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

		double tmp;
		ss >> tmp;
		if (name == "offset")
		{
			offset = tmp;
		}
		else if (name == "scale")
		{
			scale= tmp;
		}
		
		else if (name == "binWidth")
		{
			binWidth = tmp;
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
		int sweepNum = inputListData[i][COL_SWEEP];
		int chn = inputListData[i][COL_CHN];
		int tof = inputListData[i][COL_TOF];

		//scaleかけて、offset足す
		tof *= scale;
		tof += offsetAct;

		vector<int> outputLine;
		outputLine.push_back(sweepNum);
		outputLine.push_back(chn);
		outputLine.push_back(tof);

		outputListData.push_back(outputLine);
	}
}

