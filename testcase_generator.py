import random
import subprocess
import re


MAX_ARRIVAL_TIME = 100
MIN_ARRIVAL_TIME = 0
MAX_JOB_TIME = 100
MIN_JOB_TIME = 1
MAX_MEMORY = 100
MIN_PID = 0
MAX_PID = 20
pids = []
processes = []

def testcase_generator():	
	with open("benchmark-cm.txt", 'w') as f:
		for i in range(0, random.randrange(MIN_PID, MAX_PID)):
			this_pid = random.randrange(MIN_PID, MAX_PID)	
			while this_pid in pids:
				this_pid = random.randrange(MIN_PID, MAX_PID)
			pids.append(this_pid)
			processes.append((random.randrange(MIN_ARRIVAL_TIME, MAX_ARRIVAL_TIME), this_pid, random.randrange(1, MAX_MEMORY), random.randrange(MIN_JOB_TIME, MAX_JOB_TIME)))
		f.writelines(['{} {} {} {}\n'.format(process[0], process[1], process[2] ,process[3]) for process in sorted(processes, key=lambda x:x[0])])


def run():
	v = "./scheduler -f benchmark-cm.txt -a rr -s 100 -m v -q 10"
	cm = "./scheduler -f benchmark-cm.txt -a rr -s 100 -m cm -q 10"
	v_result = subprocess.run(v.split(), stdout=subprocess.PIPE)
	cm_result = subprocess.run(cm.split(), stdout=subprocess.PIPE)
	# if str(v_result.stdout.split("/n")[-2]) != str(cm_result.stdout.split("/n")[-2]):
	v_overhead = v_result.stdout.split(b"\n")[-3]
	cm_overhead = cm_result.stdout.split(b"\n")[-3]
	print(v_overhead, cm_overhead)

testcase_generator()
# print(processes)
run()
