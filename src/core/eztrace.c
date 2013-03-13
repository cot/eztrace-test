#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include "pptrace.h"
#include "eztrace.h"

#define SIZE_STRING 1024
extern int pptrace_debug_level;

int file_exists(const char *pathname) {
  if (access(pathname, F_OK) != -1) {
    return 1; // file exists
  }
  return 0;
}

int str_begins_with(char *str, char *begin) {
  return strncmp(str, begin, strlen(begin)) == 0;
}

int str_ends_with(char *str, char *end) { 
  size_t str_len = strlen(str);
  size_t end_len = strlen(end); 
  char *str_end = str + str_len - end_len; 
  return strcmp(str_end, end) == 0; 
}

void usage(const char *prog_name) {
  printf("Usage: %s [OPTION] program [arg1 arg2 ...]\n", prog_name);
  printf("\t-t \"plugin1 plugin2 ... pluginN\" Select a list of plugins\n");
  printf("\t-o <directory>	       	      Select the output directory\n");
  printf("\t-l <directory>	       	      Select a plugin directory\n");
  printf("\t-f 			              Enable EZTRACE_FLUSH\n");
  printf("\t-d                                Debug mode\n");
  printf("\t-?  -h                            Display this help and exit\n");
  /* todo: add a verbose mode */
}

int main(int argc, char **argv) {
  int debug = 0;
  int use_pp = 0; // for pptrace
  int i, j, k;
  int test=0;

  // options
  int nb_opts=0;
  for (i=1; i<argc; i++) {
	  if (!strcmp(argv[i], "-d")) {
		  debug=1;
		  nb_opts++;
	  } else if (!strcmp(argv[i], "-p")) {
		  test=1;
		  nb_opts++;
	  } else if (!strcmp(argv[i], "-t")) {
		  setenv("EZTRACE_TRACE", argv[i+1], 1);
		  i++;
		  nb_opts+=2;
	  } else if (!strcmp(argv[i], "-o")) {
		  setenv("EZTRACE_TRACE_DIR", argv[i+1], 1);
		  i++;
		  nb_opts+=2;
	  } else if (!strcmp(argv[i], "-l")) {
		  setenv("EZTRACE_LIBRARY_PATH", argv[i+1], 1);
		  i++;
		  nb_opts+=2;
	  } else if (!strcmp(argv[i], "-f")) {
		  setenv("EZTRACE_FLUSH", "1", 1);
		  nb_opts++;
	  } else if (!strcmp(argv[i], "-?") || !strcmp(argv[i], "-h")) {
		  usage(argv[0]);
		  return EXIT_SUCCESS;
	  } else if(argv[i][0] == '-' && argv[i][1] == 'v') {
		  nb_opts++;
		  for(j = 1; argv[i][j] == 'v'; j++)
			  pptrace_debug_level++;
	  }
  }

  // make sure eztrace libs are available
  char *tmp =  getenv("LD_LIBRARY_PATH");
  char ld_library_path[SIZE_STRING] = "";
  if (tmp != NULL) {
	  strcat(ld_library_path, tmp);
  }
  if (test==0) {
	  strcat(ld_library_path, ":/usr/local/lib");
  }
  // other env variables
  char ezt_all[SIZE_STRING] = "";
  char ezt_all_path[SIZE_STRING] = "";

  tmp = NULL;
  tmp = getenv("EZTRACE_UNUSED");
  char ezt_unused[SIZE_STRING] = "";
  if(tmp != NULL) {
          strcat(ezt_unused, tmp);
          strcat(ezt_all, tmp);
          strcat(ezt_all, " ");
  }

  tmp = NULL;
  tmp = getenv("EZTRACE_UNUSED_PATH");
  char ezt_unused_path[SIZE_STRING] = "";
  if(tmp != NULL) {
          strcat(ezt_unused_path, tmp);
          strcat(ezt_all_path, tmp);
          strcat(ezt_all_path, ":");
  }

  tmp = NULL;
  tmp = getenv("EZTRACE_TRACE");
  char ezt_trace[SIZE_STRING] = "";
  if(tmp != NULL) {
	  strcat(ezt_trace, tmp);
	  strcat(ezt_all, tmp);
  }

  tmp = NULL;
  tmp = getenv("EZTRACE_LIBRARY_PATH");
  char ezt_library_path[SIZE_STRING] = "";
  if(tmp != NULL) {
	  strcat(ezt_library_path, tmp);
	  strcat(ezt_all_path, tmp);
  }
  if (test==0) {
	  strcat(ezt_library_path, ":/usr/local/lib");
  }

  if(ezt_all != NULL) setenv("EZTRACE_ALL",ezt_all,1);
  tmp = getenv("EZTRACE_ALL");
  printf(" EZTRACE_ALL = %s \n",tmp);

  if(ezt_all_path != NULL) setenv("EZTRACE_ALL_PATH",ezt_all_path,1);
  tmp = getenv("EZTRACE_ALL_PATH");
  printf(" EZTRACE_ALL_PATH = %s \n",tmp);
 
  // prog_name
  char *prog_name = argv[nb_opts+1];
  if (prog_name == NULL) {
	  usage(argv[0]);
	  return EXIT_SUCCESS;
  }

  // init binary & catch all modules
  void *bin = pptrace_prepare_binary(prog_name);
  if(!bin) {
	  fprintf(stderr, "Unable to load binary %s\n", prog_name);
	  return EXIT_FAILURE;
  }

  // args of prog
  int nb_args = argc-nb_opts-2; // 2 : argv[0], prog_name

  char args_concat[SIZE_STRING] = "";
  for (i=0; i<nb_args; i++) {
	  strcat(args_concat, argv[nb_opts+2+i]);
	  if (i != nb_args-1) {
		  strcat(args_concat, " ");
	  }
  }

  // modules in ezt_trace and ezt_unused
  int nb_modules = 0;
  if (strcmp(ezt_all, "")) {
	  char *module = NULL;
	  module = strtok(ezt_all, " ");
	  while(module != NULL) {
		  if (strcmp(module, "")) {
			  nb_modules++;
		  }
		  module = strtok(NULL, " ");
	  }
  }
  printf("nb_modules = %i \n",nb_modules);

  char *modules[nb_modules];
  if(nb_modules != 0) {
	  char *module = NULL;
	  // we have to get EZTRACE_ALL again because we strtok-ed it
	  tmp = NULL;
	  tmp = getenv("EZTRACE_ALL");
	  strcpy(ezt_all, "");
	  if(tmp != NULL) {
		  strcat(ezt_all, tmp);
	  }

	  module = strtok(ezt_all, " ");
	  i=0;
	  while(module != NULL) {
		  if (strcmp(module, "")) {
			  modules[i] = module;
			  i++;
		  }
		  module = strtok(NULL, " ");
	  }
  }

  // dirs in ezt_library_path and ezt_unused_path
  int nb_dirs = 0;
  char *dir = NULL;
  strcat(tmp, "/usr/local/lib");
  if (strcmp(ezt_all_path, "")) {
	  dir = strtok(ezt_all_path, ":");
	  while(dir != NULL) {
		  if (strcmp(dir, "")) {
			  if(!strcmp(dir, tmp ))  nb_dirs++;
		  }
		  dir = strtok(NULL, ":");
	  }
  }
  printf("nb_dirs = %i \n",nb_dirs);

  char *dirs[nb_dirs];
  if(nb_dirs != 0) {
	  tmp = NULL;
	  tmp = getenv("EZTRACE_ALL_PATH");
	  strcpy(ezt_all_path, "");
	  if(tmp != NULL) {
		  strcat(ezt_all_path, tmp);
	  }
	  if(test==0) {
		  strcat(ezt_all_path, ":/usr/local/lib");
	  }
	  char *dir = NULL;
	  dir = strtok(ezt_all_path, ":");
	  i=0;
	  while(dir != NULL) {
		  if (strcmp(dir, "")) {
			  dirs[i] = dir;
			  i++;
		  }
		  dir = strtok(NULL, ":");
	  }
  }

  // libeztrace should always be preloaded!
  char files[SIZE_STRING]="";
  if (test) {
	  fprintf(stderr, "Eztrace test Mode\n");
	  strcat(files, "/home/rue/Bureau/EZTRACE/eztrace/unused-git/src/core/.libs/libeztrace-autostart.so");
  }else{
	  strcat(files, "/usr/local/lib/libeztrace-autostart.so");
  }
  if(pptrace_add_preload(bin, files)){
	  fprintf(stderr, "Unable to add %s to LD_PRELOAD\n", files);
	  return EXIT_FAILURE;
  }
  set_launcher_env();
  if (strcmp(ezt_all, "")) {
    for (i=0; i<nb_modules; i++) {
      for (j=0; j<nb_dirs; j++) {
	      char pathname[SIZE_STRING] = "";
	      strcat(pathname, dirs[j]);
	      strcat(pathname, "/libeztrace-autostart-");
	      strcat(pathname, modules[i]);
	      strcat(pathname, ".so");
	      if(file_exists(pathname)) {
            if(pptrace_load_module(bin, pathname)){
	          fprintf(stderr, "Unable to add %s to LD_PRELOAD\n", pathname);
	          return EXIT_FAILURE;
	        }
	        strcat(files, ":");
	        strcat(files, pathname);
	      }
      }
    }
  }else {
	  printf("on est clairement dans ce cas ...\n");
    for(i=0; i<nb_dirs; i++) {
      char *pathname = dirs[i];
      struct dirent *dirent;
      DIR *d;
      d = opendir(pathname);
      while ((dirent = readdir(d))) {
	      if(str_begins_with(dirent->d_name, "libeztrace-autostart-") && str_ends_with(dirent->d_name, ".so")) {
	        char file[SIZE_STRING] = "";
	        strcat(file, pathname);
	        strcat(file, "/");
	        strcat(file, dirent->d_name);
            if(pptrace_load_module(bin, file)) {
	          fprintf(stderr, "Unable to add %s to LD_PRELOAD\n", file);
	          return EXIT_FAILURE;
	        }
	        
	        strcat(files, ":");
	        strcat(files, file);
	      }
      }
      closedir(d);
    }
  }
  unset_launcher_env();
  // run
  if (debug) {
	  // TODO: make it configurable
	  char *debugger[5];
	  debugger[0] = "gdb";
	  debugger[1] = "-quiet";
	  debugger[2] = "{name}";
	  debugger[3] = "{pid}";
	  debugger[4] = NULL;
	  pptrace_add_debugger(bin, debugger);
  }
  if(pptrace_run(bin, argv+nb_opts+1, environ)) {
	  fprintf(stderr, "Unable to run the target\n");
	  return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
