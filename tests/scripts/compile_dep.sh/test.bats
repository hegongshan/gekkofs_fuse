# vim: set ft=bash:

setup() {
    GIT_ROOTDIR=$(git rev-parse --show-toplevel)
    SCRIPT_TESTDIR=${GIT_ROOTDIR}/tests/scripts/compile_dep.sh

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

@test "[compile_dep.sh] Check usage" {
    run compile_dep.sh -h
    assert_output --partial "usage: compile_dep.sh"
}

@test "[compile_dep.sh] Check --profile and --dependency option at the same time" {
    run -1 compile_dep.sh -p foobar -d barbaz
    assert_output --partial "ERROR: --profile and --dependency options are mutually exclusive" 
}

@test "[compile_dep.sh] Check --profile option without argument" {
    run -1 compile_dep.sh -p
    assert_output --partial "ERROR: Missing argument for -p/--profile option"
}

@test "[compile_dep.sh] Check invalid --profile option without SOURCES_PATH or INSTALL_PATH" {
    run -1 compile_dep.sh -p foo
    assert_output --partial "ERROR: Positional arguments missing." 
}

@test "[compile_dep.sh] Check invalid --profile option without SOURCES_PATH" {
    run -1 compile_dep.sh -p foo bar
    assert_output --partial "ERROR: Positional arguments missing." 
}

@test "[compile_dep.sh] Check invalid --profile option" {
    run -1 compile_dep.sh -p foo bar baz
    assert_output --partial "Profile 'foo:latest' does not exist." 
}

@test "[compile_dep.sh] Check valid --profile option, no PROFILE_VERSION" {

    profile_names=($(compile_dep.sh -l latest | perl -lne "print for /^\* (\w+):.*$/"))

    for pn in "${profile_names[@]}"; do

        expected_output=${SCRIPT_TESTDIR}/latest/${pn}.out

        assert_exist $expected_output

        run compile_dep.sh -n -p "${pn}" foo bar

        while IFS= read -r line
        do
            assert_output --partial "$line"
        done < "$expected_output"
    done

}

@test "[compile_dep.sh] Check valid --profile option, with PROFILE_VERSION" {

    profile_versions=($(compile_dep.sh -l | perl -lne "print for /^\s{2}(.*?):$/"))

    for pv in "${profile_versions[@]}"; do
        profile_names=($(compile_dep.sh -l ${pv} | perl -lne "print for /^\* (\w+):.*$/"))

        for pn in "${profile_names[@]}"; do

            expected_output=${SCRIPT_TESTDIR}/${pv}/${pn}.out

            assert_exist $expected_output

            run compile_dep.sh -n -p "${pn}:${pv}" foo bar

            while IFS= read -r line
            do
                assert_output --partial "$line"
            done < "$expected_output"
        done

    done
}

@test "[compile_dep.sh] Check unknown --dependency option" {

    profile_name=default
    profile_version=latest

    run -1 compile_dep.sh -n -d foobar bar baz

    assert_output --partial "Dependency 'foobar' not found in '$profile_name:$profile_version'"

}

@test "[compile_dep.sh] Check valid --dependency option, no PROFILE_NAME or PROFILE_VERSION" {

    dependency_names=($(compile_dep.sh -l default:latest | perl -lne "print for /^\s{4}(\w+(?::\w+)*):.*$/"))

    for dep in "${dependency_names[@]}"; do
        source_path=$(readlink -f ${BATS_TEST_TMPDIR}/foo)
        install_path=$(readlink -f ${BATS_TEST_TMPDIR}/bar)

        run compile_dep.sh -n -d "${dep}" ${source_path} ${install_path}

        assert_output --regexp \
"CORES = 8 \(default\)
Sources download path = ${source_path}
Installation path = ${install_path}
Profile name: default
Profile version: latest
------------------------------------


######## Installing:  ${dep} ###############################
.*
Done"

    done
}

@test "[compile_dep.sh] Check valid --dependency option, PROFILE_NAME only" {

    profile_names=($(compile_dep.sh -l latest | perl -lne "print for /^\* (\w+):.*$/"))

    for pn in "${profile_names[@]}"; do

        dependency_names=($(compile_dep.sh -l ${pn}:latest | perl -lne "print for /^\s{4}(\w+(?::\w+)*):.*$/"))

        for dep in "${dependency_names[@]}"; do
            source_path=$(readlink -f ${BATS_TEST_TMPDIR}/foo)
            install_path=$(readlink -f ${BATS_TEST_TMPDIR}/bar)

            run compile_dep.sh -n -d "${dep}@${pn}" ${source_path} ${install_path}

            assert_output --regexp \
"CORES = 8 \(default\)
Sources download path = ${source_path}
Installation path = ${install_path}
Profile name: ${pn}
Profile version: latest
------------------------------------


######## Installing:  ${dep} ###############################
.*
Done"

        done
    done
}

@test "[compile_dep.sh] Check valid --dependency option, PROFILE_NAME and PROFILE_VERSION" {

    profile_versions=($(compile_dep.sh -l | perl -lne "print for /^\s{2}(.*?):$/"))

    for pv in "${profile_versions[@]}"; do

        profile_names=($(compile_dep.sh -l latest | perl -lne "print for /^\* (\w+):.*$/"))

        for pn in "${profile_names[@]}"; do

            dependency_names=($(compile_dep.sh -l ${pn}:latest | perl -lne "print for /^\s{4}(\w+(?::\w+)*):.*$/"))

            for dep in "${dependency_names[@]}"; do
                source_path=$(readlink -f ${BATS_TEST_TMPDIR}/foo)
                install_path=$(readlink -f ${BATS_TEST_TMPDIR}/bar)

                run compile_dep.sh -n -d "${dep}@${pn}" ${source_path} ${install_path}

                assert_output --regexp \
"CORES = 8 \(default\)
Sources download path = ${source_path}
Installation path = ${install_path}
Profile name: ${pn}
Profile version: latest
------------------------------------


######## Installing:  ${dep} ###############################
.*
Done"

            done
        done
    done
}
