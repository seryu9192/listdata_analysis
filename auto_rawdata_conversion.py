#auto_rawdata_coversion.py : rawdata(imaging, tof)が更新された時に自動でCOP, decoded形式に変換するスクリプト
import os, time, subprocess

exe_directory = r'C:\Users\clusterbeam\Documents\NI_PCIe1473R\Event_by_event_measurement_program'
working_directory = r'E:\nanocapillary\2020714\20200720'

img_exe = r'Imaging_0_rawToCOP.exe'
tof_exe = r'MCS6A_0_rawToDecoded.exe'

img_outputfolder = r'COP'
tof_outputfolder = r'decoded'

def main():
    print('rawdataが更新された時に自動的にCOP, decodedに変換します（１分おきに更新を確認, ctrl + Cで終了）')
    #working_directory = input('rawdataのディレクトリパスを入力してください\n--->')
    dataname = os.path.basename(working_directory)
    
    #無限ループ
    while True:

        #imaging dataの処理: Imaging_0_rawToCOP.exe
        #連番データが更新されているかを確認する -> [start, end)区間について処理
        #outputfileを見てstartを決める:
        start = 1
        #出力が連番ファイルなので、ファイル番号の更新分だけ探す
        try:
            files_in_output = os.listdir(os.path.join(working_directory, img_outputfolder))
            while dataname + '{0:03d}.txt'.format(start) in files_in_output:
                start += 1
        #まだ出力ファイルがない場合は1からスタート
        except FileNotFoundError: 
            pass

        #endを決める:
        end = 1
        #working_directoryにあるファイルの最大値を探す
        files_in_input = os.listdir(working_directory)
        #入力が連番ファイルなので、入力ファイルの最後のfile番号+1をendにする
        exists = True
        while exists:
            #8ポート分すべてのデータがそろっていなければ存在しないものとみなす
            for port in range(8):
                if dataname + '{0:03d}_{}.txt'.format(start, port) not in files_in_output:
                    exists = False
            if(exists):
                start += 1

        #連番データが更新されていたらプログラムを実行
        if start != end:
            #読み取ったstart, endを表示
            print('Imaging_exe', end=' --> ')
            print('start:', start, end=' ')
            print('end:', end-1,)
            print('{} : ファイル#{} - {}が更新されました'.format('imaging',start, end - 1))
            print('{}を開きます'.format(img_exe))

            #exeファイルのパス
            exe_path = os.path.join(exe_directory, img_exe)
            code = subprocess.run([exe_path, working_directory, str(start), str(end-1)]) 
            if code.returncode == 0:
                print(exe_path + ': 正常に終了しました')
            else :
                print(exe_path + ': エラー停止しました')

        #tof dataの処理: MCS6A_0_rawTDecoded.exe
        #連番データが更新されているかを確認する -> [start, end)区間について処理
        #startを決める:
        start = 1
        try: 
            #出力が連番ファイルなので、ファイル番号の更新分だけ探す
            while dataname + '{0:03d}.txt'.format(start) in files_in_output:
                start += 1
            files_in_output = os.listdir(os.path.join(working_directory, tof_outputfolder))    
        #まだ出力ファイルがない場合は1からスタート
        except FileNotFoundError: 
            pass

        #endを決める:
        end = 1
        #working_directoryにあるファイルの最大値を探す
        files_in_input = os.listdir(working_directory)
        #入力が連番ファイルなので、入力ファイルの最後のfile番号+1をendにする
        while dataname + '{0:03d}.lst'.format(end) in files_in_input:
                end += 1

        #連番データが更新されていたらプログラムを実行
        if start != end:
            #読み取ったstart, endを表示
            print('TOF_exe', end=' --> ')
            print('start:', start, end=' ')
            print('end:', end-1,)
            print('{} : ファイル#{} - {}が更新されました'.format('tof',start, end - 1))
            print('{}を開きます'.format(tof_exe))

            #exeファイルのパス
            exe_path = os.path.join(exe_directory, tof_exe)
            code = subprocess.run([exe_path, working_directory, str(start), str(end-1)]) 
            if code.returncode == 0:
                print(exe_path + ': 正常に終了しました')
            else :
                print(exe_path + ': エラー停止しました')

    for sec in range(10):
        time.sleep(1)
    print('tik')

main()

