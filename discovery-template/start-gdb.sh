#!/bin/sh
gdb-multiarch -ex "target extended-remote localhost:3333" -ex "run"
