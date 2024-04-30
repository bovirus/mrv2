#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This Windows script collects all .pdb files of a Debug or RelWithDebInfo
# into a zipfile.
#



. etc/build_dir.sh

# Install zip just in case

pacman -Sy zip --noconfirm



# Output zip file name
ZIPFILE="packages/mrv2-Windows-v${mrv2_VERSION}_pdb_files.zip"

# Make path smaller as it will make -exec fail on GitHub Actions
export PATH=/bin:/usr/bin:/usr/local/bin/

find "$BUILD_DIR" -type f -name "*.pdb" -exec zip -v9 $ZIPFILE {} +

cat <<"EOF" > INSTALL.txt

To get useful debug stack traces for mrv2, you need to mimic the build
directories of the compilation (you need a D: drive):

EOF

echo $PWD >> INSTALL.txt

cat <<"EOF" >> INSTALL.txt

and unzip there this file, so that you get:

EOF

echo ${PWD}/${BUILD_DIR} >> INSTALL.txt

zip -v9 $ZIPFILE INSTALL.txt

rm INSTALL.txt

echo "PDB files from $BUILD_DIR zipped into $ZIPFILE"
