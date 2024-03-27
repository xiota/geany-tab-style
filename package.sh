#!/usr/bin/env bash

_program=xi-tweaks
_version=$(git describe --long --tags --always | sed -E 's/^v//;s/-([0-9]+)-/-r\1-/;s/-/./g')

_outdir="$_program-$_version"
mkdir -p "$_outdir"

_files=(
  data
  src
  License.md
  Makefile.am
  Readme.md
  autogen.sh
  clean-tree.sh
  configure.ac
  makefile.win32
  package.sh
  reconfigure.sh
  version.sh
)

for i in "${_files[@]}" ; do
  cp --reflink=auto -r "$i" "$_outdir/"
done

tar cf "$_outdir.tar" "$_outdir"
xz -9k "$_outdir.tar"
