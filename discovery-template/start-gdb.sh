#!/bin/sh
gdb-multiarch -ex "target extended-remote localhost:3333" -ex "monitor reset halt" -ex "monitor arm semihosting enable" -ex "run"