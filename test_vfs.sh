#!/bin/bash
cd /media/hqm/660A6E450A6E127B/Project/xcmd/example/linux

# 使用expect或直接发送命令
{
sleep 1
echo "ls"
sleep 0.5
echo "ls /proc"
sleep 0.5
echo "ls /dev"
sleep 0.5
echo "cd /proc"
sleep 0.5
echo "ls"
sleep 0.5
echo "cd /dev"
sleep 0.5
echo "ls"
sleep 0.5
echo "exit"
sleep 0.5
} | timeout 10 ./xcmder 2>&1 | head -100
