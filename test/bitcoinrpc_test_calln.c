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
#include <string.h>

#include <jansson.h>

#include "../src/bitcoinrpc.h"
#include "../src/bitcoinrpc_method.h"
#include "bitcoinrpc_test.h"


BITCOINRPC_TESTU(calln_getconnectioncount13)
{
  BITCOINRPC_TESTU_INIT;

  const size_t n = 13;

  bitcoinrpc_cl_t *cl = (bitcoinrpc_cl_t*)testdata;

  bitcoinrpc_method_t *m[n];
  bitcoinrpc_resp_t *r[n];
  bitcoinrpc_err_t e;
  json_t *j = NULL;


  for (size_t i = 0; i < n; i++)
    {
      m[i] = bitcoinrpc_method_init(BITCOINRPC_METHOD_GETCONNECTIONCOUNT);
      BITCOINRPC_ASSERT(m[i] != NULL,
                        "cannot initialise a new method");

      r[i] = bitcoinrpc_resp_init();
      BITCOINRPC_ASSERT(r[i] != NULL,
                        "cannot initialise a new response");
    }

  bitcoinrpc_calln(cl, n, m, r, &e);
  BITCOINRPC_ASSERT(e.code == BITCOINRPCE_OK,
                    "cannot perform a call");

  for (size_t i = 0; i < n; i++)
    {
      j = bitcoinrpc_resp_get(r[i]);
      BITCOINRPC_ASSERT(j != NULL,
                        "cannot parse response from the server");

      json_t *jerr = json_object_get(j, "error");
      BITCOINRPC_ASSERT(json_equal(jerr, json_null()),
                        "the server returned non zero error code");
      json_decref(jerr);

      json_t *jresult = json_object_get(j, "result");
      BITCOINRPC_ASSERT(jresult != NULL,
                        "the response has no key: \"result\"");

      BITCOINRPC_ASSERT(json_is_integer(jresult),
                        "getconnectioncount value is not an integer");
      json_decref(j);
    }

  for (size_t i = 0; i < n; i++)
    {
      bitcoinrpc_resp_free(r[i]);
      bitcoinrpc_method_free(m[i]);
    }

  BITCOINRPC_TESTU_RETURN(0);
}





BITCOINRPC_TESTU(calln)
{
  BITCOINRPC_TESTU_INIT;

  bitcoinrpc_cl_t *cl = NULL;

  cl = bitcoinrpc_cl_init_params(o.user, o.pass, o.addr, o.port);
  BITCOINRPC_ASSERT(cl != NULL,
                    "cannot initialise a new client");


  /* Perform test with the same client */
  BITCOINRPC_RUN_TEST(calln_getconnectioncount13, o, cl);

  bitcoinrpc_cl_free(cl);
  cl = NULL;

  BITCOINRPC_TESTU_RETURN(0);
}
