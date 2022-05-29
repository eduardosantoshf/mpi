## Compile

```$ mpicc -Wall -O3 -o main main.c dispatcher.c worker.c```

## Run

```$ mpiexec -n [number_of_workers] ./main -f [filenames]```