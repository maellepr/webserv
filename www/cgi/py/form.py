import cgi, cgitb

form = cgi.FieldStorage()

name = form["name"].value
firstname = form["firstname"].value
login = form["login"].value

print("Content-type:text/html\r\n\r\n")


print("<html>")
print("<head>")
print("<title> MY FIRST CGI FILE </title>")
print("</head>")
print("<body>")
print("<h3> This is HTML's Body Section </h3>")
print(name)
print(firstname)
print(login)
print("</body>")
print("</html>")