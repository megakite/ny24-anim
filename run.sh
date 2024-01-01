#!/bin/bash
hwclock -w
date -s '2023-12-31 23:59:58'
./ny24
hwclock -s

