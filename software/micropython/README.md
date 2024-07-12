# micropython submodule

cd micropython/external/micropython/V1.23.0
git submodule add https://github.com/micropython/micropython.git
cd micropython && git checkout v1.23.0


# Issues
* for alpine: disable -Werror in micropython/ports/unix/Makefile


# Patches
## create patch
cd micropython/external/micropython/V1.23.0/micropython
git diff > disable_warning_as_error.patch


## apply patch

cd micropython/external/micropython/V1.23.0/micropython