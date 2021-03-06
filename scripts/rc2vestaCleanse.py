#!/usr/bin/env python

from __future__ import print_function

import argparse
import re

if __name__ == "__main__":

    parserp = argparse.ArgumentParser(formatter_class=argparse.RawTextHelpFormatter, description='Convert special characters in RC file to \'_\' so netnames will match those found in Verilog source')
    subparsers = parserp.add_subparsers()
    parser = subparsers.add_parser('', formatter_class=argparse.RawTextHelpFormatter)

    parser.add_argument(
        dest = "blifFileName",
        help = "BLIF source of module used to determine top-leve IO\n\n"
        )

    parser.add_argument(
        dest = "rcInFileName",
        help = "input RC file generated by qrouter that will be cleansed\n\n"
        )

    parser.add_argument(
        dest = "rcOutFileName",
        help = "cleansed output RC file\n\n"
        )

    args = parser.parse_args()

    # Discover top-level IO names that will need to be protected during the cleansing

    ports = []
    with open(args.blifFileName, "r") as blif:
        for line in blif:
            inputsMatch = re.match("\.inputs (.+\s*)+", line)
            outputsMatch = re.match("\.outputs (.+\s*)+", line)
            if inputsMatch:
                ports.extend(inputsMatch.group(1).split())

            elif outputsMatch:
                ports.extend(outputsMatch.group(1).split())

    portsRegex = "^("

    for port in ports:
        portsRegex = portsRegex + port + "|"

    portsRegex = portsRegex[:-1]
    portsRegex = portsRegex + ") "
    specialCharRegex = "[$\.<>:]"
    falseStartRegex = "(^[\w$:<>\.\[\]]+) \d([^\s])"
    basicCheckRegex = "(^[\w$:<>\.\[\]]+) \d ([\w<>]+/[\w<>]+) "
    lineNum = 0

    with open(args.rcInFileName, 'r') as rcIn, open(args.rcOutFileName, 'w') as rcOut:
        for line in rcIn:
            lineNum += 1
            portsMatch = re.match(portsRegex, line)

            # Notify and prune a failure of type
            # <netname> <number of drivers><netname>
            # It appears as if a netname was started but never finished
            s = True
            while s:
                falseStartMatch = re.match(falseStartRegex, line.rstrip())
                line, s = re.subn(falseStartRegex, r'\2', line.rstrip())
                if s:
                    print("INFO: caught line number " + str(lineNum) + " being bad... Net " + falseStartMatch.group(1) + " appears to have not been finished")

            # Notify and prune a failure of type
            # <netname> <number of drivers> <netname>
            # This one is more challenging. Currently, this script depends on
            # the fact that drivers are either module PINS or output pins from
            # a standard cell. In either case, a '/' is present. If a '/' is
            # not found, we assume the line is malformed
            basicCheckRegexMatch = re.match(basicCheckRegex, line)
            if not basicCheckRegexMatch:
                print("INFO: caught line number " + str(lineNum) + " being bad... line appears as " + line)
                continue

            if portsMatch:
                line = re.sub("<(\d+)>", r'[\1]', line.rstrip())
            else:
                line = re.sub("([A-Za-z$])\.([A-Za-z$])", r'\1_\2', line.rstrip())
                line = re.sub(specialCharRegex, "_", line)

            print(line, file=rcOut)
