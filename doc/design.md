
## Error messages:

Nontrivial functions (that may fail in non obvious way)
take parameter `bitcoinrpc_err_t *e`;
Allocate e as a stack variable and pass a pointer to it.
If a function returns != BITCOINRPCE_OK, then e.msg contains
the error message.
If the parameter e == NULL, it is ignored.
