#!/usr/bin/env python3

import os

# Headers CGI
print("Content-Type: text/html")
print()

print("""<!DOCTYPE html>
<html>
<head>
    <title>CGI Info - WebServ</title>
    <style>
        body { 
            font-family: Arial, sans-serif; 
            margin: 20px; 
            background: #f5f5f5; 
        }
        .container { 
            max-width: 800px; 
            margin: 0 auto; 
            background: white; 
            padding: 30px; 
            border-radius: 10px; 
            box-shadow: 0 0 20px rgba(0,0,0,0.1); 
        }
        h1 { color: #333; text-align: center; }
        h2 { color: #667eea; border-bottom: 2px solid #667eea; padding-bottom: 5px; }
        .env-var { 
            background: #e9ecef; 
            padding: 8px; 
            margin: 3px 0; 
            border-radius: 5px; 
            font-family: monospace; 
        }
        .success { 
            background: #d4edda; 
            border: 1px solid #c3e6cb; 
            padding: 15px; 
            border-radius: 5px; 
            margin: 20px 0; 
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔍 Informations CGI - WebServ</h1>
        
        <div class="success">
            <strong>✅ CGI fonctionne parfaitement !</strong>
            <p>Ce script prouve que le serveur WebServ :</p>
            <ul>
                <li>Détecte correctement les scripts CGI (.py)</li>
                <li>Execute Python via fork/exec</li>
                <li>Transmet les variables d'environnement</li>
                <li>Récupère la sortie du script</li>
            </ul>
        </div>
        
        <h2>📋 Variables d'environnement CGI</h2>""")

# Variables CGI importantes à montrer
important_vars = [
    "REQUEST_METHOD", "REQUEST_URI", "QUERY_STRING", 
    "SERVER_NAME", "SERVER_PORT", "SERVER_PROTOCOL",
    "GATEWAY_INTERFACE", "CONTENT_LENGTH", "CONTENT_TYPE"
]

for var in important_vars:
    value = os.environ.get(var, "(non définie)")
    print(f'<div class="env-var"><strong>{var}:</strong> {value}</div>')

print("""
        <h2>📨 Headers HTTP reçus</h2>""")

# Afficher les headers HTTP
http_headers = [(key, value) for key, value in os.environ.items() if key.startswith("HTTP_")]
if http_headers:
    for key, value in sorted(http_headers):
        header_name = key[5:].replace('_', '-').title()
        print(f'<div class="env-var"><strong>{header_name}:</strong> {value}</div>')
else:
    print('<p><em>Aucun header HTTP détecté</em></p>')

print(f"""
        <h2>🔧 Détails techniques</h2>
        <div class="env-var"><strong>Script exécuté:</strong> {os.environ.get("SCRIPT_NAME", "N/A")}</div>
        <div class="env-var"><strong>Répertoire de travail:</strong> {os.getcwd()}</div>
        <div class="env-var"><strong>Variables d'environnement totales:</strong> {len(os.environ)} variables</div>
        
        <div style="text-align: center; margin-top: 30px;">
            <a href="/cgi-bin/hello.py" style="color: #667eea;">← Retour au Hello CGI</a> |
            <a href="/cgi-bin/form.py" style="color: #667eea;">Test formulaire POST →</a> |
            <a href="/" style="color: #667eea;">🏠 Accueil</a>
        </div>
    </div>
</body>
</html>""")