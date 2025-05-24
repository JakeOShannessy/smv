#!/bin/sh

hyperfine --warmup 1 \
 './cbuild-old/smvq /home/jake/smv/Tests/test_model_small/test_model_small.smv' \
 './cbuild/smvq     /home/jake/smv/Tests/test_model_small/test_model_small.smv'

hyperfine --warmup 1 \
 './cbuild-old/smvq /home/jake/smv/Tests/test_model_large/test_model_large.smv' \
 './cbuild/smvq     /home/jake/smv/Tests/test_model_large/test_model_large.smv'
