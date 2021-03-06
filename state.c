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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "state.h"
#include "thread.h"
#include "chunk.h"
#include "job.h"

/**
 * Creates a new state, initialising modules
 * @param state
 */
void state_create( struct state * state )
{
  // Initialise the chunk manager
  assert( state->chunk_mngr = (struct chunks*)malloc( sizeof( struct chunks ) ) );
  memset( state->chunk_mngr, 0, sizeof( struct chunks ) );
  chunks_create( state );

  // Initialise the job manager
  assert( state->job_mngr = (struct jobs*)malloc( sizeof( struct jobs ) ) );
  memset( state->job_mngr, 0, sizeof( struct jobs ) );
  jobs_create( state );

  // Initialise the thread manager
  assert( state->thread_mngr = (struct threads*)malloc( sizeof( struct threads ) ) );
  memset( state->thread_mngr, 0, sizeof( struct threads ) );
  threads_create( state );
}

/**
 * Bails out with an error message
 * @param state
 * @param fmt
 */
void state_error( struct state * state, const char * fmt, ... )
{
  int size = 100, n;
  char * tmp;
  va_list ap;

  assert( state->err_msg = (char*)malloc( size ) );

  while ( 1 )
  {
    va_start( ap, fmt );
    n = vsnprintf( state->err_msg, size, fmt, ap );
    va_end( ap );

    if ( -1 < n && n < size )
      break;

    size = n > -1 ? ( n + 1 ) : ( size << 1 );
    if ( !( tmp = (char*)realloc( state->err_msg, size ) ) )
    {
      free( state->err_msg );
      break;
    }

    state->err_msg = tmp;
  }

  longjmp( state->err_jump, 1 );
}

/**
 * Runs the sieve
 */
void state_run( struct state * state )
{
  threads_wait( state );
}

/**
 * Frees memory allocated by the state
 * @param state
 */
void state_destroy( struct state * state )
{
  if ( state )
  {
    if ( state->job_mngr )
    {
      jobs_destroy( state );
      free( state->job_mngr );
      state->job_mngr = NULL;
    }

    if ( state->chunk_mngr )
    {
      chunks_destroy( state );
      free( state->chunk_mngr );
      state->chunk_mngr = NULL;
    }

    if ( state->thread_mngr )
    {
      threads_destroy( state );
      free( state->thread_mngr );
      state->thread_mngr = NULL;
    }

    if ( state->sieve_file )
    {
      free( state->sieve_file );
      state->sieve_file = NULL;
    }

    if ( state->primes_file )
    {
      free( state->primes_file );
      state->primes_file = NULL;
    }
  }
}
