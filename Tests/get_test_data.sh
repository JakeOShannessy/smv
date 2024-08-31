#!/bin/sh
COMMIT_FIG=374d9db0062fd6dc38b30bc0d27cd84c9bb2b69d

TEMP_DIR=$(mktemp -d)
ZIP_PATH=$TEMP_DIR/test-data.zip
wget --tries=5 https://github.com/firemodels/fig/archive/$COMMIT_FIG.zip -O "$ZIP_PATH"
unzip "$ZIP_PATH"
rm -rf fig
mkdir -p fig
mv -f fig-$COMMIT_FIG/* fig
cp -rf fig/smv/Tests/Visualization ../Verification
