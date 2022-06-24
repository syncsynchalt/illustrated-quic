all: tools pkts unprot decrypt checksizes handshake

PACKETS=pkt01_cinit0 pkt02_sinit0 pkt03_shand0 pkt04_shand1 pkt05_cinit1 pkt06_chand0 pkt07_chand1 \
	pkt08_cdata0 pkt09_shand2 pkt10_sdata0 pkt11_cdata1 pkt12_sdata1
UNPROT_PACKETS=$(patsubst pkt%, pkt_unprot%, $(PACKETS))
DECRYPT_PACKETS=$(patsubst pkt%, pkt_payload%, $(PACKETS))
HANDSHAKE=crypto_clienthello crypto_serverhello crypto_extensions crypto_cert \
	crypto_certverify crypto_s_finished crypto_c_finished

# Must be version 1.1 or higher
OPENSSL_DIR=/usr/local/Cellar/openssl@3/3.0.2
pkts: $(PACKETS)
unprot: $(UNPROT_PACKETS)
decrypt: $(DECRYPT_PACKETS)
handshake: $(HANDSHAKE)
tools: aes_128_gcm_decrypt hp_removal

aes_128_gcm_decrypt.c:
	curl -sO https://tls13.xargs.org/archive/files/aes_128_gcm_decrypt.c

aes_128_gcm_decrypt: aes_128_gcm_decrypt.c
	cc -Wall -I $(OPENSSL_DIR)/include -o $@ $^ -L $(OPENSSL_DIR)/lib -lssl -lcrypto

hp_removal: hp_removal.c
	cc -Wall -I $(OPENSSL_DIR)/include -o $@ $^ -L $(OPENSSL_DIR)/lib -lssl -lcrypto

# key, iv, and header protection keys
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

C_DATA_KEY=e010a295f0c2864f186b2a7e8fdc9ed7
C_DATA_IV=eb3fbc384a3199dcf6b4c808
C_DATA_HP=8a6a38bc5cc40cb482a254dac68c9d2f
S_DATA_KEY=fd8c7da9de1b2da4d2ef9fd5188922d0
S_DATA_IV=02f6180e4f4aa456d7e8a602
S_DATA_HP=b7f6f021453e52b58940e4bba72a35d4

### pkts from datagrams

pkt01_cinit0: dgram01_c01
	START=0 LEN=1200; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

pkt02_sinit0: dgram02_s01
	START=0 LEN=137; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

pkt03_shand0: dgram02_s01
	START=137 LEN=1063; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

pkt04_shand1: dgram03_s02
	START=0 LEN=226; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

pkt05_cinit1: dgram04_c02
	START=0 LEN=43; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

pkt06_chand0: dgram04_c02
	START=43 LEN=1157; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

pkt07_chand1: dgram05_c03
	START=0 LEN=82; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

pkt08_cdata0: dgram05_c03
	START=82 LEN=32; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

pkt09_shand2: dgram06_s03
	START=0 LEN=41; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

pkt10_sdata0: dgram06_s03
	START=41 LEN=38; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

pkt11_cdata1: dgram07_c04
	START=0 LEN=28; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

pkt12_sdata1: dgram08_s04
	START=0 LEN=44; \
	perl -p0777 -e 's/.{'$$START'}(.{'$$LEN'}).*/\1/s' < $^ > $@

checksizes:
	$(eval SIZE1 = $(shell wc -c dgram??_* | tail -n 1 | awk '{print $$1}'))
	$(eval SIZE2 = $(shell wc -c pkt??_* | tail -n 1 | awk '{print $$1}'))
	@echo "size check $(SIZE1) vs $(SIZE2)"; \
	if test -z "$(SIZE1)" || test "$(SIZE1)" -ne "$(SIZE2)"; then echo failed; exit 1; fi
	@echo "passed"

### removal of pkt header protection

pkt_unprot01_cinit0: pkt01_cinit0
	./hp_removal $(C_INIT_HP) < $^ > $@

pkt_unprot02_sinit0: pkt02_sinit0
	./hp_removal $(S_INIT_HP) < $^ > $@

pkt_unprot03_shand0: pkt03_shand0
	./hp_removal $(S_HAND_HP) < $^ > $@

pkt_unprot04_shand1: pkt04_shand1
	./hp_removal $(S_HAND_HP) < $^ > $@

pkt_unprot05_cinit1: pkt05_cinit1
	./hp_removal $(C_INIT_HP) < $^ > $@

pkt_unprot06_chand0: pkt06_chand0
	./hp_removal $(C_HAND_HP) < $^ > $@

pkt_unprot07_chand1: pkt07_chand1
	./hp_removal $(C_HAND_HP) < $^ > $@

pkt_unprot08_cdata0: pkt08_cdata0
	./hp_removal $(C_DATA_HP) < $^ > $@

pkt_unprot09_shand2: pkt09_shand2
	./hp_removal $(S_HAND_HP) < $^ > $@

pkt_unprot10_sdata0: pkt10_sdata0
	./hp_removal $(S_DATA_HP) < $^ > $@

pkt_unprot11_cdata1: pkt11_cdata1
	./hp_removal $(C_DATA_HP) < $^ > $@

pkt_unprot12_sdata1: pkt12_sdata1
	./hp_removal $(S_DATA_HP) < $^ > $@

### decrypt the contents (usage: key, iv, hdrlen, recordnum, infile, outfile)

pkt_payload01_cinit0: pkt_unprot01_cinit0
	./decrypt.sh $(C_INIT_KEY) $(C_INIT_IV) 24 0 $^ $@

pkt_payload02_sinit0: pkt_unprot02_sinit0
	./decrypt.sh $(S_INIT_KEY) $(S_INIT_IV) 21 0 $^ $@

pkt_payload03_shand0: pkt_unprot03_shand0
	./decrypt.sh $(S_HAND_KEY) $(S_HAND_IV) 20 0 $^ $@

pkt_payload04_shand1: pkt_unprot04_shand1
	./decrypt.sh $(S_HAND_KEY) $(S_HAND_IV) 20 1 $^ $@

pkt_payload05_cinit1: pkt_unprot05_cinit1
	./decrypt.sh $(C_INIT_KEY) $(C_INIT_IV) 21 1 $^ $@

pkt_payload06_chand0: pkt_unprot06_chand0
	./decrypt.sh $(C_HAND_KEY) $(C_HAND_IV) 20 0 $^ $@

pkt_payload07_chand1: pkt_unprot07_chand1
	./decrypt.sh $(C_HAND_KEY) $(C_HAND_IV) 20 1 $^ $@

pkt_payload08_cdata0: pkt_unprot08_cdata0
	./decrypt.sh $(C_DATA_KEY) $(C_DATA_IV) 7 0 $^ $@

pkt_payload09_shand2: pkt_unprot09_shand2
	./decrypt.sh $(S_HAND_KEY) $(S_HAND_IV) 20 2 $^ $@

pkt_payload10_sdata0: pkt_unprot10_sdata0
	./decrypt.sh $(S_DATA_KEY) $(S_DATA_IV) 7 0 $^ $@

pkt_payload11_cdata1: pkt_unprot11_cdata1
	./decrypt.sh $(C_DATA_KEY) $(C_DATA_IV) 7 1 $^ $@

pkt_payload12_sdata1: pkt_unprot12_sdata1
	./decrypt.sh $(S_DATA_KEY) $(S_DATA_IV) 7 1 $^ $@

### pull out the frames

crypto_clienthello: pkt_payload01_cinit0
	START=4 LEN=238; perl -p0777 -e 's/.{'$${START}'}(.{'$${LEN}'}).*/\1/s' < $^ > $@

crypto_serverhello: pkt_payload02_sinit0
	START=10 LEN=90; perl -p0777 -e 's/.{'$${START}'}(.{'$${LEN}'}).*/\1/s' < $^ > $@

crypto_extensions: pkt_payload03_shand0
	START=4 LEN=90; perl -p0777 -e 's/.{'$${START}'}(.{'$${LEN}'}).*/\1/s' < $^ > $@

crypto_cert: pkt_payload03_shand0
	START=94 LEN=818; perl -p0777 -e 's/.{'$${START}'}(.{'$${LEN}'}).*/\1/s' < $^ > $@

crypto_certverify: pkt_payload03_shand0 pkt_payload04_shand1
	START=912 LEN=115; perl -p0777 -e 's/.{'$${START}'}(.{'$${LEN}'}).*/\1/s' < pkt_payload03_shand0 > $@
	START=5 LEN=149; perl -p0777 -e 's/.{'$${START}'}(.{'$${LEN}'}).*/\1/s' < pkt_payload04_shand1 >> $@

crypto_s_finished: pkt_payload04_shand1
	START=154 LEN=36; perl -p0777 -e 's/.{'$${START}'}(.{'$${LEN}'}).*/\1/s' < pkt_payload04_shand1 > $@

crypto_c_finished: pkt_payload07_chand1
	START=10 LEN=36; perl -p0777 -e 's/.{'$${START}'}(.{'$${LEN}'}).*/\1/s' < $^ > $@

clean:
	rm -f aes_128_gcm_decrypt aes_128_gcm_decrypt.c hp_removal
	rm -f $(PACKETS) $(UNPROT_PACKETS) $(DECRYPT_PACKETS) $(HANDSHAKE)

.PHONY: tools pkts unprot decrypt checksizes handshake clean
