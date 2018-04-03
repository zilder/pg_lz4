MODULE_big = pg_lz4
OBJS = pg_lz4.o $(WIN32RES)

EXTENSION = pg_lz4
EXTVERSION = 1.0
DATA = $(EXTENSION)--$(EXTVERSION).sql
PGFILEDESC = "lz4 compression method"

REGRESS = pg_lz4

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

LDFLAGS += -llz4
