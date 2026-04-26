#!/bin/bash

set -e

echo "== Build modules =="
make

echo
echo "== Test /proc/jiffies =="
sudo insmod jiffies.ko
if [ ! -f /proc/jiffies ]; then
    echo "ERROR: /proc/jiffies not found"
    sudo rmmod jiffies || true
    exit 1
fi

J1=$(cat /proc/jiffies)
sleep 1
J2=$(cat /proc/jiffies)

echo "First jiffies : $J1"
echo "Second jiffies: $J2"

if [ "$J2" -le "$J1" ]; then
    echo "ERROR: jiffies did not increase"
    sudo rmmod jiffies || true
    exit 1
fi

sudo rmmod jiffies
if [ -f /proc/jiffies ]; then
    echo "ERROR: /proc/jiffies still exists after rmmod"
    exit 1
fi

echo "/proc/jiffies test passed"

echo
echo "== Test /proc/seconds =="
sudo insmod seconds.ko
if [ ! -f /proc/seconds ]; then
    echo "ERROR: /proc/seconds not found"
    sudo rmmod seconds || true
    exit 1
fi

S1=$(cat /proc/seconds)
sleep 3
S2=$(cat /proc/seconds)

echo "First seconds : $S1"
echo "Second seconds: $S2"

if [ "$S2" -lt "$S1" ]; then
    echo "ERROR: /proc/seconds decreased"
    sudo rmmod seconds || true
    exit 1
fi

if [ "$S2" -eq "$S1" ]; then
    echo "WARNING: seconds did not visibly increase; kernel timing may be coarse"
fi

sudo rmmod seconds
if [ -f /proc/seconds ]; then
    echo "ERROR: /proc/seconds still exists after rmmod"
    exit 1
fi

echo "/proc/seconds test passed"

echo
echo "== Kernel log =="
dmesg | tail -n 30

echo
echo "All tests finished successfully."