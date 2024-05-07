# zone_benchmarks


To ensure that we have a reproducible software setup, we use Docker. Make sure you are running docker or a docker

To enter the Docker shell, while in the project directory, type:
```
./run_docker.sh bash
````
The first time you run this command, the image is created which take a few minutes. We are assuming that you have privileged access to the system.

Make sure you have a zone file in the current directory (or a symbolic link).

```
cmake -B build
cmake --build build
./build/benchmarks/knot se.zone.txt 
```