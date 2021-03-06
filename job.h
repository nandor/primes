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

#ifndef JOB_H
#define JOB_H

struct state;
struct column;

struct job
{
  int divider_chunk;
  int filtered_chunk;
};


struct jobs
{
  struct column * processed;
  uint8_t * chunk_saved;
  int processed_until;
  int finished_until;
  int aim;
  int working_on;
  int finished;
  int last_saved;
};

void jobs_create( struct state * );
void jobs_destroy( struct state * );
void jobs_run( struct state *, struct job * );
int  jobs_next( struct state *, struct job * );
void jobs_finish( struct state *, struct job *, int * save );
void jobs_save_finished (struct state * s, int n);

#endif
