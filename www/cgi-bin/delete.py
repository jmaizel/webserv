#!/usr/bin/env python3

import os
import urllib.parse

print("Content-Type: text/html")
print()

method = os.environ.get("REQUEST_METHOD", "GET")

print("""<!DOCTYPE html>
<html>
<head>
    <title>Suppression DELETE CGI - WebServ</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 600px; margin: 50px auto; background: linear-gradient(135deg, #fd746c 0%, #ff9068 100%); color: white; }
        .container { background: rgba(255,255,255,0.1); padding: 40px; border-radius: 20px; backdrop-filter: blur(10px); }
        h1 { text-align: center; }
        .result { background: rgba(0,0,0,0.2); padding: 20px; border-radius: 10px; margin: 20px 0; }
        a { color: #ffd700; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🗑️ Suppression DELETE CGI</h1>
""")

if method == "DELETE":
    print("<div class='result'><h2>Suppression demandée !</h2>")
    print(f"<p>Méthode: {method}</p>")
    print(f"<p>URI: {os.environ.get('REQUEST_URI', '')}</p>")
    print("<p><strong>Fichier supprimé (simulation) :</strong> /upload/demo.txt</p>")
    print("</div>")
else:
    print("<div class='result'><h2>Démo suppression DELETE</h2>")
    print("<p>Appuyez sur le bouton pour simuler une requête DELETE.</p>")
    print("<form method='POST' action='/cgi-bin/delete.py?_method=DELETE'>")
    print("<button type='submit' style='background:#fd746c;color:white;padding:10px 30px;border:none;border-radius:5px;font-size:18px;'>Supprimer (DELETE)</button>")
    print("</form>")
    print("</div>")

print("""
        <div style="text-align: center; margin-top: 30px;">
            <a href="/cgi-bin/hello.py">← Hello CGI</a> |
            <a href="/cgi-bin/form.py">Formulaire POST</a> |
            <a href="/cgi-bin/info.py">Infos CGI</a> |
            <a href="/cgi-bin/test.py">Liste scripts</a> |
            <a href="/">🏠 Accueil</a>
        </div>
    </div>
</body>
</html>""")
