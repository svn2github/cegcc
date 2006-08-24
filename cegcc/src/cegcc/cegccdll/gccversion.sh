#!/bin/bash
arm-wince-pe-gcc -v 2>&1 | grep version | cut -f 3 -d ' '
