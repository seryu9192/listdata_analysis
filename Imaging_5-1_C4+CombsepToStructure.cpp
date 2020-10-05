// Imaging_5-1_C4+CombsepToStructure.cpp : skewerNotationデータとコインシデンスしたImagingイベントを抽出(分別できるstructureの数だけ)
//

//アップデートログ
//2020.7.31:Imaging_5-1_C3+CombsepToTriLin.cppから作成
//2020.9.18:外から別のプログラムで動かせるように、コマンドライン引数から入力フォルダパスを受け取れるようにする
//2020.9.20:リストデータのうち、特定のファイル番号についてのみ処理を行うため、readListFileを変更(int col, T keyMin, T keyMax)を引数に追加
//			keyMin,keyMaxに渡すためにstart, end変数を定義。コマンドライン引数から受け取れるようにする
//			書き込み時のwriteListFile関数を、start == 1の時は新規、それ以外は追加で書き込むようにする
//2020.9.20:Imaging_5-1_C4+CombsepToLin -> Imaging_5-1_C4+CombsepToStructureに改名(一般的な構造に対応)
//2020.9.20:linear以外の構造にも対応できるように入力フォルダ名を配列にする


#include "./library.hpp"

//リストデータの列値
const int COL_FILENUM = 0;
const int COL_EVENTNUM = 1;

//SN_ROOTFOLDER(skewer notation folder)
const string SN_ROOTFOLDER = "\\5_SkewerNotation\\";
//vector<string> sn_names = {"5-1_Linear", "5-2_Circular"};
vector<string> sn_names = {"5-2_Circular"};
//outputfolderName
//vector<string> outputFolderNames = {"\\6_Linear", "\\6_Circular"};
vector<string> outputFolderNames = {"\\6_Circular"};

template <typename T = int, typename U = int>
vector<vector<T>> extractEvents(vector<vector<T>>, vector<vector<U>>);

int main(int argc, char* argv[])
{
	cout << "**************************************************************" << endl;
	cout << "****          Imaging_5-1_C4+CombsepToLin.cpp             ****" << endl;
	cout << "****     -SkewerNotationイベントとコインシデンスした      ****" << endl;
	cout << "****               Imagingイベントを抽出-                 ****" << endl;
	cout << "****                 ver.2020.09.20 written by R. Murase  ****" << endl;
	cout << "**************************************************************" << endl << endl;

	//入力イメージングデータパスの生成
	string inputDataFolder_img;
	int start, end;
	//別のプログラムから走らせる用にコマンドライン引数を定義
	//従来通り手動で走らせる
	if(argc == 1)
	{
		cout << "解析するリストデータのフォルダを入力してください\n--->";
		cin >> inputDataFolder_img;
		cout << "解析するファイル番号(start,end)を入力してください" << endl;
		cout << "start : ";
		cin >> start;
		cout << "end : ";
		cin >> end;
	}
	//コマンドライン引数で読み取る
	else if(argc == 4)
	{
		inputDataFolder_img = argv[1];
		start = stoi(argv[2]);
		end =  stoi(argv[3]);

		cout << "コマンドライン引数から読み取りました" << endl;
		cout << "inputDataFolder : " << inputDataFolder_img << endl;
		cout << "start = " << start << endl;
		cout << "end = "<< end << endl;
	}
	else
	{
		cerr << "コマンドライン引数の数が不正です" << endl;
		system("pause");
		return -1;
	}

	//structureの数だけループ
	for (int i = 0; i < sn_names.size(); i++)
	{	
		//snフォルダ
		string inputDataFolder_sn = upperFolder(inputDataFolder_img) + SN_ROOTFOLDER + sn_names[i];
		cout << "SkewerNotation フォルダ：" + inputDataFolder_sn << endl;

		//入力イメージングデータフォルダ内のファイルの名前を取得
		vector<string> fNames = getFileNameInDir(inputDataFolder_img);

		//出力データパスの生成
		string outputDataFolder = upperFolder(inputDataFolder_img) + outputFolderNames[i];
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
			auto inputListData_img = readListFile<double>(inputDataPath_img, COL_FILENUM, start, end+1);
			cout << "読み込み完了 --> ";

			string inputDataPath_sn = inputDataFolder_sn + "\\" + inputFilename;
			cout << "SkewerNotationデータ読み込み中...";
			auto inputListData_tn = readListFile<double>(inputDataPath_sn, COL_FILENUM, start, end+1);
			cout << "読み込み完了 --> 処理開始...";
			
			//コインシデンスしているイベントを抽出
			auto outputListData = extractEvents(inputListData_img, inputListData_tn);

			//ファイルに書き出し
			string outputFilePath = outputDataFolder + "\\" + inputFilename;
			//start == 1の時は新規、それ以外はappend
			bool append = true;
			if (start == 1)append = false;
			writeListFile<double>(outputFilePath, outputListData, '\t', append);
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