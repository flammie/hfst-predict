/*
 * (c) 2021 Divvun <https://divvun.org>
 */

#if HAVE_CONFIG
#include "config.h"
#endif
#include <hfst/hfst.h>
#include <getopt.h>
#include <stdio.h>

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
    fprintf(stdout, "Enter alphabets or number to predict a morph, "
            "and space to finish the word.\n");
    char* input = strdup("");
    bool predicting = true;
    std::vector<std::string> completions = std::vector<std::string>(16);
    std::set<std::string> uniqueComps;
    while (predicting)
      {
        fprintf(stdout, "%s", input);
        char* l = NULL;
        size_t n = 0;
        ssize_t len = getline(&l, &n, stdin);
        if (len == -1)
          {
            perror("getline");
            break;
          }
        l[len-1] = '\0';
        if (len == 2 && (l[len-2] >= '0') && (l[len-2] <= '9'))
          {
            input = strdup(completions[l[len-2] - '0'].c_str());
            fprintf(stdout, "Selected %c %s\n", l[len-2], input);
          }
        else {
            input = (char*)realloc(input, sizeof(char)*(strlen(input)+len+2));
            input = strcat(input, l);
        }
        if (verbose)
          {
            fprintf(stdout, "Predicting %s...\n", input);
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
            fprintf(stdout, "%u '%s' %f\n", i++, prediq.str().c_str(), p.first);
            completions[i] = prediq.str();
          }
      }
  }

int main(int argc, char **argv)
  {
    bool verbose = false;
    char* filename = NULL;
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
        fprintf(stderr, "Unrecognised argument %s\n", argv[optind]);
        print_short_help();
        return EXIT_FAILURE;
      }
    else if ((optind + 1) == argc)
      {
        if (filename != NULL)
          {
            fprintf(stderr, "Unrecognised argument %s\n", argv[optind]);
            print_short_help();
            return EXIT_FAILURE;
          }
        filename = strdup(argv[optind]);
      }
    else if (filename == NULL)
      {
        fprintf(stderr, "Required argument -a missing\n");
        print_short_help();
        return EXIT_FAILURE;
      }
    if (verbose)
      {
        fprintf(stdout, "Reading predictor from %s...\n", filename);
      }
    auto fsa = load_predictor(filename);
    if (verbose)
      {
        fprintf(stdout, "done!\nPredicting...\n");
      }
    try
      {
        predict(fsa, verbose);
      }
    catch (FunctionNotImplementedException& fnie)
      {
        fprintf(stderr, "%s does not support predictions:\n%s\n", filename,
                fnie().c_str());
      }
    return EXIT_SUCCESS;
  }

