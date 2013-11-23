# This file is part of the primes project
# Licensing information can be found in the LICENSE file

PROJECT( primes )

SET( SOURCES chunk.c
             job.c
             main.c
             thread.c )

SET( HEADERS chunk.h
             job.h
             thread.h )

ADD_EXECUTABLE( primes ${SOURCES} ${HEADERS} )
