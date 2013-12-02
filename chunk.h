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

#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>

typedef struct _state_t state_t;

typedef struct _chunks_t
{
  /// File descriptor of the output
  int primes_fd;

  /// mmapped primes_fd
  uint64_t * primes_data;

  /// Number of primes written
  uint64_t primes_count;

  /// Available storage for primes
  uint64_t primes_capacity;

  /// Size of the primes file in bytes
  size_t primes_size;

  // Maps the index of the first prime in each chunk
  uint64_t * primes_index;

  /// File descriptor of the sieve
  int sieve_fd;

  // Number of chunks stored
  uint64_t sieve_chunks;

  /// Size of the sieve cache
  size_t sieve_size;

  /// Individual bits accessed by the sieve
  uint8_t * sieve_data;
} chunks_t;

void     chunks_create( state_t * );
void     chunks_destroy( state_t * );
void     chunks_write_prime( state_t *, uint64_t );
uint64_t chunks_get_prime( state_t *, uint64_t );

#endif
