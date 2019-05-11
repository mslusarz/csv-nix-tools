#!/bin/bash

# Copyright 2019, Sebastian Pidek <sebastian.pidek@gmail.com>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#    * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#    * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in
#       the documentation and/or other materials provided with the
#       distribution.
#
#    * Neither the name of the copyright holder nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

source common.sh

show_detailed_differences=$1

TEST_CASE "empty input"
cat empty.txt | ../build/csv-cut 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh empty.csv error-no-typenames.txt "${test_name}" $show_detailed_differences
 
TEST_CASE "simple pass-through"
cat one-column-one-row.csv | ../build/csv-cut 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh cut-usage.txt empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "simple pass-through with -f"
cat one-column-one-row.csv | ../build/csv-cut -f 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh cut-usage.txt cut-no-fields-error.txt "${test_name}" $show_detailed_differences



TEST_CASE "the only field using -f "
cat one-column-one-row.csv | ../build/csv-cut -f name 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh one-column-one-row.csv empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "the only field using --fields with space"
cat one-column-one-row.csv | ../build/csv-cut --fields name 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh one-column-one-row.csv empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "the only field using --fields with ="
cat one-column-one-row.csv | ../build/csv-cut --fields=name 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh one-column-one-row.csv empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "not existing field using -f"
cat one-column-one-row.csv | ../build/csv-cut -f notExistingColumn 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh empty.txt column-not-found.txt "${test_name}" $show_detailed_differences

TEST_CASE "not existing field using --fields with space"
cat one-column-one-row.csv | ../build/csv-cut --fields notExistingColumn 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh empty.txt column-not-found.txt "${test_name}" $show_detailed_differences

TEST_CASE "not existing field using --fields with ="
cat one-column-one-row.csv | ../build/csv-cut --fields=notExistingColumn 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh empty.txt column-not-found.txt "${test_name}" $show_detailed_differences


TEST_CASE "the name field using -f"
cat 3-columns-3-rows.csv | ../build/csv-cut -f name 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh name-column-3-rows.csv empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "the name field using --fields with space"
cat 3-columns-3-rows.csv | ../build/csv-cut --fields name 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh name-column-3-rows.csv empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "the name field using --fields with ="
cat 3-columns-3-rows.csv | ../build/csv-cut --fields=name 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh name-column-3-rows.csv empty.txt "${test_name}" $show_detailed_differences


TEST_CASE "the id field using -f"
cat 3-columns-3-rows.csv | ../build/csv-cut -f id 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh id-column-3-rows.csv empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "the id field using --fields with space"
cat 3-columns-3-rows.csv | ../build/csv-cut --fields id 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh id-column-3-rows.csv empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "the id field using --fields with ="
cat 3-columns-3-rows.csv | ../build/csv-cut --fields=id 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh id-column-3-rows.csv empty.txt "${test_name}" $show_detailed_differences


TEST_CASE "the something field using -f"
cat 3-columns-3-rows.csv | ../build/csv-cut -f something 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh something-column-3-rows.csv empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "the something field using --fields with space"
cat 3-columns-3-rows.csv | ../build/csv-cut --fields something 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh something-column-3-rows.csv empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "the something field using --fields with ="
cat 3-columns-3-rows.csv | ../build/csv-cut --fields=something 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh something-column-3-rows.csv empty.txt "${test_name}" $show_detailed_differences


TEST_CASE "2 fields using -f"
cat 3-columns-3-rows.csv | ../build/csv-cut -f name,something 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh 2-columns-3-rows.csv empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "2 fields using --fields with space"
cat 3-columns-3-rows.csv | ../build/csv-cut --fields name,something 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh 2-columns-3-rows.csv empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "2 fields using --fields with ="
cat 3-columns-3-rows.csv | ../build/csv-cut --fields=name,something 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh 2-columns-3-rows.csv empty.txt "${test_name}" $show_detailed_differences


TEST_CASE "3 fields in different order using -f"
cat 3-different-order-columns-3-rows.csv | ../build/csv-cut -f id,name,something 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh 2-columns-3-rows.csv empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "3 fields in different order using --fields with space"
cat 3-different-order-columns-3-rows.csv | ../build/csv-cut --fields id,name,something 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh 2-columns-3-rows.csv empty.txt "${test_name}" $show_detailed_differences

TEST_CASE "3 fields in different order using --fields with ="
cat 3-different-order-columns-3-rows.csv | ../build/csv-cut --fields=id,name,something 1> "${test_name}.stdout.candidate" 2> "${test_name}.stderr.candidate"
./assert.sh 2-columns-3-rows.csv empty.txt "${test_name}" $show_detailed_differences
