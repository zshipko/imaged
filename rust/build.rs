fn main() {
    pkg_config::probe_library("babl").unwrap();
    pkg_config::probe_library("ezimage").unwrap();
    println!("cargo:rustc-link-lib=static=imaged");
    println!("cargo:rustc-link-search=native=..");
    println!("cargo:rustc-link-search=native=.");
}
