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

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "job.h"
#include "chunk.h"
#include "state.h"
#include "thread.h"

typedef struct _column_t
{
  int n;
  int all;
  int working;
  int done;
} column_t;

/**
 * Run the first job
 */
void startup_job( state_t * s )
{
  printf( "Startup job" );
}

/**
 * Initialises the job manager
 */
void jobs_create( state_t * s )
{
  jobs_t * j;
  job_t init;
  size_t sz;
  int i;

  if ( !( j = s->job_mngr ) )
    return;

  // Run the first job, chunk 1
  startup_job( s );

  // Allocate storage for the queue
  sz = sizeof( column_t ) * 100;
  assert( j->processed = (column_t*)malloc( sz ) );
  memset( j->processed, 0, sz );
  for ( i = 0; i < 100; i++)
  {
    j->processed[ i ].n = -1;
  }

  // Setup
  j->finished = 0;
  j->processed_until = 1;
  j->finished_until = 1;
  j->working_on = 0;
  j->aim = 3;

}

/**
 * Destroys the job manager
 */
void jobs_destroy( state_t * s )
{
  jobs_t * j;

  if ( !s || !( j = s->job_mngr ) )
    return;

  if ( j->processed )
  {
    free( j->processed );
    j->processed = NULL;
  }
}

/**
 * Executes a job
 */
void jobs_run( state_t * s, job_t * job )
{
  sleep( 1 );
  printf( "filtered %d with %d on thread %u\n", job->filtered_chunk, job->divider_chunk,
                                                pthread_self( ) );
}

/**
 * Finds the next job
 * @return Returns 1 if a job was found
 */
int jobs_next( state_t * s, job_t * job )
{
  jobs_t * j;

  if ( !( j = s->job_mngr ) )
    return 0;

  /*
  int kk = 0;
  for (kk = 0; kk<10; kk++)
  {
    printf("%d ",state->processed[kk].n);
  }
  printf("\n");
  */

  // Finding next available job
  job_t next;
  int next_index;
  next.filtered_chunk = INT_MAX;

  // Looking for the smallest chunk we can work on
  int k = 0;
  while ( j->processed[k].n != -1 )
  {
    if (j->processed[k].working < j->finished_until
        && j->processed[k].working < j->processed[k].all
        && j->processed[k].n < next.filtered_chunk )
    {
      next.filtered_chunk = j->processed[k].n;
      next_index = k;
    }
    k++;
  }

  // If there is no available job with current chunks
  if ( next.filtered_chunk == INT_MAX )
  {
    // If we can work on a new chunk
    if ( j->processed_until < j->aim )
    {
      j->working_on++;
      load_new_chunk(++j->processed_until);
      j->processed[k].n = j->processed_until;
      j->processed[k].all = j->processed[k].n-1;
      j->processed[k].working = j->processed[k].done = 0;
      next.filtered_chunk = j->processed[k].n;
      next_index = k;
    }
    else
    {
      // There is no work to be done
      return 0;
    }
  }

  next.divider_chunk = ++j->processed[next_index].working;
  *job = next;
  return 1;
}

/**
 * Marks a job as finished so other threads can fetch jobs
 * which depend on this one, calls threads_finish when there
 * are no more available jobs
 * @param s
 * @param job
 */
void jobs_finish( state_t * s, job_t * job )
{
  jobs_t * j;

  if ( !( j = s->job_mngr ) )
    return;

  // pre: state->processed contains the column for divider_chunk
  // Finding the chunk job was working on
  int k = 0;
  while ( j->processed[k].n != job->filtered_chunk )
  {
    k++;
  }

  // Updating, loading new if finished
  if ( ++j->processed[k].done == j->processed[k].all )
  {
    // Finished filtering a chunk
    save_finished_chunk(j->processed[k].n);
    j->working_on--;

    // If all of the previous chunks are finished
    if ( j->finished_until+1 == j->processed[k].n )
    {
      j->finished_until++;
    }

    // If this is the last chunk needed
    if ( j->aim == j->finished_until )
    {
      if ( j->working_on == 0)
      {
        j->finished = 1;
      }
      return;
    }
    else
    {
      // If there is a new chunk to be loaded to the place of the finished one
      if ( j->processed_until < j->aim )
      {
        j->working_on++;
        load_new_chunk(++j->processed_until);
        j->processed[k].n = j->processed_until;
        j->processed[k].all = j->processed_until-1;
        j->processed[k].working = j->processed[k].done = 0;
      }
    }
  }
}

