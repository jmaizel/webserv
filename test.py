#!/usr/bin/env python3
# persistent_clients.py
import socket
import threading
import time
import sys

HOST = "localhost"
PORT = 8083
NUM_CLIENTS = int(sys.argv[1]) if len(sys.argv) > 1 else 1000
REQUEST = b"GET / HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive"
KEEP_ALIVE = True         # if False, clients will close after reading response
READ_TIMEOUT = 5.0        # seconds to wait for initial response (set small)

def worker(i):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(10)
        s.connect((HOST, PORT))
        s.sendall(REQUEST)
        # optionally read response start (non-blocking-ish)
        try:
            s.settimeout(READ_TIMEOUT)
            data = s.recv(4096)
            # optional: print first few bytes
            # print(f"[{i}] recv: {data[:60]!r}")
        except socket.timeout:
            pass
        # keep socket open indefinitely (or until program exit)
        while KEEP_ALIVE:
            time.sleep(60)
        s.close()
    except Exception as e:
        print(f"[{i}] error: {e}")

threads = []
print(f"Starting {NUM_CLIENTS} persistent clients to {HOST}:{PORT}")
for i in range(NUM_CLIENTS):
    t = threading.Thread(target=worker, args=(i,), daemon=True)
    t.start()
    threads.append(t)
    # small sleep to avoid hammering ephemeral ports or overwhelming accept queue
    if i % 200 == 0:
        time.sleep(0.05)

# keep main alive
try:
    while True:
        time.sleep(10)
except KeyboardInterrupt:
    print("Exiting.")

