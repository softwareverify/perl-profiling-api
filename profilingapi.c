/*    profilingapi.c
 *
 *    Copyright (C) 2011 by Stephen Kellett, Software Verification Ltd, (www.softwareverify.com)
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/* This file contains the functions that implement the profiling API
   to enable third party tools (open source and commercial) to easily 
   instrument a Perl application.
 */

#include "EXTERN.h"
#define PERL_IN_DUMP_C
#include "perl.h"
#include "regcomp.h"
#include "proto.h"
#include "profilingapi.h"

/*
 type names array, useful for debugging what is passing through the profiling function
 compiler/linker will remove from final image if not referenced, so best to leave them
 present in the source to stop someone from having to type them in again in future.
 */

static const char *op_type_names[] = 
{
	"OP_NULL",
	"OP_STUB",
	"OP_SCALAR",
	"OP_PUSHMARK",
	"OP_WANTARRAY",
	"OP_CONST",
	"OP_GVSV",
	"OP_GV",
	"OP_GELEM",
	"OP_PADSV",
	"OP_PADAV",
	"OP_PADHV",
	"OP_PADANY",
	"OP_PUSHRE",
	"OP_RV2GV",
	"OP_RV2SV",
	"OP_AV2ARYLEN",
	"OP_RV2CV",
	"OP_ANONCODE",
	"OP_PROTOTYPE",
	"OP_REFGEN",
	"OP_SREFGEN",
	"OP_REF",
	"OP_BLESS",
	"OP_BACKTICK",
	"OP_GLOB",
	"OP_READLINE",
	"OP_RCATLINE",
	"OP_REGCMAYBE",
	"OP_REGCRESET",
	"OP_REGCOMP",
	"OP_MATCH",
	"OP_QR",
	"OP_SUBST",
	"OP_SUBSTCONT",
	"OP_TRANS",
	"OP_SASSIGN",
	"OP_AASSIGN",
	"OP_CHOP",
	"OP_SCHOP",
	"OP_CHOMP",
	"OP_SCHOMP",
	"OP_DEFINED",
	"OP_UNDEF",
	"OP_STUDY",
	"OP_POS",
	"OP_PREINC",
	"OP_I_PREINC",
	"OP_PREDEC",
	"OP_I_PREDEC",
	"OP_POSTINC",
	"OP_I_POSTINC",
	"OP_POSTDEC",
	"OP_I_POSTDEC",
	"OP_POW",
	"OP_MULTIPLY",
	"OP_I_MULTIPLY",
	"OP_DIVIDE",
	"OP_I_DIVIDE",
	"OP_MODULO",
	"OP_I_MODULO",
	"OP_REPEAT",
	"OP_ADD",
	"OP_I_ADD",
	"OP_SUBTRACT",
	"OP_I_SUBTRACT",
	"OP_CONCAT",
	"OP_STRINGIFY",
	"OP_LEFT_SHIFT",
	"OP_RIGHT_SHIFT",
	"OP_LT",
	"OP_I_LT",
	"OP_GT",
	"OP_I_GT",
	"OP_LE",
	"OP_I_LE",
	"OP_GE",
	"OP_I_GE",
	"OP_EQ",
	"OP_I_EQ",
	"OP_NE",
	"OP_I_NE",
	"OP_NCMP",
	"OP_I_NCMP",
	"OP_SLT",
	"OP_SGT",
	"OP_SLE",
	"OP_SGE",
	"OP_SEQ",
	"OP_SNE",
	"OP_SCMP",
	"OP_BIT_AND",
	"OP_BIT_XOR",
	"OP_BIT_OR",
	"OP_NEGATE",
	"OP_I_NEGATE",
	"OP_NOT",
	"OP_COMPLEMENT",
	"OP_SMARTMATCH",
	"OP_ATAN2",
	"OP_SIN",
	"OP_COS",
	"OP_RAND",
	"OP_SRAND",
	"OP_EXP",
	"OP_LOG",
	"OP_SQRT",
	"OP_INT",
	"OP_HEX",
	"OP_OCT",
	"OP_ABS",
	"OP_LENGTH",
	"OP_SUBSTR",
	"OP_VEC",
	"OP_INDEX",
	"OP_RINDEX",
	"OP_SPRINTF",
	"OP_FORMLINE",
	"OP_ORD",
	"OP_CHR",
	"OP_CRYPT",
	"OP_UCFIRST",
	"OP_LCFIRST",
	"OP_UC",
	"OP_LC",
	"OP_QUOTEMETA",
	"OP_RV2AV",
	"OP_AELEMFAST",
	"OP_AELEM",
	"OP_ASLICE",
	"OP_AEACH",
	"OP_AKEYS",
	"OP_AVALUES",
	"OP_EACH",
	"OP_VALUES",
	"OP_KEYS",
	"OP_DELETE",
	"OP_EXISTS",
	"OP_RV2HV",
	"OP_HELEM",
	"OP_HSLICE",
	"OP_BOOLKEYS",
	"OP_UNPACK",
	"OP_PACK",
	"OP_SPLIT",
	"OP_JOIN",
	"OP_LIST",
	"OP_LSLICE",
	"OP_ANONLIST",
	"OP_ANONHASH",
	"OP_SPLICE",
	"OP_PUSH",
	"OP_POP",
	"OP_SHIFT",
	"OP_UNSHIFT",
	"OP_SORT",
	"OP_REVERSE",
	"OP_GREPSTART",
	"OP_GREPWHILE",
	"OP_MAPSTART",
	"OP_MAPWHILE",
	"OP_RANGE",
	"OP_FLIP",
	"OP_FLOP",
	"OP_AND",
	"OP_OR",
	"OP_XOR",
	"OP_DOR",
	"OP_COND_EXPR",
	"OP_ANDASSIGN",
	"OP_ORASSIGN",
	"OP_DORASSIGN",
	"OP_METHOD",
	"OP_ENTERSUB",
	"OP_LEAVESUB",
	"OP_LEAVESUBLV",
	"OP_CALLER",
	"OP_WARN",
	"OP_DIE",
	"OP_RESET",
	"OP_LINESEQ",
	"OP_NEXTSTATE",
	"OP_DBSTATE",
	"OP_UNSTACK",
	"OP_ENTER",
	"OP_LEAVE",
	"OP_SCOPE",
	"OP_ENTERITER",
	"OP_ITER",
	"OP_ENTERLOOP",
	"OP_LEAVELOOP",
	"OP_RETURN",
	"OP_LAST",
	"OP_NEXT",
	"OP_REDO",
	"OP_DUMP",
	"OP_GOTO",
	"OP_EXIT",
	"OP_METHOD_NAMED",
	"OP_ENTERGIVEN",
	"OP_LEAVEGIVEN",
	"OP_ENTERWHEN",
	"OP_LEAVEWHEN",
	"OP_BREAK",
	"OP_CONTINUE",
	"OP_OPEN",
	"OP_CLOSE",
	"OP_PIPE_OP",
	"OP_FILENO",
	"OP_UMASK",
	"OP_BINMODE",
	"OP_TIE",
	"OP_UNTIE",
	"OP_TIED",
	"OP_DBMOPEN",
	"OP_DBMCLOSE",
	"OP_SSELECT",
	"OP_SELECT",
	"OP_GETC",
	"OP_READ",
	"OP_ENTERWRITE",
	"OP_LEAVEWRITE",
	"OP_PRTF",
	"OP_PRINT",
	"OP_SAY",
	"OP_SYSOPEN",
	"OP_SYSSEEK",
	"OP_SYSREAD",
	"OP_SYSWRITE",
	"OP_EOF",
	"OP_TELL",
	"OP_SEEK",
	"OP_TRUNCATE",
	"OP_FCNTL",
	"OP_IOCTL",
	"OP_FLOCK",
	"OP_SEND",
	"OP_RECV",
	"OP_SOCKET",
	"OP_SOCKPAIR",
	"OP_BIND",
	"OP_CONNECT",
	"OP_LISTEN",
	"OP_ACCEPT",
	"OP_SHUTDOWN",
	"OP_GSOCKOPT",
	"OP_SSOCKOPT",
	"OP_GETSOCKNAME",
	"OP_GETPEERNAME",
	"OP_LSTAT",
	"OP_STAT",
	"OP_FTRREAD",
	"OP_FTRWRITE",
	"OP_FTREXEC",
	"OP_FTEREAD",
	"OP_FTEWRITE",
	"OP_FTEEXEC",
	"OP_FTIS",
	"OP_FTSIZE",
	"OP_FTMTIME",
	"OP_FTATIME",
	"OP_FTCTIME",
	"OP_FTROWNED",
	"OP_FTEOWNED",
	"OP_FTZERO",
	"OP_FTSOCK",
	"OP_FTCHR",
	"OP_FTBLK",
	"OP_FTFILE",
	"OP_FTDIR",
	"OP_FTPIPE",
	"OP_FTSUID",
	"OP_FTSGID",
	"OP_FTSVTX",
	"OP_FTLINK",
	"OP_FTTTY",
	"OP_FTTEXT",
	"OP_FTBINARY",
	"OP_CHDIR",
	"OP_CHOWN",
	"OP_CHROOT",
	"OP_UNLINK",
	"OP_CHMOD",
	"OP_UTIME",
	"OP_RENAME",
	"OP_LINK",
	"OP_SYMLINK",
	"OP_READLINK",
	"OP_MKDIR",
	"OP_RMDIR",
	"OP_OPEN_DIR",
	"OP_READDIR",
	"OP_TELLDIR",
	"OP_SEEKDIR",
	"OP_REWINDDIR",
	"OP_CLOSEDIR",
	"OP_FORK",
	"OP_WAIT",
	"OP_WAITPID",
	"OP_SYSTEM",
	"OP_EXEC",
	"OP_KILL",
	"OP_GETPPID",
	"OP_GETPGRP",
	"OP_SETPGRP",
	"OP_GETPRIORITY",
	"OP_SETPRIORITY",
	"OP_TIME",
	"OP_TMS",
	"OP_LOCALTIME",
	"OP_GMTIME",
	"OP_ALARM",
	"OP_SLEEP",
	"OP_SHMGET",
	"OP_SHMCTL",
	"OP_SHMREAD",
	"OP_SHMWRITE",
	"OP_MSGGET",
	"OP_MSGCTL",
	"OP_MSGSND",
	"OP_MSGRCV",
	"OP_SEMOP",
	"OP_SEMGET",
	"OP_SEMCTL",
	"OP_REQUIRE",
	"OP_DOFILE",
	"OP_HINTSEVAL",
	"OP_ENTEREVAL",
	"OP_LEAVEEVAL",
	"OP_ENTERTRY",
	"OP_LEAVETRY",
	"OP_GHBYNAME",
	"OP_GHBYADDR",
	"OP_GHOSTENT",
	"OP_GNBYNAME",
	"OP_GNBYADDR",
	"OP_GNETENT",
	"OP_GPBYNAME",
	"OP_GPBYNUMBER",
	"OP_GPROTOENT",
	"OP_GSBYNAME",
	"OP_GSBYPORT",
	"OP_GSERVENT",
	"OP_SHOSTENT",
	"OP_SNETENT",
	"OP_SPROTOENT",
	"OP_SSERVENT",
	"OP_EHOSTENT",
	"OP_ENETENT",
	"OP_EPROTOENT",
	"OP_ESERVENT",
	"OP_GPWNAM",
	"OP_GPWUID",
	"OP_GPWENT",
	"OP_SPWENT",
	"OP_EPWENT",
	"OP_GGRNAM",
	"OP_GGRGID",
	"OP_GGRENT",
	"OP_SGRENT",
	"OP_EGRENT",
	"OP_GETLOGIN",
	"OP_SYSCALL",
	"OP_LOCK",
	"OP_ONCE",
	"OP_CUSTOM",
};

static int isTargetToIgnore(DWORD	opCode)
{
	switch(opCode)
	{
	case OP_PUSHRE:
	case OP_PREINC:	// Base.pm 74
	case OP_UNDEF:	// Tk.pm 86
	case OP_REFGEN:	// tainting

	case OP_ATAN2:	/* 98 */
	case OP_SIN:		/* 99 */
	case OP_COS:		/* 100 */
	case OP_RAND:	/* 101 */
	case OP_SRAND:	/* 102 */
	case OP_EXP:		/* 103 */
	case OP_LOG:		/* 104 */
	case OP_SQRT:	/* 105 */
	case OP_HEX:
	case OP_INT:
	case OP_OCT:		/* 108 */
	case OP_ABS:		/* 109 */
	case OP_LENGTH:	/* 110 */
	case OP_SUBSTR:	/* 111 */
	case OP_VEC:		/* 112 */
	case OP_INDEX:	/* 113 */
	case OP_RINDEX:	/* 114 */
	case OP_SPRINTF:	/* 115 */
	case OP_FORMLINE:	/* 116 */
	case OP_ORD:		/* 117 */
	case OP_CHR:		/* 118 */
	case OP_CRYPT:	/* 119 */

	case OP_METHOD:	/* 165 */
	case OP_SORT:	/* 150 */
	case OP_REVERSE:	/* 151 */
	case OP_GREPSTART:	/* 152 */
	case OP_GREPWHILE:	/* 153 */
	case OP_MAPSTART:	/* 154 */
	case OP_MAPWHILE:	/* 155 */
	case OP_RANGE:	/* 156 */

	case OP_ENTERITER:
	case OP_ITER:
	case OP_ENTERLOOP:	/* 182 */
	case OP_LEAVELOOP:	/* 183 */
	case OP_OPEN:	/* 191 */
	case OP_CLOSE:	/* 192 */
	case OP_PIPE_OP:	/* 193 */
	case OP_FILENO:	/* 194 */
	case OP_UMASK:	/* 195 */
	case OP_BINMODE:	/* 196 */
	case OP_TIE:		/* 197 */
	case OP_UNTIE:	/* 198 */
	case OP_TIED:	/* 199 */
	case OP_DBMOPEN:	/* 200 */
	case OP_DBMCLOSE:	/* 201 */
	case OP_SSELECT:	/* 202 */
	case OP_SELECT:	/* 203 */
	case OP_GETC:	/* 204 */
	case OP_READ:	/* 205 */
	case OP_ENTERWRITE:	/* 206 */
	case OP_LEAVEWRITE:	/* 207 */
	case OP_PRTF:	/* 208 */
	case OP_PRINT:	/* 209 */
	case OP_SYSOPEN:	/* 210 */
	case OP_SYSSEEK:	/* 211 */
	case OP_SYSREAD:	/* 212 */
	case OP_SYSWRITE:	/* 213 */
	case OP_SEND:	/* 214 */
	case OP_RECV:	/* 215 */
	case OP_EOF:		/* 216 */
	case OP_TELL:	/* 217 */
	case OP_SEEK:	/* 218 */
	case OP_TRUNCATE:	/* 219 */
	case OP_FCNTL:	/* 220 */
	case OP_IOCTL:	/* 221 */
	case OP_FLOCK:	/* 222 */
	case OP_SOCKET:	/* 223 */
	case OP_SOCKPAIR:	/* 224 */
	case OP_BIND:	/* 225 */
	case OP_CONNECT:	/* 226 */
	case OP_LISTEN:	/* 227 */
	case OP_ACCEPT:	/* 228 */
	case OP_SHUTDOWN:	/* 229 */
	case OP_GSOCKOPT:	/* 230 */
	case OP_SSOCKOPT:	/* 231 */
	case OP_GETSOCKNAME:	/* 232 */
	case OP_GETPEERNAME:	/* 233 */
	case OP_LSTAT:	/* 234 */
	case OP_STAT:	/* 235 */
	case OP_FTRREAD:	/* 236 */
	case OP_FTRWRITE:	/* 237 */
	case OP_FTREXEC:	/* 238 */
	case OP_FTEREAD:	/* 239 */
	case OP_FTEWRITE:	/* 240 */
	case OP_FTEEXEC:	/* 241 */
	case OP_FTIS:	/* 242 */
	case OP_FTEOWNED:	/* 243 */
	case OP_FTROWNED:	/* 244 */
	case OP_FTZERO:	/* 245 */
	case OP_FTSIZE:	/* 246 */
	case OP_FTMTIME:	/* 247 */
	case OP_FTATIME:	/* 248 */
	case OP_FTCTIME:	/* 249 */
	case OP_FTSOCK:	/* 250 */
	case OP_FTCHR:	/* 251 */
	case OP_FTBLK:	/* 252 */
	case OP_FTFILE:	/* 253 */
	case OP_FTDIR:	/* 254 */
	case OP_FTPIPE:	/* 255 */
	case OP_FTLINK:	/* 256 */
	case OP_FTSUID:	/* 257 */
	case OP_FTSGID:	/* 258 */
	case OP_FTSVTX:	/* 259 */
	case OP_FTTTY:	/* 260 */
	case OP_FTTEXT:	/* 261 */
	case OP_FTBINARY:	/* 262 */
	case OP_CHDIR:	/* 263 */
	case OP_CHOWN:	/* 264 */
	case OP_CHROOT:	/* 265 */
	case OP_UNLINK:	/* 266 */
	case OP_CHMOD:	/* 267 */
	case OP_UTIME:	/* 268 */
	case OP_RENAME:	/* 269 */
	case OP_LINK:	/* 270 */
	case OP_SYMLINK:	/* 271 */
	case OP_READLINK:	/* 272 */
	case OP_MKDIR:	/* 273 */
	case OP_RMDIR:	/* 274 */
	case OP_OPEN_DIR:	/* 275 */
	case OP_READDIR:	/* 276 */
	case OP_TELLDIR:	/* 277 */
	case OP_SEEKDIR:	/* 278 */
	case OP_REWINDDIR:	/* 279 */
	case OP_CLOSEDIR:	/* 280 */
	case OP_FORK:	/* 281 */
	case OP_WAIT:	/* 282 */
	case OP_WAITPID:	/* 283 */
	case OP_SYSTEM:	/* 284 */
	case OP_EXEC:	/* 285 */
	case OP_KILL:	/* 286 */
	case OP_GETPPID:	/* 287 */
	case OP_GETPGRP:	/* 288 */
	case OP_SETPGRP:	/* 289 */
	case OP_GETPRIORITY:	/* 290 */
	case OP_SETPRIORITY:	/* 291 */
	case OP_TIME:	/* 292 */
	case OP_TMS:		/* 293 */
	case OP_LOCALTIME:	/* 294 */
	case OP_GMTIME:	/* 295 */
	case OP_ALARM:	/* 296 */
	case OP_SLEEP:	/* 297 */
	case OP_SHMGET:	/* 298 */
	case OP_SHMCTL:	/* 299 */
	case OP_SHMREAD:	/* 300 */
	case OP_SHMWRITE:	/* 301 */
	case OP_MSGGET:	/* 302 */
	case OP_MSGCTL:	/* 303 */
	case OP_MSGSND:	/* 304 */
	case OP_MSGRCV:	/* 305 */
	case OP_SEMGET:	/* 306 */
	case OP_SEMCTL:	/* 307 */
	case OP_SEMOP:	/* 308 */
	case OP_REQUIRE:	/* 309 */
	case OP_DOFILE:	/* 310 */
	case OP_GHBYNAME:	/* 315 */
	case OP_GHBYADDR:	/* 316 */
	case OP_GHOSTENT:	/* 317 */
	case OP_GNBYNAME:	/* 318 */
	case OP_GNBYADDR:	/* 319 */
	case OP_GNETENT:	/* 320 */
	case OP_GPBYNAME:	/* 321 */
	case OP_GPBYNUMBER:	/* 322 */
	case OP_GPROTOENT:	/* 323 */
	case OP_GSBYNAME:	/* 324 */
	case OP_GSBYPORT:	/* 325 */
	case OP_GSERVENT:	/* 326 */
	case OP_SHOSTENT:	/* 327 */
	case OP_SNETENT:	/* 328 */
	case OP_SPROTOENT:	/* 329 */
	case OP_SSERVENT:	/* 330 */
	case OP_EHOSTENT:	/* 331 */
	case OP_ENETENT:	/* 332 */
	case OP_EPROTOENT:	/* 333 */
	case OP_ESERVENT:	/* 334 */
	case OP_GPWNAM:	/* 335 */
	case OP_GPWUID:	/* 336 */
	case OP_GPWENT:	/* 337 */
	case OP_SPWENT:	/* 338 */
	case OP_EPWENT:	/* 339 */
	case OP_GGRNAM:	/* 340 */
	case OP_GGRGID:	/* 341 */
	case OP_GGRENT:	/* 342 */
	case OP_SGRENT:	/* 343 */
	case OP_EGRENT:	/* 344 */
	case OP_GETLOGIN:	/* 345 */
	case OP_SYSCALL:
		return TRUE;

	default:
		return FALSE;
	}
}

/*
 control functions to easily turn profiling on/off
 */

static int	enableProfiling = FALSE;

EXT
void Perl_set_do_profiling(int	enable)
{
	enableProfiling = enable;
}

EXT
int Perl_get_do_profiling()
{
	return enableProfiling;
}

/*
 control function to set the coverage callback data
 */

static PERL_PROFILER_CALLBACK	coverageCallback = NULL;
static void						*coverageUserData = NULL;

EXT
void Perl_set_coverage_callback(PERL_PROFILER_CALLBACK	callback,
								void					*userData)
{
	coverageCallback = callback;
	coverageUserData = userData;
}

/*
 control function to set the profiling callback data
 */

static PERL_PROFILER_CALLBACK	profilingCallback = NULL;
static void						*profilingUserData = NULL;

EXT
void Perl_set_profiling_callback(PERL_PROFILER_CALLBACK	callback,
								 void						*userData)
{
	profilingCallback = callback;
	profilingUserData = userData;
}

/*
 The function that does the work of turning interpreter activity 
 into actionable coverage/profiling events

 The function assumes that currentOP and my_perl will be valid when passed into the function.
 */

int doProfilingCallback(struct	op		*currentOP, 
						PerlInterpreter	*my_perl)
{
	/* some previous information we need to help determine things */
	/* this will need to be stored per-thread for multi-threaded apps or per-interpreter for multi-interpreter apps */

	static const char			*prevFileName = NULL;
	static long					prevLine = 0;
	static PERL_PROFILER_EVENT	prevPPE = PPE_NONE;
	static PERL_PROFILER_EVENT	deferredPPE = PPE_NONE;

    const char					*file;
    const char					*fileName;
	char						*functionName;
	char						*packageName;
    long						line;
	PERL_PROFILER_EVENT			ppe;
	STRLEN						len = 0;

	/* don't waste time calculating stuff if this has been enabled by mistake with no callbacks set */

	if (profilingCallback == NULL && 
		coverageCallback == NULL)
		return FALSE;

	/* calculate information to give to the callback */

    file = my_perl->Icurcop ? OutCopFILE(my_perl->Icurcop) : "<null>";
    fileName = file ? file : "<free>";
    line = my_perl->Icurcop ? (long)CopLINE(my_perl->Icurcop) : 0;
	functionName = SvPV(my_perl->Isubname, len);
	packageName = SvPV(my_perl->Icurstname, len);

	/* some debugging output */
	/*
	{
		printf("%s %d: (F:%s) (P:%s) (%s) (ignore : %s)\r\n", fileName, line, functionName, packageName,
												op_type_names[currentOP->op_type], 
												isTargetToIgnore(currentOP->op_type) ? "yes" : "no");
	}
	*/

	{
		/* works, but produces information for values other than what we are looking for
		   List the parameters for the current executing perl instruction on the stack (not the current function) 

		SV	**svp;
		int	px = 1;

		for(svp = my_perl->Istack_sp; svp > my_perl->Istack_base; svp--, px++)
		{
			char	*str;

			str = SvPV(*svp, len);
			printf("\tparam %d: %s\r\n", px, str);
		}
		*/

		/* produces no output
		if (my_perl->Icompcv != NULL)
		{
			int	depth;
			SV	**names = AvARRAY(my_perl->Icomppad_name);

			depth = CvDEPTH(my_perl->Icompcv);
			for(px = 0; px < depth; px++)
			{
				char	*name = "";
				char	*value = "";

				//if (Icomppad_name[px] != NULL)
				//	name = SvPV(my_perl->Icomppad_name[px], len);
				if (names[px] != NULL)
					name = SvPV(names[px], len);

				if (my_perl->Icomppad != NULL)
				{
					SV	*sv;

					sv = *av_fetch(my_perl->Icomppad, px, TRUE);
					if (sv != NULL)
						value = SvPV(sv, len);
				}

				printf("\tvariable %d: %s => %s\r\n", px + 1, name, value);
			}
		}
		*/

		/* does not work, crashes
		if (my_perl->Itmps_stack != NULL)
		{
			for(px = 0; px > my_perl->Itmps_ix; px++)
			{
				if (my_perl->Itmps_stack[px] != NULL)
				{
					char	*str;

					str = SvPV(my_perl->Itmps_stack[px], len);
					printf("\ttemp %d: %s\r\n", px + 1, str);
				}
			}
		}
		*/
	}

	/* determine which type of event we have at this point */
	/* start of assuming its a line and set depending on what we find */

	ppe = PPE_LINE;

	switch (currentOP->op_type) 
	{
	case OP_NULL:
		ppe = PPE_IGNORE;
		break;

	case OP_STUB:
		ppe = PPE_IGNORE;
		break;

	case OP_SCALAR:
	case OP_PUSHMARK:
	case OP_WANTARRAY:
		break;

	case OP_CONST:
		ppe = PPE_IGNORE;
		break;

	case OP_GVSV:
	case OP_GV:
	case OP_GELEM:
	case OP_PADSV:
	case OP_PADAV:
	case OP_PADHV:
	case OP_PADANY:
	case OP_PUSHRE:
	case OP_RV2GV:
	case OP_RV2SV:
	case OP_AV2ARYLEN:
	case OP_RV2CV:
	case OP_ANONCODE:
	case OP_PROTOTYPE:
	case OP_REFGEN:
	case OP_SREFGEN:
	case OP_REF:
	case OP_BLESS:
	case OP_BACKTICK:
	case OP_GLOB:
	case OP_READLINE:
	case OP_RCATLINE:
	case OP_REGCMAYBE:
	case OP_REGCRESET:
	case OP_REGCOMP:
	case OP_MATCH:
	case OP_QR:
	case OP_SUBST:
	case OP_SUBSTCONT:
	case OP_TRANS:
	case OP_SASSIGN:
	case OP_AASSIGN:
	case OP_CHOP:
	case OP_SCHOP:
	case OP_CHOMP:
	case OP_SCHOMP:
	case OP_DEFINED:
	case OP_UNDEF:
	case OP_STUDY:
	case OP_POS:
	case OP_PREINC:
	case OP_I_PREINC:
	case OP_PREDEC:
	case OP_I_PREDEC:
	case OP_POSTINC:
	case OP_I_POSTINC:
	case OP_POSTDEC:
	case OP_I_POSTDEC:
	case OP_POW:
	case OP_MULTIPLY:
	case OP_I_MULTIPLY:
	case OP_DIVIDE:
	case OP_I_DIVIDE:
	case OP_MODULO:
	case OP_I_MODULO:
	case OP_REPEAT:
	case OP_ADD:
	case OP_I_ADD:
	case OP_SUBTRACT:
	case OP_I_SUBTRACT:
	case OP_CONCAT:
		break;

	case OP_STRINGIFY:
		ppe = PPE_IGNORE;
		break;

	case OP_LEFT_SHIFT:
	case OP_RIGHT_SHIFT:
	case OP_LT:
	case OP_I_LT:
	case OP_GT:
	case OP_I_GT:
	case OP_LE:
	case OP_I_LE:
	case OP_GE:
	case OP_I_GE:
	case OP_EQ:
	case OP_I_EQ:
	case OP_NE:
	case OP_I_NE:
	case OP_NCMP:
	case OP_I_NCMP:
	case OP_SLT:
	case OP_SGT:
	case OP_SLE:
	case OP_SGE:
	case OP_SEQ:
	case OP_SNE:
	case OP_SCMP:
	case OP_BIT_AND:
	case OP_BIT_XOR:
	case OP_BIT_OR:
	case OP_NEGATE:
	case OP_I_NEGATE:
	case OP_NOT:
	case OP_COMPLEMENT:
	case OP_SMARTMATCH:
	case OP_ATAN2:
	case OP_SIN:
	case OP_COS:
	case OP_RAND:
	case OP_SRAND:
	case OP_EXP:
	case OP_LOG:
	case OP_SQRT:
	case OP_INT:
	case OP_HEX:
	case OP_OCT:
	case OP_ABS:
	case OP_LENGTH:
	case OP_SUBSTR:
	case OP_VEC:
	case OP_INDEX:
	case OP_RINDEX:
	case OP_SPRINTF:
	case OP_FORMLINE:
	case OP_ORD:
	case OP_CHR:
	case OP_CRYPT:
	case OP_UCFIRST:
	case OP_LCFIRST:
	case OP_UC:
	case OP_LC:
	case OP_QUOTEMETA:
	case OP_RV2AV:
	case OP_AELEMFAST:
	case OP_AELEM:
	case OP_ASLICE:
	case OP_AEACH:
	case OP_AKEYS:
	case OP_AVALUES:
	case OP_EACH:
	case OP_VALUES:
	case OP_KEYS:
	case OP_DELETE:
	case OP_EXISTS:
	case OP_RV2HV:
	case OP_HELEM:
	case OP_HSLICE:
	case OP_BOOLKEYS:
	case OP_UNPACK:
	case OP_PACK:
	case OP_SPLIT:
	case OP_JOIN:
	case OP_LIST:
	case OP_LSLICE:
	case OP_ANONLIST:
	case OP_ANONHASH:
	case OP_SPLICE:
	case OP_PUSH:
	case OP_POP:
	case OP_SHIFT:
	case OP_UNSHIFT:
	case OP_SORT:
	case OP_REVERSE:
	case OP_GREPSTART:
	case OP_GREPWHILE:
	case OP_MAPSTART:
	case OP_MAPWHILE:
	case OP_RANGE:
	case OP_FLIP:
	case OP_FLOP:
	case OP_AND:
	case OP_OR:
	case OP_XOR:
	case OP_DOR:
	case OP_COND_EXPR:
	case OP_ANDASSIGN:
	case OP_ORASSIGN:
	case OP_DORASSIGN:
		break;

	case OP_METHOD:
		ppe = PPE_IGNORE;
		break;

	case OP_ENTERSUB:
		ppe = PPE_CALL;
		break;

	case OP_LEAVESUB:
	case OP_LEAVESUBLV:
		ppe = PPE_RETURN;
		break;

	case OP_CALLER:
	case OP_WARN:
	case OP_DIE:
	case OP_RESET:
	case OP_LINESEQ:
		break;

	case OP_NEXTSTATE:
		ppe = PPE_IGNORE;
		break;

	case OP_DBSTATE:
	case OP_UNSTACK:
		break;

	case OP_ENTER:
		ppe = PPE_IGNORE;
		break;

	case OP_LEAVE:
		ppe = PPE_IGNORE;
		break;

	case OP_SCOPE:
	case OP_ENTERITER:
	case OP_ITER:
	case OP_ENTERLOOP:
	case OP_LEAVELOOP:
		break;

	case OP_RETURN:
		ppe = PPE_RETURN;
		break;

	case OP_LAST:
	case OP_NEXT:
	case OP_REDO:
	case OP_DUMP:
	case OP_GOTO:
		break;

	case OP_EXIT:
		break;

	case OP_METHOD_NAMED:
		break;

	case OP_ENTERGIVEN:
		break;

	case OP_LEAVEGIVEN:
		break;

	case OP_ENTERWHEN:
		break;

	case OP_LEAVEWHEN:
		break;

	case OP_BREAK:
	case OP_CONTINUE:
	case OP_OPEN:
	case OP_CLOSE:
	case OP_PIPE_OP:
	case OP_FILENO:
	case OP_UMASK:
	case OP_BINMODE:
		break;

	case OP_TIE:
		ppe = PPE_CALL;
		break;

	case OP_UNTIE:
	case OP_TIED:
	case OP_DBMOPEN:
	case OP_DBMCLOSE:
	case OP_SSELECT:
	case OP_SELECT:
	case OP_GETC:
	case OP_READ:
	case OP_ENTERWRITE:
	case OP_LEAVEWRITE:
	case OP_PRTF:
	case OP_PRINT:
	case OP_SAY:
	case OP_SYSOPEN:
	case OP_SYSSEEK:
	case OP_SYSREAD:
	case OP_SYSWRITE:
	case OP_EOF:
	case OP_TELL:
	case OP_SEEK:
	case OP_TRUNCATE:
	case OP_FCNTL:
	case OP_IOCTL:
	case OP_FLOCK:
	case OP_SEND:
	case OP_RECV:
	case OP_SOCKET:
	case OP_SOCKPAIR:
	case OP_BIND:
	case OP_CONNECT:
	case OP_LISTEN:
	case OP_ACCEPT:
	case OP_SHUTDOWN:
	case OP_GSOCKOPT:
	case OP_SSOCKOPT:
	case OP_GETSOCKNAME:
	case OP_GETPEERNAME:
	case OP_LSTAT:
	case OP_STAT:
	case OP_FTRREAD:
	case OP_FTRWRITE:
	case OP_FTREXEC:
	case OP_FTEREAD:
	case OP_FTEWRITE:
	case OP_FTEEXEC:
	case OP_FTIS:
	case OP_FTSIZE:
	case OP_FTMTIME:
	case OP_FTATIME:
	case OP_FTCTIME:
	case OP_FTROWNED:
	case OP_FTEOWNED:
	case OP_FTZERO:
	case OP_FTSOCK:
	case OP_FTCHR:
	case OP_FTBLK:
	case OP_FTFILE:
	case OP_FTDIR:
	case OP_FTPIPE:
	case OP_FTSUID:
	case OP_FTSGID:
	case OP_FTSVTX:
	case OP_FTLINK:
	case OP_FTTTY:
	case OP_FTTEXT:
	case OP_FTBINARY:
	case OP_CHDIR:
	case OP_CHOWN:
	case OP_CHROOT:
	case OP_UNLINK:
	case OP_CHMOD:
	case OP_UTIME:
	case OP_RENAME:
	case OP_LINK:
	case OP_SYMLINK:
	case OP_READLINK:
	case OP_MKDIR:
	case OP_RMDIR:
	case OP_OPEN_DIR:
	case OP_READDIR:
	case OP_TELLDIR:
	case OP_SEEKDIR:
	case OP_REWINDDIR:
	case OP_CLOSEDIR:
	case OP_FORK:
	case OP_WAIT:
	case OP_WAITPID:
	case OP_SYSTEM:
	case OP_EXEC:
	case OP_KILL:
	case OP_GETPPID:
	case OP_GETPGRP:
	case OP_SETPGRP:
	case OP_GETPRIORITY:
	case OP_SETPRIORITY:
	case OP_TIME:
	case OP_TMS:
	case OP_LOCALTIME:
	case OP_GMTIME:
	case OP_ALARM:
	case OP_SLEEP:
	case OP_SHMGET:
	case OP_SHMCTL:
	case OP_SHMREAD:
	case OP_SHMWRITE:
	case OP_MSGGET:
	case OP_MSGCTL:
	case OP_MSGSND:
	case OP_MSGRCV:
	case OP_SEMOP:
	case OP_SEMGET:
	case OP_SEMCTL:
		break;

	case OP_REQUIRE:
	case OP_DOFILE:
		ppe = PPE_IGNORE;
		break;

	case OP_HINTSEVAL:
	case OP_ENTEREVAL:
	case OP_LEAVEEVAL:
	case OP_ENTERTRY:
	case OP_LEAVETRY:
	case OP_GHBYNAME:
	case OP_GHBYADDR:
	case OP_GHOSTENT:
	case OP_GNBYNAME:
	case OP_GNBYADDR:
	case OP_GNETENT:
	case OP_GPBYNAME:
	case OP_GPBYNUMBER:
	case OP_GPROTOENT:
	case OP_GSBYNAME:
	case OP_GSBYPORT:
	case OP_GSERVENT:
	case OP_SHOSTENT:
	case OP_SNETENT:
	case OP_SPROTOENT:
	case OP_SSERVENT:
	case OP_EHOSTENT:
	case OP_ENETENT:
	case OP_EPROTOENT:
	case OP_ESERVENT:
	case OP_GPWNAM:
	case OP_GPWUID:
	case OP_GPWENT:
	case OP_SPWENT:
	case OP_EPWENT:
	case OP_GGRNAM:
	case OP_GGRGID:
	case OP_GGRENT:
	case OP_SGRENT:
	case OP_EGRENT:
	case OP_GETLOGIN:
	case OP_SYSCALL:
	case OP_LOCK:
	case OP_ONCE:
	case OP_CUSTOM:
		break;

	default:
		//ASSERT(0);
		break;
	}

	/* dispatch the event to the appropriate callback */
	/* if its an 'ignore' we do not update the previous file/line/event values */

	if (ppe != PPE_IGNORE)
	{
		if (prevFileName != fileName ||
			prevLine != line ||
			prevPPE != ppe)
		{
			/* handle any deferred events (used so that PPE_CALL has the line number of the 
			   subroutine not the call to it
			 */

			if (deferredPPE != PPE_NONE)
			{
				/* this is not perfect, if the subroutine starts with a for() loop,
				   the line number will be the end of the for() loop because of how Perl
				   sets the loop up. Imperfect, but I'm happy to live with it 
				 */

				switch(deferredPPE)
				{
				case PPE_NONE:
					break;

				case PPE_LINE:
					if (coverageCallback != NULL)
					{
						(*coverageCallback)(deferredPPE, fileName, line, functionName, packageName, profilingUserData);
					}
					break;

				case PPE_CALL:
				case PPE_RETURN:
					if (profilingCallback != NULL)
					{
						(*profilingCallback)(deferredPPE, fileName, line, functionName, packageName, profilingUserData);
					}
					break;
				}

				deferredPPE = PPE_NONE;
			}

			/* keep track of where we are to prevent duplicate events for the same line */

			prevFileName = fileName;
			prevLine = line;
			prevPPE = ppe;

			/* defer any calls to the next instruction on a different line */

			if (ppe == PPE_CALL)
			{
				deferredPPE = ppe;
				ppe = PPE_NONE;
			}

			/* handle any events */

			switch(ppe)
			{
			case PPE_NONE:
				break;

			case PPE_LINE:
				if (coverageCallback != NULL)
				{
					(*coverageCallback)(ppe, fileName, line, functionName, packageName, profilingUserData);
				}
				break;

			case PPE_CALL:
			case PPE_RETURN:
				if (profilingCallback != NULL)
				{
					(*profilingCallback)(ppe, fileName, line, functionName, packageName, profilingUserData);
				}
				break;
			}
		}
	}

	return TRUE;
}
