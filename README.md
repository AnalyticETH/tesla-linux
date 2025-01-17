# Analytic's Fork

## Disclaimer
All information is provided for educational purposes only. Build and run the kernel at your own risk. Neither the authors nor their employer are responsible for any direct or consequential damage or loss arising from any person or organization acting or failing to act on the basis of information contained in this repo. All information in this repo was discerned from open source knowledge, was provided directly to the author from Tesla, or was discovered independently.

## Extracting Kernel Config

To get kernel config from a running unit:
`zcat /proc/config.gz`

To get kernel config from an iasImage:
```
iasimage extract iasImage_bank_a
mv extract/image_1.bin ./bzImage
scripts/extract-ikconfig ./bzImage
```

In either case, you may need to patch out the initramfs config
as the iasImage does not use an initramfs and one is not available.

## Building

The .config here is for 4.14.274-PLK and also 
has kernel module signing enforcement disabled.

To build properly, one must compile with:
`make -j72 LOCALVERSION=`

The output file will be in `arch/x86/boot/bzImage`
Pre-built images may be available in /build on this repo.

# Normal Readme
============
============
============
Linux kernel
============

This file was moved to Documentation/admin-guide/README.rst

Please notice that there are several guides for kernel developers and users.
These guides can be rendered in a number of formats, like HTML and PDF.

In order to build the documentation, use ``make htmldocs`` or
``make pdfdocs``.

There are various text files in the Documentation/ subdirectory,
several of them using the Restructured Text markup notation.
See Documentation/00-INDEX for a list of what is contained in each file.

Please read the Documentation/process/changes.rst file, as it contains the
requirements for building and running the kernel, and information about
the problems which may result by upgrading your kernel.
