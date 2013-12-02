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

void chunks_create( struct state * s )
{
  struct chunks * c;
  uint8_t zero = 0;

  if ( !( c = s->chunk_mngr) )
  {
    return;
  }

  /* Prepares the output file for the primes */
  c->primes_capacity = 1;
  c->primes_count = 0;
  c->primes_size = c->primes_capacity * sizeof( uint64_t );
  if ( ( c->primes_fd = open( s->primes_file, O_CREAT |
                              O_RDWR | O_TRUNC, 0666 ) ) < 0 )
  {
    state_error( s, "Cannot open file '%s'", s->primes_file );
  }

  lseek( c->primes_fd, c->primes_size - 1, SEEK_SET );
  write( c->primes_fd, &zero, 1 );
  lseek( c->primes_fd, 0, SEEK_SET );

  /* Create the index which will store the address of the
   * first prime in the output array
   */
  c->primes_index = (uint64_t*)malloc( sizeof(uint64_t) * s->chunk_count );
  if ( !c->primes_index )
  {
    state_error( s, "Cannot create index" );
  }

  /* mmap the output file */
  if ( ( c->primes_data = mmap( 0, c->primes_size, PROT_READ | PROT_WRITE,
                                MAP_SHARED, c->primes_fd, 0 ) ) == MAP_FAILED )
  {
    state_error( s, "Cannot mmap file '%s'", s->primes_file );
  }

  /* Open the chunk cache */
  c->sieve_chunks = s->chunk_count;
  c->sieve_size = c->sieve_chunks * s->chunk_size;
  if ( ( c->sieve_fd = open( s->sieve_file, O_CREAT |
                             O_RDWR | O_TRUNC, 0666 ) ) < 0 )
  {
    state_error( s, "Cannot open chunk cache '%s'", s->sieve_file );
  }

  lseek( c->sieve_fd, c->sieve_size - 1, SEEK_SET );
  write( c->sieve_fd, &zero, 1 );
  lseek( c->sieve_fd, 0, SEEK_SET );

  /* mmap the chunk cache */
  if ( ( c->sieve_data = mmap( 0, c->sieve_size, PROT_READ | PROT_WRITE,
                               MAP_SHARED, c->sieve_fd, 0 ) ) == MAP_FAILED )
  {
    state_error( s, "Cannot mmap file '%s'", s->sieve_file );
  }
}

void chunks_destroy( struct state * s )
{
  struct chunks * c;

  if ( !( c = s->chunk_mngr) )
    return;

  if ( c->primes_index )
  {
    free( c->primes_index );
    c->primes_index = NULL;
  }

  if ( c->primes_data )
  {
    munmap( c->primes_data, c->primes_size );
    c->primes_data = NULL;
  }

  if ( c->primes_fd > 0)
  {
    close( c->primes_fd );
    c->primes_fd = -1;
  }

  if ( c->sieve_data )
  {
    munmap( c->sieve_data, c->sieve_size );
    c->sieve_data = NULL;
  }

  if ( c->sieve_fd > 0)
  {
    close( c->sieve_fd );
    c->sieve_fd = -1;
  }
}

void chunks_write_prime( struct state * s, uint64_t prime )
{
  struct chunks * c;
  uint64_t * addr;

  if ( !( c = s->chunk_mngr ) )
    return;

  /* Double the size of the output file */
  if ( c->primes_count >= c->primes_capacity )
  {
    if ( ftruncate( c->primes_fd, c->primes_size << 1ull ) < 0 )
    {
      state_error( s, "Cannot resize output size" );
    }

    if ( ( addr = mremap( c->primes_data, c->primes_size,
                          c->primes_size << 1, MREMAP_MAYMOVE ) ) == MAP_FAILED )
    {
      state_error( s, "Cannot remap output file '%s'", s->primes_file );
    }

    c->primes_size <<= 1;
    c->primes_capacity <<= 1;
    c->primes_data = addr;
  }

  c->primes_data[ c->primes_count++ ] = prime;
}

uint64_t chunks_get_prime( struct state * s, uint64_t idx )
{
  struct chunks * c;

  if ( !( c = s->chunk_mngr ) )
  {
    return 0ll;
  }

  return 2ull;
}
