# Imaging_#_event_counter.py: そのフォルダにあるリストデータのイベント数を集計する

import os

def main():
    print('**** Both_#_event_counter.py  ****')
    #working directory
    working_directory = input('Input the working directory\n --> ')
    outputpath = os.path.join(working_directory, 'event_counts.txt')
    outputdata = ''
    para_tot = 0
    perp_tot = 0
    else_tot = 0
    for filename in os.listdir(working_directory):
        #avoid to open directory or event.txt (which you are going to make) file
        if '.txt' not in filename or 'event_counts' in filename:
            continue
        with open(os.path.join(working_directory, filename), 'r') as f:
            inputdat = f.read()
        
        #get line number by counting the number of '\n'
        event_counts = inputdat.count('\n')
        #compose output data
        outputdata += filename + '\t' + str(event_counts) + '\n'
        if '-30.0' in filename:
            para_tot += event_counts
        elif '-90.0' in filename:
            perp_tot += event_counts
        else:
            else_tot += event_counts
    outputdata += 'para_tot\t' + str(para_tot) + '\n'
    outputdata += 'perp_tot\t' + str(perp_tot) + '\n'
    outputdata += 'else_tot\t' + str(else_tot) + '\n'
    with open(outputpath, 'w') as f:
        f.write(outputdata)

if __name__ == '__main__':
    main()