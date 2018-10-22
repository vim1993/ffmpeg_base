#!/usr/bin/python
# -*- coding: UTF-8 -*-
import os
import sys
import commands
import time

def utilsAllCompile(target, filepath, congureCmd, makeComd, installCmd):
    if os.path.exists(filepath) == True:
        print '--------->start compiler '+ target + ':' + filepath + '<--------------'
        os.chdir(filepath)
        os.system(congureCmd)
        os.system(makeComd)
        os.system(installCmd)
        print '--------->end compiler '+ target + ':' + filepath + '<--------------'
        return True
    else:
        return False

def start_compiler_thirdparty():
    currentPath = os.getcwd()
    #print 'workspace:'+currentPath

    filepath = currentPath + "/yasm-1.2.0"
    cgcmd = './configure'
    mkcmd = 'make -j4'
    incmd = 'sudo make'
    utilsAllCompile('yasm-1.2.0', filepath, cgcmd, mkcmd, incmd)

    filepath = currentPath + "/nasm-2.13.03"
    cgcmd = './configure'
    mkcmd = 'make -j4'
    incmd = 'sudo make'
    utilsAllCompile('nasm-2.13.03', filepath, cgcmd, mkcmd, incmd)

    filepath = currentPath + "/x264"
    cgcmd = './configure --enable-shared'
    mkcmd = 'make -j4'
    incmd = 'sudo make'
    utilsAllCompile('x264', filepath, cgcmd, mkcmd, incmd)

    #filepath = currentPath + "/ffmpeg-3.2.12"
    #cgcmd = './configure --enable-memalign-hack --enable-ffplay --enable-static --enable-gpl --enable-libx264 --extra-cflags=-I/usr/local/include --extra-ldflags=-L/usr/local/lib'
    #mkcmd = 'make -j4'
    #incmd = 'sudo make'
    #utilsAllCompile('ffmpeg-3.1.11', filepath, cgcmd, mkcmd, incmd)

if __name__ == '__main__':
    start_compiler_thirdparty()


