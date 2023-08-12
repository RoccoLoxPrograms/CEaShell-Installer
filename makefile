# ----------------------------
# Makefile Options
# ----------------------------

NAME = CEASHELL
DESCRIPTION = "CEaShell Installer"
ICON = icon.png
COMPRESSED = YES
COMPRESSED_MODE = zx0
ARCHIVED = YES

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)

installer:
	@make
	@convbin -i bin/CEASHELL.8xp -o bin/installer.bin -j 8x -k bin
