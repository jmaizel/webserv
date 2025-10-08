#!/usr/bin/env python3
import sys
import os

def main():
    # Get Content-Length (if present)
    content_length = os.environ.get('CONTENT_LENGTH')
    body = ""
    if content_length:
        try:
            length = int(content_length)
            body = sys.stdin.read(length)
        except Exception as e:
            body = f"Error reading body: {e}"

    # Print HTTP response
    print("Content-Type: text/html\r\n\r\n")
    print("<html>")
    print("<head><title>POST Demo</title></head>")
    print("<body style='background:#0a0a0f; color:#e9e6ff; font-family:sans-serif;'>")
    print("<h1>POST Method Demo</h1>")
    print("<h3>Environment Variables</h3>")
    print("<pre>")
    for k, v in os.environ.items():
        if k.startswith("HTTP_") or k in ("REQUEST_METHOD", "CONTENT_TYPE", "CONTENT_LENGTH"):
            print(f"{k} = {v}")
    print("</pre>")

    print("<h3>POST Body</h3>")
    print("<pre>")
    print(body)
    print("</pre>")

    print("</body></html>")

if __name__ == "__main__":
    main()
