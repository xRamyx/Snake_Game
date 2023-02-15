 /******************************************************************************
 *
 * Module: XorShift
 *
 * File Name: XorShift.h
 *
 * Description: Header file for functions prototypes for XorShift module
 *
 * Author: Ramy Mohamed Ahmed Mohamed
 *
 *******************************************************************************/
#ifndef XORSHIFT_H_
#define XORSHIFT_H_

#include "std_types.h"


/****************************** Functions Prototypes ******************************/

/* This state represents the random position of power ups */
typedef struct xorshift32_state {
    uint32 a;
}xorshift32_state;

/* The state must be initialized to non-zero */
uint32 xorshift32(struct xorshift32_state *state);

#endif /* XORSHIFT_H_ */