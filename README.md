# memhook - memory leak detector for Linux
MemHook allows you to monitor all memory allocations during program execution.
### Building memhook
```bash
git clone https://github.com/ruslanch/memhook.git
cd ./memhook
mkdir build && cd build
export TOOLCHAIN_ROOT=<path to your toolchain> # optional
export TOOLCHAIN_HOST=<your toolchain host> # optional
../scripts/build_boost.sh
../scripts/build_libunwind.sh
../scripts/build_memhook.sh
ls $TOOLCHAIN_HOST/memhook/lib/*.so
ls $TOOLCHAIN_HOST/memhook/bin/*
```
Now you have library `libmemhook.so` and programs `memdb` and `memview`.

### Using memhook
MemHook is split into three parts:
1. `libmemhook.so` - is a dynamic library that redefines memory allocation functions in the monitored program. `libmemhook.so` can work both separately and together with `memdb`.
2. `memdb` - memory allocations database server.
3. `memview` - memory allocations database viewer.

You can use a separate memory allocations database server - `memdb`

```bash
memdb -f <path-to-db>
```

Now switch to the next terminal (or to another `screen` session) and run the monitored program:

```bash
MEMHOOK_NET_HOST=127.0.0.1 LD_PRELOAD=<patch-to-libmemhook.so> <path-to-your-app>
```

or standalone:

```bash
MEMHOOK_FILE=<path-to-db> LD_PRELOAD=<patch-to-libmemhook.so> <path-to-your-app>
```

Now the monitored program is running, switch to the next terminal (or to another `screen` session) and run `memview`:

```bash
memview -f <path-to-db> -a -c --min-time-from-start=1min --min-time-from-start=1min --min-time-from-now=10s
```

### `libmemhook.so`options:
```
MEMHOOK_NET_HOST=<host>  - memdb host (if you are using memdb)
MEMHOOK_NET_PORT=<port>  - memdb port (if you are using memdb), optional (default is 20015)
MEMHOOK_SIZE_GB=<size>   - maximum memory allocations database size in gigabytes (default 8Gb for x64 and 1Gb for x86)
MEMHOOK_SIZE_MB=<size>   - maximum memory allocations database size in megabytes
MEMHOOK_SIZE_KB=<size>   - maximum memory allocations database size in kilobytes
MEMHOOK_SIZE=<size>      - maximum memory allocations database size in bytes
MEMHOOK_FILE=<file path> - memory allocations database file path
MEMHOOK_SHM_NAME=<name>  - use named shared memory for memory allocations database
MEMHOOK_CACHE_FLUSH_INTERVAL=<time> - interval of flushing allocation information in the memory allocations database (default is 2 seconds), example: 10s, 30min, 5h, etc
MEMHOOK_CACHE_FLUSH_MAX_ITEMS=<num> - the maximum number of flushed allocation information records in the memory allocations database
```

### `memdb` options:
```
Usage: MemDB [options] [-m | -f path] [-p PORT_NUM]
Options:
  --help                                show help message
  -m [ --shared-memory ] [=arg(=ShmMemHook)]
                                        use shared memory
  -f [ --mapped-file ] arg              use memory mapped file
  -s [ --size ] arg (=8589934592)       size
  -h [ --host ] arg (=127.0.0.1)        host
  -p [ --port ] arg (=20015)            port
```

### `memview` options:
```
Usage: memview [options] [-m | -f path]
Options:
  --help                                show help message
  -m [ --shared-memory ] [=arg(=ShmMemHook)]
                                        use shared memory
  -f [ --mapped-file ] arg              use memory mapped file
  -c [ --show-callstack ] [=arg(=1)]    print callstack
  -a [ --aggregate ] [=arg(=1)]         aggregate
  -t [ --sort-by-time ]                 sort by timestamp
  -s [ --sort-by-size ]                 sort by allocation size
  -p [ --sort-by-address ]              sort by allocation address
  --min-time-from-now arg               minimal time interval from current time
                                        to allocation time (10s, 30min, 5h)
  --max-time-from-now arg               maximum time interval from current time
                                        to allocation time (10s, 30min, 5h)
  --min-time-from-start arg             minimal time interval from program
                                        start to allocation time (10s, 30min,
                                        5h)
  --max-time-from-start arg             maximum time interval from program
                                        start to allocation time (10s, 30min,
                                        5h)
  -n [ --min-time ] arg                 minimal allocation datetime (YYYY-MM-DD
                                        HH:MM:SS)
  -x [ --max-time ] arg                 maximum allocation datetime (YYYY-MM-DD
                                        HH:MM:SS)
  -z [ --min-size ] arg                 minimal allocation size (in bytes)
  --get-storage-info
  --no-lock [=arg(=1)]
```
