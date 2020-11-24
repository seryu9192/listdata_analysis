#rawdata_coversion.py : rawdata(imaging, tof)をCOP, decoded形式に変換するスクリプト
import os, subprocess

exe_directory = r'C:\Users\clusterbeam\Documents\NI_PCIe1473R\Event_by_event_measurement_program'
inputfolder = r'E:\nanocapillary'

img_exe = r'Imaging_0_rawToCOP.exe'
tof_exe = r'MCS6A_0_rawToDecoded.exe'

img_outputfolder = r'COP'
tof_outputfolder = r'decoded'

MANUAL = """---- 使い方 ----
- 単一ファイル処理：処理したいファイル番号を一つ入力
- 連番ファイル処理：開始ファイル番号と終了ファイル番号を1 4のようにスペース区切りで入力
- ディレクトリの再選択: -1 を入力
- 終了: Enterのみを入力
"""

def main():
    print('imaging と tofのraw dataを変換する(Imaging_0_rawToCOP.exe/MCS6A_0_rawToDecoded.exeを立ち上げる)')
    inputfolder = input('rawdataのディレクトリパスを入力してください\n--->')
    
    #無限ループ
    while True:
        print(MANUAL, end='---> ')
        #start endを決める
        #ファイル番号をリストで受け取り、リストの長さで連番か単一ファイルかを判断
        fileNumber = list(map(int, input().split()))
        if len(fileNumber) == 1:
            start = end = fileNumber[0]
        elif len(fileNumber) == 2:
            start = fileNumber[0]
            end = fileNumber[1]

        #ディレクトリ再選択の処理
        while start == -1 and end == -1:
            inputfolder = input('rawdataのディレクトリパスを入力してください\n--->')
            print(MANUAL, end='---> ')
            #ファイル番号をリストで受け取り、リストの長さで連番か単一ファイルかを判断
            fileNumber = list(map(int, input().split()))
            if len(fileNumber) == 1:
                start = end = fileNumber[0]
            elif len(fileNumber) == 2:
                start = fileNumber[0]
                end = fileNumber[1]

        #imaging dataの処理: Imaging_0_rawToCOP.exe
        #読み取ったstart, endを表示
        print('Imaging_exe', end=' --> ')
        print('start:', start, end=' ')
        print('end:', end,)
        print('{}を開きます'.format(img_exe))

        #exeファイルのパス
        exe_path = os.path.join(exe_directory, img_exe)
        #exeファイルの実行
        code = subprocess.run([exe_path, inputfolder, str(start), str(end)]) 

        if code.returncode == 0:
            print(exe_path + ': 正常に終了しました')
        else :
            print(exe_path + ': エラー停止しました')
        
        #tof dataの処理: MCS6A_0_rawTDecoded.exe
        #読み取ったstart, endを表示
        print('TOF_exe', end=' --> ')
        print('start:', start, end=' ')
        print('end:', end,)
        print('{} : ファイル#{} - {}が更新されました'.format('tof',start, end))
        print('{}を開きます'.format(tof_exe))
        #exeファイルのパス
        exe_path = os.path.join(exe_directory, tof_exe)
        #exeファイルの実行
        code = subprocess.run([exe_path, inputfolder, str(start), str(end)])
        if code.returncode == 0:
            print(exe_path + ': 正常に終了しました')
        else :
            print(exe_path + ': エラー停止しました')

if __name__ == '__main__':
    main()

