#/bin/bash
reporoot=$HOME/FireModels_fork
smvbin=$reporoot/smv/Build/smokeview/intel_linux_64
bindir=$reporoot/smv/Build/for_bundle
$smvbin/smokeview_win_64 -bindir $bindir $*
