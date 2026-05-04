## Testing

Testing can be done in two different ways:

1. [pytest](https://docs.pytest.org/en/stable/)
2. [embedded_test](https://docs.rs/embedded-test/0.7.0/embedded_test/)

### Pytest

Pytest is a testing framework written in Python. It is useful for this project to use it with a VulCAN adapter to test the CAN system of the ERS.

### Embedded Test

embedded_test is a testing framework written by the people at probe-rs. It allows us to write tests that get ran on the boards themselves. To create a new test, it is recommended that you copy the `blinky.rs` file as it contains the necessary boilerplate code to get the tests running, and rename it to `<testname>.rs` where `<testname>` is the name of the test you are writing. Then, in the `Cargo.toml` file at the root of the directory, add this block of code:

```rust
[[test]]
name = "<testname>"
harness = false
```

It's important that you make sure all names are consistent, because `cargo test` will look for your test based on the name of the file and the name of the test in `Cargo.toml`.

To write a test, follow the structure of the `blinky` test. It must have the `#[test]` procedural macro at the top. If it uses async functionalities like `.await`, it must be an async function. The `init()` function will be called before any `#[test]` and it will initialize the stm32. It will pass in to any `#[test]` function the struct containing all the peripherals, so if you add `p: Peripherals` to your function signature, you will gain access to the peripherals through the `p` variable. More detailed information on the `embedded_test` crate is available through the link at the top of this README.

Then after you've written the test, run:

```sh
cargo test -r --test <testname>
```
