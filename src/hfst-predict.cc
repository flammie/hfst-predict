/*
 * Copyright (c) 2021 Divvun <https://divvun.org>, GPLv3
 */

#if HAVE_CONFIG
#include "config.h"
#endif
#include <hfst/hfst.h>
#include <getopt.h>
#include <stdio.h>
#include <ncurses.h>

#define MAX_PREDICTIONS 10
#define TIME_CUTOFF -1

void
print_usage(void)
  {
    fprintf(stderr,
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
        "\n", PACKAGE_NAME);
  }

void
print_version(void)
  {
    fprintf(stderr, "%s\n", PACKAGE_STRING);
  }

void
print_short_help(void)
  {
    fprintf(stderr, "Use %s --help to find out more\n", PACKAGE_NAME);
  }

hfst::HfstTransducer*
load_predictor(const char* filename)
  {
    hfst::HfstInputStream* his = new hfst::HfstInputStream(filename);
    hfst::HfstTransducer* rv = new hfst::HfstTransducer(*his);
    return rv;
  }

void
predict(hfst::HfstTransducer* t, bool verbose)
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
            input = (char*)realloc(input, sizeof(char)*(strlen(input)+2));
            input = strcat(input, (char*)&c);
          }
        if (verbose)
          {
            printw("\nPredicting %s...\n", input);
          }
        auto predictions = t->lookup_fd(input, MAX_PREDICTIONS, TIME_CUTOFF);
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
      }
  }

int main(int argc, char **argv)
  {
    bool verbose = false;
    char* filename = NULL;
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
            {0,              0,                 0,  0 }
          };
        int option_index = 0;
        int c = getopt_long(argc, argv, "hVvqsa:", long_options, &option_index);

        if (c == -1) // no more options to look at
          {
            break;
          }
        switch (c)
          {
        case 'h':
            print_usage();
            return EXIT_SUCCESS;
            break;

        case 'V':
            print_version();
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

        default:
            print_short_help();
          return EXIT_FAILURE;
          break;
          }
      }
    if ((optind + 1) < argc)
      {
        printw("Unrecognised argument %s\n", argv[optind]);
        print_short_help();
        return EXIT_FAILURE;
      }
    else if ((optind + 1) == argc)
      {
        if (filename != NULL)
          {
            printw("Unrecognised argument %s\n", argv[optind]);
            print_short_help();
            return EXIT_FAILURE;
          }
        filename = strdup(argv[optind]);
      }
    else if (filename == NULL)
      {
        printw("Required argument -a missing\n");
        print_short_help();
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
        predict(fsa, verbose);
      }
    catch (FunctionNotImplementedException& fnie)
      {
        printw("%s does not support predictions:\n%s\n", filename,
               fnie().c_str());
      }
    endwin();
    return EXIT_SUCCESS;
  }

