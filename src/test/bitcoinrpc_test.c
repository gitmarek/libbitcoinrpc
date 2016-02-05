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
#include <getopt.h>
#include <string.h>

#include <jansson.h>

#include <bitcoinrpc.h>

#define PROGNAME "bitcoinrpc_test"

void
print_help_page (void)
{
  printf("%s %s\n", PROGNAME, BITCOINRPC_VERSION);
  printf("Usage: %s [OPTIONS] ARG1 [ARG2, ARG3, ...]\n", PROGNAME);
  printf("\n\
 Options:\n\
    -h, --help                  Print this help page and exit\n\
    -V, --BITCOINRPC_VERSION               Print BITCOINRPC_VERSION number and exit\n\
\n\
  Bitcoin Core brpc credentials:\n\
    --rpc-address=ADDRESS       IP address of the bitcoind server listening to\n\
                                incoming RPC calls (default 127.0.0.1)\n\
    --rpc-port=PORT             Port of the bitcoind node (default 8332)\n\
    --rpc-user=USER             User name for RPC calls (default: user)\n\
    --rpc-password=PASS         Password for RPC calls (dafault: password)\n\
                                See your bitcoin.conf file.\n\
\n\
Written by Marek Miller and others, see CREDITS file.\n\
Report bugs to marek.l.miller@gmail.com\n\
\n\
License: MIT (see https://opensource.org/licenses/MIT).\n\
The software is provided \"as is\", without warranty of any kind.\n\
");
}

struct options
{
  char user[BITCOINRPC_PARAM_MAXLEN];
  char pass[BITCOINRPC_PARAM_MAXLEN];
  char addr[BITCOINRPC_PARAM_MAXLEN];
  unsigned int port;
};
typedef struct options options_t;

int parse_command_options (int argc, char **argv, options_t *o)
{

  int c;

  int addr_flag = 0;
  int user_flag = 0;
  int port_flag = 0;
  int pass_flag = 0;

  while (1)
    {

      struct option long_options[] =
        {
          /* These options set a flag. */
          /* {"verbose",       no_argument,  &verbose_flag, 1}, */
          {"rpc-address",   required_argument, &addr_flag, 1},
          {"rpc-password",  required_argument, &pass_flag, 1},
          {"rpc-port",      required_argument, &port_flag, 1},
          {"rpc-user",      required_argument, &user_flag, 1},
          /* Long-only options. */

          /* These options don’t set a flag.
             We distinguish them by their indices. */
          {"help",    no_argument,       0, 'h'},
          {"BITCOINRPC_VERSION", no_argument,       0, 'V'},
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long (argc, argv, "a:p:hV",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;

      switch (c)
        {
        case 0:

          if (strncmp("rpc-user", long_options[option_index].name, 8) == 0)
          {
            if (strlen(optarg) >= BITCOINRPC_PARAM_MAXLEN)
            {
              fprintf(stderr, "error: The argument to --rpc-user is longer than %d characters.\n Either specify a shorter user name, or change the source code.\n", BITCOINRPC_PARAM_MAXLEN);
              return 1;
            }
            strncpy(o->user, optarg, BITCOINRPC_PARAM_MAXLEN);
          }

          if (strncmp("rpc-password", long_options[option_index].name, 12) == 0)
          {
            if (strlen(optarg) > BITCOINRPC_PARAM_MAXLEN)
            {
              fprintf(stderr, "error: The argument to --rpc-password is longer than %d characters.\n Either specify a shorter password, or change the source code.\n", BITCOINRPC_PARAM_MAXLEN);
              return 1;
            }
            strncpy(o->pass, optarg, BITCOINRPC_PARAM_MAXLEN);
          }
          if (strncmp("rpc-address", long_options[option_index].name, 11) == 0)
          {
            if (strlen(optarg) > BITCOINRPC_PARAM_MAXLEN)
            {
              fprintf(stderr, "error: The argument to --rpc-address is longer than %d characters.\n Either specify a shorter address, or change the source code.\n", BITCOINRPC_PARAM_MAXLEN);
              return 1;
            }
            strncpy(o->addr, optarg, BITCOINRPC_PARAM_MAXLEN);
          }
          if (strncmp("rpc-port", long_options[option_index].name, 8) == 0)
          {
            o->port = (unsigned int) atoi(optarg);
            if (o->port > 65535 || o->port <= 0)
            {
              fprintf(stderr, "error: The port number in --rpc-port is out of range.\n");
              return 1;
            }
          }

          break;

        case 'h':
          print_help_page();
          exit(EXIT_SUCCESS);

        case 'V':
          fprintf (stderr, "%s-%s\n", PROGNAME, BITCOINRPC_VERSION);
          exit(EXIT_SUCCESS);

        case '?':
          /* getopt_long already printed an error message. */
          break;

        default:
          abort ();
        }
    } /* while(1) */

  if (pass_flag == 0)
  {
    fprintf (stderr, "Please specify at least a password for RPC client.\n");
    exit (EXIT_FAILURE);
  }

  /* Print any remaining command line arguments (not options). */
  /*if (optind < argc)
  {
    printf ("non-option ARGV-elements: ");
    while (optind < argc)
      printf ("%s ", argv[optind++]);
    putchar ('\n');
  }
  */
  return 0;
}


int
main (int argc, char **argv)
{

  /* Parse command line options */
  options_t o;
  /* defaults */
  strncpy (o.user, BITCOINRPC_USER_DEFAULT, BITCOINRPC_PARAM_MAXLEN);
  strncpy (o.pass, BITCOINRPC_PASS_DEFAULT, BITCOINRPC_PARAM_MAXLEN);
  strncpy (o.addr, BITCOINRPC_ADDR_DEFAULT, BITCOINRPC_PARAM_MAXLEN);
  o.port = BITCOINRPC_PORT_DEFAULT;


  if (parse_command_options(argc, argv, &o) != 0) {
    fprintf (stderr, "There was a problem parsing command line options.\nExit.\n");
    exit (EXIT_FAILURE);
  }

  fprintf (stderr, "%s-%s starting...\n", PROGNAME, BITCOINRPC_VERSION);

  if (bitcoinrpc_global_init() != BITCOINRPCE_OK)
    abort();



  bitcoinrpc_err_t e;
  char *data = NULL;
  bitcoinrpc_resp_t   *r       = NULL;
  bitcoinrpc_method_t *m       = NULL;
  bitcoinrpc_method_t *m_settx = NULL;
  json_t *j         = NULL;
  json_t *params    = NULL;
  json_t *resp_json = NULL;

  /* initialise client */
  fprintf (stderr, "Initialising the RPC client and connecting to: "
                   "http://%s:%s@%s:%d\n", o.user, o.pass, o.addr, o.port);

  bitcoinrpc_cl_t *cl = bitcoinrpc_cl_init_params (o.user, o.pass,
                                                  o.addr, o.port);

  if (NULL == cl)
  {
    fprintf (stderr, "error: cannot initialise a new client.\n");
    abort();
  }

  /* initialise method  */
  m = bitcoinrpc_method_init (BITCOINRPC_METHOD_GETINFO);
  if (NULL == m)
  {
    fprintf (stderr, "error: cannot initialise a new method.\n");
    abort();
  }

  /* initialise response */
  r = bitcoinrpc_resp_init();
  if (NULL == r)
  {
    fprintf (stderr, "error: cannot initialise a new responce holder.\n");
    abort();
  }

  fprintf (stderr, "calling getinfo: \n");
  bitcoinrpc_call (cl, m, r, &e);
  if (e.code != BITCOINRPCE_OK)
  {
    fprintf (stderr, "error: %s\n", e.msg);
    abort();
  }


  j = bitcoinrpc_resp_get (r);
  data = json_dumps (j, JSON_INDENT(2));
  fprintf (stderr, "%s\n", data);
  json_decref (j);
  free (data);

  /* new method: settxfee */
  m_settx = bitcoinrpc_method_init (BITCOINRPC_METHOD_SETTXFEE);
  if (NULL == m_settx)
  {
    fprintf (stderr, "error: cannot initialise a new method.\n");
    abort();
  }

  double x = (double) rand() / RAND_MAX;
  fprintf (stderr, "Setting tx fee to %.8f\n", x);

  params = json_array();
  json_array_append_new (params, json_real(x));
  if ( bitcoinrpc_method_set_params (m_settx, params) != BITCOINRPCE_OK)
  {
    fprintf (stderr, "error: cannot set params\n");
    abort();
  }
  json_decref (params);

  bitcoinrpc_call(cl, m_settx, r, &e);
  if (e.code != BITCOINRPCE_OK)
  {
    fprintf(stderr, "%s\n", e.msg);
    abort();
  }

  fprintf (stderr, "new tx fee: ");

  /* call getinfo one more time */
  bitcoinrpc_call (cl, m, r, &e);
  if (e.code != BITCOINRPCE_OK)
  {
    fprintf(stderr, "%s\n", e.msg);
    abort();
  }

  j = bitcoinrpc_resp_get (r);
  fprintf (stderr, "%.8f\n", json_real_value ( json_object_get ( json_object_get (j, "result"), "paytxfee")));
  json_decref (j);

  fprintf (stderr,
    " * Test convenience functions:\n");

  char *a, *b;
  unsigned int c;
  // unsigned long long int l;
  double d;


  fprintf (stderr, "generate = ");
  resp_json = bitcoinrpc_generate (cl, &e, 10);
  if (e.code == BITCOINRPCE_OK)
  {
    fprintf (stderr, "%.60s, etc.\n", json_dumps (resp_json, JSON_COMPACT));
    json_decref (resp_json);
  }
  else
  {
    fprintf (stderr, "error(%d): %s\n", e.code, e.msg);
    /* Do not abort on error (this method can be used only in regtest mode) */
  }


  fprintf (stderr, "getbestblockhash = ");
  a = bitcoinrpc_getbestblockhash(cl, &e);
  if (e.code == BITCOINRPCE_OK)
  {
    fprintf (stderr, "%s\n", a);
    free (a);
  }
  else
  {
    fprintf (stderr, "error(%d): %s\n", e.code, e.msg);
    abort();
  }

  fprintf (stderr, "getblock = ");
  b = bitcoinrpc_getbestblockhash (cl, &e);
  a = bitcoinrpc_getblock (cl, &e, b);
  if (e.code == BITCOINRPCE_OK)
  {
    fprintf (stderr, "%.60s, etc.\n", a);
    free (a);
  }
  else
  {
    fprintf (stderr, "error(%d): %s\n", e.code, e.msg);
    abort();
  }


  fprintf (stderr, "getblock_json = ");
  resp_json = bitcoinrpc_getblock_json (cl, &e, b);
  if (e.code == BITCOINRPCE_OK)
  {
    fprintf (stderr, "%.60s, etc.\n", json_dumps (resp_json, JSON_COMPACT));
    json_decref (resp_json);
  }
  else
  {
    fprintf (stderr, "error(%d): %s\n", e.code, e.msg);
    abort();
  }
  free(b);


  fprintf (stderr, "getblockchaininfo = ");
  resp_json = bitcoinrpc_getblockchaininfo (cl, &e);
  if (e.code == BITCOINRPCE_OK)
  {
    fprintf (stderr, "%.60s, etc.\n", json_dumps (resp_json, JSON_COMPACT));
    json_decref (resp_json);
  }
  else
  {
    fprintf (stderr, "error(%d): %s\n", e.code, e.msg);
    abort();
  }


  fprintf (stderr, "getblockcount = ");
  c = bitcoinrpc_getblockcount (cl, &e);
  if (e.code == BITCOINRPCE_OK)
  {
    fprintf (stderr, "%d\n", c);
  }
  else
  {
    fprintf (stderr, "error(%d): %s\n", e.code, e.msg);
    abort();
  }

  /* if you test on regtest, remember to produce at least 10 blocks */
  fprintf (stderr, "getblockhash(10) = ");
  a = bitcoinrpc_getblockhash (cl, &e, 10);
  if (e.code == BITCOINRPCE_OK)
  {
    fprintf (stderr, "%s\n", a);
    free (a);
  }
  else
  {
    fprintf (stderr, "error(%d): %s\n", e.code, e.msg);
    abort();
  }


  fprintf (stderr, "getchaintips = ");
  resp_json = bitcoinrpc_getchaintips (cl, &e);
  if (e.code == BITCOINRPCE_OK)
  {
    fprintf (stderr, "%.60s, etc.\n", json_dumps (resp_json, JSON_COMPACT));
    json_decref (resp_json);
  }
  else
  {
    fprintf (stderr, "error(%d): %s\n", e.code, e.msg);
    abort();
  }


  fprintf (stderr, "getconnectioncount = ");
  c = bitcoinrpc_getconnectioncount(cl, &e);
  if (e.code == BITCOINRPCE_OK)
  {
    fprintf (stderr, "%d\n", c);
  }
  else
  {
    fprintf (stderr, "error(%d): %s\n", e.code, e.msg);
    abort();
  }


  fprintf (stderr, "getdifficulty = ");
  d = bitcoinrpc_getdifficulty(cl, &e);
  if (e.code == BITCOINRPCE_OK)
  {
    fprintf (stderr, "%f\n", d);
  }
  else
  {
    fprintf (stderr, "error(%d): %s\n", e.code, e.msg);
    abort();
  }


  fprintf (stderr, "getgenerate = ");
  c = bitcoinrpc_getgenerate (cl, &e);
  if (e.code == BITCOINRPCE_OK)
  {
    fprintf (stderr, "%d\n", c);
  }
  else
  {
    fprintf (stderr, "error(%d): %s\n", e.code, e.msg);
    abort();
  }


  fprintf (stderr, "getinfo = ");
  resp_json = bitcoinrpc_getinfo (cl, &e);
  if (e.code == BITCOINRPCE_OK)
  {
    fprintf (stderr, "%.60s, etc.\n", json_dumps (resp_json, JSON_COMPACT));
    json_decref (resp_json);
  }
  else
  {
    fprintf (stderr, "error(%d): %s\n", e.code, e.msg);
    abort();
  }


  fprintf (stderr, "getmempoolinfo = ");
  resp_json = bitcoinrpc_getmempoolinfo (cl, &e);
  if (e.code == BITCOINRPCE_OK)
  {
    fprintf (stderr, "%.60s, etc.\n", json_dumps (resp_json, JSON_COMPACT));
    json_decref (resp_json);
  }
  else
  {
    fprintf (stderr, "error(%d): %s\n", e.code, e.msg);
    abort();
  }


  fprintf (stderr, "getnewaddress = ");
  a = bitcoinrpc_getnewaddress(cl, &e, "myacc");
  if (e.code == BITCOINRPCE_OK)
  {
    fprintf (stderr, "%s\n", a);
    free (a);
  }
  else
  {
    fprintf (stderr, "error(%d): %s\n", e.code, e.msg);
    abort();
  }


  fprintf (stderr, "getrawmempool = ");
  resp_json = bitcoinrpc_getrawmempool (cl, &e, 0);
  if (e.code == BITCOINRPCE_OK)
  {
    fprintf (stderr, "%.60s, etc.\n", json_dumps (resp_json, JSON_COMPACT));
    json_decref (resp_json);
  }
  else
  {
    fprintf (stderr, "error(%d): %s\n", e.code, e.msg);
    abort();
  }


  fprintf (stderr, "getrawmempool (verbose) = ");
  resp_json = bitcoinrpc_getrawmempool (cl, &e, 1);
  if (e.code == BITCOINRPCE_OK)
  {
    fprintf (stderr, "%.60s, etc.\n", json_dumps (resp_json, JSON_COMPACT));
    json_decref (resp_json);
  }
  else
  {
    fprintf (stderr, "error(%d): %s\n", e.code, e.msg);
    abort();
  }


  /*
  fprintf (stderr, "stop = ");
  bitcoinrpc_stop(cl, &e);
  if (e.code != BITCOINRPCE_OK)
  {
    fprintf (stderr, "error(%d): %s\n", e.code, e.msg);
    abort();
  }
  */

  fprintf (stderr, "Free the resources... ");
  bitcoinrpc_cl_free (cl);
  bitcoinrpc_method_free (m);
  bitcoinrpc_resp_free (r);
  if (bitcoinrpc_global_cleanup() != BITCOINRPCE_OK)
  {
    fprintf (stderr, "failure.\n");
    abort ();
  }

  fprintf(stderr, "done.\n");
  exit(EXIT_SUCCESS);
}
