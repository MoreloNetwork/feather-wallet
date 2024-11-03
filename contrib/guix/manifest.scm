(use-modules (gnu packages)
             (gnu packages autotools)
             (gnu packages bash)
             (gnu packages bison)
             ((gnu packages build-tools) #:select (meson))
             ((gnu packages certs) #:select (nss-certs))
             ((gnu packages cmake) #:select (cmake-minimal))
             (gnu packages commencement)
             (gnu packages compression)
             (gnu packages cross-base)
             (gnu packages elf)
             (gnu packages file)
             (gnu packages gawk)
             (gnu packages gcc)
             ((gnu packages gettext) #:select (gettext-minimal))
             (gnu packages gperf)
             ((gnu packages installers) #:select (nsis-x86_64))
             ((gnu packages libusb) #:select (libplist))
             ((gnu packages linux) #:select (linux-libre-headers-6.1 util-linux))
             (gnu packages llvm)
             (gnu packages mingw)
             (gnu packages moreutils)
             (gnu packages ninja)
             (gnu packages perl)
             (gnu packages pkg-config)
             ((gnu packages python) #:select (python-minimal))
             ((gnu packages tls) #:select (openssl))
             ((gnu packages version-control) #:select (git-minimal))
             (gnu packages xorg)
             (gnu packages zig)
             (guix build-system cmake)
             (guix build-system gnu)
             (guix build-system trivial)
             (guix download)
             (guix gexp)
             (guix git-download)
             ((guix licenses) #:prefix license:)
             (guix packages)
             ((guix utils) #:select (substitute-keyword-arguments)))

(define-syntax-rule (search-our-patches file-name ...)
  "Return the list of absolute file names corresponding to each
FILE-NAME found in ./patches relative to the current file."
  (parameterize
      ((%patch-path (list (string-append (dirname (current-filename)) "/patches"))))
    (list (search-patch file-name) ...)))

(define building-on (string-append "--build=" (list-ref (string-split (%current-system) #\-) 0) "-guix-linux-gnu"))

(define (make-cross-toolchain target
                              base-gcc-for-libc
                              base-kernel-headers
                              base-libc
                              base-gcc)
  "Create a cross-compilation toolchain package for TARGET"
  (let* ((xbinutils (cross-binutils target))
         ;; 1. Build a cross-compiling gcc without targeting any libc, derived
         ;; from BASE-GCC-FOR-LIBC
         (xgcc-sans-libc (cross-gcc target
                                    #:xgcc base-gcc-for-libc
                                    #:xbinutils xbinutils))
         ;; 2. Build cross-compiled kernel headers with XGCC-SANS-LIBC, derived
         ;; from BASE-KERNEL-HEADERS
         (xkernel (cross-kernel-headers target
                                        #:linux-headers base-kernel-headers
                                        #:xgcc xgcc-sans-libc
                                        #:xbinutils xbinutils))
         ;; 3. Build a cross-compiled libc with XGCC-SANS-LIBC and XKERNEL,
         ;; derived from BASE-LIBC
         (xlibc (cross-libc target
                            #:libc base-libc
                            #:xgcc xgcc-sans-libc
                            #:xbinutils xbinutils
                            #:xheaders xkernel))
         ;; 4. Build a cross-compiling gcc targeting XLIBC, derived from
         ;; BASE-GCC
         (xgcc (cross-gcc target
                          #:xgcc base-gcc
                          #:xbinutils xbinutils
                          #:libc xlibc)))
    ;; Define a meta-package that propagates the resulting XBINUTILS, XLIBC, and
    ;; XGCC
    (package
      (name (string-append target "-toolchain"))
      (version (package-version xgcc))
      (source #f)
      (build-system trivial-build-system)
      (arguments '(#:builder (begin (mkdir %output) #t)))
      (propagated-inputs
       `(("binutils" ,xbinutils)
         ("libc" ,xlibc)
         ("libc:static" ,xlibc "static")
         ("gcc" ,xgcc)
         ("gcc-lib" ,xgcc "lib")))
      (synopsis (string-append "Complete GCC tool chain for " target))
      (description (string-append "This package provides a complete GCC tool
chain for " target " development."))
      (home-page (package-home-page xgcc))
      (license (package-license xgcc)))))

(define base-gcc gcc-12)
(define base-linux-kernel-headers linux-libre-headers-6.1)

(define* (make-bitcoin-cross-toolchain target
                                       #:key
                                       (base-gcc-for-libc linux-base-gcc)
                                       (base-kernel-headers base-linux-kernel-headers)
                                       (base-libc glibc-2.31)
                                       (base-gcc linux-base-gcc))
  "Convenience wrapper around MAKE-CROSS-TOOLCHAIN with default values
desirable for building Feather Wallet release binaries."
  (make-cross-toolchain target
                        base-gcc-for-libc
                        base-kernel-headers
                        base-libc
                        base-gcc))

(define (gcc-mingw-patches gcc)
  (package-with-extra-patches gcc
    (search-our-patches "gcc-remap-guix-store.patch"
                        "vmov-alignment.patch")))

(define (winpthreads-patches mingw-w64-x86_64-winpthreads)
  (package-with-extra-patches mingw-w64-x86_64-winpthreads
    (search-our-patches "winpthreads-remap-guix-store.patch")))

(define (make-mingw-pthreads-cross-toolchain target)
  "Create a cross-compilation toolchain package for TARGET"
  (let* ((xbinutils (cross-binutils target))
         (machine (substring target 0 (string-index target #\-)))
         (pthreads-xlibc (winpthreads-patches (make-mingw-w64 machine
                                         #:xgcc (cross-gcc target #:xgcc (gcc-mingw-patches base-gcc))
                                         #:with-winpthreads? #t)))
         (pthreads-xgcc (cross-gcc target
                                    #:xgcc (gcc-mingw-patches mingw-w64-base-gcc)
                                    #:xbinutils xbinutils
                                    #:libc pthreads-xlibc)))
    ;; Define a meta-package that propagates the resulting XBINUTILS, XLIBC, and
    ;; XGCC
    (package
      (name (string-append target "-posix-toolchain"))
      (version (package-version pthreads-xgcc))
      (source #f)
      (build-system trivial-build-system)
      (arguments '(#:builder (begin (mkdir %output) #t)))
      (propagated-inputs
       `(("binutils" ,xbinutils)
         ("libc" ,pthreads-xlibc)
         ("gcc" ,pthreads-xgcc)
         ("gcc-lib" ,pthreads-xgcc "lib")))
      (synopsis (string-append "Complete GCC tool chain for " target))
      (description (string-append "This package provides a complete GCC tool
chain for " target " development."))
      (home-page (package-home-page pthreads-xgcc))
      (license (package-license pthreads-xgcc)))))

(define-public mingw-w64-base-gcc
  (package
    (inherit base-gcc)
    (arguments
      (substitute-keyword-arguments (package-arguments base-gcc)
        ((#:configure-flags flags)
          `(append ,flags
            ;; https://gcc.gnu.org/install/configure.html
            (list "--enable-threads=posix",
                  building-on)))))))

(define-public linux-base-gcc
  (package
    (inherit base-gcc)
    (arguments
      (substitute-keyword-arguments (package-arguments base-gcc)
        ((#:configure-flags flags)
          `(append ,flags
            ;; https://gcc.gnu.org/install/configure.html
            (list "--enable-initfini-array=yes",
                  "--enable-default-ssp=yes",
                  "--enable-default-pie=yes",
                  building-on)))
        ((#:phases phases)
          `(modify-phases ,phases
            ;; Given a XGCC package, return a modified package that replace each instance of
            ;; -rpath in the default system spec that's inserted by Guix with -rpath-link
            (add-after 'pre-configure 'replace-rpath-with-rpath-link
             (lambda _
               (substitute* (cons "gcc/config/rs6000/sysv4.h"
                                  (find-files "gcc/config"
                                              "^gnu-user.*\\.h$"))
                 (("-rpath=") "-rpath-link="))
               #t))))))))

(define-public glibc-2.31
  (let ((commit "8e30f03744837a85e33d84ccd34ed3abe30d37c3"))
  (package
    (inherit glibc) ;; 2.35
    (version "2.31")
    (source (origin
              (method git-fetch)
              (uri (git-reference
                    (url "https://sourceware.org/git/glibc.git")
                    (commit commit)))
              (file-name (git-file-name "glibc" commit))
              (sha256
               (base32
                "1zi0s9yy5zkisw823vivn7zlj8w6g9p3mm7lmlqiixcxdkz4dbn6"))
              (patches (search-our-patches "glibc-guix-prefix.patch"
                                           "glibc-2.31-riscv64-fix-incorrect-jal-with-HIDDEN_JUMPTARGET.patch"))))
    (arguments
      (substitute-keyword-arguments (package-arguments glibc)
        ((#:configure-flags flags)
          `(append ,flags
            ;; https://www.gnu.org/software/libc/manual/html_node/Configuring-and-compiling.html
            (list "--enable-stack-protector=all",
                  "--enable-bind-now",
                  "--disable-werror",
                  building-on)))
    ((#:phases phases)
        `(modify-phases ,phases
           (add-before 'configure 'set-etc-rpc-installation-directory
             (lambda* (#:key outputs #:allow-other-keys)
               ;; Install the rpc data base file under `$out/etc/rpc'.
               ;; Otherwise build will fail with "Permission denied."
               ;; Can be removed when we are building 2.32 or later.
               (let ((out (assoc-ref outputs "out")))
                 (substitute* "sunrpc/Makefile"
                   (("^\\$\\(inst_sysconfdir\\)/rpc(.*)$" _ suffix)
                    (string-append out "/etc/rpc" suffix "\n"))
                   (("^install-others =.*$")
                    (string-append "install-others = " out "/etc/rpc\n")))))))))))))

(define-public ldid
  (package
    (name "ldid")
    (version "v2.1.5-procursus7")
    (source (origin
              (method url-fetch)
              (uri (string-append "https://github.com/ProcursusTeam/"
                     name "/archive/refs/tags/" version ".tar.gz"))
              (sha256
                (base32
                  "0ppzy4d9sl4m0sn8nk8wpi39qfimvka6h2ycr67y8r97y3363r04"))))
    (build-system gnu-build-system)
    (arguments
      `(#:phases
         (modify-phases %standard-phases
           (delete 'configure)
           (replace 'build (lambda _ (invoke "make")))
           (delete 'check)
           (replace 'install
             (lambda* (#:key outputs #:allow-other-keys)
               (let* ((out (assoc-ref outputs "out"))
                       (bin (string-append out "/bin")))
                 (install-file "ldid" bin)
                 #t)))
           )))
    (native-inputs (list pkg-config))
    (inputs (list openssl libplist))
    (home-page "https://github.com/ProcursusTeam/ldid")
    (synopsis "Link Identity Editor.")
    (description "Put real or fake signatures in a Mach-O.")
    (license license:gpl3+)))

(define osslsigncode
  (package
    (name "osslsigncode")
    (version "2.5")
    (source (origin
              (method git-fetch)
              (uri (git-reference
                     (url "https://github.com/mtrojnar/osslsigncode")
                     (commit version)))
              (sha256
                (base32
                  "1j47vwq4caxfv0xw68kw5yh00qcpbd56d7rq6c483ma3y7s96yyz"))))
    (build-system cmake-build-system)
    (inputs (list openssl))
    (home-page "https://github.com/mtrojnar/osslsigncode")
    (synopsis "Authenticode signing and timestamping tool")
    (description "osslsigncode is a small tool that implements part of the
functionality of the Microsoft tool signtool.exe - more exactly the Authenticode
signing and timestamping. But osslsigncode is based on OpenSSL and cURL, and
thus should be able to compile on most platforms where these exist.")
    (license license:gpl3+))) ; license is with openssl exception

(packages->manifest
 (append
  (list ;; The Basics
        bash
        which
        coreutils-minimal
        util-linux
        ;; File(system) inspection
        file
        grep
        diffutils
        findutils
        ;; File transformation
        patch
        gawk
        sed
        moreutils
        patchelf
        ;; Compression and archiving
        tar
        bzip2
        gzip
        xz
        p7zip
        zip
        unzip
        ;; Build tools
        gnu-make
        libtool
        autoconf-2.71
        automake
        pkg-config
        bison
        gperf
        gettext-minimal
        squashfs-tools
        cmake-minimal
        meson
        ninja
        zig
        ;; Scripting
        perl
        python-minimal
        ;; Git
        git-minimal
        ;; Xcb
        xcb-util
        xcb-util-cursor
        xcb-util-image
        xcb-util-keysyms
        xcb-util-renderutil
        xcb-util-wm
    )
  (let ((target (getenv "HOST")))
    (cond ((string-suffix? "-mingw32" target)
           ;; Windows
           (list
             gcc-toolchain-12
             (list gcc-toolchain-12 "static")
             (make-mingw-pthreads-cross-toolchain "x86_64-w64-mingw32")
             nsis-x86_64
             nss-certs
             osslsigncode))
          ((string-contains target "-linux-")
           (list
             gcc-toolchain-12
             (list gcc-toolchain-12 "static")
             (make-bitcoin-cross-toolchain target)))
          ((string-contains target "darwin")
           (list
             gcc-toolchain-11
             (list gcc-toolchain-11 "static")
             ldid
             clang-toolchain-17
             lld-17
             (make-lld-wrapper lld-17 #:lld-as-ld? #t)))
          (else '())))))
