#!/bin/bash

perf record -F 200 -a -g -- cbuild/smvq /home/jake/big_model/big_model.smv
perf script | stackcollapse-perf.pl > out.perf-folded
flamegraph.pl ./out.perf-folded  > perf.svg
