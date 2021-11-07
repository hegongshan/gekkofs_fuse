# vim: set ft=bash:

setup() {
    GIT_ROOTDIR=$(git rev-parse --show-toplevel)
    SCRIPT_TESTDIR=${GIT_ROOTDIR}/tests/scripts/dl_dep.sh

    load "${GIT_ROOTDIR}/tests/scripts/helpers/bats-support/load.bash"
    load "${GIT_ROOTDIR}/tests/scripts/helpers/bats-assert/load.bash"
    load "${GIT_ROOTDIR}/tests/scripts/helpers/bats-file/load.bash"


    # save the current PWD
    pushd . > /dev/null

    # make sure that the script is able to find the profiles
    cd ${GIT_ROOTDIR}/scripts
    
    # make executables in scripts/ visible to PATH
    PATH="${GIT_ROOTDIR}/scripts:$PATH"
}

teardown() {
    # restore the current PWD
    popd > /dev/null
}

@test "[dl_dep.sh] Check usage" {
    run dl_dep.sh -h
    assert_output --partial "usage: dl_dep.sh"
}

@test "[dl_dep.sh] Check --profile and --dependency option at the same time" {
    run -1 dl_dep.sh -p foobar -d barbaz
    assert_output --partial "ERROR: --profile and --dependency options are mutually exclusive" 
}

@test "[dl_dep.sh] Check --profile option without argument" {
    run -1 dl_dep.sh -p
    assert_output --partial "ERROR: Missing argument for -p/--profile option"
}

@test "[dl_dep.sh] Check invalid --profile option without DESTINATION_PATH" {
    run -1 dl_dep.sh -p foobar
    assert_output --partial "ERROR: Positional arguments missing." 
}

@test "[dl_dep.sh] Check valid --profile option without DESTINATION_PATH" {
    run dl_dep.sh -p default
    assert_output --partial "ERROR: Positional arguments missing." 
}

@test "[dl_dep.sh] Check valid --profile option, no PROFILE_VERSION" {

    profile_names=($(dl_dep.sh -l latest | perl -lne "print for /^\* (\w+):.*$/"))

    for pn in "${profile_names[@]}"; do

        expected_output=${SCRIPT_TESTDIR}/latest/${pn}.out

        assert_exist $expected_output

        run dl_dep.sh -n -p "${pn}" "${BATS_TEST_TMPDIR}"

        while IFS= read -r line
        do
            assert_output --partial "$line"
        done < "$expected_output"
    done
}

@test "[dl_dep.sh] Check valid --profile option, with PROFILE_VERSION" {

    profile_versions=($(dl_dep.sh -l | perl -lne "print for /^\s{2}(.*?):$/"))

    for pv in "${profile_versions[@]}"; do
        profile_names=($(dl_dep.sh -l ${pv} | perl -lne "print for /^\* (\w+):.*$/"))

        for pn in "${profile_names[@]}"; do

            expected_output=${SCRIPT_TESTDIR}/${pv}/${pn}.out

            assert_exist $expected_output

            run dl_dep.sh -n -p "${pn}:${pv}" "${BATS_TEST_TMPDIR}"

            while IFS= read -r line
            do
                assert_output --partial "$line"
            done < "$expected_output"
        done

    done
}

@test "[dl_dep.sh] Check unknown --dependency option" {

    profile_name=default
    profile_version=latest

    run -1 dl_dep.sh -n -d foobar "${BATS_TEST_TMPDIR}"

    assert_output "ERROR: 'foobar' not found in '$profile_name:$profile_version'"

}

@test "[dl_dep.sh] Check invalid --dependency option without DESTINATION_PATH" {
    run -1 dl_dep.sh -d foobar
    assert_output --partial "ERROR: Positional arguments missing." 
}

@test "[dl_dep.sh] Check valid --dependency option without DESTINATION_PATH" {
    run dl_dep.sh -d mercury
    assert_output --partial "ERROR: Positional arguments missing." 
}

@test "[dl_dep.sh] Check valid --dependency option, no PROFILE_NAME or PROFILE_VERSION" {

    dependency_names=($(dl_dep.sh -l default:latest | perl -lne "print for /^\s{4}(\w+(?::\w+)*):.*$/"))

    for dep in "${dependency_names[@]}"; do
        destination_path=$(readlink -f ${BATS_TEST_TMPDIR})

        run dl_dep.sh -n -d "${dep}" "${BATS_TEST_TMPDIR}"

        assert_output --regexp \
"Destination path is set to  \"${destination_path}\"
Profile name: default
Profile version: latest
------------------------------------
Downloaded|Cloned '.*' to '${dep}'.*
Done"

    done
}

@test "[dl_dep.sh] Check valid --dependency option, PROFILE_NAME only" {

    profile_names=($(dl_dep.sh -l latest | perl -lne "print for /^\* (\w+):.*$/"))

    for pn in "${profile_names[@]}"; do

        dependency_names=($(dl_dep.sh -l ${pn}:latest | perl -lne "print for /^\s{4}(\w+(?::\w+)*):.*$/"))

        for dep in "${dependency_names[@]}"; do
            destination_path=$(readlink -f ${BATS_TEST_TMPDIR})

            run dl_dep.sh -n -d "${dep}@${pn}" "${BATS_TEST_TMPDIR}"

            assert_output --regexp \
"Destination path is set to  \"${destination_path}\"
Profile name: ${pn}
Profile version: latest
------------------------------------
Downloaded|Cloned '.*' to '${dep}'.*
Done"

        done
    done
}

@test "[dl_dep.sh] Check valid --dependency option, PROFILE_NAME and PROFILE_VERSION" {

    profile_versions=($(dl_dep.sh -l | perl -lne "print for /^\s{2}(.*?):$/"))

    for pv in "${profile_versions[@]}"; do
        profile_names=($(dl_dep.sh -l "${pv}" | perl -lne "print for /^\* (\w+):.*$/"))

        for pn in "${profile_names[@]}"; do

            dependency_names=($(dl_dep.sh -l ${pn}:${pv} | perl -lne "print for /^\s{4}(\w+(?::\w+)*):.*$/"))

            for dep in "${dependency_names[@]}"; do
                destination_path=$(readlink -f ${BATS_TEST_TMPDIR})

                run dl_dep.sh -n -d "${dep}@${pn}:${pv}" "${BATS_TEST_TMPDIR}"

                assert_output --regexp \
"Destination path is set to  \"${destination_path}\"
Profile name: ${pn}
Profile version: ${pv}
------------------------------------
Downloaded|Cloned '.*' to '${dep}'.*
Done"

            done
        done
    done
}
