#!/usr/bin/env python3

import os
import sys
import urllib.parse

# Headers CGI
print("Content-Type: text/html; charset=utf-8")
print()

method = os.environ.get("REQUEST_METHOD", "GET")

print(f"""<!DOCTYPE html>
<html>
<head>
    <title>Formulaire CGI - WebServ</title>
    <meta charset='utf-8'>
    <style>
        body {{ font-family: Arial, sans-serif; max-width: 600px; margin: 50px auto; padding: 20px; background: linear-gradient(135deg, #74b9ff 0%, #0984e3 100%); color: white; }}
        .container {{ background: rgba(255,255,255,0.1); padding: 30px; border-radius: 15px; backdrop-filter: blur(10px); }}
        h1 {{ text-align: center; }}
        form {{ background: rgba(255,255,255,0.2); padding: 20px; border-radius: 10px; margin: 20px 0; }}
        input, textarea {{ width: 100%; padding: 10px; margin: 8px 0; border: none; border-radius: 5px; box-sizing: border-box; }}
        button {{ background: #00b894; color: white; padding: 12px 30px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; }}
        button:hover {{ background: #00a085; }}
        .result {{ background: rgba(0,0,0,0.2); padding: 20px; border-radius: 10px; margin: 20px 0; }}
        a {{ color: #ffd700; }}
    </style>
</head>
<body>
    <div class="container">
        <h1>📝 Test Formulaire CGI</h1>""")

if method == "POST":
    # Traiter les données POST
    content_length = int(os.environ.get("CONTENT_LENGTH", "0"))
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
        print(f"""
        <div class="result">
            <h2>✅ Données POST reçues !</h2>
            <p><strong>Méthode:</strong> {method}</p>
            <p><strong>Content-Length:</strong> {content_length} bytes</p>
            <p><strong>Content-Type:</strong> {os.environ.get("CONTENT_TYPE", "N/A")}</p>
            <p><strong>Données brutes:</strong></p>
            <pre style="background: rgba(0,0,0,0.3); padding: 10px; border-radius: 5px;">{post_data}</pre>
        """)
        # Décodage des paramètres POST
        params = urllib.parse.parse_qs(post_data)
        if params:
            print("<p><strong>Paramètres décodés:</strong></p>")
            for key, values in params.items():
                for value in values:
                    print(f"<p>• <strong>{key}:</strong> {value}</p>")
        print("</div>")

print("""
        <form method="POST">
            <h3>Formulaire de test :</h3>
            <label>Nom :</label>
            <input type="text" name="nom" placeholder="Votre nom" required>
            <label>Email :</label>
            <input type="email" name="email" placeholder="votre@email.com" required>
            <label>Message :</label>
            <textarea name="message" rows="4" placeholder="Votre message..." required></textarea>
            <button type="submit">📤 Envoyer (POST)</button>
        </form>
        <div style="text-align: center; margin-top: 30px;">
            <a href="/cgi-bin/hello.py">← Hello CGI</a> |
            <a href="/cgi-bin/info.py">Infos CGI</a> |
            <a href="/cgi-bin/delete.py">Suppression DELETE</a> |
            <a href="/cgi-bin/test.py">Liste scripts</a> |
            <a href="/">🏠 Accueil</a>
        </div>
    </div>
</body>
</html>""")