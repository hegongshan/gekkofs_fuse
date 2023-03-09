#!/usr/bin/env python3

import argparse
import shutil
import string
import subprocess
import sys
from collections import namedtuple
from tempfile import NamedTemporaryFile, TemporaryDirectory
from pathlib import Path
from typing import Optional, List, Union
from loguru import logger


class Command:
    def __init__(self, cmdline: List[str]):
        self._cmdline = list(filter(None, cmdline))
        self.result = None

    def run(self):
        command = self._cmdline
        self.result = subprocess.run(
            command,
            timeout=5 * 60,
            capture_output=True,
            encoding="utf-8")

        try:
            self.result.check_returncode()
        except FileNotFoundError as exc:
            logger.error(
                f"Command {command} failed because the process could not be "
                f"found.\n{exc}")
            sys.exit(1)
        except subprocess.CalledProcessError as exc:
            logger.error(f"Command {command} failed because the process "
                         f"did not return a successful return code.\n{exc}")
            logger.error(f"  STDOUT:\n{self.result.stdout}")
            logger.error(f"  STDERR:\n{self.result.stderr}")
            sys.exit(1)
        except subprocess.TimeoutExpired as exc:
            logger.error(f"Command {command} timed out.\n {exc}")
            logger.error(f"  STDOUT:\n{self.result.stdout}")
            logger.error(f"  STDERR:\n{self.result.stderr}")
            sys.exit(1)
        else:
            logger.trace("\n".join(
                filter(None, [" STDOUT:", self.result.stdout])))
            logger.trace("\n".join(
                filter(None, [" STDERR:", self.result.stderr])))

    def stdout(self):
        return self.result.stdout if self.result else ""

    def stderr(self):
        return self.result.stderr if self.result else ""

    def __str__(self):
        return ' '.join(self._cmdline)


class CommandTemplate:
    def __init__(self, cmdline: List[str]):
        self._cmdline = cmdline

    def substitute(self, **kwargs):
        return Command(list(map(
            lambda s: string.Template(s).substitute(**kwargs),
            self._cmdline)))


class Stage:
    def __init__(self, template: CommandTemplate,
                 input: Optional[Path] = None,
                 output: Union[NamedTemporaryFile, TemporaryDirectory] =
                 NamedTemporaryFile(suffix=".info")):
        self._input = input
        self._output = output
        self._command = template.substitute(
            INPUT=self.input_path,
            OUTPUT=self.output_path)

    @property
    def input_path(self) -> Optional[Path]:
        return self._input

    @property
    def output_path(self) -> Path:
        return Path(self._output.name)

    def run(self):
        self._command.run()

    def save_output(self, output_name: Path):
        if self.output_path.is_dir():
            shutil.copytree(self.output_path, output_name, dirs_exist_ok=True)
        else:
            shutil.copy(self.output_path, output_name)

    def __str__(self):
        return str(self._command)


class CommandPipeline:
    class NoOp(Stage):

        def __init__(self):
            super().__init__(CommandTemplate([]))

        @Stage.input_path.getter
        def input_path(self) -> Optional[Path]:
            return None

        @Stage.output_path.getter
        def output_path(self) -> Optional[Path]:
            return None

        def run(self):
            pass

        def save_output(self, output_name: Path):
            pass

        def __str__(self):
            return 'noop'

    def __init__(self):
        self._stages: List[Stage] = [CommandPipeline.NoOp()]

    def last(self) -> Stage:
        return self._stages[-1]

    def append(self, cmdline: List[str]):
        self._stages.append(
            Stage(
                CommandTemplate(cmdline),
                input=self.last().output_path,
                output=NamedTemporaryFile()))

    def run(self):
        for s in self._stages[1:]:
            logger.info(f"running stage: '{s}'")
            s.run()

    def save_output(self, output_path: Path):
        self.last().save_output(output_path)


def configure_logging(verbosity):
    logger.remove()

    if verbosity == 0:
        log_level = "SUCCESS"
    elif verbosity == 1:
        log_level = "INFO"
    elif verbosity == 2:
        log_level = "DEBUG"
    else:
        log_level = "TRACE"

    logger.add(sys.stderr, level=log_level)


def capture(args):
    root_directory = args.root_directory.resolve()
    sources_directory = args.sources_directory.resolve()
    output_file = args.output_file.resolve()

    pipeline = CommandPipeline()

    pipeline.append([
        "lcov",
        "--capture",
        "--quiet" if args.verbosity > 3 else "",
        "--initial" if args.initial else "",
        f"--directory={root_directory}",
        f"--include={sources_directory}/*",
        "--output-file=$OUTPUT"])

    if args.exclusion_patterns:
        pipeline.append([
            "lcov",
            "--quiet" if args.verbosity > 3 else "",
            "--remove=$INPUT",
            *args.exclusion_patterns,
            "--output-file=$OUTPUT"])

    logger.info("Executing command pipeline")
    pipeline.run()
    pipeline.save_output(output_file)
    logger.success(f"Output written to '{output_file}'")


def merge(args):
    output_file = args.output_file.resolve()
    tracefiles = args.tracefiles or []

    if args.search_pattern:
        basedir = args.search_pattern.parent.resolve() or Path.cwd()
        pattern = args.search_pattern.name
        if not pattern:
            logger.warning("Pathname pattern expansion is empty. Ignored.")
        else:
            tracefiles.extend(basedir.rglob(pattern))

    if not tracefiles:
        logger.error("No tracefiles were found that can be included in the "
                     "unified trace. Exiting.")
        sys.exit(1)

    logger.trace("The following traces will be included in the unified trace:")

    for t in tracefiles:
        logger.trace(f"  {t}")

    pipeline = CommandPipeline()

    pipeline.append([
        "lcov",
        "--quiet" if args.verbosity > 3 else "",
        *(f"--add-tracefile={t}" for t in tracefiles),
        "--output-file=$OUTPUT"])

    logger.info("Executing command pipeline")
    pipeline.run()
    pipeline.save_output(output_file)
    logger.success(f"Output written to '{output_file}'")


def summary(args):
    input_file = args.input_tracefile

    cmd = Command(
        cmdline=[
            "lcov",
            "--summary",
            f"{input_file}"
        ])

    logger.info("Generating coverage summary...")
    cmd.run()
    print(cmd.stdout())


def html_report(args):
    output_dir = args.output_directory.resolve()

    cmd = Command(
        cmdline=[
            "genhtml",
            "--quiet" if args.verbosity > 3 else "",
            "--legend",
            "--frames",
            f"{args.input_tracefile}",
            f"--prefix={args.prefix}" if args.prefix else "",
            f"--output-dir={output_dir}"])

    logger.info("Generating HTML report...")
    cmd.run()
    logger.success(f"HTML report written to '{output_dir}'")


def cobertura_report(args):
    output_file = args.output_file.resolve()

    cmd = Command(
        cmdline=[
            "lcov_cobertura",
            f"{args.input_tracefile}",
            "--base-dir={args.base_dir}",
            f"--output={output_file}"])

    logger.info("Generating Cobertura report...")
    cmd.run()
    logger.success(f"Cobertura report written to '{output_file}'")


def define_capture_mode_args(parser):
    parser.add_argument(
        "--initial",
        help="capture initial zero coverage data",
        action='store_true')

    parser.add_argument(
        "-o",
        "--output-file",
        help="write the generated coverage trace to FILENAME",
        required=True,
        type=Path,
        metavar="FILENAME")

    parser.add_argument(
        "-r",
        "--root-directory",
        help="directory where .gcda files should be searched for (typically "
             "${CMAKE_BINARY_DIR})",
        required=True,
        type=Path,
        metavar="DIR")

    parser.add_argument(
        "-s",
        "--sources-directory",
        help="directory where source files should be searched for "
             "(typically ${CMAKE_SOURCE_DIR})",
        required=True,
        type=Path,
        metavar="DIR")

    parser.add_argument(
        "-e",
        "--exclude-pattern",
        help="exclude source files that match this pattern (can be specified "
             "multiple times)",
        type=str,
        action='append',
        dest='exclusion_patterns',
        metavar="PATTERN")

    parser.add_argument(
        "-v",
        "--verbose",
        help="enable verbose output (additional flags increase verbosity)",
        action="count",
        dest='verbosity')

    parser.set_defaults(
        func=capture,
        verbosity=0
    )


def define_merge_mode_args(parser):
    parser.add_argument(
        "-o",
        "--output-file",
        help="write the unified trace to FILENAME",
        required=True,
        type=Path,
        metavar="FILENAME")

    parser.add_argument(
        "-a",
        "--add-tracefile",
        help="add the contents of FILENAME to the unified trace",
        type=Path,
        metavar="FILENAME",
        action="append",
        dest="tracefiles"
    )

    parser.add_argument(
        "-p",
        "--search-pattern",
        help="include any traces matching PATTERN (e.g. '/home/user/*.info')",
        type=Path,
        metavar="PATTERN",
    )

    parser.add_argument(
        "-v",
        "--verbose",
        help="enable verbose output (additional flags increase verbosity)",
        action="count",
        dest='verbosity')

    parser.set_defaults(
        func=merge,
        verbosity=0)


def define_summary_args(parser):
    parser.add_argument(
        "-i",
        "--input-tracefile",
        help="include coverage data found in TRACEFILE in the generated report",
        required=True,
        type=Path,
        metavar="TRACEFILE")

    parser.add_argument(
        "-v",
        "--verbose",
        help="enable verbose output (additional flags increase verbosity)",
        action="count",
        dest='verbosity')

    parser.set_defaults(
        func=summary,
        verbosity=0)


def define_html_report_args(parser):
    parser.add_argument(
        "-i",
        "--input-tracefile",
        help="include coverage data found in TRACEFILE in the generated report",
        required=True,
        type=Path,
        metavar="TRACEFILE")

    parser.add_argument(
        "-p",
        "--prefix",
        help="remove PREFIX from all directory names",
        type=Path,
        metavar="PREFIX")

    parser.add_argument(
        "--output-directory",
        help="write a HTML report for coverage data to OUTPUT_DIR",
        required=True,
        metavar="OUTPUT_DIR",
        type=Path)

    parser.add_argument(
        "-v",
        "--verbose",
        help="enable verbose output (additional flags increase verbosity)",
        action="count",
        dest='verbosity')

    parser.set_defaults(
        func=html_report,
        verbosity=0)


def define_cobertura_report_args(parser):
    parser.add_argument(
        "-i",
        "--input-tracefile",
        help="Include coverage data found in TRACEFILE in the generated report",
        required=True,
        type=Path,
        metavar="TRACEFILE")

    parser.add_argument(
        "--output-file",
        help="write the Cobertura XML report to OUTPUT_FILE",
        metavar="OUTPUT_FILE",
        type=Path)

    parser.add_argument(
        "-b",
        "--base-dir",
        help="directory where source files are located",
        type=Path,
        metavar="DIR")

    parser.add_argument(
        "-v",
        "--verbose",
        help="enable verbose output (additional flags increase verbosity)",
        action="count",
        dest='verbosity')

    parser.set_defaults(
        func=cobertura_report,
        verbosity=0)


def define_command_line_args():
    Mode = namedtuple("Mode", ["name", "help", "func"])

    modes = [
        Mode(name="capture",
             help="generate a coverage trace file from existing .gcda files",
             func=define_capture_mode_args),
        Mode(name="merge",
             help="merge existing coverage traces into a unified trace",
             func=define_merge_mode_args),
        Mode(name="summary",
             help="show summary coverage data for tracefiles",
             func=define_summary_args),
        Mode(name="html_report",
             help="generate reports from coverage trace files",
             func=define_html_report_args),
        Mode(name="cobertura",
             help="generate a Cobertura report from coverage trace files"
                  "(requires `lcov_cobertura` to be installed)",
             func=define_cobertura_report_args)
    ]

    parser = argparse.ArgumentParser(
        description="A utility script to run `lcov` and simplify the "
                    "generation of coverage reports based on multiple traces")

    subparsers = parser.add_subparsers(
        title="commands",
        help='operating modes')

    for m in modes:
        subparser = subparsers.add_parser(m.name, help=m.help)
        m.func(subparser)

    return parser


def parse_command_line():
    parser = define_command_line_args()
    args = parser.parse_args()

    if 'func' not in args:
        parser.print_help()
        sys.exit(1)

    return args


def main():
    args = parse_command_line()
    configure_logging(args.verbosity)
    args.func(args)


if __name__ == "__main__":
    main()
