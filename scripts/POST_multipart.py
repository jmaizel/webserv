#!/usr/bin/env python3

import os
import sys
import cgi

def main():
    # Create FieldStorage object from environment + stdin
    form = cgi.FieldStorage()

    print("Content-Type: text/html\r\n")
    print("<html><body>")
    print("<h1>Multipart CGI Parser</h1>")

    # Loop through all fields
    for field in form.keys():
        item = form[field]
        if item.filename:  # file upload
            file_data = item.file.read()
            print(f"<p>Uploaded file: {item.filename} ({len(file_data)} bytes)</p>")
            # (optional) save the file
            with open(item.filename, "wb") as f:
                f.write(file_data)
        else:  # regular text field
            print(f"<p>{field} = {item.value}</p>")

    print("</body></html>")

if __name__ == "__main__":
    main()
