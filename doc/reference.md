# bitcoinrpc Reference

The reference consists of two parts: ABI and API.


## ABI

Much from the libcurl ABI description applies here too,
see [libcurl ABI](http://curl.haxx.se/libcurl/abi.html).

ABI stands for *Application Binary Interface* and it
*determines such details as how functions are called and in which binary format
information should be passed from one program component to the next*
([Wikipedia has it all](https://en.wikipedia.org/wiki/Application_binary_interface)).

### Upgrades

The code is currently in beta stage and some updates could break ABI.
You are advised to follow the development and upgrade your code from time to
time until the project goes into a stable phase.

### Version number

The version numbers of the library follow
[Semantic versioning](http://semver.org/#semantic-versioning-200)
scheme, i.e. each version consists of three numbers: MAJOR.MINOR.PATCH.
The major version 0 denotes beta stage. *Except for major version 0*, only
major version updates could (and usually do) break backwards compatibility.
In the beta stage also minor version updates could break ABI, whereas in the
usual case they are reserved for extensions and major bug fixes.  
See `Changelog.md` for the detailed history and changes introduces by each
version.

### Soname bumps

Whenever there are changes done to the library that will cause an ABI breakage,
a soname major number of the library is bumped to a higher one. Again,
**except for beta**.  In fact, soname number and major version number should
stay the same in the future. The history of soname bumps looks as follows:

  * libbitcoinrpc.so.0, *January 2016*


## API

The library API consists of five segments and a call function.
See (design):

* bitcoinrpc_global
* bitcoinrpc_cl
* bitcoinrpc_method
* bitcoinrpc_resp
* bitcoinrpc_err
