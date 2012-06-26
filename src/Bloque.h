//
// File: Bloque.h
//


/*
 * A generic, vector-like container class that:
 *  - is implementated like a deque, hence the name
 *  - uses a vector of dynamic arrays as underlying container
 *  - never invalidates iterators/pointers/references as new blocks are allocated
 *  - never shrinks; doesn't allow deletion of individual elements
 *  - reclaims items marked for recycling (instead of deleting)
 *  - supports additional arbitrary bitmasks to control iteration
 *  - thread-safe
 *  - can traverse container and apply an arbitrary functor to each (possibly in parallel)
 *
 *  Created 2012 by J. DePasse (jvd10@pitt.edu)
 *
 */

