# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = /auto/home-scf-22/csci402/testgnu/decstation-ultrix/bin/
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: halt matmult sort createlock createlock2 createlock_test acquirelock acquirelock1 acquirelock2 acquirelock3 releaselock releaselock1 releaselock2 releaselock3 destroylock destroylock1 destroylock2\
 	 createcondition createcondition_test destroycondition destroycondition_right_process destroycondition_wrong_process\
 	 wait wait_normative wait_non_normative wait_destroy wait_wrong_process wait_right_process\
 	 signal signal_normative signal_non_normative signal_destroy signal_wrong_process signal_right_process\
 	 broadcast broadcast_normative broadcast_non_normative broadcast_destroy broadcast_wrong_process broadcast_right_process\
 	 yield yield_test fork fork_test exec exit exit_test printx printx_test testfiles\
 	 museum uniprogrammed_museum multiprogrammed_museum createmv destroymv setmv getmv wait1 wait2 wait3 wait_test broadcast1 broadcast2 broadcast3 broadcast4 broadcast5 broadcast_test signal1 signal2 signal3 signal_test

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

# Lock files
createlock.o: createlock.c
	$(CC) $(CFLAGS) -c createlock.c
createlock: createlock.o start.o
	$(LD) $(LDFLAGS) start.o createlock.o -o createlock.coff
	../bin/coff2noff createlock.coff createlock
	
createlock2.o: createlock2.c
	$(CC) $(CFLAGS) -c createlock2.c
createlock2: createlock2.o start.o
	$(LD) $(LDFLAGS) start.o createlock2.o -o createlock2.coff
	../bin/coff2noff createlock2.coff createlock2
	
createlock_test.o: createlock_test.c
	$(CC) $(CFLAGS) -c createlock_test.c
createlock_test: createlock_test.o start.o
	$(LD) $(LDFLAGS) start.o createlock_test.o -o createlock_test.coff
	../bin/coff2noff createlock_test.coff createlock_test
	
acquirelock.o: acquirelock.c
	$(CC) $(CFLAGS) -c acquirelock.c
acquirelock: acquirelock.o start.o
	$(LD) $(LDFLAGS) start.o acquirelock.o -o acquirelock.coff
	../bin/coff2noff acquirelock.coff acquirelock
	
acquirelock1.o: acquirelock1.c
	$(CC) $(CFLAGS) -c acquirelock1.c
acquirelock1: acquirelock1.o start.o
	$(LD) $(LDFLAGS) start.o acquirelock1.o -o acquirelock1.coff
	../bin/coff2noff acquirelock1.coff acquirelock1

acquirelock2.o: acquirelock2.c
	$(CC) $(CFLAGS) -c acquirelock2.c
acquirelock2: acquirelock2.o start.o
	$(LD) $(LDFLAGS) start.o acquirelock2.o -o acquirelock2.coff
	../bin/coff2noff acquirelock2.coff acquirelock2
	
acquirelock3.o: acquirelock3.c
	$(CC) $(CFLAGS) -c acquirelock3.c
acquirelock3: acquirelock3.o start.o
	$(LD) $(LDFLAGS) start.o acquirelock3.o -o acquirelock3.coff
	../bin/coff2noff acquirelock3.coff acquirelock3

releaselock.o: releaselock.c
	$(CC) $(CFLAGS) -c releaselock.c
releaselock: releaselock.o start.o
	$(LD) $(LDFLAGS) start.o releaselock.o -o releaselock.coff
	../bin/coff2noff releaselock.coff releaselock

releaselock1.o: releaselock1.c
	$(CC) $(CFLAGS) -c releaselock1.c
releaselock1: releaselock1.o start.o
	$(LD) $(LDFLAGS) start.o releaselock1.o -o releaselock1.coff
	../bin/coff2noff releaselock1.coff releaselock1
	
releaselock2.o: releaselock2.c
	$(CC) $(CFLAGS) -c releaselock2.c
releaselock2: releaselock2.o start.o
	$(LD) $(LDFLAGS) start.o releaselock2.o -o releaselock2.coff
	../bin/coff2noff releaselock2.coff releaselock2
	
releaselock3.o: releaselock3.c
	$(CC) $(CFLAGS) -c releaselock3.c
releaselock3: releaselock3.o start.o
	$(LD) $(LDFLAGS) start.o releaselock3.o -o releaselock3.coff
	../bin/coff2noff releaselock3.coff releaselock3
	
destroylock.o: destroylock.c
	$(CC) $(CFLAGS) -c destroylock.c
destroylock: destroylock.o start.o
	$(LD) $(LDFLAGS) start.o destroylock.o -o destroylock.coff
	../bin/coff2noff destroylock.coff destroylock
	
destroylock1.o: destroylock1.c
	$(CC) $(CFLAGS) -c destroylock1.c
destroylock1: destroylock1.o start.o
	$(LD) $(LDFLAGS) start.o destroylock1.o -o destroylock1.coff
	../bin/coff2noff destroylock1.coff destroylock1
	
destroylock2.o: destroylock2.c
	$(CC) $(CFLAGS) -c destroylock2.c
destroylock2: destroylock2.o start.o
	$(LD) $(LDFLAGS) start.o destroylock2.o -o destroylock2.coff
	../bin/coff2noff destroylock2.coff destroylock2

# Condition files
createcondition.o: createcondition.c
	$(CC) $(CFLAGS) -c createcondition.c
createcondition: createcondition.o start.o
	$(LD) $(LDFLAGS) start.o createcondition.o -o createcondition.coff
	../bin/coff2noff createcondition.coff createcondition

createcondition_test.o: createcondition_test.c
	$(CC) $(CFLAGS) -c createcondition_test.c
createcondition_test: createcondition_test.o start.o
	$(LD) $(LDFLAGS) start.o createcondition_test.o -o createcondition_test.coff
	../bin/coff2noff createcondition_test.coff createcondition_test

destroycondition.o: destroycondition.c
	$(CC) $(CFLAGS) -c destroycondition.c
destroycondition: destroycondition.o start.o
	$(LD) $(LDFLAGS) start.o destroycondition.o -o destroycondition.coff
	../bin/coff2noff destroycondition.coff destroycondition
	
destroycondition_right_process.o: destroycondition_right_process.c
	$(CC) $(CFLAGS) -c destroycondition_right_process.c
destroycondition_right_process: destroycondition_right_process.o start.o
	$(LD) $(LDFLAGS) start.o destroycondition_right_process.o -o destroycondition_right_process.coff
	../bin/coff2noff destroycondition_right_process.coff destroycondition_right_process
	
destroycondition_wrong_process.o: destroycondition_wrong_process.c
	$(CC) $(CFLAGS) -c destroycondition_wrong_process.c
destroycondition_wrong_process: destroycondition_wrong_process.o start.o
	$(LD) $(LDFLAGS) start.o destroycondition_wrong_process.o -o destroycondition_wrong_process.coff
	../bin/coff2noff destroycondition_wrong_process.coff destroycondition_wrong_process

# Wait files
wait.o: wait.c
	$(CC) $(CFLAGS) -c wait.c
wait: wait.o start.o
	$(LD) $(LDFLAGS) start.o wait.o -o wait.coff
	../bin/coff2noff wait.coff wait
	
wait_normative.o: wait_normative.c
	$(CC) $(CFLAGS) -c wait_normative.c
wait_normative: wait_normative.o start.o
	$(LD) $(LDFLAGS) start.o wait_normative.o -o wait_normative.coff
	../bin/coff2noff wait_normative.coff wait_normative

wait_non_normative.o: wait_non_normative.c
	$(CC) $(CFLAGS) -c wait_non_normative.c
wait_non_normative: wait_non_normative.o start.o
	$(LD) $(LDFLAGS) start.o wait_non_normative.o -o wait_non_normative.coff
	../bin/coff2noff wait_non_normative.coff wait_non_normative
	
wait_destroy.o: wait_destroy.c
	$(CC) $(CFLAGS) -c wait_destroy.c
wait_destroy: wait_destroy.o start.o
	$(LD) $(LDFLAGS) start.o wait_destroy.o -o wait_destroy.coff
	../bin/coff2noff wait_destroy.coff wait_destroy
	
wait_right_process.o: wait_right_process.c
	$(CC) $(CFLAGS) -c wait_right_process.c
wait_right_process: wait_right_process.o start.o
	$(LD) $(LDFLAGS) start.o wait_right_process.o -o wait_right_process.coff
	../bin/coff2noff wait_right_process.coff wait_right_process
	
wait_wrong_process.o: wait_wrong_process.c
	$(CC) $(CFLAGS) -c wait_wrong_process.c
wait_wrong_process: wait_wrong_process.o start.o
	$(LD) $(LDFLAGS) start.o wait_wrong_process.o -o wait_wrong_process.coff
	../bin/coff2noff wait_wrong_process.coff wait_wrong_process

# Signal files
signal.o: signal.c
	$(CC) $(CFLAGS) -c signal.c
signal: signal.o start.o
	$(LD) $(LDFLAGS) start.o signal.o -o signal.coff
	../bin/coff2noff signal.coff signal
	
signal_normative.o: signal_normative.c
	$(CC) $(CFLAGS) -c signal_normative.c
signal_normative: signal_normative.o start.o
	$(LD) $(LDFLAGS) start.o signal_normative.o -o signal_normative.coff
	../bin/coff2noff signal_normative.coff signal_normative

signal_non_normative.o: signal_non_normative.c
	$(CC) $(CFLAGS) -c signal_non_normative.c
signal_non_normative: signal_non_normative.o start.o
	$(LD) $(LDFLAGS) start.o signal_non_normative.o -o signal_non_normative.coff
	../bin/coff2noff signal_non_normative.coff signal_non_normative
	
signal_destroy.o: signal_destroy.c
	$(CC) $(CFLAGS) -c signal_destroy.c
signal_destroy: signal_destroy.o start.o
	$(LD) $(LDFLAGS) start.o signal_destroy.o -o signal_destroy.coff
	../bin/coff2noff signal_destroy.coff signal_destroy
	
signal_right_process.o: signal_right_process.c
	$(CC) $(CFLAGS) -c signal_right_process.c
signal_right_process: signal_right_process.o start.o
	$(LD) $(LDFLAGS) start.o signal_right_process.o -o signal_right_process.coff
	../bin/coff2noff signal_right_process.coff signal_right_process
	
signal_wrong_process.o: signal_wrong_process.c
	$(CC) $(CFLAGS) -c signal_wrong_process.c
signal_wrong_process: signal_wrong_process.o start.o
	$(LD) $(LDFLAGS) start.o signal_wrong_process.o -o signal_wrong_process.coff
	../bin/coff2noff signal_wrong_process.coff signal_wrong_process

# Broadcast files
broadcast.o: broadcast.c
	$(CC) $(CFLAGS) -c broadcast.c
broadcast: broadcast.o start.o
	$(LD) $(LDFLAGS) start.o broadcast.o -o broadcast.coff
	../bin/coff2noff broadcast.coff broadcast
	
broadcast_normative.o: broadcast_normative.c
	$(CC) $(CFLAGS) -c broadcast_normative.c
broadcast_normative: broadcast_normative.o start.o
	$(LD) $(LDFLAGS) start.o broadcast_normative.o -o broadcast_normative.coff
	../bin/coff2noff broadcast_normative.coff broadcast_normative

broadcast_non_normative.o: broadcast_non_normative.c
	$(CC) $(CFLAGS) -c broadcast_non_normative.c
broadcast_non_normative: broadcast_non_normative.o start.o
	$(LD) $(LDFLAGS) start.o broadcast_non_normative.o -o broadcast_non_normative.coff
	../bin/coff2noff broadcast_non_normative.coff broadcast_non_normative
	
broadcast_destroy.o: broadcast_destroy.c
	$(CC) $(CFLAGS) -c broadcast_destroy.c
broadcast_destroy: broadcast_destroy.o start.o
	$(LD) $(LDFLAGS) start.o broadcast_destroy.o -o broadcast_destroy.coff
	../bin/coff2noff broadcast_destroy.coff broadcast_destroy
	
broadcast_right_process.o: broadcast_right_process.c
	$(CC) $(CFLAGS) -c broadcast_right_process.c
broadcast_right_process: broadcast_right_process.o start.o
	$(LD) $(LDFLAGS) start.o broadcast_right_process.o -o broadcast_right_process.coff
	../bin/coff2noff broadcast_right_process.coff broadcast_right_process
	
broadcast_wrong_process.o: broadcast_wrong_process.c
	$(CC) $(CFLAGS) -c broadcast_wrong_process.c
broadcast_wrong_process: broadcast_wrong_process.o start.o
	$(LD) $(LDFLAGS) start.o broadcast_wrong_process.o -o broadcast_wrong_process.coff
	../bin/coff2noff broadcast_wrong_process.coff broadcast_wrong_process

# Other syscalls
yield.o: yield.c
	$(CC) $(CFLAGS) -c yield.c
yield: yield.o start.o
	$(LD) $(LDFLAGS) start.o yield.o -o yield.coff
	../bin/coff2noff yield.coff yield	
	
yield_test.o: yield_test.c
	$(CC) $(CFLAGS) -c yield_test.c
yield_test: yield_test.o start.o
	$(LD) $(LDFLAGS) start.o yield_test.o -o yield_test.coff
	../bin/coff2noff yield_test.coff yield_test	
			
fork.o: fork.c
	$(CC) $(CFLAGS) -c fork.c
fork: fork.o start.o
	$(LD) $(LDFLAGS) start.o fork.o -o fork.coff
	../bin/coff2noff fork.coff fork
	
fork_test.o: fork_test.c
	$(CC) $(CFLAGS) -c fork_test.c
fork_test: fork_test.o start.o
	$(LD) $(LDFLAGS) start.o fork_test.o -o fork_test.coff
	../bin/coff2noff fork_test.coff fork_test
		
exec.o: exec.c
	$(CC) $(CFLAGS) -c exec.c
exec: exec.o start.o
	$(LD) $(LDFLAGS) start.o exec.o -o exec.coff
	../bin/coff2noff exec.coff exec
	
exit.o: exit.c
	$(CC) $(CFLAGS) -c exit.c
exit: exit.o start.o
	$(LD) $(LDFLAGS) start.o exit.o -o exit.coff
	../bin/coff2noff exit.coff exit
	
exit_test.o: exit_test.c
	$(CC) $(CFLAGS) -c exit_test.c
exit_test: exit_test.o start.o
	$(LD) $(LDFLAGS) start.o exit_test.o -o exit_test.coff
	../bin/coff2noff exit_test.coff exit_test
	
printx.o: printx.c
	$(CC) $(CFLAGS) -c printx.c
printx: printx.o start.o
	$(LD) $(LDFLAGS) start.o printx.o -o printx.coff
	../bin/coff2noff printx.coff printx
	
printx_test.o: printx_test.c
	$(CC) $(CFLAGS) -c printx_test.c
printx_test: printx_test.o start.o
	$(LD) $(LDFLAGS) start.o printx_test.o -o printx_test.coff
	../bin/coff2noff printx_test.coff printx_test

testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles

museum.o: museum.c
	$(CC) $(CFLAGS) -c museum.c
museum: museum.o start.o
	$(LD) $(LDFLAGS) start.o museum.o -o museum.coff
	../bin/coff2noff museum.coff museum

uniprogrammed_museum.o: uniprogrammed_museum.c
	$(CC) $(CFLAGS) -c uniprogrammed_museum.c
uniprogrammed_museum: uniprogrammed_museum.o start.o
	$(LD) $(LDFLAGS) start.o uniprogrammed_museum.o -o uniprogrammed_museum.coff
	../bin/coff2noff uniprogrammed_museum.coff uniprogrammed_museum
	
multiprogrammed_museum.o: multiprogrammed_museum.c
	$(CC) $(CFLAGS) -c multiprogrammed_museum.c
multiprogrammed_museum: multiprogrammed_museum.o start.o
	$(LD) $(LDFLAGS) start.o multiprogrammed_museum.o -o multiprogrammed_museum.coff
	../bin/coff2noff multiprogrammed_museum.coff multiprogrammed_museum
	
createmv.o: createmv.c
	$(CC) $(CFLAGS) -c createmv.c
createmv: createmv.o start.o
	$(LD) $(LDFLAGS) start.o createmv.o -o createmv.coff
	../bin/coff2noff createmv.coff createmv
	
destroymv.o: destroymv.c
	$(CC) $(CFLAGS) -c destroymv.c
destroymv: destroymv.o start.o
	$(LD) $(LDFLAGS) start.o destroymv.o -o destroymv.coff
	../bin/coff2noff destroymv.coff destroymv
	
setmv.o: setmv.c
	$(CC) $(CFLAGS) -c setmv.c
setmv: setmv.o start.o
	$(LD) $(LDFLAGS) start.o setmv.o -o setmv.coff
	../bin/coff2noff setmv.coff setmv
	
getmv.o: getmv.c
	$(CC) $(CFLAGS) -c getmv.c
getmv: getmv.o start.o
	$(LD) $(LDFLAGS) start.o getmv.o -o getmv.coff
	../bin/coff2noff getmv.coff getmv
	
wait1.o: wait1.c
	$(CC) $(CFLAGS) -c wait1.c
wait1: wait1.o start.o
	$(LD) $(LDFLAGS) start.o wait1.o -o wait1.coff
	../bin/coff2noff wait1.coff wait1

wait2.o: wait2.c
	$(CC) $(CFLAGS) -c wait2.c
wait2: wait2.o start.o
	$(LD) $(LDFLAGS) start.o wait2.o -o wait2.coff
	../bin/coff2noff wait2.coff wait2
	
wait3.o: wait3.c
	$(CC) $(CFLAGS) -c wait3.c
wait3: wait3.o start.o
	$(LD) $(LDFLAGS) start.o wait3.o -o wait3.coff
	../bin/coff2noff wait3.coff wait3
	
wait_test.o: wait_test.c
	$(CC) $(CFLAGS) -c wait_test.c
wait_test: wait_test.o start.o
	$(LD) $(LDFLAGS) start.o wait_test.o -o wait_test.coff
	../bin/coff2noff wait_test.coff wait_test
	
broadcast1.o: broadcast1.c
	$(CC) $(CFLAGS) -c broadcast1.c
broadcast1: broadcast1.o start.o
	$(LD) $(LDFLAGS) start.o broadcast1.o -o broadcast1.coff
	../bin/coff2noff broadcast1.coff broadcast1
	
broadcast2.o: broadcast2.c
	$(CC) $(CFLAGS) -c broadcast2.c
broadcast2: broadcast2.o start.o
	$(LD) $(LDFLAGS) start.o broadcast2.o -o broadcast2.coff
	../bin/coff2noff broadcast2.coff broadcast2

broadcast3.o: broadcast3.c
	$(CC) $(CFLAGS) -c broadcast3.c
broadcast3: broadcast3.o start.o
	$(LD) $(LDFLAGS) start.o broadcast3.o -o broadcast3.coff
	../bin/coff2noff broadcast3.coff broadcast3

broadcast4.o: broadcast4.c
	$(CC) $(CFLAGS) -c broadcast4.c
broadcast4: broadcast4.o start.o
	$(LD) $(LDFLAGS) start.o broadcast4.o -o broadcast4.coff
	../bin/coff2noff broadcast4.coff broadcast4

broadcast5.o: broadcast5.c
	$(CC) $(CFLAGS) -c broadcast5.c
broadcast5: broadcast5.o start.o
	$(LD) $(LDFLAGS) start.o broadcast5.o -o broadcast5.coff
	../bin/coff2noff broadcast5.coff broadcast5

broadcast_test.o: broadcast_test.c
	$(CC) $(CFLAGS) -c broadcast_test.c
broadcast_test: broadcast_test.o start.o
	$(LD) $(LDFLAGS) start.o broadcast_test.o -o broadcast_test.coff
	../bin/coff2noff broadcast_test.coff broadcast_test

signal1.o: signal1.c
	$(CC) $(CFLAGS) -c signal1.c
signal1: signal1.o start.o
	$(LD) $(LDFLAGS) start.o signal1.o -o signal1.coff
	../bin/coff2noff signal1.coff signal1

signal2.o: signal2.c
	$(CC) $(CFLAGS) -c signal2.c
signal2: signal2.o start.o
	$(LD) $(LDFLAGS) start.o signal2.o -o signal2.coff
	../bin/coff2noff signal2.coff signal2

signal3.o: signal3.c
	$(CC) $(CFLAGS) -c signal3.c
signal3: signal3.o start.o
	$(LD) $(LDFLAGS) start.o signal3.o -o signal3.coff
	../bin/coff2noff signal3.coff signal3

signal_test.o: signal_test.c
	$(CC) $(CFLAGS) -c signal_test.c
signal_test: signal_test.o start.o
	$(LD) $(LDFLAGS) start.o signal_test.o -o signal_test.coff
	../bin/coff2noff signal_test.coff signal_test


clean:
	rm -f *.o *.coff
