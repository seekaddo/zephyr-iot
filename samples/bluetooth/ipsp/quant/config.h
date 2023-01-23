// SPDX-License-Identifier: BSD-2-Clause
//
// Copyright (c) 2016-2022, NetApp, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include <stddef.h>
#include <stdint.h>


// IWYU pragma: private, include <quant/quant.h>

#define QUANT "quant"

#define QUANT_VERSION_MAJOR 0
#define QUANT_VERSION_MINOR 0
#define QUANT_VERSION_PATCH 34

#define QUANT_COMMIT_HASH_STR "00"
#define QUANT_COMMIT_HASH_ABBREV_STR "00"

/// The name of this library.
extern const char quant_name[];

/// The version of this library.
extern const char quant_version[];

/// The IETF QUIC draft version implemented by the library.
#define DRAFT_VERSION 34
#define DRAFT_VERSION_STRING "34"

extern const uint8_t quant_commit_hash_abbrev[];
extern const size_t quant_commit_hash_abbrev_len;

extern const uint8_t quant_commit_hash[];
extern const size_t quant_commit_hash_len;

//#define HAVE_ASAN
