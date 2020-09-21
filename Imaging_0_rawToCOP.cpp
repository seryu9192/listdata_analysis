// Imaging_0_rawToCOP.cpp : Imagingの輝点リストデータ解析プログラム(8portsに分割された生データを処理する用)
//

//アップデートログ
//2020.3.25:作成、イメージングの生データをCOP処理せずに抽出するために作成
//2020.6.11:library.hppから関数を読み取るようにした
//2020.6.11:出力用の"COP"フォルダを自動生成するようにした
//2020.6.17:"solveOverflow","sortWithFrame","quicksort","convPortToXY","CoordinateOfPoint"関数の戻り値ありバージョンを作成（これまでのバージョンにオーバーロード）
//2020.6.17:データの変換をバッチ処理方式にする。（一気に変換するとデータ量が多すぎてbad_allocエラーになる）NUM_EVENTS_PER_BATCH 個ずつリストデータを処理してoutputする
//2020.6.17:COL_FRAMENUMの導入
//2020.7.9:VS Codeに移植
//2020.7.10:Imaging_ListDataAnalysis_8port_cpy -> Imaging_0_rawToCOPに改名
//2020.7.12:"solveOverflow","sortWithFrame","quicksort","convPortToXY","CoordinateOfPoint"を戻り値なしのバージョンに戻す(なぜかbad_allocエラーになるため)
//2020.7.17:出力フォルダを固定にする
//2020.7.21:外から自動で動かせるように、inputDataFolder, start, endをコマンドライン引数から読み取れるようにした
//2020.9.16:NUM_EVENTS_PER_BATCH : 1000000 -> 500000に変更
//2020.9.16:THRESHOLD  : 100 -> 25に変更(計測VIのものに合わせた)
//2020.9.18:outputListDataを、最後にまとめて書き込む方式から、batchごとに書き込む方式に変更
//2020.9.19:NUM_EVENTS_PER_BATCH : 500000 -> 400000に変更
//2020.9.20:読み取り時に、overflowを直しながらバッチごとに指定された範囲のみinputListBufに取り込むようしたreadListFile関数に変更

#include "./library.hpp"

//点の明るさの閾値
constexpr auto THRESHOLD = 25;
constexpr auto COL_FRAMENUM = 0;

//COPにする際に1つの点としてみなすpixelの範囲（これより近い複数の輝点は１点として見なされる）
constexpr auto ELIM_X = 5;
constexpr auto ELIM_Y = 5;

//一回のバッチで処理するイベントの個数
constexpr auto NUM_EVENTS_PER_BATCH = 400000;

//outputFolderPath
const string OUTPUT_FOLDER_NAME = "\\1_COP";

//port毎にoverflowを解消するときに使うbuffer
vector<vector<ui>> inputListBuf;
vector<vector<ui>> outputListBuf;

//overflow以降、portをすべて足し合わせたデータを保存しておく変数
vector<vector<ui>> inputListData;
vector<vector<ui>> midListData;
vector<vector<ui>> outputListData;

//イベント番号のオーバフローを直す(もともと16bitなので）
void solveOverflow();
vector<vector<ui>> solveOverflow(vector<vector<ui>>);

//もともとport番号毎に並んでいるので、frame番号でsortする（quicksortアルゴリズム）
void sortWithFrame();
vector<vector<ui>> sortWithFrame(vector<vector<ui>>);

void quicksort(int, int);
vector<vector<ui>> quicksort(vector<vector<ui>>, int, int);

//3つの数字の中央値を返す(quicksortに利用)
int med3(int, int, int);

//（ポート番号, 信号の通し番号）の形式を（X, Y)に変換する
void convPortToXY();
vector<vector<ui>> convPortToXY(vector<vector<ui>>);

// COP処理
void coordinateOfPoint();
vector<vector<ui>> coordinateOfPoint(vector<vector<ui>>);

int main(int argc, char* argv[])
{
	cout << "************************************************************" << endl;
	cout << "**          Imaging リストデータ解析プログラム            **" << endl;
	cout << "**     PCIe1473R Imaging輝点ピクセル処理 for 8 port files **" << endl;
	cout << "**   0:overflowしたデータを直す(port毎),1:frameNumでsort  **" << endl;
	cout << "**   2:port/pxNumをX/Yに変換, 3:CoordinateOfPoint         **" << endl;
	cout << "**                 NUM_PER_BATCH = " << NUM_EVENTS_PER_BATCH << "                 **" << endl;
	cout << "**                    ver.2020.09.20 written by R. Murase **" << endl;
	cout << "************************************************************" << endl << endl;

	string inputDataFolder, inputDataName, inputDataPath;
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

	//入力フォルダの名前を
	inputDataName = folderName(inputDataFolder);

	//出力フォルダの指定
	cout << "\n入力フォルダと同じフォルダ内の\"COP\"に保存します(存在しなければ自動生成)\n";
	string outputFolderPath = inputDataFolder + OUTPUT_FOLDER_NAME;
	_mkdir(outputFolderPath.c_str());
	
	//ファイル番号について連番処理
	for (int m = start; m <= end; m++)
	{
		//file indexを0詰め3桁の文字列suffixに変換
		string suf = toSuffix(m);

		//出力データパスの生成
		string outputFilePath = outputFolderPath + "\\" + inputDataName + suf + ".txt";
		cout << inputDataName + suf + ": 処理を開始します" << endl;

		//バッチ処理を開始
		int bat = 0;
		int eventMax = 0;
		while (true)
		{
			//今のバッチで読み取るイベント番号の範囲
			int bstart = bat * NUM_EVENTS_PER_BATCH;
			int bend = (bat+1) * NUM_EVENTS_PER_BATCH;

			cout << "batch num : " << bat + 1 << " 処理開始..." << endl;

			//portの数だけループを回してファイルをimport
			for (int port = 0; port < 8; port++)
			{
				//portを示す(suffix = "_0"など)
				string sufPort = "_" + to_string(port);

				//入力データパスの生成
				inputDataPath = inputDataFolder + "\\" + inputDataName + suf + sufPort + ".txt";
				ifstream ifs(inputDataPath);
				if (!ifs)
				{
					cerr << "ファイルを開けませんでした。プログラムを終了します" << endl;
					system("pause");
					return -1;
				}
				//inputListBuf(2Dvector)を構成(overflowも同時に直して、)
				cout << inputDataName + suf + sufPort + ".txt" + "--- 読み取り中...";
				inputListBuf = readListFile<ui>(inputDataPath, COL_FRAMENUM, bstart, bend);

				//inputListDataに追加
				for (int r = 0; r < inputListBuf.size(); r++)
				{
					int eventNum = inputListBuf[r][COL_FRAMENUM];
					eventMax = max(eventMax, eventNum);
					inputListData.push_back(inputListBuf[r]);
				}
				cout << "読み取り完了" << endl;
				inputListBuf.clear();
			}
			//inputListDataが空なら終わり
			if (inputListData.empty())
				break;
			cout << inputDataName + suf + ".txt----batch : " << bat + 1 << "データの読み込み完了、処理を開始します" << endl;

			//フレーム番号によってソート
			sortWithFrame();
			inputListData = midListData;
			midListData.clear();
			cout << "sorted with frame number" << endl;

			//pixel座標形式をXYに変換
			convPortToXY();
			inputListData = midListData;
			midListData.clear();
			cout << "converted into XY" << endl;

			//COP処理
			coordinateOfPoint();
			cout << "COP" << endl;

			//出力ファイルにoutputListDataを書き込み
			outputListData = midListData;
			cout << inputDataName + suf + ".txt----処理終了。ファイルに書き込んでいます...";
			if (bat == 0)
			{
				writeListFile<ui>(outputFilePath, outputListData);
				cout << "batch : " << bat+1 << "--- 新規で書き込みました" << endl;
				cout << "outputListDataは" << outputListData.size() << " 行でした" << endl;
			}
			else
			{
				writeListFile<ui>(outputFilePath, outputListData, '\t', true);
				cout << "batch : " << bat+1 << "--- 追加で書き込みました" << endl;
				cout << "outputListDataは" << outputListData.size() << " 行でした" << endl;
			}
			
			//次のバッチのためクリア
			inputListData.clear();
			midListData.clear();
			outputListData.clear();
			bat++;
		}
		cout << inputDataName + suf << "--- ファイル処理が終了しました\n" << endl;
		//次のファイルの解析のために、リストデータに使用したメモリをクリア
		inputListData.clear();
		outputListData.clear();
	}
	cout << "全ての処理が完了しました。プログラムを終了します。" << endl;
	return 0;
}

//0.NI PCIe1473Rから得られた画像データのframe番号は16bitまでであり、overflowしたデータはまた0から始まるので、二週目以降の画像番号に65536 * nをたす（n:overflow回数）
void solveOverflow()
{
	int lastFrame = 0;//最後に見たframe番号をport毎に記憶
	int overflowCnt = 0;//オーバーフローした回数
	for (int i = 0; i < inputListBuf[0].size(); i++)
	{
		//今のframe番号が前のframe番号より小さかったら、オーバーフローしているのでoverflowCnt++
		if (inputListBuf[i][0] < lastFrame) //portをindexにする
		{
			overflowCnt++;
		}
		lastFrame = inputListBuf[i][0];

		//outputData1行を構成
		vector<ui> outputLine;
		//オーバーフローした回数だけframe番号に2^16=65536だけ足す
		outputLine.push_back(inputListBuf[i][0] + overflowCnt * (int)pow(2, 16));
		outputLine.push_back(inputListBuf[i][1]);
		outputLine.push_back(inputListBuf[i][2]);
		outputLine.push_back(inputListBuf[i][3]);

		//outputListBufに追加
		outputListBuf.push_back(outputLine);
	}
}

//0.NI PCIe1473Rから得られた画像データのframe番号は16bitまでであり、overflowしたデータはまた0から始まるので、二週目以降の画像番号に65536 * nをたす（n:overflow回数）
vector<vector<ui>> solveOverflow(vector<vector<ui>> listData)
{
	int lastFrame = 0;//最後に見たframe番号を記憶
	int overflowCnt = 0;//オーバーフローした回数
	vector<vector<ui>> res; //オーバーフローを解消したデータ
	for (int i = 0; i < listData.size(); i++)
	{
		//今のframe番号が前のframe番号より小さかったら、オーバーフローしているのでoverflowCnt++
		if (listData[i][0] < lastFrame) //portをindexにする
		{
			overflowCnt++;
		}
		lastFrame = listData[i][0];

		//outputData1行を構成
		vector<ui> resLine;
		//オーバーフローした回数だけframe番号に2^16=65536だけ足す
		resLine.push_back(listData[i][0] + overflowCnt * (int)pow(2, 16));
		resLine.push_back(listData[i][1]);
		resLine.push_back(listData[i][2]);
		resLine.push_back(listData[i][3]);

		//resに追加
		res.push_back(resLine);
	}
	return res;
}

//1.NI PCIe1473Rから得られた画像データのframe番号がバラバラなので昇順にソートする
void sortWithFrame()
{
	//quick sortを利用
	quicksort(0, inputListData.size() - 1);

	//ソートされた配列をoutputListData
	midListData = inputListData;
}

//1.NI PCIe1473Rから得られた画像データのframe番号がバラバラなので昇順にソートする
vector<vector<ui>> sortWithFrame(vector<vector<ui>> listData)
{
	//quick sortを利用
	listData = quicksort(listData ,0, listData.size() - 1);

	//ソートされた配列をreturn
	return listData;
}

//quick sort
void quicksort(int left, int right)
{
	if (left < right)
	{
		int i = left, j = right;
		int pivot = med3(inputListData[i][COL_FRAMENUM], inputListData[i + (j - i) / 2][COL_FRAMENUM], inputListData[j][COL_FRAMENUM]);
		while (true)
		{
			while (inputListData[i][COL_FRAMENUM] < pivot)
				i++;
			while (inputListData[j][COL_FRAMENUM] > pivot)
				j--;
			if (i >= j)
				break;
			swap(inputListData[i], inputListData[j]);
			i++;
			j--;
		}
		quicksort(left, i - 1);
		quicksort(j + 1, right);
	}
}

//quick sort
vector<vector<ui>> quicksort(vector<vector<ui>> listData, int left, int right)
{
	if (left < right)
	{
		int i = left, j = right;
		int pivot = med3(listData[i][COL_FRAMENUM], listData[i + (j - i) / 2][COL_FRAMENUM], listData[j][COL_FRAMENUM]);
		while (true)
		{
			while (listData[i][COL_FRAMENUM] < pivot)
				i++;
			while (listData[j][COL_FRAMENUM] > pivot)
				j--;
			if (i >= j)
				break;
			swap(listData[i], listData[j]);
			i++;
			j--;
		}
		listData = quicksort(listData, left, i - 1);
		listData = quicksort(listData, j + 1, right);
	}
	cerr << left << " " << right << endl;
	return listData;
}

//x,y,zの中間値
int med3(int x, int y, int z)
{
	if (x < y)
	{
		if (y < z)
			return y;
		else if (z < x)
			return x;
		else
			return z;
	}
	else
	{
		if (z < y)
			return y;
		else if (x < z)
			return x;
		else
			return z;
	}
}

//2.NI PCIe1473Rから得られた画像データは、port番号とpixel番号で表示されているのでそれをXYに変換する
void convPortToXY()
{
	for (int i = 0; i < inputListData.size(); i++)
	{
		int portNum = inputListData[i][1];
		ui pxNum = inputListData[i][2];
		ui X, Y;

		switch (portNum)
		{
		case 0:
			X = (8 * pxNum) % 400;
			Y = (8 * pxNum) / 400;
			break;
		case 1:
			X = (8 * pxNum + 2) % 400;
			Y = (8 * pxNum + 2) / 400;
			break;
		case 2:
			X = (8 * pxNum + 4) % 400;
			Y = (8 * pxNum + 4) / 400;
			break;
		case 3:
			X = (8 * pxNum + 6) % 400;
			Y = (8 * pxNum + 6) / 400;
			break;
		case 4:
			X = (8 * pxNum + 1) % 400;
			Y = (8 * pxNum + 1) / 400;
			break;
		case 5:
			X = (8 * pxNum + 3) % 400;
			Y = (8 * pxNum + 3) / 400;
			break;
		case 6:
			X = (8 * pxNum + 5) % 400;
			Y = (8 * pxNum + 5) / 400;
			break;
		case 7:
			X = (8 * pxNum + 7) % 400;
			Y = (8 * pxNum + 7) / 400;
			break;
		default:
			break;
		}

		//outputData1行を構成
		vector<ui> outputLine;
		outputLine.push_back(inputListData[i][0]);
		outputLine.push_back(X);
		outputLine.push_back(Y);
		outputLine.push_back(inputListData[i][3]);

		//midListDataに追加
		midListData.push_back(outputLine);
	}
}

//2.NI PCIe1473Rから得られた画像データは、port番号とpixel番号で表示されているのでそれをXYに変換する(引数付き)
vector<vector<ui>> convPortToXY(vector<vector<ui>> listData)
{
	vector<vector<ui>> res;
	for (int i = 0; i < inputListData.size(); i++)
	{
		int portNum = inputListData[i][1];
		ui pxNum = inputListData[i][2];
		ui X, Y;

		switch (portNum)
		{
		case 0:
			X = (8 * pxNum) % 400;
			Y = (8 * pxNum) / 400;
			break;
		case 1:
			X = (8 * pxNum + 2) % 400;
			Y = (8 * pxNum + 2) / 400;
			break;
		case 2:
			X = (8 * pxNum + 4) % 400;
			Y = (8 * pxNum + 4) / 400;
			break;
		case 3:
			X = (8 * pxNum + 6) % 400;
			Y = (8 * pxNum + 6) / 400;
			break;
		case 4:
			X = (8 * pxNum + 1) % 400;
			Y = (8 * pxNum + 1) / 400;
			break;
		case 5:
			X = (8 * pxNum + 3) % 400;
			Y = (8 * pxNum + 3) / 400;
			break;
		case 6:
			X = (8 * pxNum + 5) % 400;
			Y = (8 * pxNum + 5) / 400;
			break;
		case 7:
			X = (8 * pxNum + 7) % 400;
			Y = (8 * pxNum + 7) / 400;
			break;
		default:
			break;
		}

		//result1行を構成
		vector<ui> resLine;
		resLine.push_back(inputListData[i][0]);
		resLine.push_back(X);
		resLine.push_back(Y);
		resLine.push_back(inputListData[i][3]);

		//resに追加
		res.push_back(resLine);
	}
	return res;
}

//3.coordinate of point解析（量研機構　千葉さんのLabVIEW VIと同じ機能,グローバル変数ver)
void coordinateOfPoint()
{
	int frameNum = inputListData[0][0];//今見ているフレームのインデックス
	int k = 0;//今見ている行のインデックス

	//リストデータの最後まで1つずつ見ていく
	while (k < inputListData.size())
	{
		vector<vector<ui>> tmp;
		frameNum = inputListData[k][0];
		tmp.push_back(inputListData[k]);

		//frame番号が変わるところまでで部分配列を抜き出す(tmp)
		while ((k + 1 < inputListData.size()) && (inputListData[k][0] == inputListData[k + 1][0]))
		{
			tmp.push_back(inputListData[k + 1]);
			k++;
		}
		//そのフレームの明るさの最大値maxBrightがthreshold未満になるまで繰り返す
		while (true)
		{
			//そのフレームで最も明るい点の座標と輝度を探す
			int Xm, Ym;
			ui maxBright = 0;
			for (int i = 0; i < tmp.size(); i++)
			{
				if (tmp[i][3] >= maxBright)
				{
					Xm = tmp[i][1];
					Ym = tmp[i][2];
					maxBright = tmp[i][3];
				}
			}
			//そのフレーム最大の輝度がthreshold未満ならばやめて、次のフレームに移る
			if (maxBright < THRESHOLD)
			{
				break;
			}
			//輝度がthreshold以上ならばその座標と輝度をoutpuLineに書き込みoutputListDataに追加したあと、
			//そのフレームのうち (X-Xm)<=elimX && (Y-Ym)<=elimYを満たす点の輝度を0にする
			else
			{
				vector<ui> outputLine;
				outputLine.push_back(frameNum);
				outputLine.push_back(Xm);
				outputLine.push_back(Ym);
				outputLine.push_back(maxBright);

				midListData.push_back(outputLine);

				for (int i = 0; i < tmp.size(); i++)
				{
					if (abs((int)tmp[i][1] - Xm) <= ELIM_X && abs((int)tmp[i][2] - Ym) <= ELIM_Y)
					{
						tmp[i][3] = 0;
					}
				}
			}
		}
		k++;
	}
}

//3.coordinate of point解析（量研機構　千葉さんのLabVIEW VIと同じ機能)
vector<vector<ui>> coordinateOfPoint(vector<vector<ui>> listData)
{
	vector<vector<ui>> res; //戻り値
	int frameNum = listData[0][0];//今見ているフレームのインデックス
	int k = 0;//今見ている行のインデックス

	//リストデータの最後まで1つずつ見ていく
	while (k < listData.size())
	{
		vector<vector<ui>> tmp;
		frameNum = listData[k][0];
		tmp.push_back(listData[k]);

		//frame番号が変わるところまでで部分配列を抜き出す(tmp)
		while ((k + 1 < listData.size()) && (listData[k][0] == listData[k + 1][0]))
		{
			tmp.push_back(listData[k + 1]);
			k++;
		}
		//そのフレームの明るさの最大値maxBrightがthreshold未満になるまで繰り返す
		while (true)
		{
			//そのフレームで最も明るい点の座標と輝度を探す
			int Xm, Ym;
			ui maxBright = 0;
			for (int i = 0; i < tmp.size(); i++)
			{
				if (tmp[i][3] >= maxBright)
				{
					Xm = tmp[i][1];
					Ym = tmp[i][2];
					maxBright = tmp[i][3];
				}
			}
			//そのフレーム最大の輝度がthreshold未満ならばやめて、次のフレームに移る
			if (maxBright < THRESHOLD)
			{
				break;
			}
			//輝度がthreshold以上ならばその座標と輝度をoutpuLineに書き込みoutputListDataに追加したあと、
			//そのフレームのうち (X-Xm)<=elimX && (Y-Ym)<=elimYを満たす点の輝度を0にする
			else
			{
				vector<ui> resLine;
				resLine.push_back(frameNum);
				resLine.push_back(Xm);
				resLine.push_back(Ym);
				resLine.push_back(maxBright);

				res.push_back(resLine);

				for (int i = 0; i < tmp.size(); i++)
				{
					if (abs((int)tmp[i][1] - Xm) <= ELIM_X && abs((int)tmp[i][2] - Ym) <= ELIM_Y)
					{
						tmp[i][3] = 0;
					}
				}
			}
		}
		k++;
	}
	return res;
}

