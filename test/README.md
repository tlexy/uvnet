# openssl_examples

## examples of using OpenSSL
-------

Running the program requires that a SSL certificate and private key are
available to be loaded. These can be generated using the 'openssl' program using
these steps:

1. Generate the private key, this is what we normally keep secret:
```console
    openssl genrsa -des3 -passout pass:ABCD -out server.pass.key 2048
    openssl rsa -passin pass:ABCD -in server.pass.key -out server.key
    rm -f server.pass.key
```
2. Next generate the CSR.  We can leave the password empty when prompted
   (because this is self-sign):
```console
    openssl req -new -key server.key -out server.csr
```
3. Next generate the self signed certificate:
```console
    openssl x509 -req -sha256 -days 365 -in server.csr -signkey server.key -out server.crt
    rm -f server.csr
```
The openssl program can also be used to connect to this program as an SSL
client. Here's an example command (assuming we're using port 55555):
```console
    openssl s_client -connect 127.0.0.1:55555 -msg -debug -state -showcerts
```

Flow of encrypted & unencrypted bytes
-------------------------------------

This diagram shows how the read and write memory BIO's (rbio & wbio) are
associated with the socket read and write respectively.  On the inbound flow
(data into the program) bytes are read from the socket and copied into the rbio
via BIO_write.  This represents the the transfer of encrypted data into the SSL
object. The unencrypted data is then obtained through calling SSL_read.  The
reverse happens on the outbound flow to convey unencrypted user data into a
socket write of encrypted data.

```
  +------+                                    +-----+
  |......|--> read(fd) --> BIO_write(rbio) -->|.....|--> SSL_read(ssl)  --> IN
  |......|                                    |.....|
  |.sock.|                                    |.SSL.|
  |......|                                    |.....|
  |......|<-- write(fd) <-- BIO_read(wbio) <--|.....|<-- SSL_write(ssl) <-- OUT
  +------+                                    +-----+

          |                                  |       |                     |
          |<-------------------------------->|       |<------------------->|
          |         encrypted bytes          |       |  unencrypted bytes  |
```
