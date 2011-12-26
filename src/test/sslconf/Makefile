#
# 	Makefile for the Appweb unit test ssl configuration
#
#	Copyright (c) Embedthis Software LLC, 2003-2012. All Rights Reserved.
#

include 	.makedep

#
#	Some targets to help you get going with SSL (using OpenSSL). All these certificates are for internal use only.
#	Use "make ServerCertReq" to generate a certificate request to send to a certificate authority like Verisign
#
#	To run appweb, you need to do:
#
#		make caCert
#		make serverCert
#
#	Setup a test certificate authority. Use this if you will be generating multiple certificates for clients and 
#	servers and want to validate them. The browser will say it doesn't recognize this certificate, but that is ok.
#
ca:
	umask 77 ; OPENSSL_CONF=openssl.ca ; export OPENSSL_CONF ; \
	openssl req -x509 -newkey rsa -passout pass:ca -out ca.crt -outform PEM; \
	openssl rsa -passin pass:ca -in ca.key -out ca.key.pem ; \
	echo "01" >ca.serial ; \
	>ca.db

#
#	Client certificate to put in your browser. Used when doing client validation
#
clientCert:
	umask 77 ; OPENSSL_CONF=openssl.conf ; export OPENSSL_CONF ; \
	rm -f client.crt client.key client.key.pem client.req ; \
	openssl req -passout pass:client -newkey rsa:1024 -keyout client.key \
		-out client.req < response.client ; \
	openssl rsa -passin pass:client -in client.key -out client.key.pem ; \
	OPENSSL_CONF=openssl.ca ; export OPENSSL_CONF ; \
	openssl ca -in client.req -out client.crt

#
#	Server certificate to put in Appweb. Necessary.
#
serverCert:
	umask 77 ; OPENSSL_CONF=openssl.conf ; export OPENSSL_CONF ; \
	rm -f server.crt server.key server.key.pem server.req ; \
	openssl req -passout pass:server -newkey rsa:1024 -keyout server.key \
		-out server.req < response.server ; \
	openssl rsa -passin pass:server -in server.key -out server.key.pem ; \
	OPENSSL_CONF=openssl.ca ; export OPENSSL_CONF ; openssl ca -in server.req -out server.crt

#
#	Server certificate to put in Appweb. Necessary.
#	Use a self-signed certificate when you just want quick and dirty testing.
#	The browser will say it doesn't recognize this certificate, but that is ok.
#
serverSelfSignCert:
	umask 77 ; OPENSSL_CONF=openssl.conf ; export OPENSSL_CONF ; \
	openssl genrsa -des3 -passout pass:server -out server.key 1024 ; \
	openssl req -passin pass:server -new -x509 -days 365 -key server.key -out server.crt < response.server ; \
	openssl rsa -passin pass:server -in server.key -out server.key.pem

#
#	Self-signed client request
#
clientSelfSignCert:
	umask 77 ; OPENSSL_CONF=openssl.conf ; export OPENSSL_CONF ; \
	openssl genrsa -des3 -passout pass:client -out client.key 1024 ; \
	openssl req -passin pass:client -new -x509 -days 365 -key client.key -out client.crt < response.client ; \
	openssl rsa -passin pass:client -in client.key -out client.key.pem

#
#	Generate a request to send to a certificate authority like Verisign
#
ServerCertReq:
	umask 77 ; \
	openssl genrsa -des3 -passout pass:server -out server.key 1024 ; \
	openssl req -new -key server.key -out server.csr

#
#	Stop the key asking for a password by decrypting it and rewriting back
#	to server.key. WARNING: this is not secure as the key is now decrypted.
#
noPassPhrase:
	openssl genrsa 1024 -out server.key
	chmod 600 server.key

#
#	Decrypt the key into server.key.pem
#
decrypted:
	openssl rsa -in server.key -out server.key.pem


#
#   Local variables:
#   tab-width: 4
#   c-basic-offset: 4
#   End:
#   vim: sw=4 ts=4 noexpandtab
#
