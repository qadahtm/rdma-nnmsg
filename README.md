# rdma-nnmsg
Nanomsg and ibverbs based implementation of RDMA for Distributed Commnication  
Trans.cpp has main function currently.

## Dependencies

CMake (version >=3.5)

## Build instruction using 

Uncompress `nanomsg` into the source directory

Create a build directory and change into it

Run to `cmake path/to/source_directory`

Run `make` to build binaries

Ensure that the singularity image `resilient` is at `../` relative to the build directory

Run `python run_resilient_db.py 5`