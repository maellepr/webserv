import os
from http.cookies import SimpleCookie

def choseColor(choice):
	print("Location: http://localhost:8080/colors.html\r\n", end='\r\n')

def makePage(choice):
	style = choice['color'].value
	print(choice, end='\r\n')
	# print("Content-Type: text/html\r\n", end='\r\n')
	print("<html><head><link href=\"../../style.css\" rel=\"stylesheet\"><title> Your favourite color </title></head><body><style>")
	if (style == 'purple'):
		print("#one {color:rgb(134, 0, 139);text-align: center;font-size:150%;}")
	elif (style == 'blue'):
		print("#one {color:rgb(111, 100, 237);text-align: center;font-size:150%;}")
	elif (style == 'pink'):
		print("#one {color:rgb(207, 48, 175);text-align: center;font-size:150%;}")
	elif (style == 'green'):
		print("#one {color:rgb(67, 214, 106);text-align: center;font-size:150%;}")
	else:
		style = 'black'
		print("#one {color:rgb(7, 6, 6);text-align: center;font-size:150%;}")
	
	print("</style><p id=\"one\">")
	print(f"{style} is a beautiful colour !")
	print("</p></body></html><div class=\"index\"><a class=\"indexButton\" href=\"/\">go back to home page</a></div></body>")


def main():

	if 'HTTP_COOKIE' in os.environ:
		choice = SimpleCookie(os.environ['HTTP_COOKIE'])
	else:
		choice = SimpleCookie()
	if 'color' not in choice:
		choice = choseColor(choice)
	else:
		makePage(choice)


main()
