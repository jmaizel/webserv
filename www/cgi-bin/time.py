#!/usr/bin/env python3
import datetime

# Headers HTTP (OBLIGATOIRE pour CGI)
print("Content-Type: text/html\r")
print("\r")  # Ligne vide obligatoire entre headers et body

# Body HTML
print("<html>")
print("<head><title>Script Python en action!</title></head>")
print("<body style='font-family: Arial; text-align: center; margin-top: 50px;'>")
print("<h1>Hello from Python CGI!</h1>")
print(f"<h2>Il est actuellement : {datetime.datetime.now().strftime('%H:%M:%S')}</h2>")
print(f"<p>Date : {datetime.datetime.now().strftime('%d/%m/%Y')}</p>")
print("<p><a href='/'>Retour à l'accueil</a></p>")
print("</body>")
print("</html>")