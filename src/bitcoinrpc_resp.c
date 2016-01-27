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

#include <jansson.h>
#include <uuid/uuid.h>

#include "bitcoinrpc.h"
#include "bitcoinrpc_global.h"
#include "bitcoinrpc_method.h"
#include "bitcoinrpc_resp.h"


struct bitcoinrpc_resp
{
  uuid_t  uuid;
  json_t  *json;

  /*
  This is a legacy pointer. You can point to an auxilliary structure,
  if you prefer not to touch this one (e.g. not to break ABI).
  */
  void* legacy_ptr_0a942f71_58af_4869_a9f8_4a7c48ddae9c;

};


/* Internal stuff */
BITCOINRPCEcode
bitcoinrpc_resp_set_json_ (bitcoinrpc_resp_t *resp, json_t *json)
{

  if (NULL == resp)
    return BITCOINRPCE_BUG;

  resp->json = json_deep_copy(json);
  if (NULL == resp->json)
    return BITCOINRPCE_JSON;

  return BITCOINRPCE_OK;
}
/* ------------------------------------------------------------------------- */

bitcoinrpc_resp_t *
bitcoinrpc_resp_init (void)
{
  bitcoinrpc_resp_t *resp = bitcoinrpc_global_allocfunc(sizeof *resp);
  if (NULL == resp)
    return NULL;

  resp->json = NULL;
  return resp;
}


BITCOINRPCEcode
bitcoinrpc_resp_free (bitcoinrpc_resp_t *resp)
{
  if (NULL == resp)
    return BITCOINRPCE_PARAM;

  json_decref (resp->json);
  bitcoinrpc_global_freefunc (resp);

  return BITCOINRPCE_OK;
}

/*
Get a deepcopy of the json object representing the response
from the server or NULL in case of error.
*/
json_t *
bitcoinrpc_resp_get (bitcoinrpc_resp_t *resp)
{
  if (NULL == resp)
    return NULL;

  return json_deep_copy (resp->json);
}

/*
Check if the resp comes as a result of calling method.
Returns BITCOINRPCE_CHECK, if not. This check is already performed by
bitcoinrpc_call()
*/
BITCOINRPCEcode
bitcoinrpc_resp_check (bitcoinrpc_resp_t *resp, bitcoinrpc_method_t *method)
{
  if (NULL == resp || NULL == method)
    return BITCOINRPCE_PARAM;

  return bitcoinrpc_method_compare_uuid_ (method, resp->uuid);
}
