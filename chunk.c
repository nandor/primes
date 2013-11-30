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
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "chunk.h"
#include "state.h"

void chunks_create( state_t * s )
{
  chunks_t * c;
  void * data;
  struct stat st;

  if ( !( c = s->chunk_mngr) )
    return;

  c->first_prime_index = (uint64_t*)malloc( sizeof(uint64_t) * s->chunk_count );
  if ( !c->first_prime_index )
    state_error( s, "Cannot create chunk index map" );

  if ( ( c->fd = open( s->cache_file, O_CREAT | O_RDWR, 0666 ) ) < 0 )
    state_error( s, "Cannot create cache file '%s'", s->cache_file );

  if ( fstat( c->fd, &st ) < 0 )
    state_error( s, "Cannot retrieve cache file size" );

  c->dataSize = s->chunk_count * s->chunk_size;
  printf("%u\n",c->dataSize);
  if ( st.st_size < c->dataSize )
  {
    uint8_t end = 0;

    lseek( c->fd, s->chunk_count * s->chunk_size - 1, SEEK_SET );
    write( c->fd, &end, 1 );
    lseek( c->fd, 0, SEEK_SET );
  }

  data = mmap( 0, c->dataSize, PROT_READ | PROT_WRITE, MAP_SHARED, c->fd, 0 );
  if ( data == MAP_FAILED )
  {
    state_error( s, "Cannot mmap file" );
  }

  c->data_sieve = (uint8_t*)data;
  c->data_primes = (uint64_t*)data;
}

void chunks_destroy( state_t * s )
{
  chunks_t * c;

  if ( !( c = s->chunk_mngr) )
    return;

  if ( c->data_sieve )
  {
    munmap( c->data_sieve, c->dataSize );
    c->data_sieve = NULL;
    c->data_primes = NULL;
  }

  if ( c->first_prime_index )
  {
    free( c->first_prime_index );
    c->first_prime_index = NULL;
  }

  if ( c->fd > 0) {
    close( c-> fd );
    c->fd = -1;
  }
}
