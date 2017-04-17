TARGET=dbfs
TARGET_SRC_DIR=source
TARGET_DIR=output
TARGET_BIN_DIR=$(TARGET_DIR)/bin

DESTDIR ?= /

# Directory to store built objects
#
OBJDIR :=.obj

## Common packaging variables
DBFS_PACKAGE_VERSION=$(shell cat version.txt)

## Debian packaging variables
DEB_PACKAGE_NAME=$(TARGET)
DROP_DIRECTORY=$(TARGET_DIR)

# Set platform type. Don't use += operator as it will stick in extra spaces.
#
PLATFORM:=$(PLATFORM)$(shell grep -qi suse /etc/os-release && echo -n "suse")
PLATFORM:=$(PLATFORM)$(shell grep -qi rhel /etc/os-release && echo -n "rhel")

all: $(TARGET)
	$(AT)mkdir -p $(TARGET_BIN_DIR)
	$(AT)cp $(TARGET_SRC_DIR)/$(TARGET) $(TARGET_BIN_DIR)/
    
$(TARGET):
	$(AT)make --no-print-directory -C $(TARGET_SRC_DIR)

install: $(TARGET)
	$(AT)mkdir -p $(DESTDIR)/usr/bin/
	$(AT)mkdir -p $(DESTDIR)/opt/mssql-dbfs/
	$(AT)mkdir -p $(DESTDIR)/usr/share/doc/dbfs/

	$(AT)cp $(TARGET_SRC_DIR)/$(TARGET) $(DESTDIR)/opt/mssql-dbfs/
	$(AT)ln -sf /opt/mssql-dbfs/dbfs $(DESTDIR)/usr/bin/dbfs
	
	$(AT)cp common/LICENSE.TXT $(DESTDIR)/usr/share/doc/dbfs/
	$(AT)cp common/THIRDPARTYNOTICES.TXT $(DESTDIR)/usr/share/doc/dbfs
	
# Need to copy the libc++* libraries when making the RHEL package
#
ifeq ($(PLATFORM), rhel)
	$(AT)mkdir -p $(DESTDIR)/usr/lib64/
	$(AT)cp /usr/lib64/libc++abi.so.1.0 $(DESTDIR)/usr/lib64/
	$(AT)ln -sf /usr/lib64/libc++abi.so.1.0 $(DESTDIR)/usr/lib64/libc++abi.so
	$(AT)ln -sf /usr/lib64/libc++abi.so.1.0 $(DESTDIR)/usr/lib64/libc++abi.so.1

	$(AT)cp /usr/lib64/libc++.so.1.0 $(DESTDIR)/usr/lib64/
	$(AT)ln -sf /usr/lib64/libc++.so.1.0 $(DESTDIR)/usr/lib64/libc++.so
	$(AT)ln -sf /usr/lib64/libc++.so.1.0 $(DESTDIR)/usr/lib64/libc++.so.1
endif

clean:
	$(AT)rm -rf $(OBJDIR)
	$(AT)rm -rf $(TARGET_DIR)
	$(AT)make clean --no-print-directory -C $(TARGET_SRC_DIR)
	$(AT)rm -rf debian/dbfs
	$(AT)rm -rf debian/files

package-ubuntu:
	make clean
	make _package-ubuntu

_package-ubuntu:
	@echo [DEBUILD] Building Debian package

	$(AT)echo "$(DEB_PACKAGE_NAME) ($(dbfs_PACKAGE_VERSION)) stable; urgency=medium" > debian/changelog
	$(AT)echo "" >> debian/changelog
	$(AT)echo "  * dbfs v($(dbfs_PACKAGE_VERSION)) release" >> debian/changelog
	$(AT)echo "" >> debian/changelog
	$(AT)echo " -- Microsoft Data Platform Group <dpgswdist@microsoft.com>  $(shell date +'%a, %d %b %Y %H:%M:%S %z')" >> debian/changelog
	$(AT)echo "" >> debian/changelog

	$(AT)cp debian/control.tmpl debian/control
	
	$(AT)rm -f ../$(DEB_PACKAGE_NAME)_*_amd64.*
	$(AT)debuild $(LINTIAN) --preserve-env -b -us -uc 2>&1
	$(AT)mkdir -p $(TARGET_DIR)/packages/ubuntu

	$(AT)rsync -av ../$(DEB_PACKAGE_NAME)_$(dbfs_PACKAGE_VERSION)_amd64.* $(DROP_DIRECTORY)/packages/ubuntu
	
	$(AT)rm -f ../$(DEB_PACKAGE_NAME)_*_amd64.*

package-rhel7:
	make clean
	make _package-rhel7

_package-rhel7:
	@echo [RPM] Building RPM package

	$(AT)mkdir -p $(OBJDIR)/rhel7
	$(AT)mkdir -p $(DROP_DIRECTORY)/packages/rhel7

	$(AT) cp rhel7/dbfs.spec.tmpl $(OBJDIR)/rhel7/dbfs.spec
	$(AT)rpmbuild --define "_srcdir $$(pwd)" --define "_topdir $$(realpath $(OBJDIR)/rhel7/rpmbuild)" -bb $(OBJDIR)/rhel7/DBFS.spec
	$(AT)mv $(OBJDIR)/rhel7/rpmbuild/RPMS/x86_64/* $(DROP_DIRECTORY)/packages/rhel7
