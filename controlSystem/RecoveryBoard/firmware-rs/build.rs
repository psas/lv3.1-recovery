fn main() {
    println!("cargo:rustc-link-arg-bins=--nmagic");
    println!("cargo:rustc-link-arg-bins=-Tlink.x");
    println!("cargo:rustc-link-arg-bins=-Tdefmt.x");
    println!("cargo::rustc-link-arg=-Tembedded-test.x");
    println!("cargo:rustc-link-arg-tests=-Tdefmt.x");
    println!("cargo:rustc-link-arg-tests=-Tlink.x");
}
