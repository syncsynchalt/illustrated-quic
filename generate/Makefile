all: site

ill-generator:
	go install github.com/syncsynchalt/ill-generator@latest
	cp ~/go/bin/ill-generator .

hkdf:
	git clone git@github.com:syncsynchalt/hkdf

ill12:
	git clone git@github.com:syncsynchalt/illustrated-tls ill12

ill13:
	git clone git@github.com:syncsynchalt/illustrated-tls13 ill13

site: ill-generator hkdf ill12 ill13
	mkdir -p ../site/
	/bin/echo -n '' > ../site/index.html
	for i in index-*.template; do \
		./ill-generator < $$i >> ../site/index.html || exit 1; \
	done
	./ill-generator < certificate.html.template > ../site/certificate.html
	mkdir -p ../site/files/
	cp -a ill13/archive/tools/aes_128_gcm_decrypt.c ../site/files/
	cp -a ../server/server.key ../server/server.crt ../site/files/
	cp -a hkdf/hkdf ../site/files/hkdf.sh
	cp -a ill12/site/files/curve25519-mult.c ../site/files/

clean:
	rm -rf ill-generator ill12 ill13 hkdf
