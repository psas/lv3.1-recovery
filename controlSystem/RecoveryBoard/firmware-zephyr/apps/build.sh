#!/bin/bash
west build -b ers-v3p1 -p -- -DBOARD_ROOT=..
exit $?
