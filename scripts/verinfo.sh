#!/usr/bin/env bash

# prevent localized logs
LC_ALL=C
export LC_ALL

source_dir="${source_dir:-"."}"
build_make="$1"
build_arch="$2"

format_url() {
    echo "$@" | sed 's/^https\?:\/\///g' | sed 's/\.git$//g'
}

format_commit() {
    echo "c$(git rev-list --count "$1" 2>/dev/null)/$(git rev-parse --short "$1" 2>/dev/null)"
}

build_branch="$(git symbolic-ref --short HEAD 2>/dev/null)"
# if [ -z "${build_branch}" ]; then
#     build_branch=""
# fi

build_head="$(format_commit HEAD)"
if ! git diff --quiet 2>/dev/null; then
  build_head="${build_head} Modified"
fi

# `branch.<name>.pushRemote` > `remote.pushDefault` > `branch.<name>.remote`
build_remote="$(git config --get "branch.${build_branch}.pushRemote" 2>/dev/null)"
if [ -z "${build_remote}" ]; then
    build_remote="$(git config --get "remote.pushDefault" 2>/dev/null)"
fi
if [ -z "${build_remote}" ]; then
    build_remote="$(git config --get "branch.${build_branch}.remote" 2>/dev/null)"
fi
# `<remote>/<branch.<name>.merge>` > `<remote>/<branch>` > `<remote>/master` > `origin/master`
build_branch_remote="${build_remote}/$(git config --get "branch.${build_branch}.merge" 2>/dev/null | sed 's/^.*\///g')"
if ! git rev-parse -q --verify "${build_branch_remote}" >/dev/null; then
    build_branch_remote="${build_remote}/${build_branch}"
fi
if ! git rev-parse -q --verify "${build_branch_remote}" >/dev/null; then
    build_branch_remote="${build_remote}/master"
fi
if ! git rev-parse -q --verify "${build_branch_remote}" >/dev/null; then
    build_branch_remote="origin/master"
fi
build_head_remote="$(format_commit ${build_branch_remote})"
build_remote="${build_branch_remote%/*}"
build_branch_remote="${build_branch_remote##*/}"

build_remote_url="$(format_url "$(git config --get "remote.${build_remote}.url" 2>/dev/null)")"

build_date="$(date "+%Y-%m-%d %T %z")"
build_time="$(date -d"${build_date}" "+%s")"

export_file="${source_dir}/verinfo_export.conf"

printf "\033[1;36mGenerating '%s'...\033[0m\n" "${export_file}"
cat > "${export_file}" <<EOF
#define BUILD_MAKE "${build_make}"
#define BUILD_ARCH "${build_arch}"
#define BUILD_BRANCH "${build_branch}"
#define BUILD_HEAD "${build_head}"
#define BUILD_BRANCH_REMOTE "${build_branch_remote}"
#define BUILD_HEAD_REMOTE "${build_head_remote}"
#define BUILD_REMOTE_URL "${build_remote_url}"
#define BUILD_DATE "${build_date}"
#define BUILD_TIME ${build_time}L
EOF
printf "\033[1;33m"
cat "${export_file}"
printf "\033[0m"
