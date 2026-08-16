# build libcdict without installing anything
git clone https://gitlab.com/rob.izzard/libcdict.git
cd libcdict
meson setup --buildtype=release builddir
ninja -C builddir
