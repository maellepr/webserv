import os

query = os.environ.get('QUERY_STRING')

content = "<!DOCTYPE html> \
<html lang=\"en\"> \
    <head> \
        <title>Response</title> \
    </head> \
    <body> \
        <h1 class=\"title\">Response</h1> \
        <h2>Hello, World!</h2> \
        <p>" + query + "</p> \
        <a href=\"../../home.html\"><button>INDEX</button></a> \
    </body> \
</html>"

print(content)