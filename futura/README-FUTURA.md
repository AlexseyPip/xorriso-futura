# GNU xorriso 1.5.8 — FuturaOS port

Build from the FuturaOS project root:

    make -f third_party/xorriso/Makefile.futura

The target installs:

    base/usr/bin/xorriso

The port is intentionally built as ET_EXEC and uses `/lib/ld.so`, matching the current FuturaOS ELF loader.

Disabled/omitted optional host integrations:
- Linux/FreeBSD/Solaris/NetBSD MMC/SCSI backends; FuturaOS uses the dummy backend.
- libreadline/libedit.
- libacl/xattr.
- zlib/libjte.

The bundled Futura compatibility layer supplies the small OS gaps needed by the standalone build (POSIX regex, libburn CRC routines, FIFO fallback, and the unused libdax abort hook).
