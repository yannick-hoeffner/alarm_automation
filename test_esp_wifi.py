import socket
import subprocess
from pathlib import Path
from datetime import datetime
import time

UDP_IP = "0.0.0.0"  # Listen on all interfaces
UDP_PORT = 64000      # Must match targetPort on the ESP8266

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.settimeout(0.5)
sock.bind((UDP_IP, UDP_PORT))
ps1 = Path(r"./test_alarm2.ps1").resolve()
assert ps1.is_file(), f"PowerShell script not found: {ps1}"
# cmd = [
#     "powershell",
#     "-NoProfile",
#     "-ExecutionPolicy", "Bypass",
#     "-File", str(ps1)
# ]
path_exe = Path(r".\build\Release\alarm.exe").resolve()
cmd = [path_exe]

print(f"Listening for UDP packets on port {UDP_PORT}...")
window_shown = False
last_received_time = time.time()
last_shown_time = 0
p = None
try:
    while True:
        try:
            data, addr = sock.recvfrom(1024)
        except socket.timeout:
            continue
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
        print(f"[{timestamp}] Received from {addr[0]}:{addr[1]} -> {data.decode().strip()}")
        last_received_time = time.time()
        if not window_shown:
            if p is None and last_shown_time + 5 < time.time():  # Add a cooldown to prevent rapid reopening
                last_shown_time = time.time()
                window_shown = True
                p = subprocess.Popen(cmd)
        else:
            # periodically check if the window is still open, if not reset the flag
            if p and p.poll() is not None:
                window_shown = False
                p = None
except KeyboardInterrupt:
    print("\nExiting...")
finally:
    sock.close()