cd vm
make clean
make
cd build
source ../../activate
#pintos -- -q run alarm-multiple
# cd threads
make check
# make
# cd build
# source ../../activate
# pintos -- -q run alarm-multiple
#pintos --fs-disk=10 -p tests/userprog/args-none:args-none --swap-disk=4 -- -q   -f run args-none