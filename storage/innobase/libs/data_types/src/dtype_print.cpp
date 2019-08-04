#include <innodb/data_types/dtype_print.h>

#ifdef UNIV_DEBUG

#include <innodb/data_types/dtype_t.h>

/** Print a data type structure.
@param[in]	type	data type */
void dtype_print(const dtype_t *type) {
  ulint mtype;
  ulint prtype;
  ulint len;

  ut_a(type);

  mtype = type->mtype;
  prtype = type->prtype;

  switch (mtype) {
    case DATA_VARCHAR:
      fputs("DATA_VARCHAR", stderr);
      break;

    case DATA_CHAR:
      fputs("DATA_CHAR", stderr);
      break;

    case DATA_BINARY:
      fputs("DATA_BINARY", stderr);
      break;

    case DATA_FIXBINARY:
      fputs("DATA_FIXBINARY", stderr);
      break;

    case DATA_BLOB:
      fputs("DATA_BLOB", stderr);
      break;

    case DATA_POINT:
      fputs("DATA_POINT", stderr);
      break;

    case DATA_VAR_POINT:
      fputs("DATA_VAR_POINT", stderr);
      break;

    case DATA_GEOMETRY:
      fputs("DATA_GEOMETRY", stderr);
      break;

    case DATA_INT:
      fputs("DATA_INT", stderr);
      break;

    case DATA_MYSQL:
      fputs("DATA_MYSQL", stderr);
      break;

    case DATA_SYS:
      fputs("DATA_SYS", stderr);
      break;

    case DATA_FLOAT:
      fputs("DATA_FLOAT", stderr);
      break;

    case DATA_DOUBLE:
      fputs("DATA_DOUBLE", stderr);
      break;

    case DATA_DECIMAL:
      fputs("DATA_DECIMAL", stderr);
      break;

    case DATA_VARMYSQL:
      fputs("DATA_VARMYSQL", stderr);
      break;

    default:
      fprintf(stderr, "type %lu", (ulong)mtype);
      break;
  }

  len = type->len;

  if ((type->mtype == DATA_SYS) || (type->mtype == DATA_VARCHAR) ||
      (type->mtype == DATA_CHAR)) {
    putc(' ', stderr);
    if (prtype == DATA_ROW_ID) {
      fputs("DATA_ROW_ID", stderr);
      len = DATA_ROW_ID_LEN;
    } else if (prtype == DATA_ROLL_PTR) {
      fputs("DATA_ROLL_PTR", stderr);
      len = DATA_ROLL_PTR_LEN;
    } else if (prtype == DATA_TRX_ID) {
      fputs("DATA_TRX_ID", stderr);
      len = DATA_TRX_ID_LEN;
    } else if (prtype == DATA_ENGLISH) {
      fputs("DATA_ENGLISH", stderr);
    } else {
      fprintf(stderr, "prtype %lu", (ulong)prtype);
    }
  } else {
    if (prtype & DATA_UNSIGNED) {
      fputs(" DATA_UNSIGNED", stderr);
    }

    if (prtype & DATA_BINARY_TYPE) {
      fputs(" DATA_BINARY_TYPE", stderr);
    }

    if (prtype & DATA_NOT_NULL) {
      fputs(" DATA_NOT_NULL", stderr);
    }
  }

  fprintf(stderr, " len %lu", (ulong)len);
}


#endif /* UNIV_DEBUG */
