/*
 * Copyright (c) 2021 Divvun <https://divvun.org>, GPLv3
 *
 *
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdlib.h>
#include <hfst/hfst.h>
#include <getopt.h>
#include <stdio.h>
#include <ncurses.h>

#define MAX_PREDICTIONS 10

static const char* progname;

void
cleanup_curses()
  {
    endwin();
    refresh();
    int rv = system("stty sane"); // Ugly hack but works
    if (rv != EXIT_SUCCESS)
      {
        fprintf(stderr, "terminal is probably messed up, say reset or stty "
                "sane");
      }
  }

void
set_program_name(const char* s)
 {
   progname = s;
 }


void
print_usage(void)
  {
    printw(
        "\n"
        "Usage: %s [OPTIONS] TRANSDUCER\n"
        "Run interactive prediction on single transducer\n"
        "\n"
        "  -h, --help                   Print this help message\n"
        "  -V, --version                Print version information\n"
        "  -v, --verbose                Be verbose\n"
        "  -q, --quiet                  Don't be verbose (default)\n"
        "  -s, --silent                 Same as quiet\n"
        "  -a, --predictor=TRANSDUCER   use TRANSDUCER to predict\n"
        "\n"
        "TRANSDUCER is required. Input read from stdin and output to stdout.\n"
        "\n", progname);
  }

void
print_version(void)
  {
    printw("%s\n", progname);
  }

void
print_short_help(void)
  {
    printw("Use %s --help to find out more\n", progname);
  }

hfst::HfstTransducer*
load_predictor(const char* filename)
  {
    hfst::HfstInputStream* his = new hfst::HfstInputStream(filename);
    hfst::HfstTransducer* rv = new hfst::HfstTransducer(*his);
    return rv;
  }

void
predict(hfst::HfstTransducer* t, bool verbose, int cutoff)
  {
    printw("Enter alphabets or number to predict a morph, "
           "and space to finish the word, enter or ^C to quit.\n");
    char* input = strdup("");
    bool predicting = true;
    std::vector<std::string> completions = std::vector<std::string>(16);
    std::set<std::string> uniqueComps;
    while (predicting)
      {
        printw("%s", input);
        int c  = getch();
        if (c == ERR)
          {
            continue;
          }
        else if ((c == KEY_ENTER) || (c == '\n'))
          {
            printw("\ndone.\n");
            predicting = false;
          }
        if (c >= '0' && c <= '9')
          {
            input = strdup(completions[c - '0' + 1].c_str());
            printw("\nSelected %c %s\n", c, input);
          }
        else if (c == ' ')
          {
            printw("\nend prediction: %s\n", input);
            free(input);
            input = strdup("");
          }
        else
          {
            char* tmp = (char*)realloc(input, sizeof(char)*(strlen(input)+2));
            if (tmp == NULL)
              {
                printw("Memory allocation fail!\n");
                free(tmp);
                return;
              }
            else
              {
                input = tmp;
              }
            input = strcat(input, (char*)&c);
          }
        if (verbose)
          {
            printw("\nPredicting %s...\n", input);
          }
        auto predictions = t->lookup_fd(input, MAX_PREDICTIONS, cutoff);
        unsigned int i = 0;
        for (auto& p : *predictions)
          {
            std::stringstream prediq;
            for (auto& res : p.second)
              {
                if (!hfst::FdOperation::is_diacritic(res))
                  {
                    prediq << res;
                  }
              }
            printw("%u '%s' %f\n", i++, prediq.str().c_str(), p.first);
            completions[i] = prediq.str();
          }
        if (predictions->size() == 0)
          {
            printw("?");
          }
      }
    free(input);
  }

int main(int argc, char **argv)
  {
    set_program_name(argv[0]);
    bool verbose = false;
    char* filename = NULL;
    int cutoff = -1;
    char* endptr;
    initscr();
    cbreak();
    nodelay(stdscr, false);
    scrollok(stdscr, true);
    while (true)
      {
        static struct option long_options[] =
          {
            {"help",         no_argument,       0, 'h'},
            {"version",      no_argument,       0, 'V'},
            {"verbose",      no_argument,       0, 'v'},
            {"quiet",        no_argument,       0, 'q'},
            {"silent",       no_argument,       0, 's'},
            {"predictor",    required_argument, 0, 'a'},
            {"time-cutoff",  required_argument, 0, 't'},
            {0,              0,                 0,  0 }
          };
        int option_index = 0;
        int c = getopt_long(argc, argv, "hVvqst:a:", long_options,
                            &option_index);

        if (c == -1) // no more options to look at
          {
            break;
          }
        switch (c)
          {
        case 'h':
            print_usage();
            cleanup_curses();
            return EXIT_SUCCESS;
            break;

        case 'V':
            print_version();
            cleanup_curses();
            return EXIT_SUCCESS;
            break;

        case 'v':
            verbose = true;
            break;

        case 'q':
        case 's':
            verbose = false;
            break;

        case 'a':
            filename = strdup(optarg);
            break;

        case 't':
            endptr = optarg;
            cutoff = strtoul(optarg, &endptr, 10);
            if (endptr == optarg)
              {
                printw("%s is not a number for time offset\n", optarg);
                print_short_help();
                cleanup_curses();
                return EXIT_FAILURE;
              }
            else if (*endptr != '\0')
              {
                printw("%s is not a number for time offset\n", optarg);
                print_short_help();
                cleanup_curses();
                return EXIT_FAILURE;
              }
            break;
        default:
            printw("Unknown or wrong argument stuff %c\n", c);
            print_short_help();
            cleanup_curses();
            return EXIT_FAILURE;
            break;
          }
      }
    if ((optind + 1) < argc)
      {
        printw("Unrecognised argument %s\n", argv[optind]);
        print_short_help();
        cleanup_curses();
        return EXIT_FAILURE;
      }
    else if ((optind + 1) == argc)
      {
        if (filename != NULL)
          {
            printw("Unrecognised argument %s\n", argv[optind]);
            print_short_help();
            cleanup_curses();
            return EXIT_FAILURE;
          }
        filename = strdup(argv[optind]);
      }
    else if (filename == NULL)
      {
        printw("Required argument -a missing\n");
        print_short_help();
        cleanup_curses();
        return EXIT_FAILURE;
      }
    if (verbose)
      {
        printw("Reading predictor from %s...\n", filename);
      }
    auto fsa = load_predictor(filename);
    if (verbose)
      {
        printw("done!\nPredicting...\n");
      }
    try
      {
        predict(fsa, verbose, cutoff);
      }
    catch (FunctionNotImplementedException& fnie)
      {
        printw("%s does not support predictions:\n%s\n", filename,
               fnie().c_str());
      }
    cleanup_curses();
    free(filename);
    return EXIT_SUCCESS;
  }

