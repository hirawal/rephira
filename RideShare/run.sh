rm -r prog
g++ main.cpp -Wl,-headerpad_max_install_names -std=c++11 -stdlib=libc++ -g -w -I/headers/ -I../../../mosek/8/tools/platform/osx64x86/h -I../../../mosek/8/tools/platform/osx64x86/include -L../../../mosek/8/tools/platform/osx64x86/bin objects/*.cpp utils/*.cpp geohash/*.cpp -o prog -pthread geohash/libgeohash.a -lcurl -ljsoncpp -lfusion64 -lmosek64
install_name_tool -change libfusion64.8.1.dylib `pwd`/../../../mosek/8/tools/platform/osx64x86/bin/libfusion64.8.1.dylib prog || rm -f prog
install_name_tool -change libmosek64.8.1.dylib `pwd`/../../../mosek/8/tools/platform/osx64x86/bin/libmosek64.8.1.dylib prog || rm -f prog
rm -r prog.dSYM
./prog 8