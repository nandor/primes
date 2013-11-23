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
#include <getopt.h>
#include "job.h"
#include "chunk.h"
#include "thread.h"


/**
 * Holds the configuration
 */
typedef struct
{
  /// Number of threads
  int num_threads;
  /// Size of a chunk ( multiple of MB )
  int chunk_size;
} options_t;


/**
 * Prints the commad line options
 */
void print_options( )
{
  fputs( "primes - multithreaded prime sieve", stderr );
  fputs( "Usage: primes [args]\n", stderr );
  fputs( "  --threads=<count> Sets the number of threads\n", stderr );
  fputs( "  --chunk=<count>   Sets the size of a chunk", stderr );
}


/**
 * Parses command line arguments
 * @param opt  Options object
 * @param argc
 * @param argv
 */
void read_options( options_t * opt, int argc, char ** argv )
{
  int c, idx;

  opt->num_threads = 2;
  opt->chunk_size  = 128 << 20;

  static struct option desc[ ] =
  {
    { "threads", required_argument, 0, 't' },
    { "chunk",   required_argument, 0, 'c' },
    { "help",    no_argument,       0, 'h' }
  };

  while ( ( c = getopt_long( argc, argv, "t:c:h", desc, &idx ) ) != -1 )
  {
    switch ( c )
    {
      case 't':
      {
        opt->num_threads = atoi( optarg );
        break;
      }
      case 'c':
      {
        opt->chunk_size = atoi( optarg ) << 20;
        break;
      }
      case 'h': 
      {
        print_options( );
        exit( EXIT_FAILURE );
      }
    }
  }
}


/**
 * Checks if the arguments are valid
 * @param opt Options object
 */
void check_options( options_t * opt )
{
  if ( opt->num_threads < 1 )
  {
    fprintf( stderr, "Invalid thread count: %d\n", opt->num_threads );
    exit( EXIT_FAILURE );
  }

  if ( opt->chunk_size < ( 1 << 20 ) )
  {
    fprintf( stderr, "Invalid chunk size: %d\n", opt->chunk_size );
    exit( EXIT_FAILURE );
  }
}


/**
 * Entry point of the application
 */
int main( int argc, char ** argv )
{
  options_t opt;

  read_options( &opt, argc, argv );
  check_options( &opt );

  return EXIT_SUCCESS;
}
