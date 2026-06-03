#!/usr/bin/python3
import os

# 1. Print the mandatory HTTP headers and the blank line (\r\n\r\n)
print("Content-Type: text/html\r\n")

# 2. Print the HTML body
print("<html>")
print("<head><title>Python CGI</title></head>")
print("<body style=\"font-family: Arial; text-align: center; margin-top: 50px;\">")
print("  <h1>Hello from Python!</h1>")
print("  <p>If you see this, your C++ execve() worked perfectly.</p>")

# Test if the C++ server passed the environment variables properly
method = os.environ.get("REQUEST_METHOD", "Variable Missing!")
print("  <p><strong>HTTP Method Used:</strong> " + method + "</p>")

print("</body>")
print("</html>")