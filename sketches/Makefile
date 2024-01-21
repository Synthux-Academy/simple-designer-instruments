.PHONY: clean connect upload

PORT = /dev/ttyACM0
FQBN = STMicroelectronics:stm32:GenH7:pnum=DAISY_SEED,upload_method=dfuMethod,xserial=generic,usb=CDCgen,xusb=FS,opt=osstd,dbg=none,rtlib=nano

all: .make/installed build

.make/installed:
	arduino-cli core install STMicroelectronics:stm32
	arduino-cli lib install \
		DaisyDuino \
    "MIDI Library"
	touch .make/installed

build: .make/installed examples/$(EXAMPLE)/*.ino
	arduino-cli compile --fqbn $(FQBN) --build-path build examples/$(EXAMPLE)

.make/uploaded: build
	arduino-cli upload --port $(PORT) --fqbn $(FQBN) --input-dir build
	touch .make/uploaded

clean:
	arduino-cli cache clean
	rm -rf .make/installed .make/uploaded
	rm -rf build

upload: build
	arduino-cli upload --port $(PORT) --fqbn $(FQBN) --input-dir build
	touch .make/uploaded

connect: .make/uploaded
	tio -b 115200 $(PORT)
