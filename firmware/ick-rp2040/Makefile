# Fixed interface: host GCC bootstraps ICK; embedded tools supply assembly/runtime.
.DEFAULT_GOAL := all
ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
ICK_SOURCE ?=
ICK_BUILD ?= $(CURDIR)/build/ick-rp2040
ICK_STAGE ?= $(CURDIR)/build/ick-rp2040-stage
ICK_REVISION = c61e448251744a2f40ad743ebef1a027bdcd2f9d
GCC_REVISION = 6294f1d9e7536e5ffcde09d1528c918d63abfef5
COMPILER = $(ICK_STAGE)/bin/arm-none-eabi-gcc
ARM_AS ?= /usr/bin/arm-none-eabi-as
ARM_LD ?= /usr/bin/arm-none-eabi-ld
ARM_READELF ?= /usr/bin/arm-none-eabi-readelf
BUILD_JOBS ?= 2
.PHONY: all qualify outputs
all: $(COMPILER)
$(ICK_BUILD)/source/.materialized:
	test "$$(git -C "$(ICK_SOURCE)" rev-parse HEAD)" = "$(ICK_REVISION)"
	test "$$(git -C "$(ICK_SOURCE)/gcc" rev-parse HEAD)" = "$(GCC_REVISION)"
	mkdir -p "$(ICK_BUILD)"
	sh "$(ICK_SOURCE)/ick/materialize.sh" "$(ICK_SOURCE)/gcc" "$(ICK_BUILD)/source"
	touch "$@"
$(ICK_BUILD)/objects/Makefile: $(ICK_BUILD)/source/.materialized
	mkdir -p "$(ICK_BUILD)/objects"
	cd "$(ICK_BUILD)/objects" && CFLAGS='-O2 -g0' CXXFLAGS='-O2 -g0' "$(ICK_BUILD)/source/configure" --target=arm-none-eabi --prefix="$(ICK_STAGE)" --with-as="$(ARM_AS)" --with-ld="$(ARM_LD)" --enable-languages=c --disable-bootstrap --disable-multilib --disable-nls --disable-shared --disable-threads --without-headers --disable-libatomic --disable-libgomp --disable-libquadmath --disable-libssp --disable-libvtv --without-isl --without-zstd
$(COMPILER): $(ICK_BUILD)/objects/Makefile
	$(MAKE) -C "$(ICK_BUILD)/objects" -j$(BUILD_JOBS) all-gcc
	$(MAKE) -C "$(ICK_BUILD)/objects" install-gcc
qualify:
	test "$$(git -C "$(ICK_SOURCE)" rev-parse HEAD)" = "$(ICK_REVISION)"
	test "$$("$(COMPILER)" -dumpmachine)" = arm-none-eabi
	test -x "$$("$(COMPILER)" -print-prog-name=cc1)"
	mkdir -p "$(ICK_STAGE)/qualification"
	"$(COMPILER)" -mcpu=cortex-m0plus -mthumb -ffreestanding -std=c11 -O2 -g -Wall -Wextra -Werror -S "$(ROOT)qualify.c" -o "$(ICK_STAGE)/qualification/glyphs.s"
	grep -F '__aeabi_idiv' "$(ICK_STAGE)/qualification/glyphs.s"
	"$(ARM_AS)" -mcpu=cortex-m0plus -mthumb "$(ICK_STAGE)/qualification/glyphs.s" -o "$(ICK_STAGE)/qualification/glyphs.o"
	"$(ARM_READELF)" -A "$(ICK_STAGE)/qualification/glyphs.o" > "$(ICK_STAGE)/qualification/attributes.txt"
	grep -F 'Tag_CPU_arch: v6S-M' "$(ICK_STAGE)/qualification/attributes.txt"
	sha256sum "$(COMPILER)" "$$("$(COMPILER)" -print-prog-name=cc1)" > "$(ICK_STAGE)/qualification/compiler.sha256"
	printf 'ick_source\t%s\ngcc_reference\t%s\ntarget\tarm-none-eabi\ncortex_m0plus_glyph_and_macro_object\tPASS\nfirmware_execution\tNOT_RUN\n' "$(ICK_REVISION)" "$(GCC_REVISION)" > "$(ICK_STAGE)/qualification/receipt.tsv"
outputs:
	printf 'compiler=%s\n' "$(COMPILER)"
