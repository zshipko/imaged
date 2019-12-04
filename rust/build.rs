fn main() {
    let make = std::env::var("MAKE").unwrap_or("make".to_string());

    std::process::Command::new(make)
        .current_dir("..")
        .output()
        .unwrap();

    println!("cargo:rustc-link-search=native=..");
    println!("cargo:rustc-link-search=native=.");

    println!("cargo:rustc-link-lib=static=imaged");
    pkg_config::probe_library("babl").unwrap();
    pkg_config::probe_library("ezimage").unwrap();
    let _ = pkg_config::probe_library("libraw");
}
