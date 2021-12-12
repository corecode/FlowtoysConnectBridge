BINFILES= \
	boot_app0.bin \
	bootloader_dio_80m.bin \
	FlowtoysConnectBridge.ino.bin \
	FlowtoysConnectBridge.ino.partitions.bin

dist: ${BINFILES}
	pyinstaller flash.spec
