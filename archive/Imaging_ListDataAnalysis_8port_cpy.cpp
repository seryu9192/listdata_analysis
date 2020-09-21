// Imaging_ListDataAnalysis_8port_cpy.cpp : Imagingの輝点リストデータ解析プログラム(8portに分割された生データを処理する用)
//
//

//アップデートログ
//2020.3.25:作成、イメージングの生データをCOP処理せずに抽出するために作成


#include "C:\Users\RyuMurase\source\repos\C++\univ\library.hpp"

//点の明るさの閾値
constexpr auto THRESHOLD = 100;

//COPにする際に1つの点としてみなすpixelの範囲（これより近い複数の点は１点として見なされる）
constexpr auto ELIM_X = 5;
constexpr auto ELIM_Y = 5;

//port毎にoverflowを解消するときに使うbuffer
vector<vector<ui>> inputListBuf;
vector<vector<ui>> outputListBuf;

//overflow以降、portをすべて足し合わせたデータを保存しておく変数
vector<vector<ui>> inputListData;
vector<vector<ui>> outputListData;

//イベント番号のオーバフローを直す(もともと16bitなので）
void solveOverflow();

//もともとport番号毎に並んでいるので、frame番号でsortする（quicksortアルゴリズム）
void sortWithFrame();
void quicksort(int, int);
//3つの数字の真ん中の数を返す(quicksortに利用)
int med3(int, int, int);

//（ポート番号, 信号の通し番号）の形式を（X, Y)に変換する
void convPortToXY();

// COP処理
void coordinateOfPoint();

int main()
{
	cout << "************************************************************" << endl;
	cout << "**          Imaging リストデータ解析プログラム            **" << endl;
	cout << "**     PCIe1473R Imaging輝点ピクセル処理 for 8 port files **" << endl;
	cout << "**   0:overflowしたデータを直す(port毎),1:frameNumでsort  **" << endl;
	cout << "**   2:port/pxNumをX/Yに変換, (3:CoordinateOfPoint)←なし **" << endl;
	cout << "**                    ver.2020.03.25 written by R. Murase **" << endl;
	cout << "************************************************************" << endl << endl;

	string inputDataFolder, inputDataName, inputDataPath;
	cout << "解析するリストデータのフォルダを入力してください\n--->";
	cin >> inputDataFolder;
	inputDataName = folderName(inputDataFolder);
	cout << "解析するリストデータのファイル名：" << inputDataName << endl;

	int start, end;
	cout << "解析するファイル番号(start,end)を入力してください" << endl;
	cout << "start : ";
	cin >> start;
	cout << "end : ";
	cin >> end;

	//出力フォルダの指定
	string outputFolderName;
	cout << "\nXYフォルダのパスを入力してください (\"@\"入力で入力フォルダ同じフォルダ内の\"XY\"に保存)\n--->";
	cin >> outputFolderName;
	if (outputFolderName == "@")
	{
		outputFolderName = inputDataFolder + "\\XY";
		_mkdir(outputFolderName.c_str());
	}

	for (int m = start; m <= end; m++)
	{
		//file indexを0詰め3桁の文字列suffixに変換
		ostringstream sout;
		sout << setfill('0') << setw(3) << m;
		string suf_file = sout.str();

		//出力データパスの生成
		string outputFilePath = outputFolderName + "\\" + inputDataName + suf_file + ".txt";

		//portの数だけループを回してファイルをimport
		for (int j = 0; j < 8; j++)
		{
			//portを示すsuffix = "_0"など
			string suf_port = "_" + to_string(j);

			//入力データパスの生成
			inputDataPath = inputDataFolder + "\\" + inputDataName + suf_file + suf_port + ".txt";
			ifstream ifs(inputDataPath);
			if (!ifs)
			{
				cerr << "ファイルを開けませんでした" << endl;
				system("pause");
				return -1;
			}
			//ifsから1行ずつ読み取ってinputListData(2Dvector)を構成
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
				inputListBuf.push_back(inputLine);
			}

			//frameNumのoverflowを解消
			solveOverflow();
			//port毎に分かれていたリストを1つにまとめる(inputListData)
			inputListData.insert(inputListData.end(), outputListBuf.begin(), outputListBuf.end());

			//次のportを読み込むためにbufをクリア
			inputListBuf.clear();
			outputListBuf.clear();

			cout << inputDataName + suf_file + suf_port + ".txt" + "--- overflow solved" << endl;
		}

		cout << inputDataName + suf_file + ".txt----データの読み込み完了、処理を開始します。" << endl;


		sortWithFrame();
		inputListData = outputListData;
		outputListData.clear();
		cout << "sorted with frame number" << endl;

		convPortToXY();
		inputListData = outputListData;
		//outputListData.clear();
		cout << "converted into XY" << endl;

		//coordinateOfPoint();
		//cout << "COP" << endl;

		cout << "----処理終了。ファイルに書き込んでいます。" << endl;

		//出力ファイルにoutputListDataを書き込み
		ofstream ofs(outputFilePath);
		for (int i = 0; i < outputListData.size(); i++)
		{
			for (int j = 0; j < outputListData[0].size(); j++)
			{
				ofs << outputListData[i][j];
				if (j != outputListData[0].size() - 1) ofs << "\t";
			}
			ofs << endl;
		}
		cout << outputFilePath << "に書き込みました" << endl << endl;

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
	for (int i = 0; i < inputListBuf.size(); i++)
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

//1.NI PCIe1473Rから得られた画像データのframe番号がバラバラなので昇順にソートする
void sortWithFrame()
{
	//quick sortを利用
	quicksort(0, inputListData.size() - 1);

	//ソートされた配列をoutputListData
	outputListData = inputListData;
}

//quick sort
void quicksort(int left, int right)
{
	if (left < right)
	{
		int i = left, j = right;
		int pivot = med3(inputListData[i][0], inputListData[i + (j - i) / 2][0], inputListData[j][0]);
		while (true)
		{
			while (inputListData[i][0] < pivot)
				i++;
			while (inputListData[j][0] > pivot)
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

		//outputListDataに追加
		outputListData.push_back(outputLine);
	}

}

//3.coordinate of point解析（量研機構　千葉さんのLabVIEW VIと同じ機能)
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

				outputListData.push_back(outputLine);

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


