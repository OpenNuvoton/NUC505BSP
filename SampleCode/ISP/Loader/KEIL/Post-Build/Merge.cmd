.\Post-build\Pad.exe 1 64 ..\Loader.bin ..\..\ISP_Demo.bin
copy/B ..\..\ISP_Demo.bin + ..\..\ISP\ISP.bin ..\..\ISP_Demo.bin
.\Post-build\Pad.exe 1 126 ..\..\ISP_Demo.bin
copy/B ..\..\ISP_Demo.bin + .\Post-build\EndTag.bin ..\..\ISP_Demo.bin
.\Post-build\Pad.exe 1 128 ..\..\ISP_Demo.bin
copy/B ..\..\ISP_Demo.bin + ..\..\DefaultApp\DefaultApp.bin ..\..\ISP_Demo.bin
.\Post-build\Pad.exe 1 640 ..\..\ISP_Demo.bin
copy/B ..\..\ISP_Demo.bin + .\Post-build\EndTag.bin ..\..\ISP_Demo.bin
copy ..\..\UpdateApp\UpdateApp.bin ..\..\UpdateApp.bin
.\Post-build\Pad.exe 1 512 ..\..\UpdateApp.bin
copy/B ..\..\UpdateApp.bin + .\Post-build\EndTag.bin ..\..\UpdateApp.bin
copy ..\..\DefaultApp\DefaultApp.bin ..\..\UpdateDefaultApp.bin
.\Post-build\Pad.exe 1 512 ..\..\UpdateDefaultApp.bin
copy/B ..\..\UpdateDefaultApp.bin + .\Post-build\EndTag.bin ..\..\UpdateDefaultApp.bin
