#!/usr/bin/env bash

while true; do echo "###########################"; date; qstat | grep -E 'melkamar'; sleep 5; done