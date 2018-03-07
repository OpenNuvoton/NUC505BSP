..\..\Loader\Post-build\Pad.exe 1 32 ..\..\Loader\Loader.bin ..\..\ISP_Demo.bin
copy/B ..\..\ISP_Demo.bin + ..\..\ISP\ISP.bin ..\..\ISP_Demo.bin
..\..\Loader\Post-build\Pad.exe 1 52 ..\..\ISP_Demo.bin
copy/B ..\..\ISP_Demo.bin + ..\..\Loader\Post-build\EndTag.bin ..\..\ISP_Demo.bin
..\..\Loader\Post-build\Pad.exe 1 64 ..\..\ISP_Demo.bin
copy/B ..\..\ISP_Demo.bin + ..\..\DefaultApp\DefaultApp.bin ..\..\ISP_Demo.bin
..\..\Loader\Post-build\Pad.exe 1 576 ..\..\ISP_Demo.bin
copy/B ..\..\ISP_Demo.bin + ..\..\Loader\Post-build\EndTag.bin ..\..\ISP_Demo.bin
copy ..\..\UpdateApp\UpdateApp.bin ..\..\UpdateApp.bin
..\..\Loader\Post-build\Pad.exe 1 512 ..\..\UpdateApp.bin
copy/B ..\..\UpdateApp.bin + ..\..\Loader\Post-build\EndTag.bin ..\..\UpdateApp.bin
copy ..\..\DefaultApp\DefaultApp.bin ..\..\UpdateDefaultApp.bin
..\..\Loader\Post-build\Pad.exe 1 512 ..\..\UpdateDefaultApp.bin
copy/B ..\..\UpdateDefaultApp.bin + ..\..\Loader\Post-build\EndTag.bin ..\..\UpdateDefaultApp.bin

