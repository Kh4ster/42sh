/*
** Coder : nicolas.blin
** Tester : 
** Reviewer : 
** Integrator : 
**
** echo commande
** UPDATE DEPENDING ON SUBJET
*/

#include "echo.h"
#include "../input_ouput/write.h"

void echo(char *message, int fd_out)
{
    xwrite(message, fd_out);
}