/** @file
* @brief SHOPT
* @author Coder : nicolas.blin
* @author Tester : nicolas.blin
* @author Reviewer :
* @author Integrator :
*/

#pragma once

/**
* @brief returns an initialized command container
* @brief filled with given parameters(for unitary tests only)
* @param options all the options
* @return 0 : success ; -1 : error
*/
int shopt(char *options[]);