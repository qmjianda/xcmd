#! /usr/bin/python3
from shutil import copy
import os, sys
import shutil

xcmd_dir = "src"

#diw 需拷贝的文件夹，newdir是拷贝的地方
def copy_dir(dir,newdir):
    for p in os.listdir(dir):
        print(p)
        filepath= os.path.join(newdir, p)
        oldpath=os.path.join(dir, p)
        if os.path.isdir(oldpath):
            # os.mkdir(filepath)
            # copy_dir(oldpath,filepath)
            continue
        if os.path.isfile(oldpath):
            if oldpath.endswith(".h") or oldpath.endswith(".c"):
                copy(oldpath,filepath)

if __name__ == "__main__":
    if(len(sys.argv) == 2):
        if(sys.argv[1] == "clean"):
            try:
                shutil.rmtree(xcmd_dir)
            except FileNotFoundError:
                pass
        else:
            print("Usage init project: python3 init.py")
            print("Usage clean project: python3 init.py clean")

    else:
        try:
            os.mkdir(xcmd_dir)
        except FileExistsError:
            pass
        path = os.path.join("..", "..", "src")
        copy_dir(path, xcmd_dir)
        path = os.path.join("..", "..", "src", "xnr")
        copy_dir(path, xcmd_dir)
        path = os.path.join("src", "xcmd_confg.h")
        copy(".xcmd_confg",  path)

