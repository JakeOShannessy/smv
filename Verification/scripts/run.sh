#!/bin/bash

xvfb-run --server-args="-screen 0 1024x768x24 -ac +extension GLX +render -noreset" ./Run_SMV_Cases.py
