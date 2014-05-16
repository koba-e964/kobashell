#include "./kobash.h"
#include <signal.h>

#include <assert.h>
#include <stdio.h>

void test_handler(int id) {
#if DEBUG
  printf("test_handler : %d\n", id);
#endif
}

void sigtstp_handler(int id) {
  assert(id == SIGTSTP);
  printf("sigtstp_handler\n");
}

void sigchld_action(int id, siginfo_t *info, void *param) {
#if DEBUG
  printf("sigchld_action:");
#endif

#if DEBUG
#define CASE(name) case name: printf("si_code = %s(%d)\n", #name, name); break;
  switch(info->si_code) {
    CASE(SI_USER);
    CASE(SI_KERNEL);
    CASE(SI_QUEUE);
    CASE(SI_TIMER);
    CASE(SI_MESGQ);
    CASE(SI_ASYNCIO);
    CASE(SI_SIGIO);
    CASE(SI_TKILL);
    CASE(CLD_EXITED);
    CASE(CLD_KILLED);
    CASE(CLD_DUMPED);
    CASE(CLD_STOPPED);
    CASE(CLD_CONTINUED);
  default:
    printf("si_code = %d\n", info->si_code);
  }  
#endif // DEBUG
  switch(info->si_code) {
  case CLD_STOPPED:
    break;
  default:
    break;
  }
}

void signal_init(void) {
  // setting handler
  struct sigaction sa;
  sa.sa_handler = test_handler;
  sa.sa_flags = 0;
  sigfillset(&sa.sa_mask);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGQUIT, &sa, NULL);
  /* Ignores SIGTTIN, SIGTTOU */
  sa.sa_handler = SIG_IGN;
  sigaction(SIGTTIN, &sa, NULL);
  sigaction(SIGTTOU, &sa, NULL);
  sa.sa_handler = test_handler;
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = sigchld_action;
  sigaction(SIGCHLD, &sa, NULL);
  sa.sa_flags = 0;
  sa.sa_handler = sigtstp_handler;
  sigaction(SIGTSTP, &sa, NULL);
}
