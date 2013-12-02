
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

struct jobs;
struct threads;
struct chunks;

struct state
{
  /* Number of threads */
  int thread_count;

  /* Number of chunks */
  int chunk_count;

  /* Size of a chunk */
  uint64_t chunk_size;

  /* Sieve file name */
  char * sieve_file;

  /* Output file name */
  char * primes_file;

  /* Job manager */
  struct jobs * job_mngr;

  /* Thread manager */
  struct threads * thread_mngr;

  /* Chunk manager */
  struct chunks * chunk_mngr;

  /* Error handler */
  jmp_buf err_jump;

  /* Error message */
  char * err_msg;
};

void state_create( struct state * state );
void state_error( struct state * state, const char * fmt, ... );
void state_run( struct state * state );
void state_destroy( struct state * state );

#endif
