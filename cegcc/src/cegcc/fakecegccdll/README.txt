Run the install.sh script to create a dummy/fake libcegcc.dll.a.
This lib constain just enough symbols to satisfy the libstdc++
configure run.

This is needed because by default arm-wince-cegcc-gcc/g++
will link with -lcegcc, which doesn't exist yet
by the time we build libstdc++.
We could solve this also by adding another gcc build step,
but this way is faster.
