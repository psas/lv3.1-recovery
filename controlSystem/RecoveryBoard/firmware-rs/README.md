## General Information
This firmware is written in Rust, using [Embassy](https://embassy.dev/), a framework for embedded applications.

## Getting Started
Make sure you have Rust installed. Information about installing Rust can be found [here](https://www.rust-lang.org/learn/get-started).
Embassy requires Rust nightly, but `rust-toolchain.toml` should handle this for you.

You will also need to install the target platform:

```bash
rustup target add thumbv6m-none-eabi
```

Now that Rust should be set up properly, you need a tool to flash code onto the STM32.
Embassy uses [probe-rs](https://probe.rs/docs/getting-started/installation/) by default for this, so you will need to install it.

Note that some operating systems (such as pop_os) may not have the proper version of glibc installed to build probe-rs via the install scripts they provide.
In this case, you may need to build probe-rs from source.

With all the dependencies installed, you should now be able to navigate to the directory containing the Cargo.toml file and run:

```rust
cargo run --bin blinky --release
```

if you are flashing onto the Nucleo-F091RC development board.

If you are flashing blinky directly onto one of the ERS boards, you will need to add a feature flag. Enter this command instead: 

```rust
cargo run --bin blinky --features=main --release
```

After it builds, terminal output should look like this if everything went according to plan:

```bash
Finished `release` profile [optimized + debuginfo] target(s) in 0.21s
Running `probe-rs run --chip STM32F091RCTX target/thumbv6m-none-eabi/release/blinky`
Erasing ✔ 100% [####################]  16.00 KiB @  46.76 KiB/s (took 0s)
Programming ✔ 100% [####################]  15.00 KiB @  20.53 KiB/s (took 1s)                                                                                                                                           Finished in 1.18s
0.000000 [TRACE] BDCR ok: 00008218 (embassy_stm32 src/rcc/bd.rs:216)
0.000000 [DEBUG] rcc: Clocks { hclk1: MaybeHertz(8000000), hsi: MaybeHertz(8000000), hsi_div_244: MaybeHertz(32786), lse: MaybeHertz(0), pclk1: MaybeHertz(8000000), pclk1_tim: MaybeHertz(8000000), pclk2: MaybeHertz(8000000), pclk2_tim: MaybeHertz(8000000), rtc: MaybeHertz(40000), sys: MaybeHertz(8000000) } (embassy_stm32 src/rcc/mod.rs:71)
0.000030 [INFO ] Hello World! (blinky src/bin/blinky.rs:14)
0.000244 [INFO ] high (blinky src/bin/blinky.rs:19)
0.300537 [INFO ] low (blinky src/bin/blinky.rs:23)
```

and the led should be blinking.
