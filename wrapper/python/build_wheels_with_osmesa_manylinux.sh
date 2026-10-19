#!/bin/bash
#The wheels with the 3D distortion rendered by OSMesa: cf build_wheels_manylinux.sh
#(OSMesa now comes from the distribution of the container, not built from
#the sources of Mesa 19.3.3, the last to build on manylinux2010.)
set -euo pipefail
exec "$(dirname "${BASH_SOURCE[0]}")/build_wheels_manylinux.sh" --osmesa "$@"
