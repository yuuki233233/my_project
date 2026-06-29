import os
import sys
import time

PIPE_PATH = "/tmp/monitor.pipe"

# 用 os.open 打开管道（非阻塞模式）
try:
    fd = os.open(PIPE_PATH, os.O_RDONLY | os.O_NONBLOCK)
except OSError as e:
    print(f"无法打开管道: {e}")
    sys.exit(1)

print(f"📡 正在监听管道: {PIPE_PATH} (非阻塞模式)")

# 使用 os.read 循环读取
with os.fdopen(fd, 'r') as pipe:
    while True:
        try:
            line = pipe.readline()
            if line:
                print("收到数据:", line.strip())
            else:
                # 没有数据时，稍微等待，避免 CPU 空转
                time.sleep(0.1)
        except Exception as e:
            print(f"读取错误: {e}")
            break
