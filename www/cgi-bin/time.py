#!/usr/bin/env python3

import os
import datetime

# Headers CGI
print("Content-Type: text/html")
print()

now = datetime.datetime.now()
server_name = os.environ.get("SERVER_NAME", "WebServ")

print(f"""<!DOCTYPE html>
<html>
<head>
    <title>Horloge CGI - WebServ</title>
    <style>
        body {{ 
            font-family: Arial, sans-serif; 
            text-align: center;
            background: linear-gradient(135deg, #a29bfe 0%, #6c5ce7 100%);
            color: white;
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            margin: 0;
        }}
        .clock {{
            background: rgba(255,255,255,0.1);
            padding: 50px;
            border-radius: 20px;
            backdrop-filter: blur(10px);
            box-shadow: 0 0 30px rgba(0,0,0,0.3);
        }}
        .time {{
            font-size: 3rem;
            font-weight: bold;
            margin: 20px 0;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.5);
        }}
        .date {{
            font-size: 1.5rem;
            margin: 10px 0;
        }}
        .server-info {{
            background: rgba(0,0,0,0.2);
            padding: 20px;
            border-radius: 10px;
            margin: 20px 0;
            font-size: 0.9rem;
        }}
        a {{ color: #ffd700; text-decoration: none; }}
        a:hover {{ text-decoration: underline; }}
    </style>
    <meta http-equiv="refresh" content="1">
</head>
<body>
    <div class="clock">
        <h1>🕒 Horloge du serveur</h1>
        
        <div class="time">{now.strftime("%H:%M:%S")}</div>
        <div class="date">{now.strftime("%A %d %B %Y")}</div>
        
        <div class="server-info">
            <p><strong>Serveur:</strong> {server_name}:{os.environ.get("SERVER_PORT", "N/A")}</p>
            <p><strong>Script CGI:</strong> Python 3</p>
            <p><strong>Mise à jour:</strong> Automatique (1 sec)</p>
        </div>
        
        <p>
            <a href="/cgi-bin/hello.py">Hello CGI</a> |
            <a href="/cgi-bin/info.py">Infos CGI</a> |
            <a href="/">🏠 Accueil</a>
        </p>
    </div>
</body>
</html>""")