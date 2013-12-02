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
#include "state.h"
#include "job.h"
#include "thread.h"

/**
 * Thread function
 * @param sp State pointer
 */
void * thread_func( void * sp )
{
  struct job job;
  struct state * s;
  struct threads * t;
  int has_next, must_save;

  if ( !( s = (struct state*)sp ) || !( t = s->thread_mngr ) )
    pthread_exit( NULL );

  has_next = 0, must_save = 0;
  while ( t->running )
  {
    pthread_mutex_lock( &t->queue_lock );

    // must_save will be one if a chunk must be saved
    // if jobs_finish will receive 1 for must_save,
    // it will know that it can execute stuff which
    // needs to be after a chunk is saved
    if ( has_next )
    {
      jobs_finish( s, &job, &must_save );
    }

    // If the last processed chunk must be saved,
    // we lock on c->data_primes and save it
    if ( must_save )
    {
      pthread_mutex_unlock( &t->queue_lock );

      pthread_mutex_lock( &t->save_lock );
      jobs_save_finished( s, job.filtered_chunk);
      pthread_mutex_unlock( &t->save_lock );
    }

    // Otherwise, we process a new chunk
    else
    {
      has_next = jobs_next( s, &job );
      pthread_mutex_unlock( &t->queue_lock );

      if ( has_next )
        jobs_run( s, &job );
    }
  }

  pthread_exit( NULL );
}

/**
 * Creates new threads
 * @param s
 */
void threads_create( struct state * s )
{
  int i;
  size_t sz;
  struct threads * t;
  pthread_attr_t attr;

  if ( !( t = s->thread_mngr ) )
    return;

  t->running = 1;
  t->finished = 0;

  // Initialise the mutex which will sync the job queue
  if ( pthread_mutex_init( &t->queue_lock, NULL ) )
    state_error( s, "Cannot create queue mutex" );

  // Initialise the mutex which will be used with the signal
  if ( pthread_mutex_init( &t->exit_lock, NULL ) )
    state_error( s, "Cannot create exit mutex" );

  // Initialse the mutex which will guard the data_primes array
  if ( pthread_mutex_init( &t->save_lock, NULL ) )
    state_error( s, "Cannot create save mutex" );

  // Initialise the cond variable which will signal
  // the main thread when we're done
  if ( pthread_cond_init( &t->exit_cond, NULL) )
    state_error( s, "Cannot create exit signal" );

  // Allocate storage space for the thread handles
  sz = sizeof( pthread_t ) * s->thread_count;
  assert( t->threads = (pthread_t*)malloc( sz ) );
  memset( t->threads, 0, sz );

  // Create joinable threads with 2Mb stack
  pthread_attr_init( &attr );
  pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );
  pthread_attr_setstacksize( &attr, 2 << 20 );

  for ( i = 0; i < s->thread_count; ++i )
  {
    if ( pthread_create( &t->threads[ i ], &attr, thread_func, s ) )
      state_error( s, "Cannot create thread #%d", i );
  }

  pthread_attr_destroy( &attr );
}

/**
 * Cleanup
 * @param s
 */
void threads_destroy( struct state * s )
{
  struct threads * t;
  int i;

  if ( !( t = s->thread_mngr ) )
    return;

  t->running = 0;

  if ( t->threads )
  {
    for ( i = 0; i < s->thread_count; ++i )
    {
      if ( t->threads[ i ] != 0 )
      {
        // Waits for threads to finish
        pthread_join( t->threads[ i ], NULL );
        pthread_detach( t->threads[ i ] );
        t->threads[ i ] = 0;
      }
    }

    free( t->threads );
    t->threads = NULL;
  }

  pthread_mutex_destroy( &t->queue_lock );
  pthread_mutex_destroy( &t->exit_lock );
  pthread_mutex_destroy( &t->save_lock );
  pthread_cond_destroy( &t->exit_cond );
}

/**
 * Waits until it gets a signal which says that
 * all threads finished their jobs
 * @param s
 */
void threads_wait( struct state * s )
{
  struct threads * t;

  if ( !( t = s->thread_mngr ) || !t->running )
    return;

  pthread_mutex_lock( &t->exit_lock );

  while ( !t->finished )
    pthread_cond_wait( &t->exit_cond, &t->exit_lock );

  pthread_mutex_unlock( &t->exit_lock );
}

/**
 * Wakes up the main thread when the threads finish
 * @param s
 */
void threads_finish( struct state * s )
{
  struct threads * t;

  if ( !( t = s->thread_mngr ) || !t->running )
    return;

  pthread_mutex_lock( &t->exit_lock);

  t->finished = 1;
  pthread_cond_signal( &t->exit_cond );

  pthread_mutex_unlock( &t->exit_lock);
}
