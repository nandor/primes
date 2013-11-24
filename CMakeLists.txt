# This file is part of the primes project
# Licensing information can be found in the LICENSE file

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

SET( CMAKE_C_FLAGS -g )

ADD_EXECUTABLE( primes ${SOURCES} ${HEADERS} )
TARGET_LINK_LIBRARIES( primes ${LIBS} )
