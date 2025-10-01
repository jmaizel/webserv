#!/usr/bin/env python3

import datetime

def main():
    # Required CGI header
    print("Content-Type: text/html\r\n")

    # Current time
    now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    # Simple HTML response
    print("<html><body>")
    print(f"<h1>Current Server Time</h1>")
    print(f"<p>{now}</p>")
    print("</body></html>")

if __name__ == "__main__":
    main()
