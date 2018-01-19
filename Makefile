# contrib/pg_udp/Makefile

MODULE_big = pg_lz4
OBJS = pg_lz4.o $(WIN32RES)

EXTENSION = pg_lz4
EXTVERSION = 1.0
DATA = $(EXTENSION)--$(EXTVERSION).sql
PGFILEDESC = "lz4 compression method"

REGRESS = pg_lz4

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/pg_lz4
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif

LDFLAGS += -llz4
