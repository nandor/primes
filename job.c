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

struct column
{
  int n;
  int all;
  int working;
  int done;
};

void cross_out (struct state *, uint64_t);

/**
 * Run the first job
 */
void startup_job( struct state * s )
{
  uint8_t * bitset;
  uint64_t limit, i, j;
  struct chunks * c;

  if ( !( c = s->chunk_mngr ) )
      return;

  /* Allocate a separate bitset so we don't overwrite stuff */
  assert( bitset = (uint8_t*)malloc( s->chunk_size ) );
  memset( bitset, 0, s->chunk_size );

  limit = s->chunk_size << 4;
  for ( i = 1; i * ( i + 1ull ) << 1ull < limit; ++i )
  {
    if ( ! ( bitset[ i >> 3ull ] & ( 1 << ( i & 7 ) ) ) )
    {
      for ( j = i * ( i + 1ull ) << 1ull;
            ( j << 1ull ) + 1ull < limit;
            j += ( i << 1ull ) + 1ull )
      {
        bitset[ j >> 3ull ] |= 1ull << ( j & 7ull );
      }
    }
  }

  chunks_write_prime( s, 2 );
  for ( i = 1ull; ( i << 1ull ) + 1ull < limit; ++i )
  {
    if ( ! ( bitset[ i >> 3ull ] & ( 1ull << ( i & 7ull ) ) ) )
    {
      chunks_write_prime( s, ( i << 1ull ) + 1ull );
    }
  }

  free( bitset );

  c->primes_index[1] = 0;
  c->primes_index[2] = c->primes_count;


  printf("finshed startup job\n");
 }

/**
 * Initialises the job manager
 */
void jobs_create( struct state * s )
{
  struct jobs * j;
  size_t sz;
  int i;

  if ( !( j = s->job_mngr ) )
    return;

  /* Run the first job, chunk 1 */
  startup_job( s );

  /* Allocate storage for the queue */
  sz = sizeof( struct column ) * 100;
  assert( j->processed = (struct column*)malloc( sz ) );
  memset( j->processed, 0, sz );
  for ( i = 0; i < 100; i++)
  {
    j->processed[ i ].n = -1;
  }

  /* Setup */
  j->finished = 0;
  j->processed_until = 1;
  j->finished_until = 1;
  j->working_on = 0;
  j->aim = s->chunk_count;
  j->last_saved = 0;
}

/**
 * Destroys the job manager
 */
void jobs_destroy( struct state * s )
{
  struct jobs * j;

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
void jobs_run( struct state * s, struct job * job )
{
  struct jobs * j;
  struct chunks * c;

  if ( !( j = s->job_mngr ) || !( c = s->chunk_mngr ) )
    return;

  /* sleep( 1 ); */
  int divider_chunk = job->divider_chunk;
  int filtered_chunk = job->filtered_chunk;

  uint64_t first_filtered = (filtered_chunk - 1) * s->chunk_size * 8;
  uint64_t filter_until = (filtered_chunk) * s->chunk_size * 8;
  uint64_t act_filter;

  int i;
  for (i = c->primes_index[divider_chunk];
       i < c->primes_index[divider_chunk+1]; i++ )
  {
    act_filter = chunks_get_prime( s, i );
    uint64_t cancel_n = ((first_filtered * 2ull + 1ull) / act_filter) * act_filter;
    if (cancel_n < first_filtered )
    {
      cancel_n += act_filter;
    }
    while (cancel_n < filter_until * 2ull + 1ull)
    {
      cross_out(s, cancel_n);
      cancel_n += act_filter;
    }
  }



  printf( "thread %u filtered %d with %d\n", pthread_self(), job->filtered_chunk, job->divider_chunk );

}


inline void cross_out (struct state * s, uint64_t n) 
{

  if ( n & 1ull == 1)
  {
    n = n >> 1;
    uint64_t byte_number = n >> 3ull;
    uint64_t mod = n & 7ull;
    s->chunk_mngr->sieve_data[byte_number] = s->chunk_mngr->sieve_data[byte_number] | (1ull << mod);
  }
}

void jobs_save_finished (struct state * s, int n)
{
  printf("saved %d: \n",n);
  n--;
  uint64_t i;
  for (i = n * s->chunk_size * 8; i < (n+1) * s->chunk_size * 8; i++)
  {
    if ( ( s->chunk_mngr->sieve_data[i >> 3ull] & (1 << (i & 7) ) ) == 0)
    {
      chunks_write_prime( s, 2*i+1 );
    }
  }

  s->chunk_mngr->primes_index[n+1]=s->chunk_mngr->primes_count;
  /*for (int i = 0; i < s->chunk_mngr->primes_count; i++) 
  {
    printf("%u ", chunks_get_prime(s,i));
  }
  printf("\n\n");*/

}

/**
 * Finds the next job
 * @return Returns 1 if a job was found
 */
int jobs_next( struct state * s, struct job * job )
{
  struct jobs * j;

  if ( !( j = s->job_mngr ) )
    return 0;
  /*
  int kk = 0;
  for (kk = 0; kk<10; kk++)
  {
    printf("%d ", j->processed[kk].n);
  }
  printf("\n");
  */
  if ( j->finished )
    return 0;

  /* Finding next available job */
  struct job next;
  int next_index;
  next.filtered_chunk = INT_MAX;

  /* Looking for the smallest chunk we can work on */
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

  /* If there is no available job with current chunks */
  if ( next.filtered_chunk == INT_MAX )
  {
    /* If we can work on a new chunk */
    if ( j->processed_until < j->aim )
    {
      j->working_on++;
      ++j->processed_until;
      j->processed[k].n = j->processed_until;
      j->processed[k].all = j->processed[k].n-1;
      j->processed[k].working = j->processed[k].done = 0;
      next.filtered_chunk = j->processed[k].n;
      next_index = k;
    }
    else
    {
      /* There is no work to be done */
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
void jobs_finish( struct state * s, struct job * job, int * save )
{
  struct jobs * j;

  if ( !( j = s->job_mngr ) )
    return;

  if ( *save == 1 )
  {
    *save = 0;

    int save_k = 0;
    while (j->processed[save_k].n != job->filtered_chunk)
    {
      save_k++;
    }

    j->working_on--;

    /* If all of the previous chunks are finished */
    if ( j->finished_until+1 == j->processed[save_k].n )
    {
      j->finished_until++;
    }

    /* If this is the last chunk needed */
    if ( j->aim == j->finished_until )
    {
      if ( j->working_on == 0)
      {
        j->finished = 1;
        threads_finish( s );
      }
      return;
    }

    else
    {
      /* If there is a new chunk to be loaded to the place of the finished one */
      if ( j->processed_until < j->aim )
      {
        j->working_on++;
        ++j->processed_until;
        j->processed[save_k].n = j->processed_until;
        j->processed[save_k].all = j->processed_until-1;
        j->processed[save_k].working = j->processed[save_k].done = 0;
      }
    }

    *save = 0;
    return;
  }


  /* pre: state->processed contains the column for divider_chunk
   * Finding the chunk job was working on
   */
  int k = 0;
  while ( j->processed[k].n != job->filtered_chunk )
  {
    k++;
  }

  /* Updating, loading new if finished */
  if ( ++j->processed[k].done == j->processed[k].all )
  {
    /* Finished filtering a chunk
     * Save_finished_chunk(j->processed[k].n);
     */
    *save = 1;
    /* Information which chunk to store is in job->filtered_chunk. Do not change it!!
     * Code from here moved to the *save == 1 part
     */
  }
}

