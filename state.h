
/******************************************************************************
The MIT License (MIT)

Copyright (c) 2013 Nandor Licker, Daniel Simig

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
******************************************************************************/

#ifndef STATE_H
#define STATE_H

#include <setjmp.h>
#include <stdint.h>

typedef struct _jobs_t    jobs_t;
typedef struct _threads_t threads_t;
typedef struct _chunks_t  chunks_t;

typedef struct _state_t
{
  /// Number of threads
  int thread_count;

  /// Number of chunks
  int chunk_count;

  /// Size of a chunk
  int64_t chunk_size;

  /// Max cache usage
  int64_t cache_limit;

  /// Sieve file name
  char * cache_file;

  /// Job manager
  jobs_t * job_mngr;

  /// Thread manager
  threads_t * thread_mngr;

  /// Chunk manager
  chunks_t * chunk_mngr;

  /// Error handler
  jmp_buf err_jump;

  /// Error message
  char * err_msg;
} state_t;

void state_create( state_t * state );
void state_error( state_t * state, const char * fmt, ... );
void state_run( state_t * state );
void state_destroy( state_t * state );

#endif
