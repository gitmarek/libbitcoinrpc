/*
   The MIT License (MIT)
   Copyright (c) 2016 Marek Miller

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
   LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
   OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>

#include <jansson.h>

#include "../src/bitcoinrpc.h"
#include "bitcoinrpc_test.h"


/* Check if can actualy create a new client */
BITCOINRPC_TESTU(client_init)
{
  BITCOINRPC_TESTU_INIT;

  bitcoinrpc_cl_t *cl = NULL;

  cl = bitcoinrpc_cl_init();
  BITCOINRPC_ASSERT(cl != NULL,
                    "cannot initialise a new client");
  bitcoinrpc_cl_free(cl);
  cl = NULL;

  cl = bitcoinrpc_cl_init_params(o.user, o.pass, o.addr, o.port);
  BITCOINRPC_ASSERT(cl != NULL,
                    "cannot initialise a new client");
  bitcoinrpc_cl_free(cl);
  cl = NULL;

  /* Check if the function checks for wrong parameters */
  cl = bitcoinrpc_cl_init_params(o.user, o.pass, NULL, o.port);
  BITCOINRPC_ASSERT(cl == NULL,
                    "bitcoinrpc_cl_init_params does not check for NULLs in addr");
  bitcoinrpc_cl_free(cl);
  cl = NULL;

  cl = bitcoinrpc_cl_init_params(o.user, NULL, o.addr, o.port);
  BITCOINRPC_ASSERT(cl == NULL,
                    "bitcoinrpc_cl_init_params does not check for NULLs in pass");
  bitcoinrpc_cl_free(cl);
  cl = NULL;

  cl = bitcoinrpc_cl_init_params(NULL, o.pass, o.addr, o.port);
  BITCOINRPC_ASSERT(cl == NULL,
                    "bitcoinrpc_cl_init_params does not check for NULLs in user");
  bitcoinrpc_cl_free(cl);
  cl = NULL;

  cl = bitcoinrpc_cl_init_params(o.user, o.pass, o.addr, 77777);
  BITCOINRPC_ASSERT(cl == NULL,
                    "bitcoinrpc_cl_init_params does not check for wrong port number");
  bitcoinrpc_cl_free(cl);
  cl = NULL;


  BITCOINRPC_TESTU_RETURN(0);
}




BITCOINRPC_TESTU(client)
{
  BITCOINRPC_TESTU_INIT;
  BITCOINRPC_RUN_TEST(client_init, o, NULL);
  BITCOINRPC_TESTU_RETURN(0);
}
