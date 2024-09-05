#!/usr/bin/python
import cgi, cgitb
cgitb.enable()
form = cgi.FieldStorage()

print("Content-type: text/html\r\n")

if form.getvalue('name'):
    name = form.getvalue('name')
    print(f"Set-Cookie: name={name}\r\n")
    print(f"""
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <title>You will be remembered, {name}</title>
    </head>
    <body>
        <a href="site_index.html"><button>go back to site index</button></a>
    </body>
    </html>
    """)
else:
    print("""
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <title>Did not quite get your name...</title>
    </head>
    <body>
        <a href="site_index.html"><button>go back to site index</button></a>
    </body>
    </html>
    """)