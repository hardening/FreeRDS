/**
 * xrdp: A Remote Desktop Protocol server.
 *
 * Copyright (C) Jay Sorg 2004-2012
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Parsing structs and macros
 *
 * based on parse.h from rdesktop
 * this is a super fast stream method, you bet
 * needed functions g_malloc, g_free, g_memset, g_memcpy
 */

#if !defined(PARSE_H)
#define PARSE_H

#include "arch.h"

#include <winpr/crt.h>
#include <winpr/stream.h>

#define s_check_rem_len(s, n) ((s)->pointer + (n) <= (s)->buffer + (s)->length)

#define make_stream(s) \
  (s) = (wStream*) g_malloc(sizeof(wStream), 1)

#define init_stream(s, v) do \
{ \
  if ((v) > (s)->capacity) \
  { \
    g_free((s)->buffer); \
    (s)->buffer = (BYTE*) g_malloc((v), 0); \
    (s)->capacity = (v); \
  } \
  (s)->pointer = (s)->buffer; \
  (s)->length = 0; \
} while (0)

#define free_stream(s) do \
{ \
  if ((s) != 0) \
  { \
    g_free((s)->buffer); \
  } \
  g_free((s)); \
} while (0)

#define in_sint8(s, v) do \
{ \
  (v) = *((signed char*)((s)->pointer)); \
  (s)->pointer++; \
} while (0)

#if defined(B_ENDIAN) || defined(NEED_ALIGN)
#define in_sint16_le(s, v) do \
{ \
  (v) = (signed short) \
    ( \
      (*((unsigned char*)((s)->pointer + 0)) << 0) | \
      (*((unsigned char*)((s)->pointer + 1)) << 8) \
    ); \
  (s)->pointer += 2; \
} while (0)
#else
#define in_sint16_le(s, v) do \
{ \
  (v) = *((signed short*)((s)->pointer)); \
  (s)->pointer += 2; \
} while (0)
#endif

#define in_uint16_be(s, v) do \
{ \
  (v) = *((unsigned char*)((s)->pointer)); \
  (s)->pointer++; \
  (v) <<= 8; \
  (v) |= *((unsigned char*)((s)->pointer)); \
  (s)->pointer++; \
} while (0)

#define in_uint32_be(s, v) do \
{ \
  (v) = *((unsigned char*)((s)->pointer)); \
  (s)->pointer++; \
  (v) <<= 8; \
  (v) |= *((unsigned char*)((s)->pointer)); \
  (s)->pointer++; \
  (v) <<= 8; \
  (v) |= *((unsigned char*)((s)->pointer)); \
  (s)->pointer++; \
  (v) <<= 8; \
  (v) |= *((unsigned char*)((s)->pointer)); \
  (s)->pointer++; \
} while (0)

#if defined(B_ENDIAN) || defined(NEED_ALIGN)
#define out_uint16_le(s, v) do \
{ \
  *((s)->pointer) = (unsigned char)((v) >> 0); \
  (s)->pointer++; \
  *((s)->pointer) = (unsigned char)((v) >> 8); \
  (s)->pointer++; \
} while (0)
#else
#define out_uint16_le(s, v) do \
{ \
  *((unsigned short*)((s)->pointer)) = (unsigned short)(v); \
  (s)->pointer += 2; \
} while (0)
#endif

#define out_uint16_be(s, v) do \
{ \
  *((s)->pointer) = (unsigned char)((v) >> 8); \
  (s)->pointer++; \
  *((s)->pointer) = (unsigned char)((v) >> 0); \
  (s)->pointer++; \
} while (0)

#define out_uint32_be(s, v) do \
{ \
  *((s)->pointer) = (unsigned char)((v) >> 24); \
  s->pointer++; \
  *((s)->pointer) = (unsigned char)((v) >> 16); \
  s->pointer++; \
  *((s)->pointer) = (unsigned char)((v) >> 8); \
  s->pointer++; \
  *((s)->pointer) = (unsigned char)(v); \
  (s)->pointer++; \
} while (0)

#endif
