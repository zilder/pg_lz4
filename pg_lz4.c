#include "postgres.h"
#include "fmgr.h"
#include "lz4.h"
#include "access/cmapi.h"
#include "commands/defrem.h"
#include "nodes/parsenodes.h"
#include "utils/builtins.h"
#include "utils/datum.h"


PG_MODULE_MAGIC;

typedef struct
{
	int	acceleration;
} lz4_option;

PG_FUNCTION_INFO_V1( lz4_handler );

static void
lz4_check(Form_pg_attribute att, List *options)
{
	ListCell   *lc;

	foreach (lc, options)
	{
		DefElem    *def = (DefElem *) lfirst(lc);

		if (strcmp(def->defname, "acceleration") == 0)
		{
			int accel = pg_atoi(defGetString(def), 4, 0);

			if (accel < 1)
				elog(WARNING, "Acceleration value <= 0 will be replaced by 1 (default)");
		}
		else
			elog(ERROR, "Unknown option '%s'", def->defname);
	}
}

static void *
lz4_initstate(Oid acoid, List *options)
{
	ListCell   *lc;
	lz4_option *opt = (lz4_option *) palloc(sizeof(lz4_option));

	/* default acceleration */
	opt->acceleration = 1;

	/* iterate through user options */
	foreach (lc, options)
	{
		DefElem    *def = (DefElem *) lfirst(lc);

		if (strcmp(def->defname, "acceleration") == 0)
			opt->acceleration = pg_atoi(defGetString(def), 4, 0);
		else
			elog(ERROR, "Unknown option '%s'", def->defname);
	}

	return (void *) opt;
}

static bytea *
lz4_compress(CompressionAmOptions *cmoptions, const bytea *value)
{
	lz4_option *opt = (lz4_option *) cmoptions->acstate;
	int		src_len = (Size) VARSIZE_ANY_EXHDR(value);
	int		dst_len;
	int		len;
	bytea  *ret;

	dst_len = LZ4_compressBound(src_len);
	ret = (bytea *) palloc(dst_len + VARHDRSZ_CUSTOM_COMPRESSED);

	len = LZ4_compress_fast((char *) VARDATA_ANY(value),
							(char *) ret + VARHDRSZ_CUSTOM_COMPRESSED,
							src_len, dst_len,
							opt->acceleration);

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

	routine->cmcheck = lz4_check;
	routine->cmdrop = NULL;
	routine->cminitstate = lz4_initstate;
	routine->cmcompress = lz4_compress;
	routine->cmdecompress = lz4_decompress;

	PG_RETURN_POINTER(routine);
}
