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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "state.h"

/**
 * Prints the commad line options
 */
void print_options( )
{
  fputs( "primes - multithreaded prime sieve                   \n", stderr );
  fputs( "Usage: primes [args]                                 \n", stderr );
  fputs( "  --threads=<count>      Sets the number of threads  \n", stderr );
  fputs( "  --chunks=<count>       Sets the number of chunks   \n", stderr );
  fputs( "  --size=<size>          Sets the size of a chunk    \n", stderr );
  fputs( "  --sieve_file=<path>)   Chooses a file for the cache\n", stderr );
  fputs( "  --primes_file=<path>)  Chooses an output file      \n", stderr );
}


/**
 * Parses command line arguments
 * @param state
 * @param argc
 * @param argv
 */
void read_options( state_t * s, int argc, char ** argv )
{
  int c, idx;

  s->thread_count = 2;
  s->chunk_count = 10;
  s->chunk_size = 1ll << 20;
  s->sieve_file = strdup( "sieve.bin" );
  s->primes_file = strdup( "primes.bin" );

  static struct option desc[ ] =
  {
    { "threads",     required_argument, 0, 't' },
    { "chunks",      required_argument, 0, 'c' },
    { "size",        required_argument, 0, 's' },
    { "sieve_file",  required_argument, 0, 'f' },
    { "primes_file", required_argument, 0, 'o' },
    { "help",        no_argument,       0, 'h' }
  };

  while ( ( c = getopt_long( argc, argv, "t:c:s:f:h", desc, &idx ) ) != -1 )
  {
    switch ( c )
    {
      case 't':
      {
        s->thread_count = atoi( optarg );
        break;
      }
      case 'c':
      {
        s->chunk_count = atoi( optarg );
        break;
      }
      case 's':
      {
        s->chunk_size = (int64_t)atoi( optarg ) << 20;
        break;
      }
      case 'f':
      {
        if ( s->sieve_file )
          free( s->sieve_file );

        s->sieve_file = strdup( optarg );
        break;
      }
      case 'o':
      {
        if ( s->primes_file )
          free( s->primes_file );

        s->primes_file = strdup( optarg );
        break;
      }
      case 'h':
      {
        print_options( );
        state_destroy( s );
        exit( EXIT_SUCCESS );
      }
    }
  }
}


/**
 * Checks if the arguments are valid
 * @param state
 */
void check_options( state_t * s )
{
  if ( s->thread_count < 1 )
  {
    state_error( s, "Invalid thread count: %d", s->thread_count );
  }

  if ( s->chunk_count < 1 )
  {
    state_error( s, "Invalid chunk count: %d", s->chunk_count );
  }

  if ( s->chunk_size < ( 1 << 20 ) )
  {
    state_error( s, "Invalid chunk size: %lld", s->chunk_size );
  }
}


/**
 * Entry point of the application
 */
int main( int argc, char ** argv )
{
  state_t state;

  memset( &state, 0, sizeof( state ) );

  // Catch errors
  if ( setjmp( state.err_jump ) )
  {
    if ( state.err_msg )
      fprintf( stderr, "%s\n", state.err_msg );

    state_destroy( &state );
    return EXIT_FAILURE;
  }

  // Configure
  read_options( &state, argc, argv );
  check_options( &state );

  // Run
  state_create( &state );
  state_run( &state );
  state_destroy( &state );

  pthread_exit( NULL );
  return EXIT_SUCCESS;
}
