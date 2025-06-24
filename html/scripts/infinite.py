#!/usr/bin/env python3

import time
import sys

print("Status: 200 OK")
print("Content-Type: text/plain\n")

sys.stdout.flush()  # Force headers out

# Infinite loop
while True:
    time.sleep(1)