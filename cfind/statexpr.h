#include <stdbool.h>
#include <sys/stat.h>

typedef void *          STAT_EXPRESSION;

extern  STAT_EXPRESSION compile_stat_expression(const char *string);

extern  bool            evaluate_stat_expression(STAT_EXPRESSION stat_expr,
                                                 const char *filename,
                                                 const struct stat *buffer);

extern  void            free_stat_expression(STAT_EXPRESSION stat_expr);

//  vim:set sw=4 ts=8: 

