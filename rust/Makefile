build: bindings
	cargo build

bindings:
	@if hash bindgen 2>/dev/null; then \
	bindgen \
		--whitelist-type "Image.*" \
		--whitelist-type "Pixel.*" \
		--rustified-enum "Image.*" \
		--whitelist-function "image.*" \
		--whitelist-function "pixel.*" \
		--with-derive-partialeq \
		--with-derive-partialord \
		--raw-line "#![allow(non_upper_case_globals)]" \
		--raw-line "#![allow(non_camel_case_types)]" \
		--raw-line "#![allow(non_snake_case)]" \
	../src/imaged.h  > src/sys.rs; \
	else \
		echo "bindgen not installed, skipping"; \
    fi
