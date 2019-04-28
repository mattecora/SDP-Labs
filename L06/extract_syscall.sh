#!/bin/bash

syscalls=("fork" "exit" "wait" "pipe" "read" "kill" "exec" "fstat" "chdir" "dup" "getpid" "sbrk" "sleep" "uptime" "open" "write" "mknod" "unlink" "link" "mkdir" "close" "sem_alloc" "sem_init" "sem_destroy" "sem_wait" "sem_post")

rm syscalls.txt

for line in $(cat gdb.log | grep "^\\$" | cut -d ' ' -f 3)
do
    echo -e "$line\\t${syscalls[$line-1]}" >> syscalls.txt
done