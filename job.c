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
#include <limits.h>
#include <string.h>
#include "job.h"
#include "chunk.h"



/**
 * Initialises the state
 */
void job_state_create( job_state_t * state )
{
  job_t init;
  init.divider_chunk = -1;
  init.filtered_chunk = 1;
  job_run(&init);

  state->finished = 0;
  state->processed_until = 1;
  state->finished_until = 1;
  state->working_on = 0;
  // First try
  state->aim = 20;
  int i;
  memset( &state->processed, 0, sizeof( state->processed ) );

  for (i=0; i<100; i++)
  {
    state->processed[i].n = -1;
  }
}


/**
 * Destroys the job state
 */
void job_state_destroy( job_state_t * state )
{

}


/**
 * Executes a job
 */
void job_run( job_t * job )
{
  job->finished = 0;
  printf( "filtered %d with %d on thread %u\n", job->filtered_chunk, job->divider_chunk,
                                                pthread_self( ) );
  job->finished = 1;
  sleep(1);
}


/**
 * Returns information about the next chunk which should be processed
 * @param job
 * @return Returns 0 is no jobs are available
 */
int job_next( job_state_t * state, job_t * job )
{
  // No more jobs left
  if ( state->finished )
  {
    return 0;
  }

  // updating state with finished job, saving/loading new chunks if necessary
  if ( job->finished ) 
  {
    // pre: state->processed contains the column for divider_chunk 
    // Finding the chunk job was working on
    int k = 0;
    while ( state->processed[k].n != job->filtered_chunk ) 
    {
      k++;
    }

    // Updating, loading new if finished
    if ( ++state->processed[k].done == state->processed[k].all )
    {
      // Finished filtering a chunk
      save_finished_chunk(state->processed[k].n);
      state->working_on--;

      // If all of the previous chunks are finished
      if ( state->finished_until+1 == state->processed[k].n ) 
      {
        state->finished_until++;
      }

      // If this is the last chunk needed
      if ( state->aim == state->finished_until )
      {
        if ( state->working_on == 0)
        {
          state->finished = 1;
        }
        return 0;
      }
      else
      {
        // If there is a new chunk to be loaded to the place of the finished one
        if ( state->processed_until < state->aim )
        {
          state->working_on++;
          load_new_chunk(++state->processed_until);
          state->processed[k].n = state->processed_until;
          state->processed[k].all = state->processed_until-1;
          state->processed[k].working = state->processed[k].done = 0;
        }
      }
    }
  }
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
  while ( state->processed[k].n != -1 )
  {
    if (state->processed[k].working < state->finished_until
        && state->processed[k].working < state->processed[k].all
        && state->processed[k].n < next.filtered_chunk )
    {
      next.filtered_chunk = state->processed[k].n;
      next_index = k;
    }
    k++;
  }

  // If there is no available job with current chunks
  if ( next.filtered_chunk == INT_MAX )
  {
    // If we can work on a new chunk
    if ( state->processed_until < state->aim )
    {
      state->working_on++;
      load_new_chunk(++state->processed_until);
      state->processed[k].n = state->processed_until;
      state->processed[k].all = state->processed[k].n-1;
      state->processed[k].working = state->processed[k].done = 0;
      next.filtered_chunk = state->processed[k].n;
      next_index = k;
    }
    else
    {
      // There is no work to be done
      job->finished = 0;
      return 0;
    }
  }

  next.divider_chunk = ++state->processed[next_index].working;
  *job = next;
  return 1;
}
