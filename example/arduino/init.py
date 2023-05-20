#! /usr/bin/python3
from shutil import copy
import os, sys
#diw 需拷贝的文件夹，newdir是拷贝的地方
def copy_dir(dir,newdir):
    for p in os.listdir(dir):
        print(p)
        filepath=newdir+'/'+p
        oldpath=dir+'/'+p
        if os.path.isdir(oldpath):
            print('是目录')
            os.mkdir(filepath)
            copy_dir(oldpath,filepath)
        if os.path.isfile(oldpath):
            copy(oldpath,filepath)

if __name__ == "__main__":
    if(len(sys.argv) == 2):
        if(sys.argv[1] == "clean"):
            remove_list = ['arduino.ino', 'README.md', 'xcmd_confg.h', 'init.py']
            files = os.listdir("./")
            for f in remove_list:
                files.remove(f)
            for f in files:
                os.remove(f)
        else:
            print("Usage init project: python3 init.py")
            print("Usage clean project: python3 init.py clean")

    else:
        copy_dir("../../src", "./")
        copy_dir("../../inc", "./")

