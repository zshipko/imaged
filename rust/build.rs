fn main() {
    pkg_config::probe_library("babl").unwrap();
    println!("cargo:rustc-link-lib=static=imaged");
    println!("cargo:rustc-link-search=native=..");
    println!("cargo:rustc-link-search=native=.");
}
