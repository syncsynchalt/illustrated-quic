all: tools packets checksizes

TARGETS=serverextensions servercert servercertverify serverfinished clientfinished \
	clientdata serverticket1 serverticket2 serverdata
PACKETS=p01_cinit0 p02_sinit0 p03_shand0 p04_shand1 p05_cinit1 p06_chand0 p07_chand1 \
	p08_cdata0 p09_shand2 p10_sdata0 p11_cdata1 p12_sdata1

# Must be version 1.1 or higher
OPENSSL_DIR=/usr/local/Cellar/openssl@3/3.0.2/
packets: $(PACKETS)
tools: aes_256_gcm_decrypt

aes_256_gcm_decrypt.c:
	curl -sO https://tls13.ulfheim.net/files/aes_256_gcm_decrypt.c

aes_256_gcm_decrypt: aes_256_gcm_decrypt.c
	cc -I $(OPENSSL_DIR)/include \
		-o aes_256_gcm_decrypt aes_256_gcm_decrypt.c \
		-L $(OPENSSL_DIR)/lib -lssl -lcrypto

# xxx TBD
S_HS_KEY=9f13575ce3f8cfc1df64a77ceaffe89700b492ad31b4fab01c4792be1b266b7f
S_HS_IV=9563bc8b590f671f488d2da3
C_HS_KEY=1135b4826a9a70257e5a391ad93093dfd7c4214812f493b3e3daae1eb2b1ac69
C_HS_IV=4256d2e0e88babdd05eb2f27

# xxx TBD
S_AP_KEY=01f78623f17e3edcc09e944027ba3218d57c8e0db93cd3ac419309274700ac27
S_AP_IV=196a750b0c5049c0cc51a541
C_AP_KEY=de2f4c7672723a692319873e5c227606691a32d1c59d8b9f51dbb9352e9ca9cc
C_AP_IV=bb007956f474b25de902432f

p01_cinit0: d01_c01
	START=0 LEN=1200; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

p02_sinit0: d02_s01
	START=0 LEN=137; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

p03_shand0: d02_s01
	START=137 LEN=1063; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

p04_shand1: d03_s02
	START=0 LEN=226; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

p05_cinit1: d04_c02
	START=0 LEN=43; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

p06_chand0: d04_c02
	START=43 LEN=1157; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

p07_chand1: d05_c03
	START=0 LEN=82; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

p08_cdata0: d05_c03
	START=82 LEN=32; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

p09_shand2: d06_s03
	START=0 LEN=41; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

p10_sdata0: d06_s03
	START=41 LEN=438; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

p11_cdata1: d07_c04
	START=0 LEN=28; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

p12_sdata1: d08_s04
	START=0 LEN=44; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

checksizes:
	$(eval SIZE1 = $(shell wc -c d??_* | tail -n 1 | awk '{print $$1}'))
	$(eval SIZE2 = $(shell wc -c p??_* | tail -n 1 | awk '{print $$1}'))
	@echo "size check $(SIZE1) vs $(SIZE2)"; \
	if test -z "$(SIZE1)" || test "$(SIZE1)" -ne "$(SIZE2)"; then echo failed; exit 1; fi
	@echo "passed"

clean:
	rm -f aes_256_gcm_decrypt aes_256_gcm_decrypt.c
	rm -f $(PACKETS)
