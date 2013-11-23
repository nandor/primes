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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "thread.h"


/**
 * Thread function, calls job_run
 * @param pool_ptr Pointer to the thread pool
 */
void * thread_func( void * pool_ptr )
{
  thread_pool_t * pool;
  job_t job;
  int has_next;

  pool = (thread_pool_t*)pool_ptr;

  while ( pool->running ) 
  {
    pthread_mutex_lock( &pool->lock );
    has_next = job_next( pool->state, &job );
    pthread_mutex_unlock( &pool->lock );

    if ( !has_next )
    {
      job_run( &job );
    }
  }

  pthread_exit( NULL );
  return NULL;
}


/**
 * Start the threads which will process the jobs
 * @param pool
 * @param n     Number of threads
 */
void thread_pool_create( thread_pool_t * pool, job_state_t * state, int n )
{
  int i;
  size_t sz;

  assert( pool );

  pool->num_threads = n;
  pool->state = state;
  pool->running = 1;

  // Allocate storage space for the thread handles
  sz = sizeof( pthread_t ) * n;
  pool->threads = (pthread_t*)malloc( sz );
  assert( pool->threads );
  memset( pool->threads, 0, sz );

  // Create the lock object
  if ( pthread_mutex_init( &pool->lock, NULL ) )
  {
    fprintf( stderr, "Cannot create mutex\n" );
    exit( 0 );
  }

  // Create suspended threads
  for ( i = 0; i < n; ++i )
  {
    if ( pthread_create( &pool->threads[ i ], NULL, thread_func, pool ) )
    {
      fprintf( stderr, "Cannot create thread #%d\n", i );
      exit( 0 );
    }
  }
}


/**
 * Stops all the threads & frees allocated memory
 * @param pool
 */
void thread_pool_destroy( thread_pool_t * pool )
{
  int i;

  if ( !pool )
    return;

  // Tell all the threads to stop
  pool->running = 0;

  // Stop & free the threads
  if ( pool->threads )
  {
    for ( i = 0; i < pool->num_threads; ++i )
    {
      if ( pool->threads[ i ] != 0 )
      {
        // Waits for threads to finish
        pthread_join( pool->threads[ i ], NULL );
        pthread_detach( pool->threads[ i ] );
        pool->threads[ i ] = 0;
      }
    }

    free( pool->threads );
    pool->threads = NULL;
  }

  // Free the mutex
  pthread_mutex_destroy( &pool->lock );
}
