#!/usr/bin/env python3
import os
import sys
import time
import hashlib
import base64
import json
import urllib.parse

# 1. Start high-precision timer
start_time = time.perf_counter()

# 2. Read input from GET or POST
method = os.environ.get("REQUEST_METHOD", "GET")
raw_input = ""

if method == "POST":
    try:
        content_length = int(os.environ.get("CONTENT_LENGTH", 0))
        if content_length > 0:
            body = sys.stdin.read(content_length)
            params = urllib.parse.parse_qs(body)
            raw_input = params.get("text", [""])[0]
    except Exception:
        raw_input = ""
else:
    query = os.environ.get("QUERY_STRING", "")
    params = urllib.parse.parse_qs(query)
    raw_input = params.get("text", [""])[0]

if not raw_input:
    raw_input = "Hello 42 Webserv!"

# 3. Perform Transformations
sha256_res = hashlib.sha256(raw_input.encode()).hexdigest()
md5_res = hashlib.md5(raw_input.encode()).hexdigest()
base64_res = base64.b64encode(raw_input.encode()).decode()
binary_res = " ".join(format(ord(c), "08b") for c in raw_input)

# 4. Stop timer
elapsed_ms = (time.perf_counter() - start_time) * 1000

# 5. Output JSON response
result = {
    "language": "Python",
    "input": raw_input,
    "time_ms": round(elapsed_ms, 4),
    "sha256": sha256_res,
    "md5": md5_res,
    "base64": base64_res,
    "binary": binary_res
}

print("Content-Type: application/json\r\n\r\n")
print(json.dumps(result))
