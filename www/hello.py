#!/usr/bin/env python3
import os
import sys

print("Content-Type: text/html")
print("")

print("<html><body>")
print("<h1>Hello from CGI!</h1>")
print(f"<p>Method: {os.environ.get('REQUEST_METHOD', 'None')}</p>")
print(f"<p>Query: {os.environ.get('QUERY_STRING', 'None')}</p>")
print(f"<p>Server: {os.environ.get('SERVER_NAME', 'None')}</p>")
print("</body></html>")
