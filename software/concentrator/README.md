

# sx1302_hal submodules

cd concentrator/external/sx1302_hal/V2.1.0
git submodule add https://github.com/Lora-net/sx1302_hal.git
cd sx1302_hal && git checkout V2.1.0

## needs to be patched
cd concentrator/external/sx1302_hal/V2.1.0/sx1302_hal
git am ../../../../patches/0001-remove-the-array-static-stuff-from-header-file.patch
