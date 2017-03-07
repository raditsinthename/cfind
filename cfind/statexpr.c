#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ctype.h>
#include <time.h>
#include <sys/param.h>
#include <regex.h>

#include "statexpr.h"

//  statexpr.c, written by Chris.McDonald@uwa.edu.au

#if	defined(__linux__)
extern	char	*strdup(const char *str);
#endif

//  THE SYMBOL AND INSTRUCTION TOKENS
typedef enum {
	T_AND, T_ANDAND, T_BAD, T_COLON,
	T_COMPL, T_DIV, T_END, T_EQ,
	T_GE, T_GLOB, T_GT, T_IF,
	T_INT, T_LE, T_LEFTB,
	T_LT, T_MINUS, T_MOD, T_MUL,
	T_NE, T_NOT, T_NOW, T_OR,
	T_OROR, T_PLUS, T_RIGHTB,
	T_SHIFTL, T_SHIFTR, T_UNIMINUS, T_XOR,

	T_gid, T_inode, T_mode, T_mtime,
	T_nlinks, T_size, T_uid
} TOKEN;

#define	SE_MAGIC	0xbeefcafe

typedef struct {
    int32_t	magic;
    int		PC;			// program counter
    int		SP;			// stack pointer
    int64_t	*program;	
    int64_t	*stack;	
    char	**strings;
    int		nstrings;
} SE;

#define CCHAR		'#'		// comment character

//  -----------------------------------------------------------------------

#define emit_instruction(se, instr)	se->program[se->PC++] = (instr)
#define emit_value(se, value)		se->program[se->PC++] = (value)

static void emit_string(SE *se, const char *str)
{
    se->strings	= realloc(se->strings, (se->nstrings+1)*sizeof(se->strings[0]));
    se->strings[se->nstrings]	= (char *)str;
    emit_value(se, se->nstrings);
    se->nstrings++;
}

static void FATAL(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

static bool valid_stat_expression(STAT_EXPRESSION stat_expr)
{
    SE	*se	= (SE *)stat_expr;

    return(se && se->magic == SE_MAGIC);
}

//  -----------------------------------------------------------------------

static int64_t execute0(SE *se, const char *filename, struct stat statbuf, int PC)
{
    char *strvalue	= se->strings[ se->program[PC+1] ];

    if(strvalue != NULL && stat(strvalue, &statbuf) != 0) {
	perror(strvalue);
	exit(EXIT_FAILURE);
    }

    switch ((int)se->program[PC]) {
	case T_gid :	return statbuf.st_gid;		break;
	case T_inode :	return statbuf.st_ino;		break;
	case T_mode :	return statbuf.st_mode;		break;
	case T_mtime :	return statbuf.st_mtime;	break;
	case T_nlinks :	return statbuf.st_nlink;	break;
	case T_size :	return statbuf.st_size;		break;
	case T_uid :	return statbuf.st_uid;		break;
    }
    return 0;
}

static bool execute(SE *se, const char *filename, const struct stat *statbuf)
{
    int64_t	*Prog	= se->program;
    int64_t	*sp;
    int		PC	= 0;
    int		SP	= 0;
    time_t	NOW	= time(NULL);

#define push(value)     se->stack[SP++] = value
    while(true) {

    sp = se->stack + SP;

    switch ((int)Prog[PC]) {

    case T_AND :	*(sp-2) &=  *(sp-1);		--SP;	break; 
    case T_ANDAND :	*(sp-2)  =  *(sp-2) && *(sp-1);	--SP;	break; 
    case T_COLON :	PC += Prog[PC+1];			break; 
    case T_COMPL :	*(sp-1)  = ~*(sp-1);			break;
    case T_DIV :	*(sp-2) /=  *(sp-1);		--SP;	break; 

    case T_END :	return(*(sp-1) != 0);			break;

    case T_EQ :		*(sp-2)  =  *(sp-2) == *(sp-1); --SP;	break; 
    case T_GE :		*(sp-2)  =  *(sp-2) >= *(sp-1); --SP;	break; 

    case T_GLOB : {
	    regex_t re;

	    regcomp(&re, se->strings[ se->program[PC+1]], REG_NOSUB);
	    push(regexec(&re, filename, 0, NULL, 0) == 0);
	    ++PC;
	    break;
    }
    case T_GT :		*(sp-2)  =  *(sp-2) >  *(sp-1); --SP;	break; 
    case T_IF :		PC += (*(sp-1)) ? 1 : Prog[PC+1]; --SP;	break; 
    case T_INT :	push(Prog[++PC]);			break;
    case T_LE :		*(sp-2)  =  *(sp-2) <= *(sp-1); --SP;	break; 
    case T_LT :		*(sp-2)  =  *(sp-2) <  *(sp-1); --SP;	break; 
    case T_MINUS :	*(sp-2) -=  *(sp-1);		--SP;	break; 
    case T_MOD :	*(sp-2) %=  *(sp-1);		--SP;	break; 
    case T_MUL :	*(sp-2) *=  *(sp-1);		--SP;	break; 
    case T_NE :		*(sp-2)  =  *(sp-2) != *(sp-1); --SP;	break; 
    case T_NOT :	*(sp-1)  = !*(sp-1);			break;
    case T_NOW :	push(NOW);				break;

    case T_OR :		*(sp-2) |=  *(sp-1);		--SP;	break; 
    case T_OROR :	*(sp-2)  =  *(sp-2) || *(sp-1);	--SP;	break; 
    case T_PLUS :	*(sp-2) +=  *(sp-1);		--SP;	break; 
    case T_SHIFTL :	*(sp-2)  =  *(sp-2) << *(sp-1);	--SP;	break; 
    case T_SHIFTR :	*(sp-2)  =  *(sp-2) >> *(sp-1);	--SP;	break; 
    case T_UNIMINUS :	*(sp-1)  = -*(sp-1);			break; 
    case T_XOR :	*(sp-2) ^=  *(sp-1);		--SP;	break; 

    case T_gid :
    case T_inode :
    case T_mode :
    case T_mtime :
    case T_nlinks :
    case T_size :
    case T_uid :
			push( execute0(se, filename, *statbuf, PC) );
			++PC;					break;

    default:		fprintf(stderr,
				"%s : bad instruction (%lli) in eval, PC=%i\n",
				__func__, (long long)Prog[PC], PC);
			exit(EXIT_FAILURE);
    }
    PC++;
    }
#undef	push

    return false;
}

//  -----------------------------------------------------------------------

static struct {
    char	*word;
    TOKEN	token;
} keywords[] = {
	{ "gid",	T_gid		},
	{ "inode",	T_inode		},
	{ "mode",	T_mode		},
	{ "mtime",	T_mtime		},
	{ "nlinks",	T_nlinks	},
	{ "size",	T_size		},
	{ "uid",	T_uid		}
};
#define NKEYWORDS	(sizeof(keywords) / sizeof(keywords[0]))

//  -----------------------------------------------------------------------

static	char	*line;
static	int	nextch;
static	TOKEN	token;
static	int64_t	value;
static	char	*strvalue;

static void get(void)
{
    nextch	= *line;
    if(*line != '\0')
	++line;
}

static void unget(void)
{
    if(*line != '\0')
	nextch = *--line;
}

static TOKEN gettoken(void)
{
    char chararray[BUFSIZ], *cp = chararray;

    get();

//  SKIP BLANKS
    while(nextch == ' ' || nextch == '\t' || nextch == CCHAR) {
	if(nextch == CCHAR)
	    return(token = T_END);
	get();
    }

    if(isalpha(nextch)) {
	while(isalpha(nextch)) {
	    *cp++ = nextch;
	    get();
	}
	*cp = '\0';
	unget();

	if(strcmp(chararray, "NOW") == 0) {
	    return(token = T_NOW);
	}

	for(int kw=0; kw < NKEYWORDS ; kw++) {		//  keyword?
	    int	i = strcmp(chararray,keywords[kw].word);

	    if(i == 0) {
		strvalue	= NULL;

		get();
		if(nextch == '(') {
		    get();
		    cp	= chararray;
		    while(nextch && nextch != ')') {
			*cp++	= nextch;
			get();
		    }
		    *cp = '\0';
		    if(nextch != ')')
			FATAL("right bracket expected");
		    strvalue	= strdup(chararray);
		}
		else
		    unget();
		return(token = keywords[kw].token);
	    }
	    else if(i<0)
		break;
	}
	return(token = T_BAD);
    }
    else if(isdigit(nextch)) {		// Collect digits of decimal or octal
	char	last = '9';
	int	base = 10;

	if(nextch == '0') {
	    last = '7'; base = 8;
	    get();
	}
	value = 0;
	while(nextch >= '0' && nextch <= last) {
	    value = value*base + nextch-'0';
	    get();
	}
	if(nextch == 'k' || nextch == 'K') {
	    value *= 1024;
	    get();
	}
	else if(nextch == 'm' || nextch == 'M') {
	    value *= (1024*1024);
	    get();
	}
	return(token = T_INT);
    }
    else if(nextch == '=') {
	get();
	if(nextch == '=')
	    return(token = T_EQ);
	else {	
	    unget(); return(token = T_BAD);
	}
    }
    else if(nextch == '&') {
	get();
	if(nextch == '&')
	    return(token = T_ANDAND);
	unget();
	return(token = T_AND);
    }
    else if(nextch == '|') {
	get();
	if(nextch == '|')
	    return(token = T_OROR);
	unget();
	return(token = T_OR);
    }

    else if(nextch == '"') {		// build a regexp from a glob pattern
	get();
	cp = chararray;
	*cp++ = '^';
	while(nextch != '\0' && nextch != '"') {
            switch (nextch) {
            case '.' :
            case '\\':
            case '$' : *cp++ = '\\'; *cp++ = nextch;	break;
            case '*' : *cp++ = '.';  *cp++ = nextch;	break;
            case '?' : *cp++ = '.';			break;
            default  : *cp++ = nextch;
            }
	    get();
	}
	if(nextch == '\0')
	    FATAL("incomplete filename pattern");
	*cp++ = '$';
	*cp   = '\0';
	strvalue = strdup(chararray);

	return(token = T_GLOB);
    }

    else if(nextch == '[') {
	extern  char	*parsedate(const char *, struct tm *);

	cp = chararray;
	get();
	while(nextch != '\0' && nextch != ']') {
	    *cp++ = nextch;
	    get();
	}
	*cp   = '\0';

	struct	tm tm;
	if(nextch == '\0' || parsedate(chararray, &tm) != NULL)
	    FATAL("incomplete date constant");

	value = (int)mktime(&tm);
	return(token = T_INT);
    }

    else if(nextch == '!') {
	get();
	if(nextch == '=')
	    return(token = T_NE);
	else {
	    unget(); return(token = T_NOT);
	}
    }
    else if(nextch == '<') {
	get();
	if(nextch == '<')
	    return(token = T_SHIFTL);
	else if(nextch == '=')
	    return(token = T_LE);
	else {
	    unget(); return(token = T_LT);
	}
    }
    else if(nextch == '>') {
	get();
	if(nextch == '=')
	    return(token = T_GE);
	else if(nextch == '>')
	    return(token = T_SHIFTR);
	else {
	    unget(); return(token = T_GT);
	}
    }

    switch (nextch) {
	case '%' : return(token = T_MOD);
	case '(' : return(token = T_LEFTB);
	case ')' : return(token = T_RIGHTB);
	case '*' : return(token = T_MUL);
	case '+' : return(token = T_PLUS);
	case '-' : return(token = T_MINUS);
	case ':' : return(token = T_COLON);
	case '?' : return(token = T_IF);
	case '^' : return(token = T_XOR);
	case '~' : return(token = T_COMPL);
	default  : return(token = ((nextch == '\0') ? T_END : T_BAD));
    }
}

//  -----------------------------------------------------------------------

#define expect(wanted, msg)	do {	\
				    if(token == wanted) gettoken(); \
				    else FATAL(msg); \
				} while(false)

static void expr00(SE *se);

static void expr12(SE *se)			// {integer|date}
{
    switch(token) {
	case T_NOW :	emit_instruction(se, T_NOW);
			gettoken();
			break;

	case T_INT :	emit_instruction(se, T_INT);
			emit_value(se, value);
			gettoken();
			break;

	case T_LEFTB :	gettoken();
			expr00(se);
			expect(T_RIGHTB, "right bracket expected");
			break;

	case T_gid:
	case T_inode:
	case T_mode:
	case T_mtime:
	case T_nlinks:
	case T_size:
	case T_uid:
			emit_instruction(se, token);
			emit_string(se, strvalue);
			gettoken();
			break;

	case T_END :    break;
	default:
			FATAL("syntax error");
    }
}


static void expr11(SE *se)			// {~|!|-} 12
{
    TOKEN	cptoken = token;

    if(token == T_NOT || token == T_COMPL || token == T_MINUS) {
	gettoken();
	expr11(se);
	emit_instruction(se, ((cptoken==T_MINUS)? T_UNIMINUS : cptoken));
    }
    else {
	expr12(se);
    }
}

static void expr10(SE *se)			// 11 {*|/|%} 11
{
    TOKEN	cptoken;

    expr11(se);
    while((cptoken = token) == T_MUL || token == T_DIV || token == T_MOD) {
	gettoken();
	expr11(se);
	emit_instruction(se, cptoken);
    }
}

static void expr09(SE *se)			// 10 {+|-} 10
{
    TOKEN	cptoken;

    expr10(se);
    while((cptoken = token) == T_PLUS || token == T_MINUS) {
	gettoken();
	expr10(se);
	emit_instruction(se, cptoken);
    }
}

static void expr08(SE *se)			// 09 {<<|>>} 09
{
    TOKEN	cptoken;

    expr09(se);
    while((cptoken = token) == T_SHIFTL || token == T_SHIFTR) {
	gettoken();
	expr09(se);
	emit_instruction(se, cptoken);
    }
}

static void expr07(SE *se)			// 08 {<|<=|>=|>} 08
{
    TOKEN	cptoken;

    expr08(se);
    while((cptoken = token)==T_LT || token==T_LE || token==T_GE || token==T_GT) {
	gettoken();
	expr08(se);
	emit_instruction(se, cptoken);
    }
}

static void expr06(SE *se)			// "pattern" | 07 {== | !=} 07
{
    TOKEN	cptoken;

    if(token == T_GLOB) {
	gettoken();
	emit_instruction(se, T_GLOB);
	emit_string(se, strvalue);		// regexp compiled at runtime
	return;
    }

    expr07(se);
    while((cptoken = token) == T_EQ || token == T_NE) {
	gettoken();
	expr07(se);
	emit_instruction(se, cptoken);
    }
}

static void expr05(SE *se)			// 06 & 06
{
    expr06(se);
    while(token == T_AND) {
	gettoken();
	expr06(se);
	emit_instruction(se, T_AND);
    }
}

static void expr04(SE *se)			// 05 ^ 05
{
    expr05(se);
    while(token == T_XOR) {
	gettoken();
	expr05(se);
	emit_instruction(se, T_XOR);
    }
}

static void expr03(SE *se)			// 04 | 04
{
    expr04(se);
    while(token == T_OR) {
	gettoken();
	expr04(se);
	emit_instruction(se, T_OR);
    }
}

static void expr02(SE *se)			// 03 && 03
{
    expr03(se);
    while(token == T_ANDAND) {
	gettoken();
	expr03(se);
	emit_instruction(se, T_ANDAND);
    }
}

static void expr01(SE *se)			// 02 || 02
{
    expr02(se);
    while(token == T_OROR) {
	gettoken();
	expr02(se);
	emit_instruction(se, T_OROR);
    }
}

static void expr00(SE *se)			// 01 ? 00 : 00
{
    expr01(se);
    if(token == T_IF) {
	int	offset_TRUE, offset_FALSE;

	gettoken();
	emit_instruction(se, T_IF);
	offset_TRUE 			= se->PC;
	emit_instruction(se, T_IF);		// reserve offset space
	expr00(se);				// 00(TRUE)
        expect(T_COLON, "colon expected");
	emit_instruction(se, T_COLON);
	offset_FALSE 			= se->PC;
	emit_instruction(se, T_COLON);		// reserve offset space
	se->program[offset_TRUE]	= se->PC - offset_TRUE;
	expr00(se);				// 00(FALSE)
	se->program[offset_FALSE]	= se->PC - offset_FALSE;
    }
}		

//  -----------------------------------------------------------------------

STAT_EXPRESSION	compile_stat_expression(const char *string)
{
    SE	*se	= calloc(1, sizeof(SE));

    if(se == NULL) {
	return NULL;
    }

    size_t max	= sizeof(se->program[0]) * 6 * strlen(string) + 1;

    se->program	= calloc(1, max);
    if(se->program == NULL) {
	free(se);
	return NULL;
    }

    se->stack	= calloc(1, max);
    if(se->stack == NULL) {
	free(se->program);
	free(se);
	return NULL;
    }
    se->magic	= SE_MAGIC;
    se->PC	= 0;
    se->SP	= 0;

    line	= (char *)string;
    gettoken();
    expr00(se);

    if(token != T_END) {
	FATAL("additional characters at end of expression");
    }
    emit_instruction(se, T_END);

    return(STAT_EXPRESSION)se;
}

bool evaluate_stat_expression(STAT_EXPRESSION stat_expr,
			      const char *filename,
                              const struct stat *buffer)
{
    if(valid_stat_expression(stat_expr) && filename != NULL && buffer != NULL) {
	SE *se	= (SE *)stat_expr;

	return execute(se, filename, buffer);
    }
    return false;
}

void free_stat_expression(STAT_EXPRESSION stat_expr)
{
    SE	*se	= (SE *)stat_expr;

    if(se != NULL) {
	if(se->program)
	    free(se->program);
	if(se->stack)
	    free(se->stack);
	if(se->strings)
	    free(se->strings);
	free(se);
    }
}

//  vim:set sw=4 ts=8: 

