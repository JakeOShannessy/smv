#!/bin/sh

build_path=cbuild
# shellcheck disable=SC2046
# clang-tidy --warnings-as-errors="*" -p $build_path $(jq '.[]|select((.file | startswith("/home/jake/smv/Source/shared"))).file' -r < $build_path/compile_commands.json)
clang-tidy --warnings-as-errors="*" -p $build_path $(jq '.[]|select((.file | startswith("/home/jake/smv/Source/shared")) or (.file | startswith("/home/jake/smv/Source/smokeview"))).file' -r < $build_path/compile_commands.json)
