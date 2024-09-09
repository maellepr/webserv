#!/usr/bin/python3
import cgi, cgitb, sys, os
cgitb.enable()

# content_length = os.environ.get('CONTENT_LENGTH', 0)
# print(f"Content-Length: {content_length}", file=sys.stderr)

form = cgi.FieldStorage()

if form.getvalue('name'):
    name = form.getvalue('name')
    print(f"Set-Cookie: name={name}; path=/; Max-Age=31536000\r\n")
    print(f"""<!DOCTYPE html>
<html lang="en">
	<head>
		<title>Saved</title>
	</head>
	<body>
		<p>You will be remembered, {name}</p>
		<a href="/"><button>Back to homepage</button></a>
	</body>
</html>""")
else:
    print("""<!DOCTYPE html>
<html lang="en">
	<head>
		<title>Say what</title>
	</head>
	<body>
		<p>Did not quite get your name...</p>
		<a href="/"><button>Back to homepage</button></a>
	</body>
</html>""")