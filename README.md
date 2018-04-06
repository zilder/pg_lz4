# pg_lz4 extension

lz4 compression for PostgreSQL

## Requirements

This extension only works with developement version of PostgreSQL with custom compression methods patch applied:
https://www.postgresql.org/message-id/20180115024930.48583c69@hh

Also `liblz4` is required to build and use this extension.

## Installation

To install `pg_lz4` extension run:

```
make install
```

or if your PostgreSQL bin directory isn't in PATH:

```
make install PG_CONFIG=/path/to/your/postgres/bin/pg_config
```

## Usage

First, create pg_lz4 extension:

```
CREATE EXTENSION pg_lz4;
```

Now you can specify pg_lz4 compression when defining new tables:

```
CREATE TABLE mytable (
	id SERIAL,
	data1 TEXT COMPRESSION pg_lz4,
	data2 TEXT
);
```

or enable compression for existing table columns:

```
ALTER TABLE mytable ALTER COLUMN data2 SET COMPRESSION pg_lz4;
```

You can also specify acceleration parameter for lz4 algorithm (see [lz4 documentation](https://github.com/lz4/lz4) for more information):

```
ALTER TABLE mytable ALTER COLUMN data2 SET COMPRESSION pg_lz4 WITH (acceleration '8');
```
