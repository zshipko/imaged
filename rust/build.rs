fn main() {
    println!("cargo:rustc-link-search=native=..");
    println!("cargo:rustc-link-search=native=.");

    println!("cargo:rustc-link-lib=static=imaged");

    pkg_config::probe_library("babl").unwrap();
    pkg_config::probe_library("ezimage").unwrap();
    let _ = pkg_config::probe_library("libraw");
}
