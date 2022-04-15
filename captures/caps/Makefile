all: tools packets unprot checksizes

TARGETS=serverextensions servercert servercertverify serverfinished clientfinished \
	clientdata serverticket1 serverticket2 serverdata
PACKETS=p01_cinit0 p02_sinit0 p03_shand0 p04_shand1 p05_cinit1 p06_chand0 p07_chand1 \
	p08_cdata0 p09_shand2 p10_sdata0 p11_cdata1 p12_sdata1
UNPROT_PACKETS=u01_cinit0 u02_sinit0

# Must be version 1.1 or higher
OPENSSL_DIR=/usr/local/Cellar/openssl@3/3.0.2
packets: $(PACKETS)
unprot: $(UNPROT_PACKETS)
tools: aes_128_gcm_decrypt hp_removal

aes_128_gcm_decrypt.c:
	curl -sO https://tls13.ulfheim.net/archive/files/aes_128_gcm_decrypt.c

aes_128_gcm_decrypt: aes_128_gcm_decrypt.c
	cc -Wall -I $(OPENSSL_DIR)/include -o $@ $^ -L $(OPENSSL_DIR)/lib -lssl -lcrypto

hp_removal: hp_removal.c
	cc -Wall -I $(OPENSSL_DIR)/include -o $@ $^ -L $(OPENSSL_DIR)/lib -lssl -lcrypto

# key, iv, and header protection key
C_INIT_KEY=b14b918124fda5c8d79847602fa3520b
C_INIT_IV=ddbc15dea80925a55686a7df
C_INIT_HP=6df4e9d737cdf714711d7c617ee82981
S_INIT_KEY=d77fc4056fcfa32bd1302469ee6ebf90
S_INIT_IV=fcb748e37ff79860faa07477
S_INIT_HP=440b2725e91dc79b370711ef792faa3d

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

u01_cinit0: p01_cinit0
	./hp_removal $(C_INIT_HP) < $^ > $@

u02_sinit0: p02_sinit0
	./hp_removal $(S_INIT_HP) < $^ > $@

clean:
	rm -f aes_128_gcm_decrypt aes_128_gcm_decrypt.c hp_removal
	rm -f $(PACKETS) $(UNPROT_PACKETS)
