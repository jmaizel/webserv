#!/usr/bin/env python3
import os
import sys

print("Content-Type: text/html")
print("")

print("<h1>CGI Test</h1>")
print(f"<p>Method: {os.environ.get('REQUEST_METHOD', 'None')}</p>")
print(f"<p>Query: {os.environ.get('QUERY_STRING', 'None')}</p>")

if os.environ.get('REQUEST_METHOD') == 'POST':
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
        print(f"<p>POST Data: {post_data}</p>")