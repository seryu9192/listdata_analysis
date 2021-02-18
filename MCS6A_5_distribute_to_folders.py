# MCS6A_#_distribute_to_folders.py: Imagingリストデータを配向ごとにフォルダに分ける
# 具体的には、"5_coinc_sep"フォルダ内のリストファイルを配向毎に数えて足し合わせる

import os
import shutil

dest_dirnames = {
    '_0.0-30.0' : 'para',
    '_30.0-60.0' : 'mid',
    '_60.0-90.0' : 'perp'
}

def main():
    print('**** Imaging_#_event_counter.py  ****')
    #working directory
    working_directory = input('Input the working directory\n --> ')

    #loop for filename
    for filename in os.listdir(working_directory):
        #loop for outputfile
        for k, v in dest_dirnames.items():
            if k in filename:
                curr_path = os.path.join(working_directory, filename)
                dest_dirpath = os.path.join(working_directory, v)
                os.makedirs(dest_dirpath, exist_ok=True)
                shutil.move(curr_path, dest_dirpath)

    print('done')

if __name__ == '__main__':
    main()