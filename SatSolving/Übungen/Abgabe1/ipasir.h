#ifndef IPASIR_H
#define IPASIR_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Return the name and the version of the incremental SAT
 * solving library.
 */
const char * ipasir_signature (void);

/**
 * Construct a new solver and return a pointer to it.
 * Use the returned pointer as the first parameter in each
 * of the following functions.
 */
void * ipasir_init (void);

/**
 * Release the given solver resources.
 */
void ipasir_release (void * solver);

/**
 * Add the given literal into the clause currently under construction.
 * Literal 0 terminates the clause.
 */
void ipasir_add (void * solver, int lit_or_zero);

/**
 * Add an assumption for the next solve call.
 */
void ipasir_assume (void * solver, int lit);

/**
 * Solve the formula with given assumptions.
 * Return:
 *   10 if the formula is satisfiable
 *   20 if the formula is unsatisfiable
 *    0 if interrupted
 */
int ipasir_solve (void * solver);

/**
 * Get the truth value of the given literal in the found model.
 * Return:
 *   > 0 if literal is true
 *   < 0 if literal is false
 *     0 if literal is not assigned
 */
int ipasir_val (void * solver, int lit);

/**
 * Set a callback function used to notify the caller of learned clauses.
 */
void ipasir_set_learn (void * solver, void * state, int max_length, void (*learn)(void * state, int * clause));

/**
 * Set a callback function used to check for termination requests.
 */
void ipasir_set_terminate (void * solver, void * state, int (*terminate)(void * state));

#ifdef __cplusplus
}
#endif

#endif

