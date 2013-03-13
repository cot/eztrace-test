include Makefile.config

#
# Modify this if you want to build patches yourself
#

PATCH_PREFIX=$(KERN_VERSION)

SYS_PATCH=$(PATCH_PREFIX)/sys.diff
FS_PATCH=$(PATCH_PREFIX)/fs.diff
NET_PATCH=$(PATCH_PREFIX)/net.diff
NETDRIVER_3C59X_PATCH=$(PATCH_PREFIX)/netdriver-3c59x.diff
NETDRIVER_ACENIC_PATCH=$(PATCH_PREFIX)/netdriver-acenic.diff
NETDRIVER_TULIP_PATCH=$(PATCH_PREFIX)/netdriver-tulip.diff
SCSIGEN_PATCH=$(PATCH_PREFIX)/scsigen.diff
FKT_PATCH=$(PATCH_PREFIX)/fkt.diff

APPLY_PATCHES_SCRIPT=apply_patches

PATCHES=$(FKT_PATCH)\
	$(SYS_PATCH)\
	$(FS_PATCH)\
	$(NET_PATCH)\
	$(NETDRIVER_3C59X_PATCH)\
	$(NETDRIVER_ACENIC_PATCH)\
	$(NETDRIVER_TULIP_PATCH)\
	$(SCSIGEN_PATCH)

FKT_KERN_SOURCE=$(shell find linux-fkt -type f ! -path "*/SCCS/*" 2>/dev/null)

LINUX_PATCHES=linux-patches-$(KERN_VERSION)

# some help
help:
	@echo Targets:
	@echo
	@echo "  patches		will make the patches for the different install"
	@echo "			targets below"
	@echo
	@echo "  install-fkt		will install the fkt driver files in directory"
	@echo "			\"$(INSTALL_LINUX_SOURCE)\""
	@echo
	@echo "  install-sys		will patch the kernel to add helper functions and links"
	@echo "			to fkt."
	@echo
	@echo "  install-fs"
	@echo "  install-net"
	@echo "  install-netdriver-3c59x"
	@echo "  install-netdriver-acenic"
	@echo "  install-netdriver-tulip"
	@echo "  install-scsigen	will patch the kernel to add probes to the relevant part"

# System patch, mandatory to get fkt working

$(FKT_PATCH): $(FKT_KERN_SOURCE)
	@echo making $@
	@mkdir -p `dirname $@`
	@mkdir linux
	@diff -urN linux linux-fkt > $@ || /bin/true
	@rmdir linux

$(SYS_PATCH): \
	$(shell find \
		$(LINUX_PATCHES)/arch \
		$(LINUX_PATCHES)/include/linux \
		$(LINUX_PATCHES)/include/asm-i386 \
		$(LINUX_PATCHES)/kernel \
		$(LINUX_PATCHES)/mm \
		$(LINUX_PATCHES)/fs/buffer.c.patch \
		$(LINUX_PATCHES)/fs/exec.c.patch \
		$(LINUX_PATCHES)/fs/read_write-sys.c.patch \
		! -path $(LINUX_PATCHES)/kernel/softirq.c.patch \
		! -path "*/SCCS/*" \
	-type f 2>/dev/null | sort )

# File System patch

$(FS_PATCH): \
	$(shell find \
		$(LINUX_PATCHES)/fs \
		$(LINUX_PATCHES)/drivers/block \
		! -path $(LINUX_PATCHES)/fs/buffer.c.patch \
		! -path $(LINUX_PATCHES)/fs/exec.c.patch \
		! -path $(LINUX_PATCHES)/fs/read_write-sys.c.patch \
		! -path "*/SCCS/*" \
		-type f 2>/dev/null)

# TCP/IP patch

$(NET_PATCH): \
	$(shell find \
		$(LINUX_PATCHES)/net \
		$(LINUX_PATCHES)/kernel/softirq.c.patch \
		$(LINUX_PATCHES)/include/net \
		! -path "*/SCCS/*" \
		-type f 2>/dev/null)

# some network drivers patch

$(NETDRIVER_3C59X_PATCH): $(LINUX_PATCHES)/drivers/net/3c59x.c.patch

$(NETDRIVER_ACENIC_PATCH): $(LINUX_PATCHES)/drivers/net/acenic.c.patch

$(NETDRIVER_TULIP_PATCH): \
	$(shell find \
		$(LINUX_PATCHES)/drivers/net/tulip \
		! -path "*/SCCS/*" \
		-type f 2>/dev/null)

# scsi generic layer patch

$(SCSIGEN_PATCH): \
	$(shell find \
		$(LINUX_PATCHES)/drivers/scsi \
		! -path "*/SCCS/*" \
		-type f 2>/dev/null)

patches: $(PATCHES)

#
# these are the real rules for all .diff targets
#

install-%: $(PATCH_PREFIX)/%.diff
	@echo applying $<
	@(cd $(INSTALL_LINUX_SOURCE) && patch -p1) < $<

%.diff:
	@echo making $@
	@mkdir -p `dirname $@`
	@cat /dev/null $^ > $@

#
# time to clean...
#

clean:
	rm -fr $(KERN_VERSION)
