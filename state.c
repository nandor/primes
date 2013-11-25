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
#include "job.h"

/**
 * Creates a new state, initialising modules
 * @param state
 */
void state_create( state_t * state )
{
  // Initialise the job manager
  assert( state->job_mngr = (jobs_t*)malloc( sizeof( jobs_t ) ) );
  memset( state->job_mngr, 0, sizeof( jobs_t ) );
  jobs_create( state );

  // Initialise the thread manager
  assert( state->thread_mngr = (threads_t*)malloc( sizeof( threads_t ) ) );
  memset( state->thread_mngr, 0, sizeof( threads_t ) );
  threads_create( state );
}

/**
 * Bails out with an error message
 * @param state
 * @param fmt
 */
void state_error( state_t * state, const char * fmt, ... )
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
void state_run( state_t * state )
{
  threads_wait( state );
}

/**
 * Frees memory allocated by the state
 * @param state
 */
void state_destroy( state_t * state )
{
  if ( state )
  {
    if ( state->job_mngr )
    {
      jobs_destroy( state );
      free( state->job_mngr );
      state->job_mngr = 0;
    }

    if ( state->thread_mngr )
    {
      threads_destroy( state );
      free( state->thread_mngr );
      state->thread_mngr = 0;
    }
  }
}