// Imaging_5-1_C3+CombsepToTriLin.cpp : TriangleNotationデータとコインシデンスしたImagingイベントを抽出
//

//アップデートログ
//2020.7.27:作成
//2020.9.18:入力フォルダ名をコマンドライン引数から読み取れるようにする

#include "./library.hpp"

//リストデータの列値
const int COL_FILENUM = 0;
const int COL_EVENTNUM = 1;

//TN_FOLDER(Triangle notation folder)
const vector<string> SN_FOLDERS = {"\\5_TriangleNotation\\5-2_Triangle", "\\5_TriangleNotation\\5-1_Linear"};

//outputfolderName
const vector<string> OUTPUT_FOLDERNAMES = {"\\6_Triangle", "\\6_Linear"};

template <typename T = int, typename U = int>
vector<vector<T>> extractEvents(vector<vector<T>>, vector<vector<U>>);

int main(int argc, char* argv[])
{
	cout << "**************************************************************" << endl;
	cout << "****          Imaging_5-1_C3+CombsepToTriLin.cpp          ****" << endl;
	cout << "****     -TriangleNotationイベントとコインシデンスした    ****" << endl;
	cout << "****               Imagingイベントを抽出-                 ****" << endl;
	cout << "****                 ver.2020.09.18 written by R. Murase  ****" << endl;
	cout << "**************************************************************" << endl << endl;

	//入力イメージングデータパスの生成
	string inputDataFolder_img;
	//別のプログラムから走らせる用にコマンドライン引数を定義
	//従来通り手動で走らせる
	if(argc == 1)
	{
		cout << "解析するリストデータのフォルダパスを入力してください\n--->";
		cin >> inputDataFolder_img;
	}
	//コマンドライン引数で読み取る
	else if(argc == 2)
	{
		inputDataFolder_img = argv[1];
		cout << "コマンドライン引数から読み取りました" << endl;
		cout << "inputDataFolder : " << inputDataFolder_img << endl;
	}
	else
	{
		cerr << "コマンドライン引数の数が不正です" << endl;
		system("pause");
		return -1;
	}

	//triangle, linearについて処理
	for (int i = 0; i < SN_FOLDERS.size(); i++)
	{		
		//tnフォルダ
		string inputDataFolder_tn = upperFolder(inputDataFolder_img) + SN_FOLDERS[i];
		cout << "TriangleNotation フォルダ：" + inputDataFolder_tn << endl;

		//入力イメージングデータフォルダ内のファイルの名前を取得
		vector<string> fNames = getFileNameInDir(inputDataFolder_img);

		//出力データパスの生成
		string outputDataFolder = upperFolder(inputDataFolder_img) + OUTPUT_FOLDERNAMES[i];
		if (_mkdir(outputDataFolder.c_str()) == 0)
		{
			cout << "新たなフォルダを作成し出力ファイルを保存します。" << endl;
		}
		else
		{
			cout << "既存のフォルダに出力ファイルを保存します。" << endl;
		}

		//入力ファイルごとにループ
		for (auto inputFilename : fNames)
		{
			string inputDataPath_img = inputDataFolder_img + "\\" + inputFilename;
			cout << inputFilename + " ---Combseparatedデータ読み込み中...";
			auto inputListData_img = readListFile<double>(inputDataPath_img);
			cout << "読み込み完了 --> ";

			string inputDataPath_tn = inputDataFolder_tn + "\\" + inputFilename;
			cout << "TNデータ読み込み中...";
			auto inputListData_tn = readListFile<double>(inputDataPath_tn);
			cout << "読み込み完了 --> 処理開始...";
			
			//コインシデンスしているイベントを抽出
			auto outputListData = extractEvents(inputListData_img, inputListData_tn);

			//ファイルに書き出し
			string outputDataPath = outputDataFolder + "\\" + inputFilename;
			writeListFile<double>(outputDataPath, outputListData);
			cout << "処理完了！" << endl;
		}
	}
	
	cout << "全ての処理完了。プログラムを終了します" << endl;
	return 0;
}

//コインシデンスしたデータを抽出
template <typename T = int, typename U = int>
vector<vector<T>>extractEvents(vector<vector<T>> dataToResolve, vector<vector<U>> dataToRef)
{
	vector<vector<T>> res;
	if (dataToRef.empty())return res;

	//dataToResolveについて、線形探索
	for(int i = 0; i < dataToResolve.size(); i++)
	{
		int resolve_fileNum = int(dataToResolve[i][COL_FILENUM]);
		int resolve_eventNum = int(dataToResolve[i][COL_EVENTNUM]);
		//dataToRefについて二分探索(0列目：ファイル番号、1列目：イベント番号)
		int left = 0;
		int right = dataToRef.size() - 1;
		int mid, ref_fileNum, ref_eventNum;

		while (left <= right)
		{
			mid = (left + right) / 2;
			ref_fileNum = int(dataToRef[mid][COL_FILENUM]);
			ref_eventNum = int(dataToRef[mid][COL_EVENTNUM]);
			if (ref_fileNum == resolve_fileNum && ref_eventNum == resolve_eventNum)
			{
				res.push_back(dataToResolve[i]);
				break;
			}
			//fileNumの大小から評価
			else if (ref_fileNum < resolve_fileNum)
				left = mid + 1;
			else if (ref_fileNum > resolve_fileNum)
				right = mid - 1;
			//fileNumが等しかったら、eventNumの大小で評価
			else if (ref_eventNum < resolve_eventNum)
				left = mid + 1;
			else
				right = mid - 1;
		}
	}
	return res;
}