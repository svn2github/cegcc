#!/bin/bash
arm-wince-cegcc-gcc -v 2>&1 | grep version | cut -f 3 -d ' '
