# Syspro2nd

to run multiple mirror_client programms:
./create_infiles.sh ../1_input 10 5 2

to clean and build program:
rm -rf *_mirror && rm -rf common/ && rm log_*
make clean && make

then run multiple clients:
./mirror_client -n 1 -c ./common -i ./1_input -m ./1_mirror -b 100 -l log_file1
./mirror_client -n 2 -c ./common -i ./2_input -m ./2_mirror -b 200 -l log_file2

to test the results run:
cat log_file1 log_file2 | ./bashScripts/get_stats.sh
