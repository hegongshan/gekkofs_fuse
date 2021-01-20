#!/usr/bin/env bash

################################################################################
#                                                                              #
# Copyright 2018-2020, Barcelona Supercomputing Center (BSC), Spain            #
# Copyright 2015-2020, Johannes Gutenberg Universitaet Mainz, Germany          #
#                                                                              #
# This software was partially supported by the                                 #
# EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).    #
#                                                                              #
# This software was partially supported by the                                 #
# ADA-FS project under the SPPEXA project funded by the DFG.                   #
#                                                                              #
# SPDX-License-Identifier: MIT                                                 #
#                                                                              #
################################################################################

# default values
export CCOV_ROOT_DIR="${PWD}"
export CCOV_BUILD_DIR="${PWD}"
export CCOV_MODE=""
export CCOV_CAPTURE_NAME=""
export CCOV_EXCLUSIONS_FILE=".coverage-exclusions"
export GCOVR_EXTRA_OPTIONS=()

usage() {

# `cat << EOF` This means that cat should stop reading when EOF is detected
cat << EOF
Usage: coverage.sh MODE [options] -- [extra_gcovr_options]

A helper script to capture coverage information and generate reports.

Mode:
  -c, --capture NAME    Capture coverage data and generate a JSON report for it
                        in $PWD/.coverage/partial/<NAME>/.
  -m, --merge           Combine coverage data from several JSON reports and
                        produce a Cobertura XML report.

Options:
  -h, --help            Show this help message, then exit.
  -r, --root-dir ROOT_DIR
                        The root directory of the target source files.
                        Defaults to '$PWD', the current directory.
  -b, --build-dir BUILD_DIR
                        The build directory for the project.
                        Defaults to '$PWD', the current directory.
  -e, --exclusions EXCLUSIONS_FILE
                        Exclude any source files that match the filters
                        contained in EXCLUSIONS_FILE. Each filter must be in a
                        line of its own and may include optional [[ROOT_DIR]]
                        and/or [[BUILD_DIR]] tags that will be expanded with the
                        appropriate values.
                        Defaults to .coverage-exclusions.
EOF
# EOF is found above and hence cat command stops reading. This is equivalent to
# echo but much neater when printing out.
}

parse_args() {

    # $@ is all command line parameters passed to the script.
    # -o is for short options like -v
    # -l is for long options with double dash like --version
    # the comma separates different long options
    options=$(getopt -l \
                "capture:,merge,help,root-dir:,build-dir:,exclusions:,output:" \
                -o "cmhr:b:e:o:" -- "$@")

    # set --:
    # If no arguments follow this option, then the positional parameters are
    # unset. Otherwise, the positional parameters are set to the arguments,
    # even if some of them begin with a ‘-’.
    eval set -- "${options}"

    while true;
    do
        opt=$1
        case $opt in
            -c|--capture)
                shift

                if [[ -z $1 ]]; then
                    echo "Missing mandatory argument for '${opt}'."
                    exit 1
                fi

                if [[ $1 =~ ^--.* ]]; then
                    echo "Invalid argument '${1}' for '${opt}'."
                    exit 1
                fi

                export CCOV_MODE="capture"
                export CCOV_CAPTURE_NAME=$1
                ;;

            -m|--merge)
                CCOV_MODE="merge"
                ;;

            -r|--root-dir)
                shift

                if [[ -z $1 ]]; then
                    echo "Missing mandatory argument for '${opt}'."
                    exit 1
                fi

                if ! [[ -d $1 ]]; then
                    echo "directory '${1}' does not exist."
                    exit 1
                fi

                export CCOV_ROOT_DIR=$1
                ;;

            -b|--build-dir)
                shift
                if [[ -z $1 ]]; then
                    echo "Missing mandatory argument for '${opt}'."
                    exit 1
                fi

                if ! [[ -d $1 ]]; then
                    echo "directory '${1}' does not exist."
                    exit 1
                fi

                export CCOV_BUILD_DIR=$1
                ;;

            -e|--exclusions)
                shift

                if [[ -z $1 ]]; then
                    echo "Missing mandatory argument for '${opt}'."
                    exit 1
                fi

                if [[ $1 =~ ^--.* ]]; then
                    echo "Invalid argument '${1}' for '${opt}'."
                    exit 1
                fi

                if ! [[ -n $1 && -f $1 && -r $1 ]]; then
                    echo "file '${1}' does not exist or cannot be read."
                    exit 1
                fi

                export CCOV_EXCLUSIONS_FILE=$1
                ;;

            --)
                shift
                GCOVR_EXTRA_OPTIONS="$@"
                break;;

            -h|--help|*)
                usage
                exit 0
                ;;
        esac
        shift
    done

    if [[ -z "${CCOV_MODE}" ]]; then
        echo -e "ERROR: working mode is mandatory.\n"
        usage
        exit 1
    fi
}

parse_exclusions_file() {

    if [[ -n ${CCOV_EXCLUSIONS_FILE} ]]; then
        mapfile -t tmp < "${CCOV_EXCLUSIONS_FILE}"
    fi

    export CCOV_EXCLUSIONS=()

    for exc in "${tmp[@]}";
    do
        # expand [[ROOT_DIR]]
        exc="${exc/\[\[ROOT_DIR\]\]/${CCOV_ROOT_DIR}}"

        # expand [[BUILD_DIR]]
        CCOV_EXCLUSIONS+=( "${exc/\[\[BUILD_DIR\]\]/${CCOV_BUILD_DIR}}" )
    done
}

capture() {

    COVERAGE_OUTPUT_DIR="${PWD}/.coverage/partial/${CCOV_CAPTURE_NAME}"

    ! [[ -d "${COVERAGE_OUTPUT_DIR}" ]] && mkdir -p "${COVERAGE_OUTPUT_DIR}"

    exclude_args=()

    for p in "${CCOV_EXCLUSIONS[@]}";
    do
        exclude_args+=( "--exclude '${p}'" )
    done

    gcovr \
        --root "${CCOV_ROOT_DIR}" \
        "${CCOV_EXCLUSIONS[@]/#/--exclude=/}" \
        --json \
        --output "${COVERAGE_OUTPUT_DIR}/coverage.json" \
        ${GCOVR_EXTRA_OPTIONS[@]}

    echo "Coverage report written to ${COVERAGE_OUTPUT_DIR}/coverage.json"
}

merge() {

    COVERAGE_OUTPUT_DIR="${PWD}/.coverage"

    ! [[ -d "${COVERAGE_OUTPUT_DIR}" ]] && mkdir -p "${COVERAGE_OUTPUT_DIR}"

    tracefiles=()

    mapfile -d $'\0' tracefiles < \
        <(find "${PWD}/.coverage/partial" -name coverage.json -print0)

    gcovr \
        --root "${CCOV_ROOT_DIR}" \
        "${tracefiles[@]/#/--add-tracefile=}" \
        --html-details "${COVERAGE_OUTPUT_DIR}/coverage.html" \
        --xml \
        --output "${COVERAGE_OUTPUT_DIR}/coverage-cobertura.xml" \
        --print-summary \
        ${GCOVR_EXTRA_OPTIONS[@]}

    echo "Cobertura XML report written to ${COVERAGE_OUTPUT_DIR}/coverage-cobertura.xml"
    echo "HTML report written to ${COVERAGE_OUTPUT_DIR}/coverage.html"

    exit 0
}

################################################################################
##  MAIN
################################################################################
parse_args "$@"

if [[ x"$CCOV_MODE" == x"capture" ]]; then
    parse_exclusions_file
    capture
else
    merge
fi

exit 0
