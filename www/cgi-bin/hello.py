#!/usr/bin/env python3

import os

# Headers CGI
print("Content-Type: text/html")
print()

# Récupérer les paramètres GET
query_string = os.environ.get("QUERY_STRING", "")
name = "Visiteur"

if query_string:
    # Parser simple : name=Jacob
    if "name=" in query_string:
        name = query_string.split("name=")[1].split("&")[0]

print(f"""<!DOCTYPE html>
<html>
<head>
    <title>Hello CGI</title>
    <style>
        body {{ 
            font-family: Arial, sans-serif; 
            max-width: 600px; 
            margin: 50px auto; 
            text-align: center;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            justify-content: center;
        }}
        .container {{
            background: rgba(255,255,255,0.1);
            padding: 40px;
            border-radius: 20px;
            backdrop-filter: blur(10px);
        }}
        h1 {{ font-size: 2.5rem; margin-bottom: 1rem; }}
        .info {{ background: rgba(0,0,0,0.2); padding: 20px; border-radius: 10px; margin: 20px 0; }}
        a {{ color: #ffd700; text-decoration: none; }}
    </style>
</head>
<body>
    <div class="container">
        <h1>🐍 Hello from Python CGI!</h1>
        <h2>Salut <strong>{name}</strong> ! 👋</h2>
        
        <div class="info">
            <h3>📊 Informations CGI :</h3>
            <p><strong>Méthode:</strong> {os.environ.get("REQUEST_METHOD", "N/A")}</p>
            <p><strong>Query String:</strong> {query_string or "(vide)"}</p>
            <p><strong>Serveur:</strong> {os.environ.get("SERVER_NAME", "N/A")}:{os.environ.get("SERVER_PORT", "N/A")}</p>
        </div>
        
        <p>
            <a href="?name=Jacob">Test avec Jacob</a> | 
            <a href="?name=Alice">Test avec Alice</a> | 
            <a href="/cgi-bin/info.py">Infos détaillées</a> |
            <a href="/">🏠 Accueil</a>
        </p>
    </div>
</body>
</html>""")