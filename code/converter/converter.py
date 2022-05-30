##
# @file converter.py
# @author Ilya Savitsky
# @brief Converter of Huawei data format to regular data format. 
# @date 2022-05-30
# 
# This module contains the converter from one of the Huawei data formats to the following data format:
# {task_num} {proc_num} {edge_num}
# {C_ij matrix, proc_num x task_num}
# {D_ij matrix, proc_num x proc_num}
# {Pairs of edges are separated by a space, edge_num x 2}

import logging
import argparse
import numpy as np
import pathlib

parser = argparse.ArgumentParser(
    description="Convert Huawei data format to our data format"
)
parser.add_argument(
    "-i", "--input", type=pathlib.Path, help="input directory", required=True
)
parser.add_argument(
    "-c",
    "--class",
    dest="class_num",
    choices=[1, 2],
    type=int,
    help="Huawei class of source class",
    required=True,
)
parser.add_argument(
    "-o", "--output", type=pathlib.Path, help="output directory", required=True
)

args = parser.parse_args()

logging.basicConfig(level=logging.DEBUG)

# typehint so that pylance works better :)
path: pathlib.Path = args.input

if not args.input.is_dir():
    logging.error("Input is not a directory")
    raise RuntimeError("Input is not a directory")

task_num = 0
proc_num = 0
edge_num = 0

logging.info("searching for class type 1")
last = path.name
logging.debug(last)
base_filename = last + ".txt"
dly_filename = last + "_dly.txt"
com_filename = last + "_com.txt"

if (path / base_filename).exists():
    logging.info("found base file")
    with (path / base_filename).open("r") as f:
        task_num, edge_num = [int(x) for x in f.readline().split()]
        logging.debug(f"task_num: {task_num}, edge_num: {edge_num}")
        # edges = np.zeros((task_num, task_num), dtype=int)
        edges = np.loadtxt(f, dtype=int)

if (path / dly_filename).exists():
    logging.info("found dly file")
    tran_time = np.loadtxt(path / dly_filename, dtype=int)
    proc_num = tran_time.shape[0]
    logging.debug(f"proc_num: {proc_num}")

if args.class_num == 1:
    logging.info("searching for class type 1")
    if (path / com_filename).exists():
        logging.info("found com file")
        task_time = np.loadtxt(path / com_filename, dtype=int)
        task_time = np.tile(task_time, (proc_num, 1))
elif args.class_num == 2:
    logging.info("searching for class type 2")
    task_time = np.full((proc_num, task_num), 1, dtype=int)

with open(args.output, "w") as f:
    f.write(f"{task_num} {proc_num} {edge_num}\n")
    np.savetxt(f, task_time, fmt="%d")
    np.savetxt(f, tran_time, fmt="%d")
    np.savetxt(f, edges, fmt="%d")
