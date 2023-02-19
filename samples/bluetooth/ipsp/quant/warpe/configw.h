// SPDX-License-Identifier: BSD-2-Clause
//
// Copyright (c) 2014-2022, NetApp, Inc.
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

/// Major version of warpcore.
#define WARPCORE_VERSION_MAJOR 1

/// Minor version of warpcore.
#define WARPCORE_VERSION_MINOR 7

/// Patch level of warpcore.
#define WARPCORE_VERSION_PATCH 2

/// The name of this library (warpcore).
extern const char warpcore_name[];

/// The version of this library (1.7.2) as a string.
extern const char warpcore_version[];

#ifndef HAVE_64BIT
#define HAVE_64BIT 1
#endif
#if 1 // todo: fix this
//#define HAVE_ASAN
//#define HAVE_BACKTRACE
#define HAVE_ENDIAN_H
//#define HAVE_EPOLL
/* #undef HAVE_KQUEUE */
//#define HAVE_RECVMMSG
//#define HAVE_SENDMMSG
#endif
/* #undef HAVE_SYS_ENDIAN_H */
