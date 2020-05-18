#[cfg(feature = "docs-rs")]
fn main() {}

#[cfg(not(feature = "docs-rs"))]
fn main() {
    let make = std::env::var("MAKE").unwrap_or_else(|_| "make".to_string());

    std::process::Command::new(make)
        .current_dir("..")
        .output()
        .unwrap();

    let dir = std::env::var("CARGO_MANIFEST_DIR").unwrap();
    println!("cargo:rustc-link-search=native={}/../../..", dir);
    println!("cargo:rustc-link-search=native=.");

    println!("cargo:rustc-link-lib=tiff");
    println!("cargo:rustc-link-lib=static=imaged");
    pkg_config::probe_library("babl").unwrap();
    let _ = pkg_config::probe_library("libraw");
}
