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

#include "bitcoinrpc.h"


/*
Names of Bitcoin RPC methods and how to get them from BITCOINRPC_METHOD codes.
*/
struct BITCOINRPC_METHOD_struct_
{
    BITCOINRPC_METHOD m;
    char *str;
    unsigned char requires_params;
};

#define BITCOINRPC_METHOD_names_len_ 4   /* remember to update it! */

const struct BITCOINRPC_METHOD_struct_
BITCOINRPC_METHOD_names_[BITCOINRPC_METHOD_names_len_] =
{
  { BITCOINRPC_METHOD_GETINFO,            "getinfo",             0 },
  { BITCOINRPC_METHOD_GETNETWORKINFO,     "getnetworkinfo",      0 },
  { BITCOINRPC_METHOD_GETWALLETINFO,      "getwalletinfo",       0 },
  { BITCOINRPC_METHOD_HELP,               "help",                0 },
};

const struct BITCOINRPC_METHOD_struct_ *
bitcoinrpc_method_st_ (const BITCOINRPC_METHOD m)
{
  for (int i = 0; i < BITCOINRPC_METHOD_names_len_; i++)
  {
    if (BITCOINRPC_METHOD_names_[i].m == m)
      return &BITCOINRPC_METHOD_names_[i];
  }
  return NULL;
}
