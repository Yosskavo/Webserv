#!/usr/bin/env python3                                                                       
import os                                                                                    
import sys                                                                                   
import urllib.parse                                                                          
                                                                                             
# 1. MUST output the HTTP Content-Type header + blank line                                   
print("Content-Type: text/html\r\n\r\n")                                                     
                                                                                             
# 2. Read standard CGI Environment Variables                                                 
method = os.environ.get("REQUEST_METHOD", "GET")                                             
query_string = os.environ.get("QUERY_STRING", "")                                            
cookie_header = os.environ.get("HTTP_COOKIE", "No cookies received")                         
                                                                                             
# 3. If POST, read the body from STDIN                                                       
post_body = ""                                                                               
if method == "POST":                                                                         
    try:                                                                                     
        content_length = int(os.environ.get("CONTENT_LENGTH", 0))                            
        if content_length > 0:                                                               
            post_body = sys.stdin.read(content_length)                                       
    except Exception as e:                                                                   
        post_body = f"Error reading STDIN: {e}"                                              

# 4. Generate dynamic HTML output
print("<!DOCTYPE html>")
print("<html>")
print("<head><title>CGI Execution Result</title></head>")
print("<body>")
print("    <h1>Python CGI Script Output</h1>")
print(f"   <p><strong>Request Method:</strong> {method}</p>")
print(f"   <p><strong>Query String (GET):</strong> {query_string}</p>")
print(f"   <p><strong>POST Body Data:</strong> {post_body if post_body else 'None'}</p>")    
print(f"   <p><strong>Cookies Received:</strong> {cookie_header}</p>")
print("    <hr>")
print("    <a href='/'>Back to Webserv Dashboard</a>")
print("</body>")
print("</html>")
