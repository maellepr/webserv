#!/usr/bin/python
import cgi, cgitb, sys, os
cgitb.enable()

form = cgi.FieldStorage()

if form.getvalue('color'):
	color = form.getvalue('color')
else:
	color = 'black'

print(f"Set-Cookie: color={color}", end='\r\n')
print("Location: color.py\r\n", end='\r\n')