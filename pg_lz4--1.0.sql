CREATE FUNCTION lz4_handler(INTERNAL)
RETURNS compression_am_handler
AS 'pg_lz4', 'lz4_handler'
LANGUAGE C STRICT;

CREATE ACCESS METHOD pg_lz4
TYPE COMPRESSION
HANDLER lz4_handler;
