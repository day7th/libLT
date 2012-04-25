# libLT: a free fountain code library

# This file is part of libLT.

# libLT is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation version 3 of the License.

# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

#!/usr/bin/env python
"""
@file    apply_astyle.py
@author  Pasquale Cataldi
@date    2010

Applies astyle with the proper settings on all
 source files.

Copyright (C) 2008 DLR/TS, Germany
All rights reserved
"""

import os, os.path, sys

srcRoot = os.path.join(os.path.dirname(sys.argv[0]), "../")
for root, dirs, files in os.walk(srcRoot):
    for name in files:
        if name.endswith(".h") or name.endswith(".c"):
            os.system("astyle --style=kr --indent=spaces -U -l -n " + os.path.join(root, name))
        for ignoreDir in ['.svn', 'tools']:
            if ignoreDir in dirs:
                dirs.remove(ignoreDir)
