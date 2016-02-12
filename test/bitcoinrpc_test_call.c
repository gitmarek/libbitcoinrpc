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


BITCOINRPC_TESTU(call_getconnectioncount)
{
  BITCOINRPC_TESTU_INIT;

  bitcoinrpc_cl_t *cl = (bitcoinrpc_cl_t*)testdata;
  bitcoinrpc_method_t *m = NULL;
  bitcoinrpc_resp_t *r = NULL;
  bitcoinrpc_err_t e;
  json_t *j = NULL;

  m = bitcoinrpc_method_init(BITCOINRPC_METHOD_GETCONNECTIONCOUNT);
  BITCOINRPC_ASSERT(m != NULL,
                    "cannot initialise a new method");

  r = bitcoinrpc_resp_init();
  BITCOINRPC_ASSERT(r != NULL,
                    "cannot initialise a new response");

  bitcoinrpc_call(cl, m, r, &e);

  BITCOINRPC_ASSERT(e.code == BITCOINRPCE_OK,
                    "cannot perform a call");

  j = bitcoinrpc_resp_get(r);
  BITCOINRPC_ASSERT(j != NULL,
                    "cannot parse response from the server");

  json_t *jerr = json_object_get(j, "error");
  BITCOINRPC_ASSERT(json_equal(jerr, json_null()),
                    "the server returned non zero error code");
  json_decref(jerr);

  json_t *jval = json_object_get(j, "result");
  BITCOINRPC_ASSERT(jval != NULL,
                    "the response has no key: \"result\"");

  BITCOINRPC_ASSERT(json_is_integer(jval),
                    "getconnectioncount value is not an integer");

  json_decref(j);

  bitcoinrpc_resp_free(r);
  bitcoinrpc_method_free(m);
  BITCOINRPC_TESTU_RETURN(0);
}


BITCOINRPC_TESTU(call_getblock)
{
  BITCOINRPC_TESTU_INIT;
  bitcoinrpc_cl_t *cl = (bitcoinrpc_cl_t*)testdata;
  bitcoinrpc_method_t *m = NULL;
  bitcoinrpc_method_t *m2 = NULL;
  bitcoinrpc_resp_t *r = NULL;
  bitcoinrpc_err_t e;
  json_t *j = NULL;
  json_t *j2 = NULL;
  json_t *jval = NULL;
  json_t *jval2 = NULL;
  json_t *jparams = NULL;

  m = bitcoinrpc_method_init(BITCOINRPC_METHOD_GETBESTBLOCKHASH);
  BITCOINRPC_ASSERT(m != NULL,
                    "cannot initialise a new method");

  r = bitcoinrpc_resp_init();
  BITCOINRPC_ASSERT(r != NULL,
                    "cannot initialise a new response");

  bitcoinrpc_call(cl, m, r, &e);
  BITCOINRPC_ASSERT(e.code == BITCOINRPCE_OK,
                    "cannot perform a call");

  j = bitcoinrpc_resp_get(r);
  BITCOINRPC_ASSERT(j != NULL,
                    "cannot parse response from the server");

  json_t *jerr = json_object_get(j, "error");
  BITCOINRPC_ASSERT(json_equal(jerr, json_null()),
                    "the server returned non zero error code");
  json_decref(jerr);

  jval = json_object_get(j, "result");
  BITCOINRPC_ASSERT(jval != NULL,
                    "the response has no key: \"result\"");

  BITCOINRPC_ASSERT(json_is_string(jval),
                    "getbestblockhash value is not a string");


  jparams = json_array();
  json_array_append(jparams, jval);
  json_array_append_new(jparams, json_false());

  m2 = bitcoinrpc_method_init_params(BITCOINRPC_METHOD_GETBLOCK, jparams);
  BITCOINRPC_ASSERT(m2 != NULL,
                    "cannot initialise a new method");

  bitcoinrpc_call(cl, m2, r, &e);
  BITCOINRPC_ASSERT(e.code == BITCOINRPCE_OK,
                    "cannot perform a call: getbestblockhash");

  j2 = bitcoinrpc_resp_get(r);
  BITCOINRPC_ASSERT(j2 != NULL,
                    "cannot parse response from the server: getblock");

  jval2 = json_object_get(j2, "result");
  BITCOINRPC_ASSERT(jval != NULL,
                    "the response has no key: \"result\"");

  BITCOINRPC_ASSERT(json_is_string(jval2),
                    "getblock value is not a string");

  //fprintf(stderr, "\n\n\n%s\n\n\n", json_string_value(jval2));

  json_decref(j2);
  json_decref(j);
  json_decref(jparams);


  bitcoinrpc_resp_free(r);
  bitcoinrpc_method_free(m2);
  bitcoinrpc_method_free(m);

  BITCOINRPC_TESTU_RETURN(0);
}


/* Warning: this method is deprecated! */
BITCOINRPC_TESTU(call_getinfo)
{
  BITCOINRPC_TESTU_INIT;
  bitcoinrpc_cl_t *cl = (bitcoinrpc_cl_t*)testdata;
  bitcoinrpc_method_t *m = NULL;
  bitcoinrpc_resp_t *r = NULL;
  bitcoinrpc_err_t e;
  json_t *j = NULL;

  m = bitcoinrpc_method_init(BITCOINRPC_METHOD_GETINFO);
  BITCOINRPC_ASSERT(m != NULL,
                    "cannot initialise a new method");

  r = bitcoinrpc_resp_init();
  BITCOINRPC_ASSERT(r != NULL,
                    "cannot initialise a new response");

  bitcoinrpc_call(cl, m, r, &e);

  BITCOINRPC_ASSERT(e.code == BITCOINRPCE_OK,
                    "cannot perform a call");

  j = bitcoinrpc_resp_get(r);
  BITCOINRPC_ASSERT(j != NULL,
                    "cannot parse response from the server");

  json_t *jerr = json_object_get(j, "error");
  BITCOINRPC_ASSERT(json_equal(jerr, json_null()),
                    "the server returned non zero error code");
  json_decref(jerr);

  json_t *jval = json_object_get(j, "result");
  BITCOINRPC_ASSERT(jval != NULL,
                    "the response has no key: \"result\"");

  BITCOINRPC_ASSERT(json_is_object(jval),
                    "getinfo value is not an object");

  /* Is it really getinfo ?*/
  BITCOINRPC_ASSERT(json_object_del(jval, "version") != -1,
                    "getinfo has not \"version\" key; is it really getinfo?");

  BITCOINRPC_ASSERT(json_object_del(jval, "testnet") != -1,
                    "getinfo has not \"testnet\" key; is it really getinfo?");

  BITCOINRPC_ASSERT(json_object_del(jval, "connections") != -1,
                    "getinfo has not \"connections\" key; is it really getinfo?");

  json_decref(j);

  bitcoinrpc_resp_free(r);
  bitcoinrpc_method_free(m);
  BITCOINRPC_TESTU_RETURN(0);
}


BITCOINRPC_TESTU(call)
{
  BITCOINRPC_TESTU_INIT;

  bitcoinrpc_cl_t *cl = NULL;

  cl = bitcoinrpc_cl_init_params(o.user, o.pass, o.addr, o.port);
  BITCOINRPC_ASSERT(cl != NULL,
                    "cannot initialise a new client");


  /* Perform test with the same client */
  BITCOINRPC_RUN_TEST(call_getconnectioncount, o, cl);
  BITCOINRPC_RUN_TEST(call_getblock, o, cl);
  BITCOINRPC_RUN_TEST(call_getinfo, o, cl);

  bitcoinrpc_cl_free(cl);
  cl = NULL;

  BITCOINRPC_TESTU_RETURN(0);
}
