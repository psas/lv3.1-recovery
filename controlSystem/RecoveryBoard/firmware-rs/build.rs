use std::env;

fn main() {
    println!("cargo:rustc-link-arg-bins=--nmagic");
    println!("cargo:rustc-link-arg-bins=-Tlink.x");
    println!("cargo:rustc-link-arg-bins=-Tdefmt.x");
    println!("cargo::rustc-link-arg=-Tembedded-test.x");
    println!("cargo:rustc-link-arg-tests=-Tdefmt.x");
    println!("cargo:rustc-link-arg-tests=-Tlink.x");

    println!("cargo::rustc-check-cfg=cfg(drogue,main)");
    if let Ok(board) = env::var("BOARD") {
        if board == "drogue" {
            println!("cargo::rustc-cfg=drogue");
        }
        if board == "main" {
            println!("cargo::rustc-cfg=main");
        }
    }
}
