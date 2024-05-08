# zone_benchmarks

This is a zone-file parsing benchmark. We measure complete parsing.

## Zone files


- The Sweden zone file (small) is available from https://internetstiftelsen.se/en/zone-data/.
- Major zone files such as .com are available from  https://czds.icann.org/.
- The root zone can be found at: https://www.iana.org/domains/root/files.

## Docker container

To ensure that we have a reproducible software setup, we suggest you use Docker. 
Make sure you are running docker or a docker compatible system on your test platform. 
You do not have to use docker but it makes it easy to ensure that you have the same
compiler and dependency. Under Linux, Docker has minimal overhead for computational
tasks and is thus suitable for benchmarking.

To enter the Docker shell, while in the project directory, type:
```
./run_docker.sh bash
````
The first time you run this command, the image is created which take a few minutes. 
We are assuming that you have privileged access to the system---as this is useful to
get performance counters.

Make sure you have a zone file in the current directory prior to entering the container.

## Requirements

If you use the container, these are taken care of, but otherwise, you need the following:

- Recent C and C++ compilers (Linux or macOS expected).
- CMake
- Git
- Knot (zscanner)

## Running benchmarks

```
cmake -B build
cmake --build build
./build/benchmarks/bench se.zone.txt
```

Example:

```
$ ./build/benchmarks/bench se.zone.txt
Benchmarking file se.zone.txt
Volume: 1361515551 bytes
simdzonehaswell                               :   0.81 GB/s ( 0 %)   2.72 GHz   3.38 c/b  11.90 i/b   3.52 i/c 
simdzonewestmere                              :   0.73 GB/s ( 0 %)   2.75 GHz   3.78 c/b  13.76 i/b   3.64 i/c 
simdzonefallback                              :   0.38 GB/s ( 0 %)   2.97 GHz   7.91 c/b  21.76 i/b   2.75 i/c 
knot3.3.4                                     :   0.19 GB/s ( 0 %)   3.16 GHz  17.06 c/b  23.45 i/b   1.37 i/c 
```

## Running just some benchmarks

An additional string parameter might be used as a filter:

```
$ ./build/benchmarks/bench se.zone.txt knot
knot3.3.4                                     :   0.19 GB/s ( 0 %)   3.17 GHz  17.02 c/b  23.45 i/b   1.38 i/c 
```

## Profiling

Within the image, you can profile the benchmark like so:

```
/usr/lib/linux-tools/6.8.0-31-generic/perf record ./build/benchmarks/bench se.zone.txt haswell
/usr/lib/linux-tools/6.8.0-31-generic/perf report
```

The result might be as follow:
```
  31.60% parse_rrsig_rdata
  27.97% maybe_take
  15.28% parse
```