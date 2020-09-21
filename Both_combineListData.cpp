// Both_combineListData.cpp : 複数のリストデータのファイルを、ファイル番号を0列目にして1つのファイルにまとめる
//

//アップデートログ
//2019.8.8:filename.txtから自動でファイル名を読み込ませるようにした
//2019.10.9:ファイルを出力するときに、イベント番号が1,000,000を超えるとデフォルトのdouble型では桁落ちするので、setprecision関数で精度を7桁に設定した
//2020.1.17:ファイルを書き込むときに書き込みモードを指定できるようにした(新規書き込みモード/追加書き込みモード)
//2020.2.20:filename.txtを2つ上と3つ上のフォルダから探すようにした（これまでは2つ上のフォルダのみ）
//2020.2.21:"non_valid"の呼び方を"invalid"にした
//2020.7.9:VS Codeに移植
//2020.7.16:"library.hpp"を導入
//2020.7.20:"invalid.txt"を読み込む場所を変える
//2020.7.28:writing_modeをstart番号で判断するようにする。start == 1 なら新規書き込み、それ以外は追加書き込み
//2020.8.11:外から自動で動かせるように、inputDataFolder, start, endをコマンドライン引数から読み取れるようにした
//2020.9.17:入力ファイルが見つからなかった時の処理をreturn -1 から continueに変更

#include "./library.hpp"

vector<vector<double>> inputListData;
vector<vector<double>> outputListData;
string inputDataName;

//ナンバリングがずれていて使えないデータを除外するためのファイル番号リスト
vector<int> invalidList;//実験時にナンバリングがずれていたり使えなかったファイル#

void addList(int);

int main(int argc, char* argv[])
{
	cout << "******************************************************************" << endl;
	cout << "****             　  Both_combineListData.cpp                 ****" << endl;
	cout << "****                -リストデータ結合プログラム-              ****" << endl;
	cout << "****                  複数ファイルのリストデータを            ****" << endl;
	cout << "****      0列目をファイル番号にして 一つのファイルに結合する  ****" << endl;
	cout << "****     ※使えないファイルがある場合は\"invalid.txt\"を参照     ****" << endl;
	cout << "****                    ver.2020.09.17 written by R. Murase   ****" << endl;
	cout << "******************************************************************" << endl << endl;

	string inputDataFolder, inputDataPath;
	//ファイル番号
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

	//出力データパスの生成
	string outputFilePath = inputDataFolder + "\\" + inputDataName + ".txt";

	//書き込みモードの設定(start == 1:新規書き込み, else:追加書き込み)
	if (start == 1)
		//新規書き込みモード
		cout << "新規書き込みモードで書き込みます" << endl;
	else
		//追加書き込みモード
		cout << "追加書き込みモードで書き込みます" << endl;

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

	//連番処理開始
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
			cerr << inputDataName << "---ファイルを開けませんでした" << endl;
			continue;
		}
		//リストデータを1行ずつ読み取り、\tで分けてinputListDataにpush_back
		cout << inputDataName + suf + ".txt" + "----データを読み込んでいます...";
		inputListData = readListFile<double> (inputDataPath);
		cout << "読み込み完了 -> 処理中...";

		//outputListDataに、0列目にファイル番号を挿入したリストデータを追加
		addList(m);
		cout << "処理終了" << endl;

		//次のファイルの解析のために、inputリストデータに使用したメモリをクリア
		inputListData.clear();
	}

	cout << "----全てのデータの処理終了。書き込んでいます...." << endl;

	//outputListDataを書き込み
	cout << outputFilePath << "に書き込んでいます...";
	if(start == 1)
		writeListFile<double> (outputFilePath, outputListData);
	else
		writeListFile<double> (outputFilePath, outputListData, '\t', true);
	cout << "書き込みました" << endl;
	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	return 0;
}

//0列目にファイル番号を挿入し、outputListDataに追加
void addList(int m)
{
	for (int i = 0; i < inputListData.size(); i++)
	{
		//ouputListData1行を構成
		vector<double> outputLine;

		//ファイル番号を0列目に挿入
		outputLine.push_back(m);
		for (int j = 0; j < inputListData[i].size(); j++)
		{
			outputLine.push_back(inputListData[i][j]);
		}
		//outputListDataに追加
		outputListData.push_back(outputLine);
	}
}

