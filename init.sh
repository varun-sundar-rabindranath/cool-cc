#!/usr/bin/env bash

set -o errexit
set -o nounset
set -o pipefail

YELLOW_COLOR="$(tput setaf 3)"
GREEN_COLOR="$(tput setaf 2)"
RED_COLOR="$(tput setaf 1)"
NORMAL_COLOR="$(tput sgr0)"
BOLD="$(tput bold)"

log_info() {
  echo "${GREEN_COLOR}${@}${NORMAL_COLOR}"
}

log_error() {
  echo "${BOLD}${RED_COLOR}${@}${NORMAL_COLOR}"
}

log_warn() {
  echo "${YELLOW_COLOR}${@}${NORMAL_COLOR}"
}

# Check if already in the init shell
if [[ -v COOLCC_SOURCE_DIR ]]; then
  log_error "Already in init shell !!"
  exit
fi


SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"

PROJECT_SOURCE_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}" )"  &> /dev/null && pwd)
PROJECT_BASHRC_PATH=${PROJECT_SOURCE_DIR}"/bash_env/project_bashrc.in"

export PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}

# compiler vars
COMPILER_SOURCE_DIR=${PROJECT_SOURCE_DIR}"/compiler"
COMPILER_BUILD_DIR=${COMPILER_SOURCE_DIR}"/out/build"
COMPILER_INSTALL_DIR=${COMPILER_BUILD_DIR}"/install"
COMPILER_BIN_PATH=${COMPILER_INSTALL_DIR}"/bin"

export COMPILER_SOURCE_DIR=${COMPILER_SOURCE_DIR}
export COMPILER_BUILD_DIR=${COMPILER_BUILD_DIR}
export COMPILER_INSTALL_DIR=${COMPILER_INSTALL_DIR}

# PL vars
PL_SOURCE_DIR=${PROJECT_SOURCE_DIR}"/pl"
PL_BUILD_DIR=${PL_SOURCE_DIR}"/out/build"
PL_INSTALL_DIR=${PL_BUILD_DIR}"/install"
PL_BIN_DIR=${PL_INSTALL_DIR}"/bin"

export PL_SOURCE_DIR=${PL_SOURCE_DIR}
export PL_BUILD_DIR=${PL_BUILD_DIR}
export PL_INSTALL_DIR=${PL_INSTALL_DIR}

export PATH=${PATH}:${COMPILER_BIN_PATH}:${PL_BIN_DIR}

unset YELLOW_COLOR GREEN_COLOR RED_COLOR NORMAL_COLOR

exec /bin/bash --rcfile ${PROJECT_BASHRC_PATH}
