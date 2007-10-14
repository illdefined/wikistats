#ifndef COMMON_H
#define COMMON_H

#define DEF_DB "/var/db/wikistats/database"

#define USAGE "usage: %s [options]\n"
#define OPT_B "  -b num    Input buffer size\n"
#define OPT_C "  -c num    Cache buckets\n"
#define OPT_D "  -d path   Path to database\n"
#define OPT_E "  -e exp    Regular expression\n"
#define OPT_H "  -h        Issue this help\n"
#define OPT_M "  -m num    Lower limit\n"
#define OPT_N "  -n num    Database buckets\n"
#define OPT_V "  -v        Show version\n"

#define REQ_EXP  "You must specify an expression!\n"
#define REQ_NUM  "You must specify a number!\n"
#define REQ_PATH "You must specify a path!\n"

#define INV_OPT "Invalid command line option -%c!\n"
#define INV_ARG "Unknown command line argument %s!\n"

#define align(size, alignment) (((unsigned long int) (size) + (unsigned long int) (alignment) - 1ul) & ~((unsigned long int) (alignment) - 1ul))

#endif
