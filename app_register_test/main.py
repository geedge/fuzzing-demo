import multiprocessing
import threading
import os
import random
import time
import signal
import sys

working_directory = "/root/work/dp_telemetry_app/build/support/marsio/src/marsio/build/examples"

# 命令列表
commands = [
    "./mrtest-l2fwd-nf -a l2fw0 -s nf_0",
    "./mrtest-l2fwd-nf -a l2fw1 -s nf_1",
    "./mrtest-l2fwd-nf -a l2fw2 -s nf_2"
]

# 线程函数，随机选择并执行一个命令
def execute_command(stop_event):
    while stop_event.is_set():
        command = random.choice(commands)
        thread = threading.Thread(target=os.system, args=(command,))
        thread.daemon = True  # 使线程在主线程退出时也退出
        thread.start()
        time.sleep(2)  # 定时执行间隔，可以根据需要调整

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
        time.sleep(1)  # 定时检查间隔，可以根据需要调整

def signal_handler(signum, frame, stop_event, processes):
    print("Caught signal, terminating processes...")
    stop_event.set()
    for process in processes:
        process.terminate()
    sys.exit(0)

if __name__ == "__main__":
    os.chdir(working_directory)

    stop_event = multiprocessing.Event()

    # 创建并启动执行命令的进程
    command_process = multiprocessing.Process(target=execute_command,args=(stop_event,))
    command_process.start()

    # 创建并启动杀死进程的进程
    # kill_process_process = multiprocessing.Process(target=kill_process,args=(stop_event,))
    # kill_process_process.start()

    processes = [command_process]

    # 设置信号处理程序来捕获 SIGINT (Ctrl-C)
    signal.signal(signal.SIGINT, lambda signum, frame: signal_handler(signum, frame, stop_event, processes))

    # 等待两个进程结束
    for process in processes:
        process.join()
