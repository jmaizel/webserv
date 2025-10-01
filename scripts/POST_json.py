#!/usr/bin/env python3

import os
import sys
import json

def main():
    method = os.environ.get("REQUEST_METHOD", "GET")
    content_length = int(os.environ.get("CONTENT_LENGTH", "0"))

    # Read body
    body = sys.stdin.read(content_length) if content_length > 0 else ""

    # Try to parse JSON
    try:
        data = json.loads(body) if body else {}
    except json.JSONDecodeError as e:
        print("Content-Type: application/json\r\n")
        print(json.dumps({"error": "Invalid JSON", "message": str(e)}))
        return

    # Respond
    print("Content-Type: application/json\r\n")
    response = {
        "method": method,
        "raw_body": body,
        "parsed": data
    }
    print(json.dumps(response))

if __name__ == "__main__":
    main()
