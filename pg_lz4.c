#include "postgres.h"
#include "fmgr.h"
#include "lz4.h"
#include "access/cmapi.h"
#include "utils/datum.h"


PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1( lz4_handler );

static bytea *
lz4_compress(CompressionAmOptions *cmoptions, const bytea *value)
{
	int		src_len = (Size) VARSIZE_ANY_EXHDR(value);
	int		dst_len;
	int		len;
	bytea  *ret;

	dst_len = LZ4_compressBound(src_len);
	ret = (bytea *) palloc(dst_len + VARHDRSZ_CUSTOM_COMPRESSED);

	len = LZ4_compress_fast((char *) VARDATA_ANY(value),
							(char *) ret + VARHDRSZ_CUSTOM_COMPRESSED,
							src_len, dst_len, 1);

	if (len >= 0)
	{
		SET_VARSIZE_COMPRESSED(ret, len + VARHDRSZ_CUSTOM_COMPRESSED);
		return ret;
	}

	pfree(ret);
	return NULL;
}

static bytea *
lz4_decompress(CompressionAmOptions *cmoptions, const bytea *value)
{
	int		src_len = VARSIZE_ANY(value);
	int		dst_len = VARRAWSIZE_4B_C(value);
	int		len;
	bytea  *ret;

	ret = (bytea *) palloc(dst_len + VARHDRSZ);
	SET_VARSIZE(ret, dst_len + VARHDRSZ);
	len = LZ4_decompress_safe((char *) value + VARHDRSZ_CUSTOM_COMPRESSED,
							  (char *) VARDATA(ret),
							  src_len - VARHDRSZ_CUSTOM_COMPRESSED,
							  dst_len);

	if (len != dst_len)
		elog(ERROR, "Decompression error");

	return ret;
}

Datum
lz4_handler(PG_FUNCTION_ARGS)
{
	CompressionAmRoutine *routine = makeNode(CompressionAmRoutine);

	routine->cmcheck = NULL;
	routine->cmdrop = NULL;
	routine->cminitstate = NULL;
	routine->cmcompress = lz4_compress;
	routine->cmdecompress = lz4_decompress;

	PG_RETURN_POINTER(routine);
}
