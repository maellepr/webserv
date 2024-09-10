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
		<link href="../../style.css" rel="stylesheet">
		<title>Saved</title>
	</head>
	<body>
		<div class="title2"><br><br>You will be remembered, {name}<br><br></div>
		<div class="indexremember">
            <a class="indexButton" href="/">go back to home page</a>
        </div>
	</body>
</html>""")
else:
    print("""<!DOCTYPE html>
<html lang="en">
	<head>
		<link href="../../style.css" rel="stylesheet">
		<title>Saved</title>
	</head>
	<body>
		<div class="title2"><br><br>Did not quite get your name...<br><br></div>
		<div class="indexremember">
            <a class="indexButton" href="/">go back to home page</a>
        </div>
	</body>
</html>""")