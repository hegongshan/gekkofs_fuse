#!/bin/bash
################################################################################
# Copyright 2018-2021, Barcelona Supercomputing Center (BSC), Spain            #
# Copyright 2015-2021, Johannes Gutenberg Universitaet Mainz, Germany          #
#                                                                              #
# This software was partially supported by the                                 #
# EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).    #
#                                                                              #
# This software was partially supported by the                                 #
# ADA-FS project under the SPPEXA project funded by the DFG.                   #
#                                                                              #
# This file is part of GekkoFS.                                                #
#                                                                              #
# GekkoFS is free software: you can redistribute it and/or modify              #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation, either version 3 of the License, or            #
# (at your option) any later version.                                          #
#                                                                              #
# GekkoFS is distributed in the hope that it will be useful,                   #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU General Public License for more details.                                 #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with GekkoFS.  If not, see <https://www.gnu.org/licenses/>.            #
#                                                                              #
# SPDX-License-Identifier: GPL-3.0-or-later                                    #
################################################################################

COMMON_CURL_FLAGS="--silent --fail --show-error --location -O"
COMMON_GIT_FLAGS="--quiet --single-branch -c advice.detachedHead=false"
PATCH_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PATCH_DIR="${PATCH_DIR}/patches"
DEPENDENCY=""
VERBOSE=false

DEFAULT_PROFILE="default"
DEFAULT_VERSION="latest"
PROFILES_DIR="${PWD}/profiles"
SOURCES_FILE="${PROFILES_DIR}/sources.list"
declare -a PROFILE_DEP_NAMES
declare -A PROFILE_WGETDEPS PROFILE_CLONEDEPS PROFILE_SOURCES
declare -A PROFILE_CLONEDEPS_ARGS PROFILE_CLONEDEPS_PATCHES

# Stop all backround jobs on interruption.
# "kill -- -$$" sends a SIGTERM to the whole process group,
# thus killing also descendants.
# Use single quotes, otherwise this expands now rather than when signalled.
# See shellcheck SC2064.
trap 'trap - SIGTERM && kill -- -$$' SIGINT SIGTERM

exit_child() {
    if [ ! $? -eq 0 ]; then
        # notify the parent
        kill -s SIGTERM -- -$$
    fi
}

error_exit() {
    echo "$1" >&2  ## Send message to stderr. Exclude >&2 if you don't want it that way.
    exit "${2:-1}" ## Return a code specified by $2 or 1 by default.
}

list_versions() {

    if [[ ! -d "${PROFILES_DIR}" ]]; then
        echo "Directory '${PROFILES_DIR}' does not exist. No profiles available."
        exit 1
    fi

    declare -A versions

    while IFS= read -r -d '' filename; do
        id="$(basename $(dirname ${filename}))"
        profile="$(basename ${filename%%.specs})"

        versions[$id]+="${profile} "
    done < <(find -L "${PROFILES_DIR}" -type f -name "*.specs" -print0 | sort -z) 

    echo -e "Available versions and configuration profiles:\n"

    for id in "${!versions[@]}"; do
        echo "  ${id}:"
        echo -e "    ${versions[${id}]}\n"
    done

    exit 0
}

list_profiles() {

    local TAG=$1

    if [[ "$TAG" =~ ^(.*):(.*)$ ]]; then
        PROFILE="${BASH_REMATCH[1]}.specs"

        if [[ -n ${BASH_REMATCH[2]} ]]; then
            VERSION="${BASH_REMATCH[2]}"
        else
            VERSION="latest"
        fi

    else
        VERSION="${TAG}"
    fi

    if [[ ! -d "${PROFILES_DIR}" ]]; then
        echo "Directory '${PROFILES_DIR}' does not exist. No configuration profiles found."
        exit 1
    fi

    if [[ ! -d "${PROFILES_DIR}/${VERSION}" ]]; then
        echo "Version ${VERSION} does not exist. No configuration profiles found."
        exit 1
    fi

    echo -e "Configuration profiles for '${VERSION}':\n"

    find "${PROFILES_DIR}/${VERSION}/${PROFILE}" -type f -name "*.specs" -print0 | sort -z | while IFS= read -r -d '' filename; do
        basename=$(basename "${filename}")
        version=$(basename $(dirname "${filename}"))
        profile="${basename%.*}"

        echo "* ${profile}:${version} (${filename})"

        source "${filename}"

        if [[ -n "${comment}" ]]; then
            echo -e "\n  ${comment}\n"
        fi

        for d in "${order[@]}";
        do
            if [[ -n ${wgetdeps[${d}]} ]]; then
                echo "    ${d}: ${wgetdeps[${d}]}"
            elif [[ -n ${clonedeps[${d}]} ]]; then
                echo "    ${d}: ${clonedeps[${d}]}"
            else
                echo "    ${d}: ???"
            fi
        done

        echo ""

        unset wgetdeps
        unset clonedeps
        unset clonedeps_args
        unset clonedeps_patches
        unset comment
        unset order
    done
    exit 0

}

load_profile() {

    local profile=$1
    local version=$2
    shift

    # make sure we are in a known state
    PROFILE_DEP_NAMES=()
    PROFILE_CLONEDEPS=()
    PROFILE_CLONEDEPS_ARGS=()
    PROFILE_CLONEDEPS_PATCHES=()
    PROFILE_WGETDEPS=()

    local filename="${PROFILES_DIR}/${version}/${profile}.specs"

    if [[ ! -f "${filename}" ]]; then
        echo "Profile '${profile}:${version}' does not exist."
        exit 1
    fi

    source "${filename}"

    # some checks
    if [[ -z "${wgetdeps[*]}" && -z "${clonedeps[*]}" ]]; then
        echo "Profile '${profile}' is invalid."
        exit 1
    fi

    if [[ -z "${order[*]}" ]]; then
        echo "Profile '${profile}' is invalid."
        exit 1
    fi

    if [[ "$((${#wgetdeps[@]}+${#clonedeps[@]}))" -ne "${#order[@]}" ]]; then
        echo "Profile '${profile}' is invalid."
        exit 1
    fi

    # propagate results outside of function
    for i in "${!order[@]}"; do
        PROFILE_DEP_NAMES[$i]="${order[${i}]}"
    done

    for k in "${!clonedeps[@]}"; do
        PROFILE_CLONEDEPS["${k}"]="${clonedeps[${k}]}"
    done

    for k in "${!clonedeps_args[@]}"; do
        PROFILE_CLONEDEPS_ARGS["${k}"]="${clonedeps_args[${k}]}"
    done

    for k in "${!clonedeps_patches[@]}"; do
        PROFILE_CLONEDEPS_PATCHES["${k}"]="${clonedeps_patches[${k}]}"
    done

    for k in "${!wgetdeps[@]}"; do
        PROFILE_WGETDEPS["${k}"]="${wgetdeps[${k}]}"
    done
}

load_sources() {

    if [[ ! -f "${SOURCES_FILE}" ]]; then
        echo "Missing dependency sources at '${SOURCES_FILE}'."
        exit 1
    fi

    source "${SOURCES_FILE}"

    # propagate sources outside of function
    for k in "${!sources[@]}"; do
        PROFILE_SOURCES["${k}"]="${sources[${k}]}"
    done
}

clonedeps() {
    if [[ "$VERBOSE" == true ]]; then
        set -ex
    else
        set -e
    fi
    trap exit_child EXIT

    local FOLDER=$1
    local REPO=$2
    local COMMIT=$3
    local GIT_FLAGS=$4
    local PATCH=$5

    local ACTION

    if [[ -d "${SOURCE}/${FOLDER}/.git" ]]; then
        cd "${SOURCE}/${FOLDER}" && git fetch -q
        ACTION="Pulled"
    else
        git clone ${COMMON_GIT_FLAGS} ${GIT_FLAGS} -- "${REPO}" "${SOURCE}/${FOLDER}"
        ACTION="Cloned"
    fi
    # fix the version
    cd "${SOURCE}/${FOLDER}" && git checkout -qf "${COMMIT}"
    echo "${ACTION} '${REPO}' to '${FOLDER}' with commit '[${COMMIT}]' and flags '${GIT_FLAGS}'"

    # apply patch if provided
    if [[ -n "${PATCH}" ]]; then
        git apply --verbose "${PATCH_DIR}/${PATCH}"
    fi
}

wgetdeps() {
    if [[ "$VERBOSE" == true ]]; then
        set -ex
    else
        set -e
    fi
    trap exit_child EXIT

    FOLDER=$1
    URL=$2
    if [[ -d "${SOURCE}/${FOLDER}" ]]; then
        # SC2115 Use "${var:?}" to ensure this never expands to /* .
        rm -rf "${SOURCE:?}/${FOLDER:?}"
    fi
    mkdir -p "${SOURCE}/${FOLDER}"
    cd "${SOURCE}"
    FILENAME="$(basename $URL)"
    if [[ -f "${SOURCE}/$FILENAME" ]]; then
        rm -f "${SOURCE}/$FILENAME"
    fi
    curl ${COMMON_CURL_FLAGS} "$URL" || error_exit "Failed to download ${URL}" $?
    tar -xf "$FILENAME" --directory "${SOURCE}/${FOLDER}" --strip-components=1
    rm -f "$FILENAME"
    echo "Downloaded '${URL}' to '${FOLDER}'"
}

usage_short() {
    echo "
usage: dl_dep.sh [-h]
                 [-l [[PROFILE_NAME:]VERSION]]
                 [-p PROFILE_NAME[:VERSION]]
                 [-d DEPENDENCY_NAME[[@PROFILE_NAME][:VERSION]]
                 DESTINATION_PATH
	"
}

help_msg() {

    usage_short
    echo "
This script gets all GekkoFS dependency sources (excluding the fs itself)

positional arguments:
        DESTINATION_PATH        path where dependencies should be downloaded


optional arguments:
        -h, --help              shows this help message and exits
        -l, --list-dependencies [[PROFILE_NAME:]VERSION]
                                list dependency configuration profiles available for download
        -p, --profile PROFILE_NAME[:VERSION]
                                allows downloading a pre-defined set of dependencies as defined
                                in ${PROFILES_DIR}/PROFILE_NAME.specs. This is useful to 
                                deploy specific library versions and/or configurations,
                                using a recognizable name. Optionally, PROFILE_NAME may include
                                a specific version for the profile, e.g. 'mogon2:latest' or
                                'ngio:0.8.0', which will download the dependencies defined for
                                that specific version. If unspecified, the 'default:latest' profile
                                will be used, which should include all the possible dependencies.
        -d, --dependency DEPENDENCY_NAME[[@PROFILE_NAME][:VERSION]]
                                build and install a specific dependency, ignoring any --profile
                                option provided. If PROFILE_NAME is unspecified, the 'default'
                                profile will be used. Similarly, if VERSION is unspecified, the
                                'latest' version of the specified profile will be used.
        -v, --verbose           Increase download verbosity
        "
}

# load default profile for now, might be overridden later
load_profile "${DEFAULT_PROFILE}" "${DEFAULT_VERSION}"

# load source URLs for dependencies
load_sources

POSITIONAL=()
while [[ $# -gt 0 ]]; do
    key="$1"

    case ${key} in
    -p | --profile)
        if [[ -z "$2" ]]; then
            echo "ERROR: Missing argument for -p/--profile option"
            exit 1
        fi

        if [[ "$2" =~ ^(.*):(.*)$ ]]; then
            PROFILE_NAME="${BASH_REMATCH[1]}"
            PROFILE_VERSION="${BASH_REMATCH[2]}"
        else
            PROFILE_NAME="$2"
            PROFILE_VERSION="${DEFAULT_VERSION}"
        fi

        load_profile "${PROFILE_NAME}" "${PROFILE_VERSION}"
        shift # past argument
        shift # past value
        ;;

    -d | --dependency)
        if [[ -z "$2" ]]; then
            echo "ERROR: Missing argument for -d/--dependency option"
            exit 1
        fi
        DEPENDENCY="$2"
        shift # past argument
        shift # past value
        ;;
    -l | --list-dependencies)

        if [[ -z "$2" ]]; then
            list_versions
        else
            list_profiles "$2"
        fi

        exit
        ;;
    -h | --help)
        help_msg
        exit
        ;;
    -v | --verbose)
        VERBOSE=true
        shift # past argument
        ;;
    *) # unknown option
        POSITIONAL+=("$1") # save it in an array for later
        shift              # past argument
        ;;
    esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters

# positional arguments
if [[ -z ${1+x} ]]; then
    echo "ERROR: Positional arguments missing."
    usage_short
    exit 1
fi
SOURCE="$(readlink -mn "${1}")"

echo "Source path is set to  \"${SOURCE}\""
echo "------------------------------------"

mkdir -p "${SOURCE}"

## download dependencies
for dep in "${PROFILE_DEP_NAMES[@]}"; do

    if [[ ! -z "${PROFILE_WGETDEPS[${dep}]:-}" ]]; then

        # dependency names can include a TAG after a colon (e.g. ofi:verbs),
        # remove it
        dep_id=${dep%%:*}

        # find required version for dependency
        dep_version="${PROFILE_WGETDEPS[${dep}]}"

        # build URL for dependency
        dep_url="${PROFILE_SOURCES[${dep_id}]}"

        if [[ -z "${dep_url}" ]]; then
            echo "Missing source URL for '${dep_id}'. Verify ${SOURCES_FILE}."
            exit 1
        fi

        dep_url="${dep_url/\{\{VERSION\}\}/${dep_version}}"

        wgetdeps "${dep_id}" "${dep_url}" &

    elif [[ ! -z "${PROFILE_CLONEDEPS[${dep}]:-}" ]]; then

        # dependency names can include a TAG after a colon (e.g. ofi:verbs),
        # remove it
        dep_id=${dep%%:*}

        dep_args=""

        # find required version for dependency
        dep_version="${PROFILE_CLONEDEPS[${dep}]}"

        # version may be a commit hash, a tag or something like HEAD@BRANCH_NAME
        # if it's the latter, remove the @BRANCH_NAME
        if [[ "${dep_version}" =~ ^(.*)@(.*)$ ]]; then
            dep_args+="-b ${BASH_REMATCH[2]}"
            dep_version=${BASH_REMATCH[1]}
        fi

        # build URL for dependency
        dep_url="${PROFILE_SOURCES[${dep_id}]}"

        if [[ -z "${dep_url}" ]]; then
            echo "Missing source URL for '${dep_id}'. Verify ${SOURCES_FILE}."
            exit 1
        fi

        dep_url="${dep_url/\{\{VERSION\}\}/${dep_version}}"

        # check if extra args are required
        dep_args+="${PROFILE_CLONEDEPS_ARGS[${dep}]}"

        dep_patch=${PROFILE_CLONEDEPS_PATCHES[${dep}]}

        clonedeps "${dep}" "${dep_url}" "${dep_version}" "${dep_args}" "${dep_patch}" &

    else
        echo "Unknown dependency '${dep}'."
        exit 1
    fi
done

# Wait for all download to be completed
wait
echo "Done"

exit 0
