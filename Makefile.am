ACLOCAL_AMFLAGS = -I m4
SUBDIRS = cmake include src packages

DISTCHECK_CONFIGURE_FLAGS = --with-tfel=$(TFELHOME)
if QEMACS_QT4
DISTCHECK_CONFIGURE_FLAGS += --enable-qt4
endif
if QEMACS_QT5
DISTCHECK_CONFIGURE_FLAGS += --enable-qt5
endif

EXTRA_DIST=CMakeLists.txt
