# Syspro2nd

<<<<>>>>
I have to check why if i give for commondir a path like ./test/common it gives me a segfault
https://keramida.wordpress.com/2009/07/05/fts3-or-avoiding-to-reinvent-the-wheel/
<<<<>>>>
to run multiple mirror_client programms:
./create_infiles.sh ../1_input 10 5 2

to clean and build program
$ make clean && make

then run multiple clients:
./mirror_client -n 1 -c ./common -i ./1_input -m ./1_mirror -b 100 -l log_file1
./mirror_client -n 2 -c ./common -i ./2_input -m ./2_mirror -b 200 -l log_file2

to test the results run:
cat log_file1 log_file2 | ./bashScripts/get_stats.sh
