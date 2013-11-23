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
#include "job.h"


/**
 * Initialises the state
 */
void job_state_create( job_state_t * state )
{
  state->finished = 0;
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
  int count = 0, i;
  for ( i = 0; i < 1000000; ++i ) 
  {
    if ( i & 1 )
    {
      ++count;
    }
  }
}


/**
 * Returns information about the next chunk which should be processed
 * @param job
 * @return Returns -1 if no jobs available
 */
int job_next( job_state_t * state, job_t * job )
{
  static int a = 0;
  if ( ++a >= 10000 ) {
    state->finished = 1;
    return -1;
  }

  job->divider_chunk = job->filtered_chunk = a;
  return 0;
}
