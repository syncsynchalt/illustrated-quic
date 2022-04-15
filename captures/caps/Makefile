all: tools packets unprot checksizes handshake

TARGETS=serverextensions servercert servercertverify serverfinished clientfinished \
	clientdata serverticket1 serverticket2 serverdata
PACKETS=p01_cinit0 p02_sinit0 p03_shand0 p04_shand1 p05_cinit1 p06_chand0 p07_chand1 \
	p08_cdata0 p09_shand2 p10_sdata0 p11_cdata1 p12_sdata1
UNPROT_PACKETS=$(patsubst p%, u%, $(PACKETS))
DECRYPT_PACKETS=$(patsubst p%, x%, $(PACKETS))
HANDSHAKE=crypto_clienthello crypto_serverhello

# Must be version 1.1 or higher
OPENSSL_DIR=/usr/local/Cellar/openssl@3/3.0.2
packets: $(PACKETS)
unprot: $(UNPROT_PACKETS)
decrypt: $(DECRYPT_PACKETS)
handshake: $(HANDSHAKE)
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

C_HAND_KEY=30a7e816f6a1e1b3434cf39cf4b415e7
C_HAND_IV=11e70a5d1361795d2bb04465
C_HAND_HP=84b3c21cacaf9f54c885e9a506459079
S_HAND_KEY=17abbf0a788f96c6986964660414e7ec
S_HAND_IV=09597a2ea3b04c00487e71f3
S_HAND_HP=2a18061c396c2828582b41b0910ed536

### packets from datagrams

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

### removal of packet header protection

u01_cinit0: p01_cinit0
	./hp_removal $(C_INIT_HP) < $^ > $@

u02_sinit0: p02_sinit0
	./hp_removal $(S_INIT_HP) < $^ > $@

u03_shand0: p03_shand0
	./hp_removal $(S_HAND_HP) < $^ > $@

u04_shand1: p04_shand1
	./hp_removal $(S_HAND_HP) < $^ > $@

u05_cinit1: p05_cinit1
	./hp_removal $(C_INIT_HP) < $^ > $@

u06_chand0: p06_chand0
	./hp_removal $(C_HAND_HP) < $^ > $@

u07_chand1: p07_chand1
	./hp_removal $(C_HAND_HP) < $^ > $@

u09_shand2: p09_shand2
	./hp_removal $(S_HAND_HP) < $^ > $@

### decrypt the contents

x01_cinit0: u01_cinit0
	# 24 bytes of header to skip, recordnum 0
	./decrypt.sh $(C_INIT_KEY) $(C_INIT_IV) 24 0 $^ $@

x02_sinit0: u02_sinit0
	# 21 bytes of header to skip, recordnum 0
	./decrypt.sh $(S_INIT_KEY) $(S_INIT_IV) 21 0 $^ $@

### pull out the frames

crypto_clienthello: x01_cinit0
	$(eval START = 0)
	$(eval LEN = 242)
	perl -p0777 -e 's/.{$(START)}(.{$(LEN)}).*/\1/s' < $^ > $@

crypto_serverhello: x02_sinit0
	$(eval START = 6)
	$(eval LEN = 94)
	perl -p0777 -e 's/.{$(START)}(.{$(LEN)}).*/\1/s' < $^ > $@

clean:
	rm -f aes_128_gcm_decrypt aes_128_gcm_decrypt.c hp_removal
	rm -f $(PACKETS) $(UNPROT_PACKETS) $(DECRYPT_PACKETS) $(HANDSHAKE)
