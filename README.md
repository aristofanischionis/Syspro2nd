# Syspro2nd

to run multiple mirror_client programms:
./bashScripts/create_infiles.sh 1_input 10 5 2

to clean and build program:
rm -rf *_mirror && rm -rf common/ && rm log_*
make clean && make

then run multiple clients:
./mirror_client -n 1 -c ./common -i ./1_input -m ./1_mirror -b 100 -l log_file1
./mirror_client -n 2 -c ./common -i ./2_input -m ./2_mirror -b 200 -l log_file2
./mirror_client -n 3 -c ./common -i ./3_input -m ./3_mirror -b 100 -l log_file3
./mirror_client -n 4 -c ./common -i ./4_input -m ./4_mirror -b 100 -l log_file4

to test the results run:
cat log_file* | ./bashScripts/get_stats.sh

to check if two folders are identical:
diff -r -q path1/ path2/

to kill bg processes:
kill $(ps -A -ostat,ppid | awk '/[zZ]/ && !a[$2]++ {print $2}')

delete any remaining .asc files in input:
find  *_input | grep .asc | xargs rm -f