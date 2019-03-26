# Syspro2nd

shell script to generate random files and folders
<<<<>>>>
I have to check why if i give for commondir a path like ./test/common it gives me a segfault <<<<>>>>
to run multiple mirror_client programms:
$ make
./mirror_client -n 1 -c ./common -i ./1_input -m ./1_mirror -b 100 -l log_file1
./mirror_client -n 2 -c ./common -i ./2_input -m ./2_mirror -b 200 -l log_file2