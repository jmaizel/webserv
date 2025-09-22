#!/usr/bin/env python3

import os
import sys
import urllib.parse

# En-têtes CGI
print("Content-Type: text/html")
print()  # Ligne vide obligatoire pour séparer headers et body

# Début du HTML
print("""<!DOCTYPE html>
<html>
<head>
    <title>Test CGI WebServ</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        .header { color: #667eea; border-bottom: 2px solid #667eea; padding-bottom: 10px; }
        .section { margin: 20px 0; padding: 15px; background-color: #f8f9fa; border-radius: 5px; }
        .env-var { background-color: #e9ecef; padding: 5px; margin: 2px 0; border-radius: 3px; }
    </style>
</head>
<body>
    <h1 class="header">🐍 Test CGI Python - WebServ</h1>
""")

# Afficher les informations de requête
print('<div class="section">')
print('<h2>📋 Informations de la requête</h2>')
print(f'<p><strong>Méthode:</strong> {os.environ.get("REQUEST_METHOD", "Non définie")}</p>')
print(f'<p><strong>URI:</strong> {os.environ.get("REQUEST_URI", "Non définie")}</p>')
print(f'<p><strong>Query String:</strong> {os.environ.get("QUERY_STRING", "Non définie")}</p>')
print(f'<p><strong>Script Name:</strong> {os.environ.get("SCRIPT_NAME", "Non définie")}</p>')
print('</div>')

# Traiter les paramètres GET (query string)
query_string = os.environ.get("QUERY_STRING", "")
if query_string:
    print('<div class="section">')
    print('<h2>🔍 Paramètres GET décodés</h2>')
    params = urllib.parse.parse_qs(query_string)
    for key, values in params.items():
        for value in values:
            print(f'<div class="env-var"><strong>{key}:</strong> {value}</div>')
    print('</div>')

# Traiter les données POST si présentes
if os.environ.get("REQUEST_METHOD") == "POST":
    content_length = int(os.environ.get("CONTENT_LENGTH", "0"))
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
        print('<div class="section">')
        print('<h2>📤 Données POST reçues</h2>')
        print(f'<p><strong>Content-Length:</strong> {content_length}</p>')
        print(f'<p><strong>Content-Type:</strong> {os.environ.get("CONTENT_TYPE", "Non défini")}</p>')
        print('<h3>Données brutes :</h3>')
        print(f'<pre>{post_data}</pre>')
        
        # Essayer de décoder comme form-data
        try:
            post_params = urllib.parse.parse_qs(post_data)
            print('<h3>Paramètres décodés :</h3>')
            for key, values in post_params.items():
                for value in values:
                    print(f'<div class="env-var"><strong>{key}:</strong> {value}</div>')
        except:
            print('<p><em>Impossible de décoder les paramètres POST</em></p>')
        print('</div>')

# Afficher les variables d'environnement CGI importantes
print('<div class="section">')
print('<h2>🌍 Variables d\'environnement CGI</h2>')

important_vars = [
    "REQUEST_METHOD", "REQUEST_URI", "QUERY_STRING", "CONTENT_TYPE", 
    "CONTENT_LENGTH", "SERVER_NAME", "SERVER_PORT", "SERVER_PROTOCOL",
    "GATEWAY_INTERFACE", "SCRIPT_NAME", "PATH_INFO"
]

for var in important_vars:
    value = os.environ.get(var, "Non définie")
    print(f'<div class="env-var"><strong>{var}:</strong> {value}</div>')

print('</div>')

# Afficher tous les headers HTTP reçus
print('<div class="section">')
print('<h2>📨 Headers HTTP reçus</h2>')
http_headers = [(key, value) for key, value in os.environ.items() if key.startswith("HTTP_")]
if http_headers:
    for key, value in sorted(http_headers):
        header_name = key[5:].replace('_', '-').title()  # Enlever HTTP_ et formater
        print(f'<div class="env-var"><strong>{header_name}:</strong> {value}</div>')
else:
    print('<p><em>Aucun header HTTP détecté</em></p>')
print('</div>')

# Footer
print("""
    <div class="section">
        <h2>✅ Test CGI réussi !</h2>
        <p>Si vous voyez cette page, cela signifie que :</p>
        <ul>
            <li>✓ Le serveur WebServ détecte correctement les scripts CGI</li>
            <li>✓ Les pipes et fork() fonctionnent</li>
            <li>✓ Les variables d'environnement sont bien transmises</li>
            <li>✓ La communication serveur ↔ script est opérationnelle</li>
        </ul>
        <p><a href="/">← Retour à l'accueil</a></p>
    </div>

</body>
</html>""")