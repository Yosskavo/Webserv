# Nginx HTTP Request Errors and Status Codes

## 4xx --- Client Request Errors

  Code   Name                              Meaning
  ------ --------------------------------- ---------------------------------------------
  400    Bad Request                       Malformed request syntax or invalid request
  401    Unauthorized                      Authentication required
  402    Payment Required                  Reserved, rarely used
  403    Forbidden                         Access denied
  404    Not Found                         Resource doesn't exist
  405    Method Not Allowed                HTTP method not allowed
  406    Not Acceptable                    Requested representation unavailable
  407    Proxy Authentication Required     Proxy auth needed
  408    Request Timeout                   Client took too long
  409    Conflict                          Request conflicts with server state
  410    Gone                              Resource permanently removed
  411    Length Required                   Missing Content-Length
  412    Precondition Failed               Condition header failed
  413    Payload Too Large                 Request body too large
  414    URI Too Long                      URL too long
  415    Unsupported Media Type            Content type unsupported
  416    Range Not Satisfiable             Invalid range request
  417    Expectation Failed                Expect header failure
  421    Misdirected Request               Sent to wrong server
  426    Upgrade Required                  Protocol upgrade needed
  428    Precondition Required             Preconditions required
  429    Too Many Requests                 Rate limit exceeded
  431    Request Header Fields Too Large   Headers too large
  451    Unavailable For Legal Reasons     Restricted by law

## Nginx-Specific Client Cases

  Code   Cause
  ------ --------------------------------------
  400    Invalid headers or malformed request
  413    client_max_body_size exceeded
  414    Request URI too large
  494    Request header too large
  495    SSL certificate error
  496    Client certificate required
  497    HTTP request sent to HTTPS port
  499    Client closed request

## 5xx --- Server Errors

  Code   Name                              Meaning
  ------ --------------------------------- ----------------------------------
  500    Internal Server Error             Generic server failure
  501    Not Implemented                   Feature not supported
  502    Bad Gateway                       Upstream sent invalid response
  503    Service Unavailable               Server unavailable or overloaded
  504    Gateway Timeout                   Upstream timeout
  505    HTTP Version Not Supported        Unsupported HTTP version
  506    Variant Also Negotiates           Configuration problem
  507    Insufficient Storage              Server lacks storage
  508    Loop Detected                     Infinite loop detected
  510    Not Extended                      Further extensions needed
  511    Network Authentication Required   Network login required

## Common Upstream Errors

  Code   Typical Reason
  ------ ------------------------------
  502    Upstream process unavailable
  503    Upstream overloaded
  504    Upstream response timeout

Typical log messages:

``` text
connect() failed (111: Connection refused)
upstream timed out
no live upstreams
upstream prematurely closed connection
host not found in upstream
connection reset by peer
```

## Nginx-Only Codes

  Code   Meaning
  ------ -----------------------------------
  444    Close connection without response
  494    Request header too large
  495    SSL certificate error
  496    Client certificate missing
  497    HTTP request sent to HTTPS port
  499    Client aborted request
