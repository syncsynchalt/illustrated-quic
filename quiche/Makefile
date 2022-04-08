all: link

quiche/checkout.stamp:
	git clone --recursive --branch 0.12.0 https://github.com/cloudflare/quiche
	# cd quiche && patch -p1 < ../instruments.patch
	touch quiche/checkout.stamp

quiche/build.stamp: quiche/checkout.stamp
	cd quiche && cargo build --features ffi
	touch quiche/build.stamp

link: quiche/build.stamp
	rm -f libquiche.a include
	ln -sf ./quiche/quiche/include ./include
	ln -sf ./quiche/target/debug/deps/libquiche.a libquiche.a

clean:
	rm -rf quiche include libquiche.a

.PHONY: all link clean
