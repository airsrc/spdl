#!/usr/bin/env python3

# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

"""Generate interface files for extension modules."""

import logging
from argparse import ArgumentParser
from pathlib import Path

from nanobind.stubgen import StubGen
from spdl.io.lib import _archive, _import_libspdl, _import_libspdl_cuda


def _parse_args():
    parser = ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", "-o", required=True, type=Path)
    return parser.parse_args()


def _generate(mod, output):
    sg = StubGen(mod)
    sg.put(mod)
    with open(output, "w") as f:
        f.write(sg.get())


def _main():
    args = _parse_args()
    logging.basicConfig(level=logging.DEBUG)
    args.output_dir.mkdir(exist_ok=True, parents=True)
    _generate(_archive, args.output_dir / "_archive.pyi")
    _generate(_import_libspdl(), args.output_dir / "_libspdl.pyi")
    _generate(_import_libspdl_cuda(), args.output_dir / "_libspdl_cuda.pyi")


if __name__ == "__main__":
    _main()
