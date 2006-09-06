#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>

#include <sys/sysconf.h>

#include "sys/wcetrace.h"

#include "sys/shared.h"
#include "sys/io.h"
#include "sys/fixpath.h"

void
_start_process(wchar_t *exe, wchar_t *cmd)
{
  STARTUPINFOW si;
  PROCESS_INFORMATION pi;

  memset(&si, 0, sizeof(si));

  CreateProcessW(exe, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
}

/* called with an exception stack, we must switch to the old stack... */
void __eh_continue(void * a);


struct exception_map_t
{
  DWORD exception;
  int signal;
  const char* str;
};

/* from pocket console */
struct exception_map_t __exception_map[] = 
{
  { STATUS_ACCESS_VIOLATION,         SIGSEGV,  "Access Violation" },
  { STATUS_ILLEGAL_INSTRUCTION,      SIGILL, "Illegal Instruction"},
  { STATUS_PRIVILEGED_INSTRUCTION,   SIGILL, "Privileged Instruction" },
  /*      { (unsigned long)STATUS_NONCONTINUABLE_EXCEPTION, NOSIG,   SIG_DIE }, */
  /*      { (unsigned long)STATUS_INVALID_DISPOSITION,      NOSIG,   SIG_DIE }, */
  { STATUS_INTEGER_DIVIDE_BY_ZERO,   SIGFPE, "Integer divide by zero" },
  { STATUS_FLOAT_DENORMAL_OPERAND,   SIGFPE, "Float denormal operand" },
  { STATUS_FLOAT_DIVIDE_BY_ZERO,     SIGFPE, "Float divide by zero" },
  { STATUS_FLOAT_INEXACT_RESULT,     SIGFPE, "Float inexact result" },
  { STATUS_FLOAT_INVALID_OPERATION,  SIGFPE, "Float invalid operation" },
  { STATUS_FLOAT_OVERFLOW,           SIGFPE, "Float overflow" },
  { STATUS_FLOAT_STACK_CHECK,        SIGFPE, "Float stack check" },
  { STATUS_FLOAT_UNDERFLOW,          SIGFPE, "Float underflow" },
  /*      { (unsigned long)STATUS_INTEGER_DIVIDE_BY_ZERO,   NOSIG }, */
  /*      { (unsigned long)STATUS_STACK_OVERFLOW,           NOSIG } */
};

static struct exception_map_t* get_exception_map_for(DWORD xcptnum)
{
  struct exception_map_t* it = __exception_map;
  struct exception_map_t* end = it + (sizeof __exception_map)/sizeof(__exception_map[0]);

  for (; it != end; it++) {
    if (it->exception == xcptnum)
      return it;
  }
  return 0;
}

// in assembly
extern void _call_raise_asm(int sig);

extern void Sleep(DWORD );

void call_raise_c(int sig)
{
  printf("calling raise with sig: %d\n", sig);
  int ret = raise(sig);
  printf("raise returned with: %d\n", ret);
  if (ret)
  {
    printf("exiting\n", ret);
    exit(3);
  }
  else
  {
    printf("resuming code\n");
    Sleep(2000);
  }
}


// called from startup-stub.c

EXCEPTION_DISPOSITION
_eh_handler_(struct _EXCEPTION_RECORD *ExceptionRecord,
	    void *EstablisherFrame, 
	    struct _CONTEXT *ContextRecord,
	    struct _DISPATCHER_CONTEXT *DispatcherContext)
{
  // ### What is this needed for?
  static int NestedException=0;
  if(NestedException)
  {
  	printf("nested exception\n");
  	goto Nest;
  }
  NestedException=1;

  if (ExceptionRecord->ExceptionCode == EXCEPTION_DATATYPE_MISALIGNMENT)
  {
    int i;
    DWORD Cmd;
    DWORD DataAddr;

#if 0
    printf("Trying to handle EXCEPTION_DATATYPE_MISALIGNMENT Flags:%x Addr:%x "
      "SP:%x LR:%x R0:%x R1:%x R2:%x R3:%x R4:%x R5:%x R12:%x FP:%x\n", 
      ExceptionRecord->ExceptionFlags,
      ExceptionRecord->ExceptionAddress,
      ContextRecord->Sp,
      ContextRecord->Lr,
      ContextRecord->R0,
      ContextRecord->R1,
      ContextRecord->R2,
      ContextRecord->R3,
      ContextRecord->R4,
      ContextRecord->R5,
      ContextRecord->R12,
      EstablisherFrame
    );
#endif

    Cmd=*(DWORD*)(ExceptionRecord->ExceptionAddress);  // this may cause other exception

    if((Cmd&0xfff00000)==0xe5900000)
    {
      int Src, Dst, Off;
      DWORD *Regs=(DWORD*)&ContextRecord->R0;
      Src=(Cmd>>16)&0xf;
      Dst=(Cmd>>12)&0xf;
      Off=Cmd&0xfff;
      DataAddr=Regs[Dst]+Off;
      fprintf(stderr, "Warning: Emulating unaligned LDR R%d,[R%d+%d] (DataAddr:%d) (Cmd:%x)\n",
                Src, Dst, Off, DataAddr, Cmd);
      memcpy(Regs+Dst, (char*)DataAddr, 4);
    } 
    else if((Cmd&0xfff00000)==0xe5800000)
    {
      int Src, Dst, Off;
      DWORD *Regs=(DWORD*)&ContextRecord->R0;
      Dst=(Cmd>>16)&0xf;
      Src=(Cmd>>12)&0xf;
      Off=Cmd&0xfff;
      DataAddr=Regs[Dst]+Off;
      fprintf(stderr, "Warning: Emulating unaligned STR R%d,[R%d+%d] (DataAddr:%d) (Cmd:%x)\n",
                Src, Dst, Off, DataAddr, Cmd);
      memcpy((char*)DataAddr, Regs+Src, 4);
    }
    else {
      printf("Unhandled command:%x\n", Cmd);
      goto Cont;
    }

    ContextRecord->Pc+=4;	// skip faulty instruction

    NestedException=0;
    return ExceptionContinueExecution;
Cont:
    printf("Unhandled ");
  }
Nest:
  printf("Exception: Code:%x Flags:%x Addr:%x "
    "SP:%x LR:%x R0:%x R1:%x R2:%x R3:%x R4:%x R5:%x R12:%x FP:%x\n", 
    ExceptionRecord->ExceptionCode,
    ExceptionRecord->ExceptionFlags,
    ExceptionRecord->ExceptionAddress,
    ContextRecord->Sp,
    ContextRecord->Lr,
    ContextRecord->R0,
    ContextRecord->R1,
    ContextRecord->R2,
    ContextRecord->R3,
    ContextRecord->R4,
    ContextRecord->R5,
    ContextRecord->R12,
    EstablisherFrame
    );

  DWORD* sp = (DWORD*)ContextRecord->Sp;
  *--sp = ContextRecord->Pc;
  *--sp = ContextRecord->Lr;
  *--sp = ContextRecord->Sp;
  *--sp = ContextRecord->R12;
  *--sp = ContextRecord->R11;
  *--sp = ContextRecord->R10;
  *--sp = ContextRecord->R9;
  *--sp = ContextRecord->R8;
  *--sp = ContextRecord->R7;
  *--sp = ContextRecord->R6;
  *--sp = ContextRecord->R5;
  *--sp = ContextRecord->R4;
  *--sp = ContextRecord->R3;
  *--sp = ContextRecord->R2;
  *--sp = ContextRecord->R1;
  *--sp = ContextRecord->R0;
  *--sp = ContextRecord->Psr;

  ContextRecord->Sp = (DWORD) sp;
  ContextRecord->Pc = (DWORD) _call_raise_asm;

  NestedException = 0;

  struct exception_map_t* expt = get_exception_map_for(ExceptionRecord->ExceptionCode);
  if (!expt)
  {
    printf("Unhandled kernel exception %x\n", ExceptionRecord->ExceptionCode);
    exit(-1);
  }
  printf("%s\n", expt->str);
  ContextRecord->R0 = expt->signal;

  /* raise(SIGSEGV); */
  __eh_continue(ContextRecord);

  /* NOT REACHED */
  printf("Signal handler returned!\n");
  exit(1);
}

void _parse_tokens(char * string, char * tokens[]);

/*
 * The current thinking here is that we will set up this default environment
 * block.  WCESYS can override this by setting the `environ' pointer to the 
 * shared memory segment it manages
 */
int   __pgid = 0;

#define ENV_LEN     (64)
char *__env[ENV_LEN];

extern char ** environ;

#define CMDLINE_LEN 1024
#define ARGV_LEN    96
#define NAME_LEN    64

static char __cmdlinebuf[CMDLINE_LEN];
char * __argv[ARGV_LEN];
int   __argc;

HANDLE __main_thread = 0;

typedef int fnmain(int argc, char** argv); 
typedef void fngccmain(void); 

static _SHMBLK  shmblk = NULL;
extern void _initenv(_SHMBLK shmblk);

static void process_args(void)
{
  wchar_t  cmdnameBufW[NAME_LEN];
  char     buf[MAX_PATH];
  int      cmdlineLen = 0;
  wchar_t* cmdlinePtrW;
  int pgid = 0;
  int i;
  int len;
  int cmdlen = 0;

  /* argv[0] is the path of invoked program - get this from CE */
  len = GetModuleFileNameW(NULL, cmdnameBufW, NAME_LEN);
  wcstombs(buf, cmdnameBufW, wcslen(cmdnameBufW) + 1);

  /* fixpath replaces backslashes with slashes */
  fixpath(buf, __cmdlinebuf);

  char* p = __cmdlinebuf;
  while (*p)
  {
    if(*p == '\\')
      *p = '/';
    p++;
  }

  __argv[0] = __cmdlinebuf;
  cmdlen = strlen(__cmdlinebuf);

  cmdlinePtrW = GetCommandLineW();
  if (!cmdlinePtrW)
    return;
  cmdlineLen = wcslen(cmdlinePtrW);
  if (cmdlineLen > 0) {
    char* argv1 = __cmdlinebuf + cmdlen + 1;
    wcstombs(argv1, cmdlinePtrW, cmdlineLen + 1);
    WCETRACE(WCE_IO, "command line: \"%s\"", argv1);
    __argc = ARGV_LEN - 1;
    _parse_tokens(argv1, &__argv[1]);
  }

  /* Add one to account for argv[0] */
  __argc++;

  if (__argc <= 1)
    return;

  for (i = 1; i < __argc; i++) {
    if (strncmp(__argv[i], "-pgid", 5) == 0) {
      pgid = atoi(__argv[__argc-1]);
      WCETRACE(WCE_IO, "attaching to pgid %d", pgid);
      shmblk = _shared_init(pgid);
      __pgid = pgid;
      WCETRACE(WCE_IO, "shmblk @%x", shmblk);
      if (i < __argc - 2) {
        memmove(&__argv[i], &__argv[i+2], (__argc-i-2) * sizeof(char *));
      }
      __argv[__argc-2] = NULL;
      __argv[__argc-1] = NULL;
      __argc -= 2;
      break;
    }
  }
}

int __init_c__()
{
  static int initted = 0;
  NKDbgPrintfW(L"__init_c__: initted = %d\n", initted);
  if (initted++)
    return initted;

  NKDbgPrintfW(L"__init_c__: calling __init_ce_reent\n");

  __init_ce_reent();

  char    *ptr;
  int      len = 0;

  _initfds();

  /* Initialize environment block */
  environ = __env;
  memset(__env, 0, sizeof(char *) * ENV_LEN);
  memset(__argv, 0, sizeof(char *) * ARGV_LEN);

#if 0
  int trace = 0;
  trace |= WCE_IO;
  trace |= WCE_SYNCH;
  trace |= WCE_NETWORK;
  trace |= WCE_SIGNALS;
  trace |= WCE_FIFOS;
  trace |= WCE_TIME;
//  trace |= WCE_MALLOC;
//  trace |= WCE_VM;
  WCETRACE_DEBUGGER_SET(trace);
  WCETRACE(WCE_IO, "WCETRACE initialized");
#endif

  /* Trigger _initenv_from_reg.  */
  _initenv(0);

  WCETRACE(WCE_IO, "in __init_c__()");
  return initted;
}

static void inherit_parent(void)
{
  char buf[MAX_PATH];
  char envbuf[MAX_PATH];
  int  fd = 0, stdfd = 0;
  int  stdinfd, stdoutfd;
  char     fifoBuf[NAME_LEN];
  struct   termios tbuf;

  /* Inherit environment, fds, cwd, etc. from parent */
  if (shmblk != NULL) {
    WCETRACE(WCE_IO, "inherit_parent: inhering from parent", stdfd);

    atexit(_ioatexit);
    _shared_dump(shmblk);
    _shared_getenvblk(shmblk, environ);
    __pgid = _shared_getpgid(shmblk);
    _shared_getcwd(shmblk, buf);

    WCETRACEGETENV();
//    WCETRACE(WCE_IO, "_startup: child \"%s\"", cmdnameBuf);
    WCETRACE(WCE_IO, "_startup: setting cwd to \"%s\"", buf);
    chdir(buf);
    sprintf(envbuf, "PWD=%s", buf);
    putenv(envbuf);

    if ((fd = _shared_getstdinfd(shmblk)) >= 0) {
      sprintf(fifoBuf, "fifo%d", fd);
      stdinfd = stdfd = open(fifoBuf, O_CREAT | O_EXCL | O_RDWR, 0660);
      if (stdfd != 0) {
        WCETRACE(WCE_IO, "_startup ERROR stdfd != 0 (%d)", stdfd);
      }
      _initstdfifofd(stdin, 0, __SRD);
    } else {
      /* We might want to do the following, but it "pops up console" */
      /* _initstdfd(stdin, 0, (HANDLE)_fileno(_getstdfilex(0)), __SRD); */
    }

    if ((fd = _shared_getstdoutfd(shmblk)) >= 0) {
      sprintf(fifoBuf, "fifo%d", fd);
      stdoutfd = stdfd = open(fifoBuf, O_CREAT | O_EXCL | O_RDWR, 0660);
      if (stdfd != 1) {
        WCETRACE(WCE_IO, "_startup ERROR stdfd != 1 (%d)", stdfd);
      }
      _initstdfifofd(stdout, 1, __SWR);
     
      /* Set up the plumbing so that echo will work (if enabled later) */
      _initecho(stdinfd, stdoutfd);
    } else {
      /* We might want to do the following, but it "pops up console" */
      /* _initstdfd(stdout, 1, (HANDLE)_fileno(_getstdfilex(1)), __SWR); */
    }

    if ((fd = _shared_getstderrfd(shmblk)) >= 0) {
      sprintf(fifoBuf, "fifo%d", fd);
      stdfd = open(fifoBuf, O_CREAT | O_EXCL | O_RDWR, 0660);
      if (stdfd != 2) {
        WCETRACE(WCE_IO, "_startup ERROR stdfd != 2 (%d)", stdfd);
      }
      _initstdfifofd(stderr, 2, __SWR);
    } else {
      /* We might want to do the following, but it "pops up console" */
      /* _initstdfd(stderr, 2, (HANDLE)_fileno(_getstdfilex(2)), __SWR); */
    }

  } else {
    WCETRACEGETENV();

    WCETRACE(WCE_IO, "inherit_parent: unattached");
    /* If we are unattached to a shmblk, we might want to do the following */
    /* _initstdio(); */
    char  tmp_path[MAXPATHLEN];
    wchar_t mod_fname[MAXPATHLEN];
    int i;

    /* mamaich: make current directory to be the path extracted from EXE filename 
       and read some environment vars from registry */

  	GetModuleFileNameW(0,mod_fname,MAXPATHLEN);
  	wcstombs(tmp_path,mod_fname,MAXPATHLEN);
  	*strrchr(tmp_path,'\\') = 0;

    //for(i=0; i<strlen(tmp_path); i++)
		  //if(tmp_path[i]=='\\') tmp_path[i]='/';

    chdir(tmp_path);
    sprintf(envbuf, "PWD=%s", tmp_path);
    putenv(envbuf);
//    if(rgetenv("PATH", tmp_path, MAXPATHLEN)!=-1)
    char* env = getenv("PATH");
    if (env) {
	    sprintf(envbuf, "PATH=%s", env);
    	putenv(envbuf);
    }
    else
      putenv("PATH=.:/:/Windows");
#if 0
    if(rgetenv("WCETRACE", tmp_path, MAXPATHLEN)!=-1)
    {
	    sprintf(envbuf, "WCETRACE=%s", tmp_path);
    	putenv(envbuf);
	    WCETRACEGETENV();
    }
#endif

  }

  memset(&tbuf, 0, sizeof(tbuf));
  tbuf.c_lflag = (ECHO | ICANON);
  tcsetattr(0, 0, &tbuf);		/* This always fails when under GDB */
  					/* It's odd that this is happening before _initstdio() */
}

extern void __s_reinit(struct _reent *s);

static jmp_buf exitjmp;
static int exitcode;

void
_exit(int code)
{
	WCETRACE(WCE_IO, "_exit: %d", code);
	exitcode = code;
	longjmp(exitjmp, 1);
}

//called from startup_thunk.c
int
_startup_(fnmain *_main)
{
  int initres = __init_c__();
  WCETRACE(WCE_IO, "initres = %d", initres);

  process_args();

  WCETRACE(WCE_IO, "__argc = %d", __argc);
  int i;
  for (i = 0 ; i < __argc ;i++)
    WCETRACE(WCE_IO, "__argv[%d] = %s", i, __argv[i]);

  _initenv(shmblk);
  _shared_attach();

  __main_thread = GetCurrentThread();

  inherit_parent();
  __s_reinit(_REENT);
  _initstdio();

  WCETRACE(WCE_IO, "_startup_: calling main");
  exitcode = 1;
  if(setjmp(exitjmp) == 0)
  {
    WCETRACE(WCE_IO, "_startup_: calling _main");
    exitcode=_main(__argc, __argv);
    WCETRACE(WCE_IO, "_startup: calling registered atexit() functions");
    exit(exitcode); // call registered atexit() functions
  }

  _shared_dettach();

  WCETRACE(WCE_IO, "_startup: going to terminate with exitcode: %d", exitcode);
  WCETRACECLOSE();

  TerminateProcess(GetCurrentProcess(), exitcode);
  return exitcode;
}

void
_parse_tokens(char * string, char * tokens[]) 
/*-------------------------------------------------------------------------*
 * Extract whitespace- and quotes- delimited tokens from the given string
 * and put them into the tokens array. Set *length to the number of tokens
 * extracted. On entry, *length must specify the size of tokens[].
 * THIS METHOD MODIFIES string.
 *-------------------------------------------------------------------------*/
{
  char *  whitespace = " \t\r\n";
  char *  tokenEnd;
  char *  quoteCharacters = "\"\'";
  int     numQuotes = strlen(quoteCharacters);
  int     quoteIndex;
  char    quote;
  int     index = 0;

  if (string != NULL) {
    while (index < __argc) {
       /* Skip over initial whitespace */
      string += strspn(string, whitespace);
      if (strlen(string) == 0) break;
      quoteIndex = numQuotes;
      while (--quoteIndex >= 0) if (*string == quoteCharacters[quoteIndex]) break;
      if (quoteIndex >= 0) {
         /* Token is quoted */
        quote = *string++;
        tokenEnd = strchr(string, quote);
         /* If there is no endquote, the token is the rest of the string */
        if (tokenEnd == NULL) tokenEnd = string + strlen(string);
      } else {
        tokenEnd = string + strcspn(string, whitespace);
      }
      tokens[index++] = string;
      if (*tokenEnd == '\0') break;
      *tokenEnd = '\0';
      string = tokenEnd + 1;
    }
  }
  __argc = index;
}
