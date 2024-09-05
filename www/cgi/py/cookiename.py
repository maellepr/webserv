#!/usr/bin/python3
import cgi, cgitb, sys, os
print("ICI 0", file=sys.stderr)
cgitb.enable()
print("ICI 0.1", file=sys.stderr)

content_length = os.environ.get('CONTENT_LENGTH', 0)
print(f"Content-Length: {content_length}", file=sys.stderr)

form = cgi.FieldStorage()

if form is None:
    print("No form data received", file=sys.stderr)

print("ICI 1", file=sys.stderr)

print("Content-type: text/html\r\n")

if form.getvalue('name'):
    print("ICI 2", file=sys.stderr)
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
    print("ICI 3", file=sys.stderr)
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