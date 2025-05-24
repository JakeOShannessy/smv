#!/bin/sh

sudo -v

hyperfine --prepare 'sync; echo 3 | sudo tee /proc/sys/vm/drop_caches' \
 './cbuild-old/smvq /home/jake/smv/Tests/test_model_small/test_model_small.smv' \
 './cbuild/smvq     /home/jake/smv/Tests/test_model_small/test_model_small.smv'

hyperfine --prepare 'sync; echo 3 | sudo tee /proc/sys/vm/drop_caches' \
 './cbuild-old/smvq /home/jake/smv/Tests/test_model_large/test_model_large.smv' \
 './cbuild/smvq     /home/jake/smv/Tests/test_model_large/test_model_large.smv'
