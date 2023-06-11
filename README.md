# clipboardExFixAsText

This Kirikiri (2/Z) plugin provides functionality to fix the `Clipboard.asText` behavior when there is already existing content.  

## Building

After cloning submodules and placing `ncbind` and `tp_stub` in the parent directory, a simple `make` will generate `clipboardExFixAsText.dll`.

## How to use

After `Plugins.link("clipboardExFixAsText.dll");` is used, the additional functions will be exposed under the `Clipboard` class.

## License

This project is licensed under the MIT license. Please read the `LICENSE` file for more information.  
