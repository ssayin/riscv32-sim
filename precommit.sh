#!/usr/bin/bash

# SPDX-FileCopyrightText: 2022 - 2023 Serdar Sayın <https://serdarsayin.com>
#
# SPDX-License-Identifier: MIT

git diff --name-only | grep "\.hpp$\|\.cpp$" | xargs clang-format -i --verbose
