import os
import sys
import time

cpu = [3,6,7,10,11]

def __main__(cpu,replica=5):
    create_ifconfig(cpu,replica)
    time.sleep(5)
    create_bash_scripts(cpu, replica)
    command = ''
    for x in range(replica - 1):
        command += 'sbatch s' + str(x) + '.sh && sleep 2 &&'
    command += 'sbatch s' + str(replica - 1) + '.sh'
    num = 0
    # for y in range(replica,replica+client - 1):
    #     command += 'sbatch c' + str(y-replica) + '.sh && '
    #     num += 1
    # command += 'sbatch c' + str(num) + '.sh'
    #print(command)
    stream = os.popen(command)
    stream = stream.read()
    print(stream)
        
def create_ifconfig(cpu, replica):
    IP = []
    for index in range(replica):
        var = cpu[index]
        ip = "172.19.4."+str(var + 1)+"\n"
        IP.append(ip)
    PATH = './ifconfig.txt'
    f = open(PATH,'w')
    f.writelines(IP)
    f.close()
    return True

def create_bash_scripts(cpu, replica):
    for r in range(replica):
        lines = []
        lines.append("#!/bin/bash\n")
        lines.append("#SBATCH --time=01:00:00\n")
        lines.append("#SBATCH --nodelist=cpu-" + str(cpu[r]) + "\n")
        lines.append("#SBATCH --account=cpu-s2-moka_blox-0"+"\n")
        lines.append("#SBATCH --partition=cpu-s2-core-0"+"\n")
        lines.append("#SBATCH --reservation=cpu-s2-moka_blox-0_18"+"\n")
        lines.append("#SBATCH --mem=2G"+"\n")
        lines.append("#SBATCH -o out" + str(r) + ".txt"+"\n")
        lines.append("#SBATCH -e err" + str(r) + ".txt"+"\n")
        lines.append("sbcast -f ifconfig.txt ifconfig.txt"+"\n")
        lines.append("sleep 5"+"\n")
        #lines.append("sbcast -f config.h config.h"+"\n")
        lines.append("srun --nodelist=cpu-" + str(cpu[r]) + " sleep 10" + "\n")
        lines.append("srun --exclusive --ntasks=1 --cpus-per-task=16 --nodelist=cpu-" + str(cpu[r]) + " singularity exec ../resilient ./bin/trans " + str(r)+"\n")
        PATH = './s' + str(r) + '.sh'
        f = open(PATH,'w')
        f.writelines(lines)
        f.close()

    # for c in range(replica,replica + client):
    #     lines = []
    #     lines.append("#!/bin/bash\n")
    #     lines.append("#SBATCH --time=01:00:00\n")
    #     lines.append("#SBATCH --nodelist=cpu-" + str(cpu[c]) + "\n")
    #     lines.append("#SBATCH --account=cpu-s2-moka_blox-0"+"\n")
    #     lines.append("#SBATCH --partition=cpu-s2-core-0"+"\n")
    #     lines.append("#SBATCH --reservation=cpu-s2-moka_blox-0_18"+"\n")
    #     lines.append("#SBATCH --mem=2G"+"\n")
    #     lines.append("#SBATCH -o out" + str(c) + ".txt"+"\n")
    #     lines.append("#SBATCH -e err" + str(c) + ".txt"+"\n")
    #     lines.append("sbcast -f ifconfig.txt ifconfig.txt"+"\n")
    #     lines.append("sleep 5"+"\n")
    #     #lines.append("sbcast -f config.h config.h"+"\n")
    #     lines.append("srun --nodelist=cpu-" + str(cpu[c]) + " sleep 10" + "\n")
    #     lines.append("srun --exclusive --ntasks=1 --cpus-per-task=16 --nodelist=cpu-" + str(cpu[c]) + " singularity exec resilient ./trans " + str(c)+"\n")
    #     PATH = './c' + str(c-replica) + '.sh'
    #     f = open(PATH,'w')
    #     f.writelines(lines)
    #     f.close()
    return True
    
if(len(sys.argv) == 2):
    print("Warning: rundb & runcl must be complied with correct values in config.h")
    __main__(cpu,int(sys.argv[1]))
else:
    print("argv={}".format(sys.argv))
    print("Command line arguments absent or invalid, please retry. \n Format: python run_resilient_db <replica> <client>")