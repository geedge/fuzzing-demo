import multiprocessing
import threading
import os
import random
import time
import signal
import sys
import glob
from datetime import datetime

working_directory = "/root/work/dp_telemetry_app/build/support/marsio/src/marsio/build_v474/examples"

# 命令列表
commands = [
    "./mrtest-l2fwd-nf -a l2fw0 -s nf_0",
    "./mrtest-l2fwd-nf -a l2fw1 -s nf_1",
    "./mrtest-l2fwd-nf -a l2fw2 -s nf_2"
]

# 线程函数，随机选择并执行一个命令
def execute_command(stop_event, process_id):
    count = 0
    while not stop_event.is_set():
        command = random.choice(commands)
        # thread = threading.Thread(target=os.system, args=(command,))
        thread = threading.Thread(target=lambda: os.system(f"{command} > /dev/null 2>&1"))
        thread.daemon = True  # 使线程在主线程退出时也退出
        thread.start()
        count += 1
        print(f"Process {process_id} executed command count: {count}")
        time.sleep(random.uniform(2, 4))

# 进程函数，定期检查并杀死随机一个mrtest-l2fwd-nf进程
def kill_process(stop_event):
    while True:
        pid_output = os.popen("pidof mrtest-l2fwd-nf").read().strip()
        if pid_output:
            pids = pid_output.split()
            pid_to_kill = random.choice(pids)
            os.system(f"kill {pid_to_kill}")
            print(f"Killed process with PID: {pid_to_kill}")
        elif stop_event.is_set():
            break
        time.sleep(random.uniform(2, 4))

# 打印 mrzcpd 进程使用的文件描述符个数
def print_fd_count(stop_event):
    while not stop_event.is_set():
        pid_output = os.popen("pidof mrzcpd").read().strip()
        if pid_output:
            pid = pid_output.split()[0]
            fd_count = len(glob.glob(f"/proc/{pid}/fd/*"))
            print(f"mrzcpd process with PID {pid} is using {fd_count} file descriptors")
        else:
            print("mrzcpd process not found")
        time.sleep(1) 

def signal_handler(signum, frame, stop_event, processes, start_time):
    print("Caught signal, terminating processes...")
    stop_event.set()
    end_time = datetime.now()
    elapsed_time = end_time - start_time
    hours, remainder = divmod(elapsed_time.total_seconds(), 3600)
    minutes, seconds = divmod(remainder, 60)
    print(f"Program ran for: {int(hours)}:{int(minutes)}:{int(seconds)}")
    # for process in processes:
    #     process.terminate()
    # sys.exit(0)

if __name__ == "__main__":
    os.chdir(working_directory)

    stop_event = multiprocessing.Event()
    start_time = datetime.now()

    # 创建并启动执行命令的进程
    command_process_1 = multiprocessing.Process(target=execute_command,args=(stop_event,1))
    command_process_1.start()

    command_process_2 = multiprocessing.Process(target=execute_command,args=(stop_event,2))
    command_process_2.start()

    command_process_3 = multiprocessing.Process(target=execute_command,args=(stop_event,3))
    command_process_3.start()

    # 创建并启动杀死进程的进程
    kill_process_process_1 = multiprocessing.Process(target=kill_process,args=(stop_event,))
    kill_process_process_1.start()

    kill_process_process_2 = multiprocessing.Process(target=kill_process,args=(stop_event,))
    kill_process_process_2.start()

    # 创建并启动打印文件描述符的进程
    fd_count_process = multiprocessing.Process(target=print_fd_count, args=(stop_event,))
    fd_count_process.start()

    processes = [command_process_1, command_process_2, command_process_3, kill_process_process_1, kill_process_process_2, fd_count_process]

    # 设置信号处理程序来捕获 SIGINT (Ctrl-C)
    signal.signal(signal.SIGINT, lambda signum, frame: signal_handler(signum, frame, stop_event, processes, start_time))

    # 等待两个进程结束
    for process in processes:
        process.join()
