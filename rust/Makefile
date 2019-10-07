build: bindings
	cargo build

bindings:
	bindgen \
		--whitelist-type "Image.*" \
		--rustified-enum "Image.*" \
		--whitelist-function "image.*" \
		--raw-line "#![allow(non_upper_case_globals)]" \
		--raw-line "#![allow(non_camel_case_types)]" \
		--raw-line "#![allow(non_snake_case)]" \
	../src/imaged.h  > src/ffi.rs
