#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

filename="$PWD/$1"
sed -e '/^#: /d' -e '/^# Copyright/d' -e '/^"POT-Creation-Date/d' -e '/^#, c.*-format/d' "$filename"
