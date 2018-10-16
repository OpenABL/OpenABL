#!/bin/bash

# Copyright 2017 OpenABL Contributors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
shopt -s nullglob

if [ -e $DIR/OpenABL ]; then
  OPENABL_BIN=$DIR/OpenABL
elif [ -e $DIR/build/OpenABL ]; then
  OPENABL_BIN=$DIR/build/OpenABL
else
  echo "Could not find OpenABL binary"
  exit 1
fi

EXIT_CODE=0

# Save compilation time (FlameGPU)
export SMS=32

# Perform lint-only tests in test/ directory.
# These check that errors are detected correctly.
for file in $DIR/test/*.abl; do
  noExtName=${file%.abl}
  echo $file
  $OPENABL_BIN --lint-only -i $file 2> $noExtName.out
  rm -f $noExtName.diff
  if [ -f $noExtName.exp ]; then
    diff -b $noExtName.out $noExtName.exp > $noExtName.diff
    if [ $? -ne 0 ]; then
      echo "DIFF $noExtName.diff"
      cat $noExtName.diff
      EXIT_CODE=1
    fi
  else
    echo "OUT Missing .exp file"
    cat $noExtName.out
    EXIT_CODE=1
  fi
done

TMP_DIR=$DIR/test-tmp
mkdir -p $TMP_DIR
for file in $DIR/examples/*abl; do
  noExtName=${file%.abl}
  baseName=$(basename $noExtName)
  echo $file

  TARGET_DIR=$TMP_DIR/$baseName
  mkdir -p $TARGET_DIR

  # Lint-only test
  $OPENABL_BIN --lint-only -i $file > $TARGET_DIR/lint.out
  if [ $? -ne 0 ]; then
    echo "LINT-FAIL $TARGET_DIR/lint.out"
    cat $TARGET_DIR/lint.out
    EXIT_CODE=1
    continue
  fi

  for backend in ${BACKENDS:-c flame flamegpu mason}; do
    echo "BACKEND $backend"

    BACKEND_DIR=$TARGET_DIR/$backend
    mkdir -p $BACKEND_DIR

    # Codegen + Build test
    $OPENABL_BIN -i $file -o $BACKEND_DIR -b $backend -B > build.log 2>&1
    BUILD_EXIT_CODE=$?
    if [ $BUILD_EXIT_CODE -eq 2 ]; then
      # Some feature not supported by backend
      echo "BUILD-SKIP $BACKEND_DIR/build.log"
      cat build.log
    elif [ $BUILD_EXIT_CODE -ne 0 ]; then
      # Actual build failure
      echo "BUILD-FAIL $BACKEND_DIR/build.log"
      cat build.log
      EXIT_CODE=1
    fi
  done
done

exit $EXIT_CODE
