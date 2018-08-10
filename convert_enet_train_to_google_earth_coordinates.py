'''
Input:  A text file formatted for ENet road mapper training
Output: A text file formatted for to be imported by Google Earth
'''
import sys
import numpy as np
import cv2
import time
import os
import argparse


g_count_lines = 0


def process_line(line, outfile):
    global g_count_lines
    g_count_lines += 1
    tokens = line.split()
    if len(tokens != 2):
        print("Line:", g_count_lines, "  Expected format: <filename1> <filename2>   Error:", line)
        return
    filename = tokens[0].split('/')[-1]
    info = filename[1:-4].split('_')
    if len(info) != 4:
        print("Line:", g_count_lines, "  Expected file format: <type><lat>_<long>_<off>_<rot>.png   Error:", filename)
        return
    if float(info[3]) == 0.0:
        outfile.write(info[0] + ',' + info[1] + '\n')
    return


def main():
    parser = argparse.ArgumentParser(description='This program reads a text file formatted for ENet road mapper '
                                                 'training and creates a text file formatted for to be imported by '
                                                 'Google Earth.',
                                     formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('-i', '--inputfile', help='text file formatted for ENet road mapper training)',
                        nargs='?', type=argparse.FileType('r'), default=sys.stdin)
    parser.add_argument('-o', '--outputfile', help='text file formatted for to be imported by Google Earth',
                        nargs='?', type=argparse.FileType('w'), default=sys.stdout)
    args = parser.parse_args()
    if args.inputfile == sys.stdin:
        sys.stderr.write('Keyboard input <<<')
    infile = open(args.inputfile, 'r')
    outfile = open(args.outputfile, 'w')
    for line in infile:
        process_line(line, outfile)
    outfile.close()
    return


if __name__ == "__main__":
    main()
