#!/bin/bash
set -euxo pipefail

build_path=$1
src_dir=$(realpath Source)
# shellcheck disable=SC2046
err_lines=$(parallel clang-tidy \
    --warnings-as-errors="*" \
    -p "$build_path" \
    --config-file="./.clang-tidy" \
    ::: $(jq -r ".[].file | select(startswith(\"${src_dir}/shared\") or startswith(\"${src_dir}/smokeview\"))" \
          < "$build_path"/compile_commands.json) \
    | wc -l)
echo "Errors: " "$err_lines"
if [ "$err_lines" -gt 0 ]; then
    # if we have any lines of output than clang-tidy has found errors and we
    # should return with a non-zero exit code
    exit 1
fi
