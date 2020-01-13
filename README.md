# 3ds_gpio

Open source implementation of `gpio` system module.\
With intent of being a documentation supplement, but also working as replacement.\
Also in mind trying to make result binary as small as possible.

## Building

Just run `make`.\
It will create a cxi file, and you can extract `code.bin` and `exheader.bin` with `ctrtool`, or some other tool, to place it in `/luma/titles/0004013000001B02/`.\
This requires game patching to be enabled on luma config.

## License

This code itself is under Unlicense. Read `LICENSE.txt`\
The folders `source/3ds` and `include/3ds` are source files taken from [ctrulib](https://github.com/smealum/ctrulib), with modifications for the purpose of this program.\
Copy of ctrulib's license in `LICENSE.ctrulib.txt`

## Modifications to ctrulib

Ctrulib changed to generate smaller code, slimmed down sources and headers for a quicker read, and not depend on std libraries.\
As well some changes to behavior on result throw.
