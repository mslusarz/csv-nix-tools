#
# SPDX-License-Identifier: BSD-3-Clause
#
# Copyright 2021-2026, Marcin Ślusarz <marcin.slusarz@gmail.com>
#

test("csv-printf '%d-%s:%d %s\n' id name something name" data/3-columns-3-rows.csv printf/basic.txt data/empty.txt 0
	printf_basic)

test("csv-printf '0x%x - 0%o - %d\n' col2 col3 col1" data/3-numeric-columns-4-rows.csv printf/numeric.txt data/empty.txt 0
	printf_numeric)

test([=[csv-printf '%+05i|%hhd|%hd|%-4ld|%lld|%jd|%zd|%td\n%+05i|%hhd|%hd|%-4ld|%lld|%jd|%zd|%td\n' \
	number number number number number number number number \
	fraction fraction fraction fraction fraction fraction fraction fraction]=]
	printf/typed-values.csv printf/signed.txt data/empty.txt 0 printf_signed)

test([=[csv-printf '%u|%hhu|%hu|%lu|%llu|%ju|%zu|%tu|%#X\n%u|%hhu|%hu|%lu|%llu|%ju|%zu|%tu|%#X\n' \
	number number number number number number number number number \
	fraction fraction fraction fraction fraction fraction fraction fraction fraction]=]
	printf/typed-values.csv printf/unsigned.txt data/empty.txt 0 printf_unsigned)

test([=[csv-printf '%.1f|%.0F|%.2e|%.2E|%g|%G|%.2a|%.2A\n' number fraction fraction fraction fraction fraction number fraction]=]
	printf/typed-values.csv printf/floating-point.txt data/empty.txt 0 printf_floating_point)

test([=[csv-printf '%c|%lc|%c|%lc|%p\n' number number fraction fraction number]=]
	printf/typed-values.csv printf/characters-and-pointer.txt data/empty.txt 0 printf_characters_and_pointer)

test([=[csv-printf '%%:%d\n\rX\\\a\b\t\f\v\' number]=]
	printf/typed-values.csv printf/escapes.txt data/empty.txt 0 printf_escapes)

test("csv-printf '%d%' number" printf/typed-values.csv printf/trailing-percent.txt data/empty.txt 0
	printf_trailing_percent)

test("csv-printf 'literal'" data/rpn-filter-no-rows.csv data/empty.txt data/empty.txt 0
	printf_header_only)

foreach(conversion d u f c s p)
	test("csv-printf '%d%${conversion}' number"
		printf/typed-values.csv printf/first-number.txt printf/not-enough-columns.txt 2
		printf_missing_column_for_${conversion})
endforeach()

foreach(conversion d u f c p)
	test("csv-printf '%${conversion}' bad_int"
		printf/typed-values.csv data/empty.txt printf/invalid-integer.txt 2
		printf_invalid_integer_for_${conversion})
endforeach()

foreach(conversion d u f c)
	test("csv-printf '%${conversion}' bad_float"
		printf/typed-values.csv data/empty.txt printf/invalid-float.txt 2
		printf_invalid_float_for_${conversion})
endforeach()

foreach(conversion d u c)
	test("csv-printf '%${conversion}' name"
		data/3-columns-3-rows.csv data/empty.txt printf/string-as-number.txt 2
		printf_string_as_${conversion})
endforeach()

test("csv-printf '%f' name" data/3-columns-3-rows.csv data/empty.txt printf/string-as-float.txt 2
	printf_string_as_float)

test("csv-printf '%s' number" printf/typed-values.csv data/empty.txt printf/integer-as-string.txt 2
	printf_integer_as_string)

test("csv-printf '%s' fraction" printf/typed-values.csv data/empty.txt printf/float-as-string.txt 2
	printf_float_as_string)

test("csv-printf '%p' fraction" printf/typed-values.csv data/empty.txt printf/float-as-pointer.txt 2
	printf_float_as_pointer)

test("csv-printf '%p' name" data/3-columns-3-rows.csv data/empty.txt printf/string-as-pointer.txt 2
	printf_string_as_pointer)

test([=[csv-printf '\q' number]=] printf/typed-values.csv data/empty.txt printf/unsupported-escape.txt 2
	printf_unsupported_escape)

test("csv-printf '%d' missing" printf/typed-values.csv data/empty.txt printf/unknown-column.txt 1
	printf_unknown_column)

test("csv-printf '%d' numbers" printf/typed-values.csv data/empty.txt printf/unsupported-type.txt 2
	printf_unsupported_type)

test("csv-printf '%s' name" data/quot-invalid.csv data/empty.txt data/quot-invalid-stderr.txt 2
	printf_invalid_csv)

test("csv-printf" data/empty.csv data/empty.txt printf/help.txt 2
	printf_no_format)

test("csv-printf --help" data/empty.csv printf/help.txt data/empty.txt 2
	printf_help)

test("csv-printf --version" data/empty.csv data/git-version.txt data/empty.txt 0
	printf_version)
