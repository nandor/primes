# This file is part of the primes project
# Licensing information can be found in the LICENSE file
# (C) 2013 Nandor Licker, Daniel Simig
CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )
PROJECT( primes )

SET( SOURCES chunk.c
             job.c
             main.c
             state.c
             thread.c)

SET( HEADERS chunk.h
             job.h
             state.h
             thread.h )

SET( LIBS pthread )

ADD_DEFINITIONS( -D_GNU_SOURCE )
SET( CMAKE_C_FLAGS "-g -m64 -std=c99 -pedantic -Wall -Werror" )

ADD_EXECUTABLE( primes ${SOURCES} ${HEADERS} )
TARGET_LINK_LIBRARIES( primes ${LIBS} )
