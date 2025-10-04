#!/usr/bin/env python3

import os
import sys
import urllib.parse

def main():
    # Get environment info
    method = os.environ.get("REQUEST_METHOD", "GET")
    content_length = int(os.environ.get("CONTENT_LENGTH", "0"))
    content_type = os.environ.get("CONTENT_TYPE", "")

    # Read body from stdin (for POST/PUT)
    body = sys.stdin.read(content_length) if content_length > 0 else ""

    # Try to parse if form data
    parsed_data = {}
    if content_type.startswith("application/x-www-form-urlencoded"):
        parsed_data = urllib.parse.parse_qs(body)

    # Send response headers
    print("Content-Type: text/html\r\n")

    # Send body
    print("<html><body>")
    print(f"<h1>CGI Python Echo</h1>")
    print(f"<p>Method: {method}</p>")
    print(f"<p>Content-Type: {content_type}</p>")
    print(f"<p>Raw Body: {body}</p>")
    print("<h2>Parsed Data:</h2>")
    for key, values in parsed_data.items():
        print(f"<p>{key} = {', '.join(values)}</p>")
    print("</body></html>")

if __name__ == "__main__":
    main()
