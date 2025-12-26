# Basic Downloader (using libcurl)

[![Build status](https://github.com/karnkaul/downloader/actions/workflows/ci.yml/badge.svg)](https://github.com/karnkaul/downloader/actions/workflows/ci.yml)

`downloader` is a simple wrapper over libcurl, specifically performing requests through easy handles.

## Usage

- Only build-from-source workflows are supported
- Use CMake and a preset / generator of your choice
  - Set `DOWNLOADER_VENDOR_CURL=OFF` if the `CURL::libcurl` CMake target is already in the build tree
  - Linking/loading libcurl dynamically is not supported
- Link to the `downloader::downloader-lib` CMake target
- Use the top-level `downloader::Curl` RAII wrapper unless libcurl is already initialized/shut-down elsewhere
- TLS/SSL support depends on libcurl defaults for the build environment and/or user customization

### Example

See the included [example](example/src/main.cpp).

### Requirements

- Desktop OS (`int main()` entrypoint)
- CMake 3.24+
- C++23 compiler (and stdlib)

## Contributing

Pull requests are welcome.

[**Original Repository**](https://github.com/karnkaul/downloader)
