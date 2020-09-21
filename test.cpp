#include "./library.hpp"

int main(int argc, char* argv[])
{
    string filePath;
    cin >> filePath;

	ifstream ifs(filePath);
	string line;
	
	//overflowの管理
	int overflowCnt = 0;
	int previous = 0;
	const int US_MAX = pow(2, 16);

	//ファイルから１行ずつ見ていく
	while (getline(ifs, line))
	{
		stringstream ss(line);

		string tmp;
		vector<int> inputLine(4);
        //一行分の配列を構成
        int i = 0;
		while (ss >> tmp)
		{
			inputLine[i] = stoT<int>(tmp);
            i++;
		}
		//大小関係が反転していたらoverflowと認識してcnt++
		if (previous > inputLine[0])
		{
			overflowCnt++;
		}
		//現在の値を覚えておく
		previous = inputLine[0];

		//overflowした回数だけUS_MAX=65536を足す
		inputLine[0] += overflowCnt * US_MAX;
    }
    cout << previous + overflowCnt * US_MAX << endl; 
}